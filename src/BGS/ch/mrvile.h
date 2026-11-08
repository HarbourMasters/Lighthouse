// [port] mrvile.c's typedef and states, moved here for the Anchor Mr. Vile sync (VileSync.c).
#ifndef BANJO_KAZOOIE_BGS_CH_MRVILE_H
#define BANJO_KAZOOIE_BGS_CH_MRVILE_H

typedef struct chmrvile_s{
    u8 playerWithinRange; // can be 2 for some reason?
    // u8  pad1[0x3];
    BKModelBin *yumblieModel;
    ActorMarker *game_marker;
    u8  action;
    // u8  padD[0x3];
    f32 movementSpeed;
    f32 eatingTimer;
    f32 target_position[3];
    f32 animationSpeed;
    f32 rotation[3];
} ActorLocal_MrVile;

enum chMrVileState {
    CH_MR_VILE_STATE_1_IDLE_STILL = 1,
    CH_MR_VILE_STATE_2_TALKING,
    CH_MR_VILE_STATE_3_MINIGAME,
    CH_MR_VILE_STATE_4_ATTACK_PLAYER,
    CH_MR_VILE_STATE_5_IDLE_WALKING,
    CH_MR_VILE_STATE_6_RUN_FROM_PLAYER
};

#endif
