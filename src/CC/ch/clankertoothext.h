// [port] clankertoothext.c's typedef and states, moved here for the Anchor catch-up (Puzzles/ClankersCavern.cpp).
#ifndef BANJO_KAZOOIE_CC_CH_CLANKERTOOTHEXT_H
#define BANJO_KAZOOIE_CC_CH_CLANKERTOOTHEXT_H

typedef struct{
    s32 unk0;
    s32 egg_count;
    f32 unk8;
}ActorLocal_CC_BF0;

enum maClankerTooth_state_e {
    CLANKER_TOOTH_STATE_1_UPRIGHT = 1,
    CLANKER_TOOTH_STATE_2_FALLING_OVER,
    CLANKER_TOOTH_STATE_3_KNOCKED_OUT,
};

#endif
