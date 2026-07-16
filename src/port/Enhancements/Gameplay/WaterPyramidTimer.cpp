// Water Pyramid Timer Enhancement
//
// Adds extra seconds to the GV water pyramid hatch timer.

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#include "enums.h"

#define CVAR_NAME CVAR_ENHANCEMENT("Gameplay.WaterPyramidTimer")

void RegisterWaterPyramidTimer_Init() {
    COND_HOOK(OnWaterPyramidTimer, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NAME, 0), [](IEvent* event) {
        auto* ev = reinterpret_cast<OnWaterPyramidTimer*>(event);
        *ev->timer = 29;
    });
}

static RegisterShipInitFunc initFunc(RegisterWaterPyramidTimer_Init, { CVAR_NAME });
