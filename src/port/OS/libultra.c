// BanjoDecomp: This file no longer exists upstream,
// but we do wrap its functions with our own.

// Include the headers that declare what this file defines, so the compiler
// checks the definitions against them.
#include "libultraship/libultra/types.h"
#include "libultraship/libultra/interrupt.h"
#include "libultraship/libultra/sptask.h"
#include "libultraship/libultra/thread.h"
#include "libultraship/libultra/os.h"
#include "functions.h"
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
// On N64 this was a fixed-address depth buffer at 0x8000E800 (naturally 0x40-aligned).
// On PC we need a properly sized and aligned buffer to avoid the alignment loop in func_80253428.
_Alignas(0x40) u8 D_8000E800[DEFAULT_FRAMEBUFFER_WIDTH * DEFAULT_FRAMEBUFFER_HEIGHT * sizeof(u16)];

u16 gFramebuffers[2][DEFAULT_FRAMEBUFFER_WIDTH * DEFAULT_FRAMEBUFFER_HEIGHT];

// Threads are in src/port/OS/OS.cpp.
void OS_CreateThread(OSThread* thread, OSId id, void* entry, void* arg, void* sp, OSPri p);
void OS_StartThread(OSThread* thread);
void OS_StopThread(OSThread* thread);
void OS_DestroyThread(OSThread* thread);
void OS_SetThreadPri(OSThread* thread, OSPri p);

void osCreateThread(OSThread* thread, OSId id, void* entry, void* arg, void* sp, OSPri p) {
    OS_CreateThread(thread, id, entry, arg, sp, p);
}
void osStartThread(OSThread* thread) {
    OS_StartThread(thread);
}
void osStopThread(OSThread* t) {
    OS_StopThread(t);
}
void osDestroyThread(OSThread* thread) {
    OS_DestroyThread(thread);
}
void osViExtendVStart(u32 arg0) {
}
void osSetThreadPri(OSThread* thread, OSPri p) {
    OS_SetThreadPri(thread, p);
}
s32 osContSetCh(u8 ch) {
    return 0;
}
u32 __osGetSR(void) {
    return 0;
}
void __osSetSR(u32 value) {
}
// All interrupt-enable bits set. thread5_checkAndExecutePreNMI reads SR_IBIT5
// and treats a clear bit as "reset pressed"; with retraces running, returning
// 0 fires the PreNMI handler 60 times a second.
u32 bkGetSR(void) {
    return 0xFFFFFFFFu;
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
