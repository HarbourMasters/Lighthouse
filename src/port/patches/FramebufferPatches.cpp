// [port] Framebuffer model patches — aux picture readback, picture/transition
// model DL patching. Replaces per-tile texture loads with full-buffer loads,
// remaps vertex UVs, and registers GPU FBs as direct texture sources.

#include <libultraship.h>

extern "C" {

#include "core1/core1.h"
#include "model.h"

int gfx_create_framebuffer(unsigned int width, unsigned int height, unsigned int native_width,
                           unsigned int native_height, unsigned char resize);
void gfx_register_fb_texture(const void* cpuAddr, int fbId);
int port_getViewportWidth(void);
BKGfxList* model_getDisplayList(BKModelBin* arg0);

// ---------------------------------------------------------------------------
// DL walking helper
// ---------------------------------------------------------------------------

// Walk a model's display list, replacing segment-addressed tile loads with a
// single full-buffer gDPLoadTextureTile and optionally remapping vertex UVs
// and forcing a texture filter mode.
static void patchModelDL(BKModelBin* model_bin, uintptr_t seg_start, uintptr_t seg_end, const void* tex_addr, s32 tex_w,
                         s32 tex_h, void (*set_uv)(Vtx* v),
                         s32 force_filter,      // -1 = don't touch
                         s32 uv_only_in_tile) { // only remap UVs after a matched G_SETTIMG
    BKGfxList* gfx_list = model_getDisplayList(model_bin);
    Vtx* vtx_list = model_getVtxList(model_bin)->vtx_18;
    Gfx* cur_gfx = &gfx_list->list[0];
    Gfx* end_gfx = &gfx_list->list[*(s32*)&gfx_list->pad0[0]];
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

// ---------------------------------------------------------------------------
// Aux picture FB readback (Bottles Bonus / SNS pictures)
// ---------------------------------------------------------------------------

extern s16* D_80382450; // aux picture CPU buffer (picturebuffer.c)
extern s32 sAuxGpuFbId; // aux picture GPU FB id (picturebuffer.c)

s32 port_getAuxGpuFbId(void) {
    return sAuxGpuFbId;
}

void port_readAuxFbToCpu(Gfx** gfx) {
    if (sAuxGpuFbId >= 0 && D_80382450 != NULL) {
        gDPReadFB((*gfx)++, sAuxGpuFbId, (u16*)D_80382450, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
        __gSPInvalidateTexCache((*gfx)++, 0);
    }
}

// ---------------------------------------------------------------------------
// Picture model patching (Bottles Bonus / SNS pictures)
// ---------------------------------------------------------------------------

#define TILE_SIZE 32
#define IMAGE_WIDTH (TILE_SIZE * 5)
#define IMAGE_HEIGHT (TILE_SIZE * 4)
#define SEG4_TAGGED ((uintptr_t)0x04000000 | 1)
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
    if (model_bin->pad0[0] == 0xBA)
        return;
    model_bin->pad0[0] = 0xBA;
    sPicFrom = from;
    sPicMinXY = min_xy;
    sPicMaxXY = max_xy;
    sPicMinZ = min_z;
    sPicMaxZ = max_z;
    patchModelDL(model_bin, 0x04000000, 0x04100000, (const void*)SEG4_TAGGED, IMAGE_WIDTH, IMAGE_HEIGHT,
                 setPictureVertexTexcoord, G_TF_BILERP, 1);
}

// ---------------------------------------------------------------------------
// Transition model patching (falling jiggy pieces)
// ---------------------------------------------------------------------------

// Dummy address registered as a GPU FB mirror via gfx_register_fb_texture.
// ImportTexture detects this and uses SelectTextureFb — full GPU resolution.
static u16 sTransitionFbDummy[1];
static s32 sTransitionGpuFbId = -1;

s32 port_getTransitionGpuFbId(void) {
    if (sTransitionGpuFbId < 0) {
        sTransitionGpuFbId = gfx_create_framebuffer(DEFAULT_FRAMEBUFFER_WIDTH, DEFAULT_FRAMEBUFFER_HEIGHT,
                                                    DEFAULT_FRAMEBUFFER_WIDTH, DEFAULT_FRAMEBUFFER_HEIGHT, 1);
        gfx_register_fb_texture(sTransitionFbDummy, sTransitionGpuFbId);
    }
    return sTransitionGpuFbId;
}

void port_readTransitionFbToCpu(Gfx** gfx) {
    if (sTransitionGpuFbId >= 0) {
        gsSPResetFB((*gfx)++);
    }
}

static f32 sTransitionXCompensate;

static void setTransitionVertexTexcoord(Vtx* v) {
    // Perspective projection: 320/1400 scale, centered on 292x216 (offsets -14, -52)
    f32 texelS = (v->v.ob[0] + 700.0f) * 320.0f / 1400.0f - 14.0f;
    f32 texelT = (700.0f - v->v.ob[1]) * 320.0f / 1400.0f - 52.0f;
    // Widescreen: expand S around center to counter the projection X-scale
    texelS = (texelS - 146.0f) * sTransitionXCompensate + 146.0f;
    v->v.tc[0] = (s16)(texelS * 64.0f);
    v->v.tc[1] = (s16)(texelT * 64.0f);
}

void port_patchTransitionModel(BKModelBin* model_bin) {
    if (model_bin->pad0[0] == 0xBA)
        return;
    model_bin->pad0[0] = 0xBA;
    sTransitionXCompensate = (f32)port_getViewportWidth() / 320.0f;
    if (sTransitionXCompensate < 1.01f)
        sTransitionXCompensate = 1.0f;
    patchModelDL(model_bin, 0x02000000, 0x02028000, sTransitionFbDummy, DEFAULT_FRAMEBUFFER_WIDTH,
                 DEFAULT_FRAMEBUFFER_HEIGHT, setTransitionVertexTexcoord, G_TF_POINT, 0);
}

} // extern "C"
