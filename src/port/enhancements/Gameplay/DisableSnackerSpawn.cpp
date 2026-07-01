// Disable Snacker Spawn
//
// Prevents Snacker the shark from spawning in Treasure Trove Cove and Rusty Bucket Bay.

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
}

#define CVAR_DISABLE_SNACKER CVAR_ENHANCEMENT("Gameplay.DisableSnackerSpawn")
#define CVAR CVarGetInteger(CVAR_DISABLE_SNACKER, 0)

void RegisterDisableSnacker_Init() {
    COND_VB_SHOULD(VB_DISABLE_SNACKER, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_DISABLE_SNACKER, 0),
                   { *should = false; });
}

static RegisterShipInitFunc initFunc(RegisterDisableSnacker_Init, { CVAR_DISABLE_SNACKER });