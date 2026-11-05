// [port] ccgrates.c's typedef and states, moved here for the Anchor catch-up (Puzzles/ClankersCavern.cpp).
#ifndef BANJO_KAZOOIE_CC_CH_CCGRATES_H
#define BANJO_KAZOOIE_CC_CH_CCGRATES_H

typedef struct {
    u8 *unk0;
    f32 unk4;
    s32 unk8;
} ActorLocal_CC_3400;

enum chCCGrates_state_e {
    CC_GRATES_STATE_1_IDLE = 1,
    CC_GRATES_STATE_2_UNK,
    CC_GRATES_STATE_3_UNK,
    CC_GRATES_STATE_4_DESPAWN,
};

#endif
