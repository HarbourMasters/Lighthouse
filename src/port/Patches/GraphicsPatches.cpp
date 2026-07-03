#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
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

static int sDrawDistanceLevel = 0;
static int sDisableLOD = 0;

extern "C" {
#include "enums.h"
#include "functions.h"

extern s32 gFramebufferWidth;
extern s32 gFramebufferHeight;
float GameEngine_GetAspectRatio(void);

// Widescreen HUD edge anchoring (centered-ortho HUD geometry).
float port_hudOrthoShift(float refX) {
    float halfW = (f32)gFramebufferWidth * 0.5f;
    float extraHalf = (f32)gFramebufferHeight * 0.5f * GameEngine_GetAspectRatio() - halfW;
    if (extraHalf < 0.0f) {
        extraHalf = 0.0f; // narrower than 4:3 (e.g. pillarboxed): never pull inward
    }
    if (refX < halfW) {
        return -extraHalf; // left-anchored
    }
    if (refX > halfW) {
        return extraHalf; // right-anchored
    }
    return 0.0f; // centered
}

int port_getDrawDistanceLevel(void) {
    int mul = CVarGetInteger(CVAR_ENHANCEMENT("Graphics.DrawDistance"), 1);
    if (mul < 1) {
        mul = 1;
    }
    if (mul > kMaxDrawDistanceMul) {
        mul = kMaxDrawDistanceMul;
    }
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
    sDrawDistanceLevel = CVarGetInteger(CVAR_DRAW_DISTANCE, 1);
    sDisableLOD = CVarGetInteger(CVAR_DISABLE_LOD, 0);
}

static RegisterShipInitFunc drawDistanceCVarCache(RefreshDrawDistanceCVars, { CVAR_DRAW_DISTANCE, CVAR_DISABLE_LOD });
