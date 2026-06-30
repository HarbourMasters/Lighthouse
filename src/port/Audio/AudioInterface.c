#include "libultraship/libultra/types.h"

extern s32 osViClock;

s32 osAiSetFrequency(u32 frequency) {
    f32 dacRateF = ((f32)osViClock / frequency) + 0.5f;
    u32 dacRate = (u32)dacRateF;
    if (dacRate < 132) {
        return -1;
    }
    return osViClock / (s32)dacRate;
}
