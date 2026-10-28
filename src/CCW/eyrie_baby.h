// [port] eyrie_baby.c's typedefs and states, moved here for the Anchor catch-up (Puzzles/ClickClockWood.cpp).
#ifndef BANJO_KAZOOIE_CCW_EYRIE_BABY_H
#define BANJO_KAZOOIE_CCW_EYRIE_BABY_H

typedef struct {
    f32 unk0;
    s16 unk4;
    f32 unk8;
    s16 unkC;
} Eyrie_Baby_SFX;

typedef struct {
    s16 map_id;
    s16 requiredFileProgress;
    s16 unlocksFileProgress;
    s16 waitingAnimation;
    Eyrie_Baby_SFX *waitingSFX;
    s16 nextStateWhenComplete; // Probably only used for Spring
    s16 yawningAnimation;
    Eyrie_Baby_SFX *yawnSFX;
    s16 goingToSleepAnimation;
    // u8 pad16[2];
    Eyrie_Baby_SFX *goingToSleepSFX;
    s16 sleepingAnimation;
    // u8 pad1E[0x2];
    Eyrie_Baby_SFX *sleepingSFX;
    u8 cameraNode;
    u8 caterpillarRequirement;
    s16 eatingAnimation;
    s16 meetDialog;
    // u8 pad2A[2];
} Eyrie_Baby_Dialog;

typedef struct {
    Eyrie_Baby_Dialog *progression;
    u8 eatenCount;
    //u8 pad5[3];
    Eyrie_Baby_SFX *sfx;
    s32 unkC[3]; // something about player position?
} ActorLocal_CCW_3310;

enum cheyriebaby_state_e {
    CH_EYRIE_BABY_STATE_0_NOT_HATCHED,
    CH_EYRIE_BABY_STATE_1_IDLE,
    CH_EYRIE_BABY_STATE_2_YAWNING,
    CH_EYRIE_BABY_STATE_3_GOING_TO_SLEEP,
    CH_EYRIE_BABY_STATE_4_SLEEPING,
    CH_EYRIE_BABY_STATE_5_EATING_CATERPILLARS
};

#endif
