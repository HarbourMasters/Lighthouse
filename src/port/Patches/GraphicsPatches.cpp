#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Patches/GeoCull.h"

static int sDrawDistanceLevel = 0;
static int sDisableLOD = 0;

extern "C" {
#include "enums.h"
#include "functions.h"

int port_getDrawDistanceLevel(void) {
    int level = sDrawDistanceLevel;
    int mode = getGameMode();
    if (mode == GAME_MODE_7_ATTRACT_DEMO || mode == GAME_MODE_9_BANJO_AND_KAZOOIE) {
        level = 0;
    }
    return level;
}

float port_drawDistanceMul(void) {
    int lvl = port_getDrawDistanceLevel();
    if (lvl >= 4) {
        return 1e9f;
    }
    if (lvl > 0) {
        static const float scale[] = { 1.0f, 1.25f, 1.5811f, 2.2361f };
        return scale[lvl];
    }
    return 1.0f;
}

void port_applyModelDrawDistanceCull(int* fadeFlag, float* cullMult, float* cullDist) {
    int lvl = port_getDrawDistanceLevel();
    if (lvl >= 4) {
        *fadeFlag = 0;
        *cullMult = 1e30f;
        *cullDist = 1e30f;
    } else if (lvl > 0) {
        static const float cullDistScale[] = { 1.0f, 1.25f, 1.5811f, 2.2361f };
        *cullMult *= cullDistScale[lvl];
        *cullDist *= cullDistScale[lvl];
    }
}

int port_spriteSizeCulled(float depth, float size, float baseThreshold, int disableFlag) {
    if (disableFlag) {
        return 0;
    }
    int lvl = port_getDrawDistanceLevel();
    if (lvl >= 4) {
        return 0;
    }
    float scale = 1.0f;
    if (lvl > 0) {
        static const float spriteCullScale[] = { 1.0f, 1.6667f, 2.1082f, 2.9814f };
        scale = spriteCullScale[lvl];
    }
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
    bool maxed = CVarGetInteger(CVAR_ENHANCEMENT("Graphics.DrawDistance"), 0) >= 4;
    GeoCull_SetConsumer(GEOCULL_CONSUMER_ENHANCEMENT, maxed);
    COND_HOOK(OnGeoCull, EVENT_PRIORITY_NORMAL, maxed, OnGeoCull_LevelOcclusion);
}

static RegisterShipInitFunc sInitLevelOcclusion(RegisterLevelOcclusion_Init,
                                                { CVAR_ENHANCEMENT("Graphics.DrawDistance") });

static const int kCubeWidthByLevel[] = { 4, 6, 8, 10, 18 };

static void RegisterDrawDistanceGraphics_Init() {
    REGISTER_LISTENER(DrawDistanceCubeWidth, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        int lvl = port_getDrawDistanceLevel();
        if (lvl <= 0) {
            return;
        }
        auto* ev = (DrawDistanceCubeWidth*)event;
        int width = (lvl <= 4) ? kCubeWidthByLevel[lvl] : 4;
        if (width > ev->mapWidth) {
            width = ev->mapWidth;
        }
        *ev->width = width;
    });
}

static RegisterShipInitFunc drawDistanceGraphicsInit(RegisterDrawDistanceGraphics_Init);

static void RefreshDrawDistanceCVars() {
    sDrawDistanceLevel = CVarGetInteger(CVAR_ENHANCEMENT("Graphics.DrawDistance"), 0);
    sDisableLOD = CVarGetInteger(CVAR_ENHANCEMENT("Graphics.DisableLOD"), 0);
}

static RegisterShipInitFunc drawDistanceCVarCache(RefreshDrawDistanceCVars,
                                                  { CVAR_ENHANCEMENT("Graphics.DrawDistance"),
                                                    CVAR_ENHANCEMENT("Graphics.DisableLOD") });
