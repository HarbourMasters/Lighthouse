#ifndef BANJO_KAZOOIE_CORE2_ABILITYPROGRESS_H
#define BANJO_KAZOOIE_CORE2_ABILITYPROGRESS_H

#include <ultra64.h>
#include "bool.h"

enum ability_used_e {
    ABILITY_USED_0_JUMP,
    ABILITY_USED_1_FLAP,
    ABILITY_USED_2_FLIP,
    ABILITY_USED_3_SWIM,
    ABILITY_USED_4_CLIMB,
    ABILITY_USED_5_BEAK_BARGE,
    ABILITY_USED_6_SLIDE,
    ABILITY_USED_7_EGG,
    ABILITY_USED_8_FLY,
    ABILITY_USED_9_SHOCK,
    ABILITY_USED_A_PECK,
    ABILITY_USED_B_CLAW,
    ABILITY_USED_C_TWIRL
};

enum ability_e {
    ABILITY_0_BARGE,
    ABILITY_1_BEAK_BOMB,
    ABILITY_2_BEAK_BUSTER,
    ABILITY_3_CAMERA_CONTROL,
    ABILITY_4_CLAW_SWIPE,
    ABILITY_5_CLIMB,
    ABILITY_6_EGGS,
    ABILITY_7_FEATHERY_FLAP,
    ABILITY_8_FLAP_FLIP,
    ABILITY_9_FLIGHT, 
    ABILITY_A_HOLD_A_JUMP_HIGHER,
    ABILITY_B_RATATAT_RAP,
    ABILITY_C_ROLL,
    ABILITY_D_SHOCK_JUMP,
    ABILITY_E_WADING_BOOTS,
    ABILITY_F_DIVE,
    ABILITY_10_TALON_TROT,
    ABILITY_11_TURBO_TALON,
    ABILITY_12_WONDERWING,
    ABILITY_13_1ST_NOTEDOOR
};

void ability_setUsedWithDialog(enum ability_used_e ability);
bool ability_hasUsed(enum ability_used_e ability);
void ability_setUsed(enum ability_used_e ability);
bool ability_hasLearned(enum ability_e ability);
u32 ability_getAllLearned(void);
void ability_debug(void);
void ability_clearAll(void);
void ability_setLearned(enum ability_e ability, bool has_learned);
void ability_setAllLearned(u32 val);
void ability_setAllUsed(u32 val);
void ability_getSizeAndPtr(s32 *size, u8 **addr);

#endif
