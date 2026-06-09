// Easier Boggy Races
//
// Reduces Boggy's max speed during both sled races in Freezeezy Peak.

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
}

#define CVAR_NAME CVAR_ENHANCEMENT("EasierBoggyRaces")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterEasierBoggyRaces_Init() {
    COND_HOOK(OnBoggyRaceSetSpeed, EVENT_PRIORITY_NORMAL, CVAR, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnBoggyRaceSetSpeed*>(event);
        *ev->speed *= 0.95f;
    });
}

static RegisterShipInitFunc initFunc(RegisterEasierBoggyRaces_Init, { CVAR_NAME });
