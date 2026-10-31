// [port] whistleswitch.c's typedefs, moved here for AnimVertexPatches.cpp.
#ifndef BANJO_KAZOOIE_RBB_CH_WHISTLESWITCH_H
#define BANJO_KAZOOIE_RBB_CH_WHISTLESWITCH_H

typedef struct {
    s16 actorId;
    f32 position[3];
    s16 mapSpecificFlag;
} Struct_ChWhistleSwitchInfo;

typedef struct {
    u8 unk0;
    u8 pad1[3];
    Struct_ChWhistleSwitchInfo *whistleSwitchInfo;
    f32 unk8;
    BKVertexList *unkC;
} ActorLocal_RBB_47D0;

#endif
