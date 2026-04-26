/**
 * BK time accessors (frame-pacing, delta timers).
 *
 * [port] Named bk_time.h rather than time.h to avoid shadowing the system
 * <time.h> on modern toolchains — Lighthouse's CMake adds include/ to -I, so
 * a local time.h would be picked up ahead of the C standard header. Upstream
 * decomp calls this file time.h because N64/libultra has no system <time.h>
 * to collide with.
 */
#ifndef __GL_TIME_H__
#define __GL_TIME_H__

#include <ultra64.h>
#include "libultraship/libultra/types.h"

void time_reset(void);
void time_setDeltaReal_sec(f32 d_seconds);
void time_setDeltaReal_frames(s32 d_frames);
s32 time_getDeltaReal_frames(void);
f32 time_getDelta(void);
f32 time_func_8033DDB8(void);
f32 time_getDelta_frames(void);
void time_setMultiplier(f32 multiplier);

#endif
