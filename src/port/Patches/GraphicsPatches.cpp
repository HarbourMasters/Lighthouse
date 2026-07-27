#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Engine.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"
#include "port/Patches/GeoCull.h"

#define CVAR_DRAW_DISTANCE CVAR_ENHANCEMENT("Graphics.DrawDistance")
#define CVAR_DISABLE_LOD CVAR_ENHANCEMENT("Graphics.DisableLOD")

static const int kMaxDrawDistanceMul = 6;
static int sDrawDistanceCubeWidth(int mul) {
    return 4 * mul;
}

static int sDrawDistanceLevel = 1;
static int sDisableLOD = 0;

extern "C" {
#include <ultra64.h>
#include "enums.h"
#include "functions.h"
#include "model.h"
#include "libultraship/libultra/gbi.h"
#include "port/Romhack/RomhackConfig.h"

extern s32 gFramebufferWidth;
extern s32 gFramebufferHeight;

// Romhack model display lists can leave a palette (TLUT) mode enabled and leak it
// into the next model's draw.
void port_modelRenderResetTLUT(Gfx** gfx) {
    if (!port_isRomhack()) {
        return;
    }
    gDPSetTextureLUT((*gfx)++, G_TT_NONE);
}

// When disabling LOD, Banjo's belt buckle will render and enforce transparency
// clipping through his stomach. Patch the subDL to change the depth write.
void port_patchModelXluDepthWrite(void* model_bin, s32 asset_id) {
    if (model_bin == NULL ||
        (asset_id != ASSET_34D_MODEL_BANJOKAZOOIE_LOW_POLY && asset_id != ASSET_34E_MODEL_BANJOKAZOOIE_HIGH_POLY)) {
        return;
    }

    BKGfxList* gfx_list = modelbin_getGfxList((BKModelBin*)model_bin);
    if (gfx_list == NULL) {
        return;
    }

    for (s32 i = 0; i < (s32)gfx_list->size; i++) {
        Gfx* cmd = &gfx_list->list[i];
        if ((cmd->words.w0 >> 24) != G_DL) {
            continue;
        }

        if ((cmd->words.w1 & ~(uintptr_t)1) == 0x03000060) {
            cmd->words.w1 = (cmd->words.w1 & (uintptr_t)1) | 0x03000120;
        }
    }
}

} // extern "C"

static void RegisterModelXluDepthWrite_Init() {
    COND_VB_SHOULD(VB_MODEL_XLU_DEPTH_WRITE, EVENT_PRIORITY_NORMAL, true, { *should = false; });
}

static RegisterShipInitFunc sModelXluDepthWriteInit(RegisterModelXluDepthWrite_Init);

extern "C" {

// Widescreen HUD edge anchoring (centered-ortho HUD geometry).

static const float kHudCenterBand = 32.0f;

float port_hudOrthoShift(float refX) {
    float halfW = (f32)gFramebufferWidth * 0.5f;
    float extraHalf = (f32)gFramebufferHeight * 0.5f * GameEngine_GetAspectRatio() - halfW;
    if (extraHalf < 0.0f) {
        extraHalf = 0.0f; // narrower than 4:3 (e.g. pillarboxed): never pull inward
    }
    if (refX < halfW - kHudCenterBand) {
        return -extraHalf; // left-anchored
    }
    if (refX > halfW + kHudCenterBand) {
        return extraHalf; // right-anchored
    }
    return 0.0f; // centered
}

int port_getDrawDistanceSetting(void) {
    return sDrawDistanceLevel;
}

int port_getDrawDistanceLevel(void) {
    int mul = port_getDrawDistanceSetting();
    if (IsDemoMode() && getGameMode() != GAME_MODE_4_PAUSED) {
        mul = 1;
    }
    return mul;
}

float port_drawDistanceMul(void) {
    int level = port_getDrawDistanceLevel();
    if (level <= 1) {
        return 1.0f;
    }
    return (float)level + 0.1f; // Nudge
}

void port_applyModelDrawDistanceCull(int* fadeFlag, float* cullMult, float* cullDist) {
    float mul = port_drawDistanceMul();
    *cullMult *= mul;
    *cullDist *= mul;
}

int port_spriteSizeCulled(float depth, float size, float baseThreshold, int disableFlag) {
    if (disableFlag) {
        return 0;
    }
    float scale = port_drawDistanceMul();
    return (3000.0f * scale < depth) && (((size / depth) * scale) < baseThreshold);
}

int port_shouldDisableLOD(void) {
    return sDisableLOD;
}
}

// ============================================================================
// LEVEL OCCLUSION — extend draw distance to camera-area portal geometry
// ============================================================================
//
// Some distant level geometry is hidden by BK's camera-area portal culling rather than the
// distance/LOD culls the prop draw-distance enhancement covers. Disabling that culling
// wholesale floods the render buffer, so at the maxed draw-distance level we force just the
// specific chunks known to suffer from it. Currently the only one is the Mumbo's Mountain
// stonehenge: a single "outside areas {1,2}" CAMERA command in the opaque map model.

static void OnGeoCull_LevelOcclusion(IEvent* event) {
    auto* ev = reinterpret_cast<OnGeoCull*>(event);
    if (ev->type != OCCLUSION_CMD_CAMERA) {
        return;
    }
    if (gsworld_getMap() == MAP_2_MM_MUMBOS_MOUNTAIN && ev->offset == 0x2CD0 &&
        ev->modelBin == (const void*)mapModel_getModelBin(0)) {
        *ev->forceDraw = true;
    }
}

void RegisterLevelOcclusion_Init() {
    bool maxed = CVarGetInteger(CVAR_DRAW_DISTANCE, 1) >= kMaxDrawDistanceMul;
    GeoCull_SetConsumer(GEOCULL_CONSUMER_ENHANCEMENT, maxed);
    COND_HOOK(OnGeoCull, EVENT_PRIORITY_NORMAL, maxed, OnGeoCull_LevelOcclusion);
}

static RegisterShipInitFunc sInitLevelOcclusion(RegisterLevelOcclusion_Init, { CVAR_DRAW_DISTANCE });

static void RegisterDrawDistanceGraphics_Init() {
    COND_HOOK(DrawDistanceCubeWidth, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_DRAW_DISTANCE, 1) > 1,
              [](IEvent* event) {
                  int mul = port_getDrawDistanceLevel();
                  if (mul <= 1) {
                      return;
                  }
                  auto* ev = (DrawDistanceCubeWidth*)event;
                  int width = sDrawDistanceCubeWidth(mul);
                  if (width > ev->mapWidth) {
                      width = ev->mapWidth;
                  }
                  *ev->width = width;
              });
}

static RegisterShipInitFunc drawDistanceGraphicsInit(RegisterDrawDistanceGraphics_Init, { CVAR_DRAW_DISTANCE });

static void RefreshDrawDistanceCVars() {
    int mul = CVarGetInteger(CVAR_DRAW_DISTANCE, 1);
    if (mul < 1) {
        mul = 1;
    }
    if (mul > kMaxDrawDistanceMul) {
        mul = kMaxDrawDistanceMul;
    }
    sDrawDistanceLevel = mul;
    sDisableLOD = CVarGetInteger(CVAR_DISABLE_LOD, 0);
}

static RegisterShipInitFunc drawDistanceCVarCache(RefreshDrawDistanceCVars, { CVAR_DRAW_DISTANCE, CVAR_DISABLE_LOD });
