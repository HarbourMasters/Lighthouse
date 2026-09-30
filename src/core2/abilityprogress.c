#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#include "core2/core2.h"
#include "port/Romhack/RomhackConfig.h"

// Special note: The learned abilities and used abilities IDs are different!

//u32 learnedAbilities;
//u32 usedAbilities;
// [port] These must be contiguous — ability_getSizeAndPtr returns &learned with
// size 8, expecting used to follow immediately. Separate globals aren't guaranteed
// contiguous by the linker on PC. Use a struct to enforce layout.
static struct {
    u32 learned;
    u32 used;
} abilityprogress;
#define learnedAbilities abilityprogress.learned
#define usedAbilities    abilityprogress.used

// [port] The tutorial's flags and dialogs belong to Spiral Mountain, but a romhack may
// place it in any map of that level, so gate on the level rather than one map id.
static s32 inSpiralMountain(void) {
    return map_getLevel(gsworld_getMap()) == LEVEL_B_SPIRAL_MOUNTAIN;
}

void ability_setUsedWithDialog(enum ability_used_e ability) {
    s32 dialog_id;
    bool play_ding;

    dialog_id = 0;
    play_ding = TRUE;

    // Has this ability been used before?
    if (usedAbilities & (1 << ability)) {
        return;
    }

    switch (ability) {
        case ABILITY_USED_0_JUMP:
            // [port] guard with SM check — flags collide with map-specific usage in other worlds (e.g. BGS switches)
            if (inSpiralMountain())
                mapSpecificFlags_set(SM_SPECIFIC_FLAG_8_ABILITY_HOLD_A_JUMP_HIGHER_UNLOCKED, TRUE);
            play_ding = TRUE;
            break;

        case ABILITY_USED_1_FLAP:
            if (inSpiralMountain())
                mapSpecificFlags_set(SM_SPECIFIC_FLAG_9_ABILITY_FEATHERY_UNLOCKED, TRUE);
            play_ding = TRUE;
            break;

        case ABILITY_USED_2_FLIP:
            if (inSpiralMountain())
                mapSpecificFlags_set(SM_SPECIFIC_FLAG_A_ABILITY_FLIP_UNLOCKED, TRUE);
            play_ding = TRUE;
            break;

        case ABILITY_USED_3_SWIM:
//          if (gsworld_getMap() == MAP_1_SM_SPIRAL_MOUNTAIN) {
            if (inSpiralMountain()) {
                dialog_id = ASSET_DFC_BOTTLES_UNDERWATER_TUTORIAL;
            }
            break;

        case ABILITY_USED_4_CLIMB:
//          if (gsworld_getMap() == MAP_1_SM_SPIRAL_MOUNTAIN) {
            if (inSpiralMountain()) {
                dialog_id = ASSET_E02_DIALOG_BOTTLES_CLIMB_OTHER;
            }
            break;

        case ABILITY_USED_5_BEAK_BARGE:
//          if (gsworld_getMap() == MAP_1_SM_SPIRAL_MOUNTAIN) {
            if (inSpiralMountain()) {
                dialog_id = ASSET_E05_DIALOG_BOTTLES_KAZOOIE_BARGE;
            }
            break;

        case ABILITY_USED_6_SLIDE:
            play_ding = FALSE;
            if (!player_isAbilityUnlocked(ABILITY_10_TALON_TROT)) {
                if (gsworld_getMap() == MAP_2_MM_MUMBOS_MOUNTAIN) {
                    dialog_id = ASSET_B4D_DIALOG_BOTTLES_MM_SLIP_ON_HILL;
                } else {
                    return;
                }
            } else {
                usedAbilities |= (1 << ability);
            }
            break;

        case ABILITY_USED_8_FLY:
            dialog_id = ASSET_A26_DIALOG_NEED_RED_FEATHERS_TO_FLY;
            break;

        case ABILITY_USED_7_EGG:
        case ABILITY_USED_9_SHOCK:
            break;
    }

    if (play_ding) {
        comusic_playTrack(COMUSIC_2B_DING_B);
    }

    if (dialog_id) {
        gcdialog_showDialog(dialog_id, 4, NULL, NULL, NULL, 0);
    }

    usedAbilities |= (1 << ability);
}

bool ability_hasUsed(enum ability_used_e ability) {
    return (1 << ability) & usedAbilities;
}

void ability_setUsed(enum ability_used_e ability) {
    usedAbilities |= (1 << ability);
}

bool ability_hasLearned(enum ability_e ability) {
    return (1 << ability) & learnedAbilities;
}

u32 ability_getAllLearned(void) {
    return learnedAbilities;
}

void ability_debug(void) {}

void ability_clearAll(void) {
    if (port_isRomhack()) {
        return;
    }

    learnedAbilities = 0;
    usedAbilities = 0;
}

void ability_setLearnedEx(s32 ability, s32 has_learned, s32 triggerEvent) {
    s32 prev = (learnedAbilities >> ability) & 1;

    if (has_learned) {
        learnedAbilities |= (1 << ability);
    } else {
        learnedAbilities &= ~(1 << ability);
    }

    if (triggerEvent && prev != (has_learned ? 1 : 0)) {
        CALL_EVENT(OnAbilityLearned, ability, has_learned ? 1 : 0);
    }
}

void ability_setLearned(enum ability_e ability, bool has_learned) {
    ability_setLearnedEx(ability, has_learned, 1);
}

void ability_setAllLearned(u32 val) {
    learnedAbilities = val;
}

void ability_setAllUsed(u32 val) {
    usedAbilities = val;
}

// Gets the size of the bitflags needed and address for which moves have been learned
void ability_getSizeAndPtr(s32 *size, u8 **addr) {
    *size = 8;
//  *addr = &learnedAbilities;
    *addr = (u8 *)&learnedAbilities;
}
