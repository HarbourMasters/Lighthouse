// [port] pinkegg.c's typedef and states, moved here for the Anchor catch-up (Puzzles/BubblegloopSwamp.cpp).
#ifndef BANJO_KAZOOIE_BGS_CH_PINKEGG_H
#define BANJO_KAZOOIE_BGS_CH_PINKEGG_H

typedef struct chpinkegg_s{
    u32 actorId;
    u32 shellsLeft;
} ActorLocal_PinkEgg;

enum chPinkEggStates {
    CH_PINK_EGG_STATES_1_INIT = 1,
    CH_PINK_EGG_STATES_2_IDLE,
    CH_PINK_EGG_STATES_3_BREAKING
};

#endif
