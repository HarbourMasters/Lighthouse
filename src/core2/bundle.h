#pragma once
#include "libultraship/libultra/gbi.h"

typedef struct {
    s16 flags;
    // u8 pad2[0x2];
    s32 actor_id;
    s32 count;
    s16 sfx_id;
    s16 sfx_volume;
    s16 sfx_sampleRate;
    // u8 pad12[0x2];
    f32 velocity_x;
    f32 randomVelocity_x;
    f32 velocity_y;
    f32 randomVelocity_y;
    f32 velocity_z;
    f32 randomVelocity_z;
    f32 bounce_factor;
    f32 yaw;
} BundleInfo;
