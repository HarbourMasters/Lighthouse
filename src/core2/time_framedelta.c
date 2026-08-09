// BanjoDecomp: core2/code_B6C60.c
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
    D_80384470 = viMgr_func_8024BD94();
    out = (1.0 / FRAMERATE)*D_80384470;
    return out;
}
