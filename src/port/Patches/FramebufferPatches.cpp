// Framebuffer patches — vi-black emulation, on-demand readback, pause FB,
// aux picture readback, picture/transition model DL patching. Replaces per-tile
// texture loads with full-buffer loads, remaps vertex UVs, and registers GPU FBs
// as direct texture sources.

#include <libultraship.h>

#include "port/Engine.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

// From Patches.h; declared here because this file's definitions use concrete
// Gfx/model types where the header declares void*.
extern "C" void port_runOnRenderThread(void (*fn)(void*), void* arg);

extern "C" {

#include "core1/core1.h"
#include "model.h"

void gfx_register_fb_texture(const void* cpuAddr, int fbId);
BKGfxList* modelbin_getGfxList(BKModelBin* arg0);
s32 getGameMode(void);

// During FADE_IN, the game disables scene drawing one frame before
// capturing gFramebuffers. Without freezing, the readback would overwrite
// gFramebuffers with the blank frame. Freezing preserves the world data for
// the transition capture.
static bool s_freezeReadback = false;

// On-demand readback
static int s_readbackRequestFrames = 0;

void port_freezeReadback(int freeze) {
    s_freezeReadback = (freeze != 0);
}

void port_requestReadback(void) {
    s_readbackRequestFrames = 2;
}

// Consumed by the display list builder (bufferreadback.c) to emit
// gDPReadFB into the DL, populating gFramebuffers at native resolution.
int port_consumeReadbackRequest(void) {
    if (s_freezeReadback || s_readbackRequestFrames <= 0)
        return 0;
    s_readbackRequestFrames--;
    return 1;
}

// No-op — readback is done via gDPReadFB in the display list
void Framebuffer_ReadbackGPU(int bufferIndex) {
    (void)bufferIndex;
}

// Pause menu GPU FB.

static int s_pauseFbId = -1;
static int s_pauseFbW = 0;
static int s_pauseFbH = 0;

static void currentRenderSize(int* w, int* h) {
    *w = (int)OTRGetGameRenderWidth();
    *h = (int)OTRGetGameRenderHeight();
    if (*w < 1) {
        *w = gFramebufferWidth;
    }
    if (*h < 1) {
        *h = gFramebufferHeight;
    }
}

// Framebuffer creation talks to the renderer, so under thread5 mode it has to
// happen on the render thread rather than the game tick.
static void createPauseFb(void* arg) {
    (void)arg;
    s_pauseFbId = gfx_create_framebuffer(s_pauseFbW, s_pauseFbH, s_pauseFbW, s_pauseFbH, 0, 0);
}

int port_getPauseFramebufferId(void) {
    if (s_pauseFbId < 0) {
        currentRenderSize(&s_pauseFbW, &s_pauseFbH);
        port_runOnRenderThread(createPauseFb, NULL);
    }
    return s_pauseFbId;
}

// Capture-frame entry point: ensure the FB matches the current render resolution before the
// snapshot copy, recreating it only when the resolution actually changed since the last
// capture. Recreating leaks the previous FB id (there is no destroy API), but this fires only
// on a real resolution change at pause entry, not every pause, so it is bounded and rare.
int port_capturePauseFramebuffer(void) {
    int w;
    int h;
    currentRenderSize(&w, &h);
    if (s_pauseFbId < 0 || w != s_pauseFbW || h != s_pauseFbH) {
        s_pauseFbW = w;
        s_pauseFbH = h;
        port_runOnRenderThread(createPauseFb, NULL);
    }
    return s_pauseFbId;
}

void port_getPauseFramebufferSize(int* w, int* h) {
    *w = s_pauseFbW;
    *h = s_pauseFbH;
}

static int s_recapLastW = 0;
static int s_recapLastH = 0;
static int s_recapStable = 0;

int port_pauseConsumeRecaptureRequest(void) {
    if (s_pauseFbId < 0 || s_pauseFbW < 1 || s_pauseFbH < 1) {
        return 0;
    }
    int w;
    int h;
    currentRenderSize(&w, &h);
    if (w != s_recapLastW || h != s_recapLastH) {
        s_recapLastW = w;
        s_recapLastH = h;
        s_recapStable = 0;
        return 0;
    }
    if (s_recapStable < 4) {
        s_recapStable++;
        return 0;
    }
    float capAspect = (float)s_pauseFbW / (float)s_pauseFbH;
    float curAspect = (float)w / (float)h;
    float d = curAspect - capAspect;
    if (d < 0.0f) {
        d = -d;
    }
    return (d > 0.02f) ? 1 : 0;
}

// DL walking helper

// Walk a model's display list, replacing segment-addressed tile loads with a
// single full-buffer gDPLoadTextureTile and optionally remapping vertex UVs
// and forcing a texture filter mode.
static void patchModelDL(BKModelBin* model_bin, uintptr_t seg_start, uintptr_t seg_end, const void* tex_addr, s32 tex_w,
                         s32 tex_h, void (*set_uv)(Vtx* v),
                         s32 force_filter,      // -1 = don't touch
                         s32 uv_only_in_tile) { // only remap UVs after a matched G_SETTIMG
    BKGfxList* gfx_list = modelbin_getGfxList(model_bin);
    Vtx* vtx_list = modelbin_getVtxList(model_bin)->vertices;
    Gfx* cur_gfx = &gfx_list->list[0];
    Gfx* end_gfx = &gfx_list->list[(s32)gfx_list->size];
    s32 in_tile = 0;

    while (cur_gfx < end_gfx) {
        if ((cur_gfx->words.w0 >> 24) == G_SETTIMG) {
            s32 match =
                ((cur_gfx->words.w1 & ~(uintptr_t)1) >= seg_start && (cur_gfx->words.w1 & ~(uintptr_t)1) < seg_end);
            if (uv_only_in_tile)
                in_tile = match;
            if (match) {
                Gfx* g = cur_gfx;
                Gfx* ng = cur_gfx + 1;
                u8 fmt = (ng->words.w0 >> 21) & 0x7;
                gDPLoadTextureTile(g++, tex_addr, fmt, G_IM_SIZ_16b, tex_w, tex_h, 0, 0, tex_w - 1, tex_h - 1, 0,
                                   G_TX_CLAMP | G_TX_NOMIRROR, G_TX_CLAMP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOMASK,
                                   G_TX_NOLOD, G_TX_NOLOD);
            }
        } else if (force_filter >= 0 && (cur_gfx->words.w0 >> 24) == G_SETOTHERMODE_H) {
            u32 sft = (cur_gfx->words.w0 >> 8) & 0xFF;
            u32 len = (cur_gfx->words.w0 & 0xFF) + 1;
            if (sft == (32 - G_MDSFT_TEXTFILT - 2) && len == 2) {
                cur_gfx->words.w1 = (uintptr_t)force_filter;
            }
        } else if ((cur_gfx->words.w0 >> 24) == G_VTX && (!uv_only_in_tile || in_tile)) {
            u32 count = (cur_gfx->words.w0 >> 10) & 0x3F;
            Vtx* cur_vtx;
            if ((cur_gfx->words.w1 >> 24) == 0x01) {
                cur_vtx = (Vtx*)((cur_gfx->words.w1 & 0x00FFFFFE) + (u8*)vtx_list);
            } else {
                cur_vtx = (Vtx*)(uintptr_t)cur_gfx->words.w1;
            }
            for (u32 i = 0; i < count; i++) {
                set_uv(&cur_vtx[i]);
            }
        }
        cur_gfx++;
    }
}

#define TILE_SIZE 32
#define IMAGE_WIDTH (TILE_SIZE * 5)
#define IMAGE_HEIGHT (TILE_SIZE * 4)

// Aux picture FB (Bottles Bonus / SNS pictures)

// Scale the framebuffer from 160x128 based on the application width

static u16 sAuxFbDummy[1];

#define AUX_FB_SCALE_MAX 2
#define AUX_FB_SCALE_STEP_WIDTH 640

static s32 sAuxGpuFbId = -1;
static int s_auxFbRenderW = 0;
static int s_auxFbRenderH = 0;

static int auxFbScale(void) {
    int scale = s_auxFbRenderW / AUX_FB_SCALE_STEP_WIDTH;

    if (scale > AUX_FB_SCALE_MAX) {
        scale = AUX_FB_SCALE_MAX;
    }
    if (scale < 1) {
        scale = 1;
    }
    return scale;
}

static void createAuxFb(void* arg) {
    (void)arg;
    int scale = auxFbScale();
    int nativeW = s_auxFbRenderW / scale;
    int nativeH = s_auxFbRenderH / scale;

    if (nativeW < 1) {
        nativeW = 1;
    }
    if (nativeH < 1) {
        nativeH = 1;
    }
    sAuxGpuFbId = gfx_create_framebuffer(IMAGE_WIDTH, IMAGE_HEIGHT, nativeW, nativeH, 1, true);
    if (sAuxGpuFbId >= 0) {
        gfx_register_fb_texture(sAuxFbDummy, sAuxGpuFbId);
    }
}

static s32 auxGpuFbId(void) {
    int w;
    int h;
    currentRenderSize(&w, &h);
    if (sAuxGpuFbId < 0 || w != s_auxFbRenderW || h != s_auxFbRenderH) {
        s_auxFbRenderW = w;
        s_auxFbRenderH = h;
        port_runOnRenderThread(createAuxFb, NULL);
    }
    return sAuxGpuFbId;
}

// Picture model patching (Bottles Bonus / SNS pictures)

#define FROM_XZ 0
#define FROM_YZ 1

static s32 sPicFrom, sPicMinXY, sPicMaxXY, sPicMinZ, sPicMaxZ;

static void setPictureVertexTexcoord(Vtx* v) {
    f32 xy_frac, z_frac;
    if (sPicFrom == FROM_YZ) {
        xy_frac = (v->v.ob[1] - (f32)sPicMinXY) / (sPicMaxXY - sPicMinXY);
        z_frac = (v->v.ob[2] - (f32)sPicMinZ) / (sPicMaxZ - sPicMinZ);
        v->v.tc[0] = (s16)(z_frac * IMAGE_WIDTH * 64.0f);
        v->v.tc[1] = (s16)((1.0f - xy_frac) * IMAGE_HEIGHT * 64.0f);
    } else {
        xy_frac = (v->v.ob[0] - (f32)sPicMinXY) / (sPicMaxXY - sPicMinXY);
        z_frac = (v->v.ob[2] - (f32)sPicMinZ) / (sPicMaxZ - sPicMinZ);
        v->v.tc[0] = (s16)(xy_frac * IMAGE_WIDTH * 64.0f);
        v->v.tc[1] = (s16)((1.0f - z_frac) * IMAGE_HEIGHT * 64.0f);
    }
}

void port_patchPictureModel(BKModelBin* model_bin, s32 min_xy, s32 max_xy, s32 min_z, s32 max_z, u32 from) {
    if (((u8*)model_bin)[0] == 0xBA)
        return;
    ((u8*)model_bin)[0] = 0xBA;
    sPicFrom = from;
    sPicMinXY = min_xy;
    sPicMaxXY = max_xy;
    sPicMinZ = min_z;
    sPicMaxZ = max_z;
    patchModelDL(model_bin, 0x04000000, 0x04100000, sAuxFbDummy, IMAGE_WIDTH, IMAGE_HEIGHT, setPictureVertexTexcoord,
                 G_TF_BILERP, 1);
}

// Transition model patching (falling jiggy pieces)

// Dummy address registered as a GPU FB mirror via gfx_register_fb_texture.
// ImportTexture detects this and uses SelectTextureFb — full GPU resolution.
static u16 sTransitionFbDummy[1];
static s32 sTransitionGpuFbId = -1;

static void createTransitionFb(void* arg) {
    (void)arg;
    sTransitionGpuFbId = gfx_create_framebuffer(DEFAULT_FRAMEBUFFER_WIDTH, DEFAULT_FRAMEBUFFER_HEIGHT,
                                                DEFAULT_FRAMEBUFFER_WIDTH, DEFAULT_FRAMEBUFFER_HEIGHT, 1, 0);
    gfx_register_fb_texture(sTransitionFbDummy, sTransitionGpuFbId);
}

s32 port_getTransitionGpuFbId(void) {
    if (sTransitionGpuFbId < 0) {
        port_runOnRenderThread(createTransitionFb, NULL);
    }
    return sTransitionGpuFbId;
}

// Capture the finished scene for the falling-jiggy piece textures by blitting the
// main framebuffer into the transition FB (same technique as the pause snapshot).
// Called after the scene draw.
void port_captureTransitionFb(Gfx** gfx) {
    s32 trFb = port_getTransitionGpuFbId(); // create + register on first use
    if (trFb >= 0) {
        gDPCopyFB((*gfx)++, trFb, 0, 0, NULL); // copy main FB -> transition FB
    }
}

static void setTransitionVertexTexcoord(Vtx* v) {
    // The transition FB captures the scene with AdjXForAspectRatio applied
    // (widescreen content in the FB). The projection X-scale and AdjX cancel
    // on the geometry side, so UVs map 1:1 to the viewport area without
    // needing widescreen compensation. The -14/-52 offsets align to the
    // viewport origin within the 320x240 logical framebuffer.
    f32 texelS = (v->v.ob[0] + 700.0f) * 320.0f / 1400.0f - 14.0f;
    f32 texelT = (700.0f - v->v.ob[1]) * 320.0f / 1400.0f - 52.0f;
    v->v.tc[0] = (s16)(texelS * 64.0f);
    v->v.tc[1] = (s16)(texelT * 64.0f);
}

void port_patchTransitionModel(BKModelBin* model_bin) {
    if (((u8*)model_bin)[0] == 0xBA)
        return;
    ((u8*)model_bin)[0] = 0xBA;
    patchModelDL(model_bin, 0x02000000, 0x02028000, sTransitionFbDummy, DEFAULT_FRAMEBUFFER_WIDTH,
                 DEFAULT_FRAMEBUFFER_HEIGHT, setTransitionVertexTexcoord, G_TF_POINT, 0);
}

} // extern "C"

// Submitted picture-mode frames since the last map load. The first few frames a
// reloaded map composes are structurally normal DLs that draw nothing visible.
static s32 sPicFramesSinceMapLoad = 0;
static constexpr s32 kPicWarmupFrames = 3;

void RegisterFramebufferPatches_Init() {
    REGISTER_VB_SHOULD(VB_PICTUREBOX_TARGET_FB, EVENT_PRIORITY_NORMAL, {
        Gfx** gdl = va_arg(args, Gfx**);
        s32 auxFb = auxGpuFbId();
        if (gdl != nullptr && auxFb >= 0) {
            gsSPSetFB((*gdl)++, auxFb);
        }
        (void)should;
    });

    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent*) { sPicFramesSinceMapLoad = 0; });

    REGISTER_VB_SHOULD(VB_PICTUREBOX_SUBMIT_FRAME, EVENT_PRIORITY_NORMAL, {
        s32 gfxCount = va_arg(args, s32);
        s32 mode = getGameMode();
        if (mode == GAME_MODE_8_BOTTLES_BONUS || mode == GAME_MODE_A_SNS_PICTURE) {
            sPicFramesSinceMapLoad++;
            if (sPicFramesSinceMapLoad <= kPicWarmupFrames || gfxCount < 100) {
                SPDLOG_INFO("[pictureDiag] dropped frame (sinceLoad={}, gfxCount={})", sPicFramesSinceMapLoad,
                            gfxCount);
                *should = false;
            }
        }
    });
}

static RegisterShipInitFunc initFramebufferPatches(RegisterFramebufferPatches_Init);
