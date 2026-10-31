#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Romhack/RomhackConfig.h"
#include "port/Rando/Rando.h"

#include "enums.h"
#include "functions.h"
#include "core2/abilityprogress.h"

extern "C" float sItemscoreTimeScores[];
extern "C" void __chSmBottles_setHasUsedSpiralMountainAbilities(void);
extern "C" void warp_lairEnterLairFromSMLevel(s32, s32);

#define CVAR_NAME CVAR_ENHANCEMENT("Gameplay.SkipSMTutorial")

namespace {

constexpr ability_e kSpiralMountainAbilities[] = {
    ABILITY_F_DIVE,          ABILITY_4_CLAW_SWIPE, ABILITY_C_ROLL,
    ABILITY_B_RATATAT_RAP,   ABILITY_0_BARGE,      ABILITY_A_HOLD_A_JUMP_HIGHER,
    ABILITY_7_FEATHERY_FLAP, ABILITY_8_FLAP_FLIP,  ABILITY_5_CLIMB,
};

} // namespace

void RegisterSkipSMTutorial_Init() {
    COND_HOOK(OnNewGame, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NAME, 0), [](IEvent* event) {
        if (port_isRomhack() || IS_RANDO) {
            return;
        }

        for (ability_e ability : kSpiralMountainAbilities) {
            player_unlockAbility(ability);
        }

        __chSmBottles_setHasUsedSpiralMountainAbilities();

        for (int honeycomb = HONEYCOMB_13_SM_STUMP; honeycomb <= HONEYCOMB_18_SM_QUARRIES; honeycomb++) {
            honeycombscore_set((honeycomb_e)honeycomb, true);
        }
        func_8034789C();
        item_adjustByDiffWithoutHud(ITEM_14_HEALTH,
                                    item_getCount(ITEM_15_HEALTH_TOTAL) - item_getCount(ITEM_14_HEALTH));

        fileProgressFlag_set(FILEPROG_BD_ENTER_LAIR_CUTSCENE, 1);
        sItemscoreTimeScores[LEVEL_B_SPIRAL_MOUNTAIN] = 122.0f; // Average speedrun time for SM completion (2:02)
    });
    COND_VB_SHOULD(VB_GAMESELECT_NEW_GAME_INTRO, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NAME, 0), {
        if (!port_isRomhack() && !IS_RANDO) {
            timedFunc_set_2(0.0f, (GenFunction_2)warp_lairEnterLairFromSMLevel, 0, 0);
            timedFunc_set_1(0.0f, (GenFunction_1)gsworld_setEnableUpdate, 1);
            *should = false;
        }
    });
}

static RegisterShipInitFunc skipIntroInitFunc(RegisterSkipSMTutorial_Init, { CVAR_NAME });
