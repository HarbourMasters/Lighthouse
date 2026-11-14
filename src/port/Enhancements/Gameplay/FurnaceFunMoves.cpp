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
#define CVAR CVarGetInteger(CVAR_NAME, 0)

enum FurnaceFunMovesMode { FFM_OFF, FFM_AT_START, FFM_CONSOLE };
static s32 sPendingGame = -1;

static void grantFurnaceFunMoves() {
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
}

void RegisterFurnaceFunMoves_Init() {
    sPendingGame = -1;

    COND_HOOK(OnNewGame, EVENT_PRIORITY_NORMAL, CVAR != FFM_OFF, [](IEvent* event) {
        if (RomhackConfig_IsActive()) {
            return;
        }
        if (CVAR == FFM_AT_START) {
            grantFurnaceFunMoves();
        } else {
            sPendingGame = gameSelect_getGameNumber();
        }
    });

    COND_HOOK(OnLairRelease, EVENT_PRIORITY_NORMAL, CVAR == FFM_CONSOLE, [](IEvent* event) {
        if (sPendingGame != -1 && sPendingGame == gameSelect_getGameNumber()) {
            sPendingGame = -1;
            grantFurnaceFunMoves();
        }
    });
}

static RegisterShipInitFunc furnaceFunMovesInitFunc(RegisterFurnaceFunMoves_Init, { CVAR_NAME });
