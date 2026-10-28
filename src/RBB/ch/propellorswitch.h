// [port] propellorswitch.c's typedefs and states, moved here for the Anchor catch-up (Puzzles/RustyBucketBay.cpp).
#ifndef BANJO_KAZOOIE_RBB_CH_PROPELLORSWITCH_H
#define BANJO_KAZOOIE_RBB_CH_PROPELLORSWITCH_H

typedef struct {
    s16 unk0[3];
    s16 unk6;
    s16 unk8;
    s16 unkA;
}Struct_RBB_3CB0;

typedef struct {
    Struct_RBB_3CB0 *unk0;
    void *unk4;
}ActorLocal_RBB_3CB0;

enum chrbbgreenpropellerswitch_state_e {
    CH_RBB_GREEN_PROPELLER_SWITCH_STATE_0_NOT_INIT,
    CH_RBB_GREEN_PROPELLER_SWITCH_STATE_1_NOT_PRESSED,
    CH_RBB_GREEN_PROPELLER_SWITCH_STATE_2_PRESSED
};

#endif
