// [port] outsideautumnnabnut.c's typedef and states, moved here for the Anchor catch-up (Puzzles/ClickClockWood.cpp).
#ifndef BANJO_KAZOOIE_CCW_CH_OUTSIDEAUTUMNNABNUT_H
#define BANJO_KAZOOIE_CCW_CH_OUTSIDEAUTUMNNABNUT_H

typedef struct {
    s32 returned_acorn_count;
}ActorLocal_ChAutumnNabnutPosition;

enum chautumnoutdoornabnut_state_e{
    CH_AUTUMN_OUTDOOR_NABNUT_STATE_0_NOT_INIT,
    CH_AUTUMN_OUTDOOR_NABNUT_STATE_1_SAD,
    CH_AUTUMN_OUTDOOR_NABNUT_STATE_2_WAIT,
    CH_AUTUMN_OUTDOOR_NABNUT_STATE_3_BACKFLIP,
    CH_AUTUMN_OUTDOOR_NABNUT_STATE_4_THANK_PLAYER,
    CH_AUTUMN_OUTDOOR_NABNUT_STATE_5_EXIT,
    CH_AUTUMN_OUTDOOR_NABNUT_STATE_6_DESPAWN
};

#endif
