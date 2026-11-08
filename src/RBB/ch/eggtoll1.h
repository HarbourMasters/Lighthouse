// [port] eggtoll1.c's typedefs and states, moved here for the Anchor egg toll sync (Packets/EggToll.cpp).
#ifndef BANJO_KAZOOIE_RBB_CH_EGGTOLL1_H
#define BANJO_KAZOOIE_RBB_CH_EGGTOLL1_H

typedef struct {
    s32 eggsPaid;
    s32 tollId;
    s32 tollNum; // 0 for first toll; 1 for second toll
    s32 unkC; // Either 0 or 3?
} ActorLocal_ChRBBEggToll;

typedef struct {
    u8 cameraNode;
    u8 eggCost1;
    u8 eggCost2; // Only used in Toll 2
    //u8 pad3[1];
    f32 unk4;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    //u8 padE[2];
} chRBBEggTollInfo;

enum chrbbeggtoll_state_e {
    CH_RBB_EGG_TOLL_STATE_0_NOT_INIT,
    CH_RBB_EGG_TOLL_STATE_1_AWAITING_TOLL,
    CH_RBB_EGG_TOLL_STATE_2_FIRST_EGG_PAID,
    CH_RBB_EGG_TOLL_STATE_3_TOLL_COMPLETE,
    CH_RBB_EGG_TOLL_STATE_4_UNK
};

#endif
