#include "libultraship/libultra/types.h"
#include "libultraship/libultra/interrupt.h"
#include "libultraship/libultra/sptask.h"
#include "libultraship/libultra/thread.h"
#include "bk_string.h"

#include <libultra/convert.h>
#include <libultra/exception.h>
#include <libultra/rcp.h>

#define DEFAULT_FRAMEBUFFER_WIDTH 292
#define DEFAULT_FRAMEBUFFER_HEIGHT 216

u64 osClockRate = OS_CLOCK_RATE;
s32 osViClock = VI_NTSC_CLOCK;
u32 __osShutdown = 0;
u32 __OSGlobalIntMask = OS_IM_ALL;
s32 osCicId = 6103;
// [port] On N64 this was a fixed-address depth buffer at 0x8000E800 (naturally 0x40-aligned).
// On PC we need a properly sized and aligned buffer to avoid the alignment loop in func_80253428.
_Alignas(0x40) u8 D_8000E800[DEFAULT_FRAMEBUFFER_WIDTH * DEFAULT_FRAMEBUFFER_HEIGHT * sizeof(u16)];

u16 gFramebuffers[2][DEFAULT_FRAMEBUFFER_WIDTH * DEFAULT_FRAMEBUFFER_HEIGHT];

void osCreateThread(OSThread* thread, OSId id, void* entry, void* arg, void* sp, OSPri p) {
}
void osStartThread(OSThread* thread) {
}
void osStopThread(OSThread* t) {
}
void osDestroyThread(OSThread* thread) {
}
void osSpTaskYield(void) {
}
void osSpTaskLoad(OSTask* task) {
}
void osSpTaskStartGo(OSTask* task) {
}
void osViExtendVStart(u32 arg0) {
}
void osSetThreadPri(OSThread* thread, OSPri p) {
}
s32 osContSetCh(u8 ch) {
    return 0;
}
u32 __osGetSR(void) {
    return 0;
}
void __osSetSR(u32 value) {
}
u32 bkGetSR(void) {
    return 0;
}
OSYieldResult osSpTaskYielded(OSTask* task) {
    return 0;
}
// Lighthouse TODO these need to be implemented in LUS
int osStartTimer(void* t) {
    return 0;
}
int osStopTimer(void* t) {
    return 0;
}

void osDpSetStatus(u32 data) {
}

void __osError(s16 error_code, s16 num_args, ...) {
}

#if 0
s32 osMotorStop(void* pfs) {
    return 0;
}
s32 osMotorStart(void* pfs) {
    return 0;
}
#endif

s32 eeprom_writeBlocks(s32 file, s32 offset, void* buffer, s32 count) {
    return 0;
}

s32 eeprom_readBlocks(s32 file, s32 offset, void* buffer, s32 count) {
    return 0;
}

/* BSD memory functions */
// void bzero(void* s, size_t n) {
//     memset(s, 0, n);
// }

// void bcopy(const void* src, void* dest, size_t n) {
//     memmove(dest, src, n);
// }
