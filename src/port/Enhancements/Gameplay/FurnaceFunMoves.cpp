#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Romhack/RomhackConfig.h"

#include "enums.h"
#include "functions.h"
#include "core2/abilityprogress.h"

extern "C" void __chSmBottles_setHasUsedSpiralMountainAbilities(void);

#define CVAR_NAME CVAR_ENHANCEMENT("Gameplay.FurnaceFunMoves")

void RegisterFurnaceFunMoves_Init() {
    COND_HOOK(OnNewGame, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NAME, 0), [](IEvent* event) {
        if (port_isRomhack()) {
            return;
        }
        for (int ability = ABILITY_0_BARGE; ability <= ABILITY_11_TURBO_TALON; ability++) {
            // Speedrunners don't learn these moves for FFM so they can get the free consumables.
            if (ability == ABILITY_6_EGGS || ability == ABILITY_9_FLIGHT || ability == ABILITY_12_WONDERWING) {
                continue;
            }
            player_unlockAbility(static_cast<ability_e>(ability));
        }
        // Outside the loop's range. Without it the note doors never open and Bottles
        // force-triggers his tutorial on approach to the 50-note door.
        player_unlockAbility(ABILITY_13_1ST_NOTEDOOR);
        __chSmBottles_setHasUsedSpiralMountainAbilities();
        ability_setUsed(ABILITY_USED_9_SHOCK);
    });
}

static RegisterShipInitFunc furnaceFunMovesInitFunc(RegisterFurnaceFunMoves_Init, { CVAR_NAME });
