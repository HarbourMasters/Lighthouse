#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Romhack/RomhackConfig.h"
#include "port/Rando/Rando.h"

#include "enums.h"
#include "functions.h"
#include "core2/abilityprogress.h"

extern "C" float D_80386000[];

#define CVAR_NAME CVAR_ENHANCEMENT("Gameplay.SkipSMTutorial")

namespace {

constexpr ability_e kSpiralMountainAbilities[] = {
    ABILITY_F_DIVE,          ABILITY_4_CLAW_SWIPE, ABILITY_C_ROLL,
    ABILITY_B_RATATAT_RAP,   ABILITY_0_BARGE,      ABILITY_A_HOLD_A_JUMP_HIGHER,
    ABILITY_7_FEATHERY_FLAP, ABILITY_8_FLAP_FLIP,  ABILITY_5_CLIMB,
};

constexpr ability_used kSpiralMountainUsedMoves[] = {
    ABILITY_USED_JUMP,       ABILITY_USED_FLAP, ABILITY_USED_FLIP, ABILITY_USED_SWIM,  ABILITY_USED_CLIMB,
    ABILITY_USED_BEAK_BARGE, ABILITY_USED_PECK, ABILITY_USED_CLAW, ABILITY_USED_TWIRL,
};

} // namespace

void RegisterSkipSMTutorial_Init() {
    COND_HOOK(OnNewGame, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NAME, 0), [](IEvent* event) {
        if (port_isRomhack()) {
            return;
        }
        auto* ev = reinterpret_cast<OnNewGame*>(event);

        if (!IS_RANDO) {
            for (ability_e ability : kSpiralMountainAbilities) {
                ability_unlock(ability);
            }

            for (ability_used move : kSpiralMountainUsedMoves) {
                ability_setHasUsed(static_cast<ability_e>(move));
            }

            for (int honeycomb = HONEYCOMB_13_SM_STUMP; honeycomb <= HONEYCOMB_18_SM_QUARRIES; honeycomb++) {
                honeycombscore_set((honeycomb_e)honeycomb, true);
            }
            func_8034789C();
            item_adjustByDiffWithoutHud(ITEM_14_HEALTH,
                                        item_getCount(ITEM_15_HEALTH_TOTAL) - item_getCount(ITEM_14_HEALTH));
        }

        fileProgressFlag_set(FILEPROG_BD_ENTER_LAIR_CUTSCENE, 1);
        D_80386000[LEVEL_B_SPIRAL_MOUNTAIN] = 122.0f; // Average speedrun time for SM completion (2:02)

        *ev->skipIntro = 1;
    });
}

static RegisterShipInitFunc skipIntroInitFunc(RegisterSkipSMTutorial_Init, { CVAR_NAME });
