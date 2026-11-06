// [port] yumblie.c's typedef and states, moved here for the Anchor Mr. Vile sync (VileSync.c).
#ifndef BANJO_KAZOOIE_BGS_CH_YUMBLIE_H
#define BANJO_KAZOOIE_BGS_CH_YUMBLIE_H

enum chyumblie_state_e{
    YUMBLIE_STATE_1_UNDER_GROUND = 1,
    YUMBLIE_STATE_2_APPEAR,
    YUMBLIE_STATE_3_ABOVE_GROUND,
    YUMBLIE_STATE_4_DISAPPEAR,
    YUMBLIE_STATE_5_BEING_EATEN
};

typedef struct chyumblie_s{
    f32 animTimer;
    u8  isGrumblie;
    u8  pad7[3];
    f32 waitTimer;
    ActorMarker *game_marker;
} ActorLocal_Yumblie;

#endif
