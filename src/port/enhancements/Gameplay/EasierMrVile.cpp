// Easier Mr Vile
//
// Reduces Mr Vile's max speed during all three phases of his mini game in Bubblegloop Swamp.

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
}

#define CVAR_EASIER_MR_VILE CVAR_ENHANCEMENT("EasierMrVile")
#define CVAR CVarGetInteger(CVAR_EASIER_MR_VILE, 0)

void RegisterEasierMrVile_Init() {
    COND_HOOK(OnMrVileSetSpeed, EVENT_PRIORITY_NORMAL, CVAR, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnMrVileSetSpeed*>(event);
        *ev->speed *= 0.95f;
    });
}

static RegisterShipInitFunc initFunc(RegisterEasierMrVile_Init, { CVAR_EASIER_MR_VILE });