#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Patches/GeoCull.h"

extern "C" {
#include "enums.h"
#include "functions.h"

int port_getDrawDistanceLevel(void) {
    int level = CVarGetInteger(CVAR_ENHANCEMENT("Graphics.DrawDistance"), 0);
    int mode = getGameMode();
    if (mode == GAME_MODE_7_ATTRACT_DEMO || mode == GAME_MODE_9_BANJO_AND_KAZOOIE) {
        level = 0;
    }
    return level;
}

int port_shouldDisableLOD(void) {
    return CVarGetInteger(CVAR_ENHANCEMENT("Graphics.DisableLOD"), 0);
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
