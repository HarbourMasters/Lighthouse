#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "core2/abilityprogress.h"
#include "port/Romhack/RomhackConfig.h"

// [port] These must be contiguous — ability_getSizeAndPtr returns &learned with
// size 8, expecting used to follow immediately. Separate globals aren't guaranteed
// contiguous by the linker on PC. Use a struct to enforce layout.
static struct {
    s32 learned;
    s32 used;
} abilityprogress;
#define abilityprogress_learnedAbilities abilityprogress.learned
#define abilityprogress_usedAbilities    abilityprogress.used

// [port] The tutorial's flags and dialogs belong to Spiral Mountain, but a romhack may
// place it in any map of that level, so gate on the level rather than one map id.
static s32 inSpiralMountain(void){
    return map_getLevel(gsworld_getMap()) == LEVEL_B_SPIRAL_MOUNTAIN;
}

void ability_use(s32 arg0){
    s32 sp2C;
    s32 sp28;

    sp2C = 0;
    sp28 = 1;

    if(abilityprogress_usedAbilities & (1 << arg0))
        return;

    switch(arg0){
        case ABILITY_USED_JUMP:
            // [port] guard with SM check — flags collide with map-specific usage in other worlds (e.g. BGS switches)
            if (inSpiralMountain())
                mapSpecificFlags_set(8, true);
            sp28 = 1;
            break;
        case ABILITY_USED_FLAP:
            if (inSpiralMountain())
                mapSpecificFlags_set(9, true);
            sp28 = 1;
            break;
        case ABILITY_USED_FLIP:
            if (inSpiralMountain())
                mapSpecificFlags_set(0xa, true);
            sp28 = 1;
            break;
        case ABILITY_USED_SWIM:
            if(inSpiralMountain()){
                sp2C = ASSET_DFC_BOTTLES_UNDERWATER_TUTORIAL;
            }
            break;
        case ABILITY_USED_CLIMB:
            if(inSpiralMountain()){
                sp2C = ASSET_E02_DIALOG_BOTTLES_CLIMB_OTHER;
            }
            break;
        case ABILITY_USED_BEAK_BARGE:
            if(inSpiralMountain()){
                sp2C = ASSET_E05_DIALOG_BOTTLES_KAZOOIE_BARGE;
            }
            break;
        case ABILITY_USED_SLIDE:
            sp28 = 0;
            if (!ability_isUnlocked(ABILITY_10_TALON_TROT)) {
                if (gsworld_getMap() == MAP_2_MM_MUMBOS_MOUNTAIN) {
                    sp2C = ASSET_B4D_DIALOG_BOTTLES_MM_SLIP_ON_HILL;
                }
                else {
                    return;
                }
            }
            else {
                abilityprogress_usedAbilities |= (1 << arg0);
            }
            break;
        case ABILITY_USED_FLY:
            sp2C = ASSET_A26_DIALOG_NEED_RED_FEATHERS_TO_FLY;
            break;
        case ABILITY_USED_EGG:
        case ABILITY_USED_SHOCK:
            break;
    }

    if (sp28) {
        comusic_playTrack(COMUSIC_2B_DING_B);
    }

    if (sp2C) {
        gcdialog_showDialog(sp2C, 4, NULL, NULL, NULL, 0);
    }

    abilityprogress_usedAbilities |= (1 << arg0);
}

int ability_hasUsed(enum ability_e move){
    return (1 << move) & abilityprogress_usedAbilities;
}

void ability_setHasUsed(enum ability_e move){
    abilityprogress_usedAbilities |= (1 << move);
}

int ability_hasLearned(enum ability_e move){
    return (1 << move) & abilityprogress_learnedAbilities;
}

s32 ability_getAllLearned(void){
    return abilityprogress_learnedAbilities;
}

void ability_debug(void){}

void ability_clearAll(void){
    if (port_isRomhack()) {
        return;
    }
    abilityprogress_learnedAbilities = 0;
    abilityprogress_usedAbilities = 0;
}

void ability_setLearnedEx(s32 move, s32 val, s32 triggerEvent){
    s32 prev = (abilityprogress_learnedAbilities >> move) & 1;
    if(val){
        abilityprogress_learnedAbilities |= (1 << move);
    }else{
        abilityprogress_learnedAbilities &= ~(1 << move);
    }
    if(triggerEvent && prev != (val ? 1 : 0)){
        CALL_EVENT(OnAbilityLearned, move, val ? 1 : 0);
    }
}

void ability_setLearned(s32 move, s32 val){
    ability_setLearnedEx(move, val, 1);
}

void ability_setAllLearned(s32 val){
    abilityprogress_learnedAbilities = val;
}

void ability_setAllUsed(s32 val){
    abilityprogress_usedAbilities = val;
}

void ability_getSizeAndPtr(s32 *size, u8 **addr){
    *size = 8;
    *addr = (u8 *)&abilityprogress_learnedAbilities;
}
