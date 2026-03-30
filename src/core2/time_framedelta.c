#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#include <SDL2/SDL.h> // [port] wall-clock timing

/* .bss */
s32 D_80384470;

/* .code */
f32 func_8033DBF0(void){
    return (f32)D_80384470;
}

void func_8033DC04(void){
    D_80384470 = 0;
}

void func_8033DC10(void){}

void func_8033DC18(void){}

f32 func_8033DC20(void){
    // [port] Original N64: returns D_80280724 * (1/60.0) — fixed delta per VI count.
    // On PC, we measure real wall-clock time. Callers MUST use this value directly
    // via time_setDeltaReal_sec(), not convert through integer frames (which truncates).
    static u64 last_ticks = 0;
    u64 now = SDL_GetPerformanceCounter();
    u64 freq = SDL_GetPerformanceFrequency();
    f32 out;

    if (last_ticks == 0) {
        last_ticks = now;
        out = 1.0f / 30.0f; // first frame: assume 1/30 (N64 rate)
    } else {
        out = (f32)(now - last_ticks) / (f32)freq;
        last_ticks = now;
    }

    // Cap at 50ms (matching time_setDeltaReal_sec cap)
    if (out > 0.05f) out = 0.05f;
    if (out <= 0.0f) out = 1.0f / 60.0f;

    // Keep D_80384470 updated for anything that reads it
    D_80384470 = (s32)(out * 60.0f + 0.5f);
    if (D_80384470 < 1) D_80384470 = 1;

    return out;
}
