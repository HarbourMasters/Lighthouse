#include <libultraship/bridge.h>

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/UI/cvar_prefixes.h"

extern "C" {
#include "enums.h"
#include "functions.h"
}

#define CVAR_ALWAYS_UPDATE_ACTORS CVAR_ENHANCEMENT("Graphics.AlwaysUpdateActors")

static bool IsActorUpdatePlaybackMode() {
    return getGameMode() == GAME_MODE_2_UNKNOWN || func_802E4A08();
}

static void RegisterAlwaysUpdateActors_Init() {
    COND_VB_SHOULD(VB_ACTOR_UPDATE_DISTANCE, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_ALWAYS_UPDATE_ACTORS, 0), {
        if (!IsActorUpdatePlaybackMode()) {
            *should = true;
        }
    });
}

static RegisterShipInitFunc sInitAlwaysUpdateActors(RegisterAlwaysUpdateActors_Init, { CVAR_ALWAYS_UPDATE_ACTORS });
