// [port] blubber.c's typedef and states, moved here for the Anchor catch-up (Puzzles/TreasureTroveCove.cpp).
#ifndef BANJO_KAZOOIE_TTC_CH_BLUBBER_H
#define BANJO_KAZOOIE_TTC_CH_BLUBBER_H

typedef struct {
    u8 unk0[0xE]; // unused?
    s16 unkE; // some sort of index
    f32 throw_target_position[3];
    s32 throw_target_radius;
    NodeProp* gold_bullion_throw_target_node_prop;
    s32 unk24; // bullions left to collect ? (changes from 4 to 0)
} ActorLocal_Blubber;

enum ch_blubber_states_e {
    CH_BLUBBER_STATE_1_UNKNOWN = 1, // initial state (80387A84)
    CH_BLUBBER_STATE_2_UNKNOWN = 2, // 80387B10
    CH_BLUBBER_STATE_3_UNKNOWN = 3,
    CH_BLUBBER_STATE_4_UNKNOWN = 4, // 80387C04
    CH_BLUBBER_STATE_5_UNKNOWN = 5  // 80387CD4
};

#endif
