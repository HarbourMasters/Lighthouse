// BanjoDecomp: (port-specific, no decomp origin)
#include "ResourceHelpers.h"
#include "libultraship/libultra/gbi.h"
#include "libultraship/libultra/interrupt.h"
#include "libultraship/libultra/sptask.h"
#include "libultraship/libultra/thread.h"
#include "libultraship/bridge/resourcebridge.h"
#include <math.h>
#include <string.h>
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

int ResourceMgr_OTRSigCheck(char* imgData) {
    uintptr_t i = (uintptr_t)(imgData);

    // [port] Reject N64 segmented addresses and special sentinels.
    // Segmented addresses fit in 32 bits with a non-zero segment byte (bits 24-31).
    // Bit 0 set = already-tagged segmented address from stub GBI functions.
    if ((i & 1) == 1)
        return 0;
    if (i != 0 && i <= 0xFFFFFFFF && (i >> 24) != 0)
        return 0;

    // if ((i & 0xFF000000) != 0xAB000000 && (i & 0xFF000000) != 0xCD000000 && i != 0) {
    if (i != 0) {
        if (imgData[0] == '_' && imgData[1] == '_' && imgData[2] == 'O' && imgData[3] == 'T' && imgData[4] == 'R' &&
            imgData[5] == '_' && imgData[6] == '_')
            return 1;
    }

    return 0;
}

void gSPDisplayList(Gfx* pkt, Gfx* dl) {
    char* imgData = (char*)dl;

    if (ResourceMgr_OTRSigCheck(imgData) == 1) {

        // ResourceMgr_PushCurrentDirectory(imgData);
        // gsSPPushCD(pkt++, imgData);
        dl = ResourceMgr_LoadGfxByName(imgData);
    }

    __gSPDisplayList(pkt, dl);
}

void gSPDisplayListOffset(Gfx* pkt, Gfx* dl, int offset) {
    char* imgData = (char*)dl;

    if (ResourceMgr_OTRSigCheck(imgData) == 1)
        dl = ResourceMgr_LoadGfxByName(imgData);

    __gSPDisplayList(pkt, dl + offset);
}

void gSPVertex(Gfx* pkt, uintptr_t v, int n, int v0) {
    if (ResourceMgr_OTRSigCheck((char*)v) == 1)
        v = (uintptr_t)ResourceMgr_LoadVtxByName((char*)v);

    // [port] Mark N64 segmented addresses with bit 0 so the interpreter's SegAddr resolves them.
    // SEGMENT_ADDR(num, off) produces values like 0x01000000 without bit 0 set.
    // Segmented addresses fit in 32 bits with a non-zero segment byte (bits 24-31).
    if (v != 0 && v <= 0xFFFFFFFF && (v >> 24) != 0) {
        v |= 1;
    }

    __gSPVertex(pkt, v, n, v0);
}

void gSPInvalidateTexCache(Gfx* pkt, uintptr_t texAddr) {
    char* imgData = (char*)texAddr;

    if (texAddr != 0 && ResourceMgr_OTRSigCheck(imgData)) {
        // Temporary solution to the mq/nonmq issue, this will be
        // handled better with LUS 1.0
        texAddr = (uintptr_t)ResourceMgr_LoadTexOrDListByName(imgData);
    }

    __gSPInvalidateTexCache(pkt, texAddr);
}

void gSPSegment(void* value, int segNum, uintptr_t target) {
    // [port] BK never passes OTR paths through gSPSegment — segment addresses are
    // always raw data pointers (vertices, textures, render mode tables). Skipping
    // the OTR signature check avoids reading from potentially-freed model blob memory.
    __gSPSegment(value, segNum, target);
}

void gSPSegmentLoadRes(void* value, int segNum, uintptr_t target) {
    char* imgData = (char*)target;

    int res = ResourceMgr_OTRSigCheck(imgData);

    if (res) {
        target = (uintptr_t)ResourceMgr_LoadTexOrDListByName(imgData);
    }

    __gSPSegment(value, segNum, target);
}

// The OtrSignatureCheck and gfx_check_image_signature in LUS have been patched to
// handle raw heap pointers without crashing (byte-by-byte check, address filtering).
// Raw sprite texture data embedded in BKSprite structures passes through as-is;
// OTR-tagged model textures (__OTR__ paths) are handled by LUS natively.
void port_gDPSetTextureImage(Gfx* pkt, int fmt, int siz, int width, const void* img) {
    Gfx* _g = (Gfx*)(pkt);
    _g->words.w0 = _SHIFTL(G_SETTIMG, 24, 8) | _SHIFTL(fmt, 21, 3) | _SHIFTL(siz, 19, 2) | _SHIFTL((width)-1, 0, 12);
    _g->words.w1 = (uintptr_t)(img);
}

f32 gu_sqrtf(f32 f) {
    return sqrtf(f);
}
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
OSIntMask osSetIntMask(OSIntMask a) {
    return 0;
}

void __osError(s16 error_code, s16 num_args, ...) {
}

// [port] These mirror BK's handwritten 8-byte-aligned memcpy/memset assembly. Implement as plain memcpy/memset.
void bkmemcpy64(void* dest, void* src, s32 size) {
    memcpy(dest, src, size);
}
void bkmemset64(void* dest, s32 value, s32 size) {
    memset(dest, value, size);
}

#if 0
s32 osMotorStop(void* pfs) {
    return 0;
}
s32 osMotorStart(void* pfs) {
    return 0;
}
#endif

s32 osAiSetFrequency(u32 frequency) {
    f32 dacRateF = ((f32)osViClock / frequency) + 0.5f;
    u32 dacRate = (u32)dacRateF;
    if (dacRate < 132) {
        return -1;
    }
    return osViClock / (s32)dacRate;
}

s32 eeprom_writeBlocks(s32 file, s32 offset, void* buffer, s32 count) {
    return 0;
}

s32 eeprom_readBlocks(s32 file, s32 offset, void* buffer, s32 count) {
    return 0;
}

u32 func_8025C29C(u32* seed) {
    // Treat as two u32 values (lower and upper half of u64)
    u32 result = seed[0] ^ seed[1];
    // Simple transformation to update seed
    seed[0] = (seed[0] >> 1) ^ seed[1];
    seed[1] = (seed[1] << 1) ^ seed[0];
    return result;
}

/* BSD memory functions */
// void bzero(void* s, size_t n) {
//     memset(s, 0, n);
// }

// void bcopy(const void* src, void* dest, size_t n) {
//     memmove(dest, src, n);
// }
