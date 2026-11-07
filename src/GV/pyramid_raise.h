// [port] pyramid_raise.c's typedef and states, moved here for the Anchor catch-up (Puzzles/GobisValley.cpp).
#ifndef BANJO_KAZOOIE_GV_PYRAMID_RAISE_H
#define BANJO_KAZOOIE_GV_PYRAMID_RAISE_H

typedef struct {
    s32 unk0;
    u8 sfxsourceIdx;
    //u8 pad5[3];
    u8 *unk8;
    f32 unkC;
    u8 unk10;
    //u8 pad11[3];
    ActorMarker *unk14;
    f32 unk18[3];
    f32 unk24;
}ActorLocal_GV_6370;

enum chjinxyheadsandybutt_state_e {
    CHJINXYHEADSANDYBUTT_STATE_0_NOT_INIT,
    CHJINXYHEADSANDYBUTT_STATE_1_UNK,
    CHJINXYHEADSANDYBUTT_STATE_2_UNK,
    CHJINXYHEADSANDYBUTT_STATE_3_UNK,
    CHJINXYHEADSANDYBUTT_STATE_4_UNK,
    CHJINXYHEADSANDYBUTT_STATE_5_UNK,
    CHJINXYHEADSANDYBUTT_STATE_6_FED,
    CHJINXYHEADSANDYBUTT_STATE_7_GO_AWAY,
    CHJINXYHEADSANDYBUTT_STATE_8_COMPLETED
};

#endif
