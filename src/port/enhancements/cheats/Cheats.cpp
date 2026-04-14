// Cheats
//
// Cheat enhancement hooks.

#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
void volatileFlag_set(enum volatile_flags_e index, s32 set);
}

#define CVAR_INFINITE_HEALTH CVAR_ENHANCEMENT("Cheats.InfiniteHealth")

// Infinite health — drives the game's native sandcastle infinite-health flag
// so item_adjustByDiff's existing infinite-item gate handles it.
void RegisterInfiniteHealth_Init() {
    volatileFlag_set(VOLATILE_FLAG_94_SANDCASTLE_INFINITE_HEALTH, 0);
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_INFINITE_HEALTH, 0),
              [](IEvent* event) { volatileFlag_set(VOLATILE_FLAG_94_SANDCASTLE_INFINITE_HEALTH, 1); });
}

static RegisterShipInitFunc initInfiniteHealthFunc(RegisterInfiniteHealth_Init, { CVAR_INFINITE_HEALTH });
