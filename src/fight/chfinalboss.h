// [port] chfinalboss.c's typedef, moved here for the Anchor fight sync (FightSync.c).
#ifndef BANJO_KAZOOIE_FIGHT_CHFINALBOSS_H
#define BANJO_KAZOOIE_FIGHT_CHFINALBOSS_H

typedef struct chfinalboss_s {
    u8 phase;
    u8 hits;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;
    u8 player_hit_in_phase3;
    u8 mirror_phase5;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    u8 unkC;
    u8 unkD;
    u8 unkE;
    u8 unkF;
    s32 player_hit_in_phase1;
    f32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    u8 pad24[4];
    f32 unk28;
    f32 unk2C;
} ActorLocal_FinalBoss;

#endif
