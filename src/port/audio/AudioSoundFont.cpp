/**
 * AudioSoundFont.c — [port] N64 soundfont binary parser for 64-bit
 *
 * The N64 .ctl (control) files store ALBankFile/ALBank/ALInstrument/ALSound/ALWaveTable
 * as flat binary with 32-bit offsets in big-endian byte order. On N64, alBnkfNew() patches
 * these offsets to pointers in-place — this works because pointers are 32-bit and the CPU
 * is big-endian.
 *
 * On 64-bit little-endian PC, the struct layouts differ (pointers are 8 bytes instead of 4)
 * and all multi-byte integers need byte-swapping. This file provides port_alBnkfNew() which
 * reads the N64 binary and allocates proper native structs.
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

extern "C" {
#include <ultra64.h>
#include "2.0L/PR/libaudio.h"

// Read big-endian values from byte buffer (no-op on native BE systems)
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
static u16 read_u16_be(const u8* d) { u16 v; memcpy(&v, d, 2); return v; }
static s16 read_s16_be(const u8* d) { return (s16)read_u16_be(d); }
static u32 read_u32_be(const u8* d) { u32 v; memcpy(&v, d, 4); return v; }
static s32 read_s32_be(const u8* d) { return (s32)read_u32_be(d); }
#else
static u16 read_u16_be(const u8* d) { return ((u16)d[0] << 8) | d[1]; }
static s16 read_s16_be(const u8* d) { return (s16)read_u16_be(d); }
static u32 read_u32_be(const u8* d) { return ((u32)d[0] << 24) | ((u32)d[1] << 16) | ((u32)d[2] << 8) | d[3]; }
static s32 read_s32_be(const u8* d) { return (s32)read_u32_be(d); }
#endif

/*
 * N64 binary layout offsets (all pointer/offset fields are u32 BE, relative to ctl start):
 *
 * ALBankFile:  [0] s16 revision, [2] s16 bankCount, [4+] u32 bankOffsets[]
 * ALBank:      [0] s16 instCount, [2] u8 flags, [3] u8 pad, [4] s32 sampleRate,
 *              [8] u32 percussion, [12+] u32 instOffsets[]
 * ALInstrument:[0] u8 volume..vibDelay (12 bytes), [12] s16 bendRange,
 *              [14] s16 soundCount, [16+] u32 soundOffsets[]
 * ALSound:     [0] u32 envelope, [4] u32 keyMap, [8] u32 wavetable,
 *              [12] u8 samplePan, [13] u8 sampleVolume, [14] u8 flags
 * ALWaveTable: [0] u32 base, [4] s32 len, [8] u8 type, [9] u8 flags,
 *              [12] ADPCM: u32 loop, u32 book  |  RAW16: u32 loop
 * ALEnvelope:  [0] s32 attackTime, [4] s32 decayTime, [8] s32 releaseTime,
 *              [12] u8 attackVolume, [13] u8 decayVolume
 * ALKeyMap:    [0] u8 velocityMin, [1] u8 velocityMax, [2] u8 keyMin,
 *              [3] u8 keyMax, [4] u8 keyBase, [5] s8 detune
 * ALADPCMBook: [0] s32 order, [4] s32 npredictors, [8+] s16 book[]
 * ALADPCMloop: [0] u32 start, [4] u32 end, [8] u32 count, [12] s16 state[16]
 * ALRawLoop:   [0] u32 start, [4] u32 end, [8] u32 count
 */

static ALEnvelope* parseEnvelope(const u8* ctl, u32 off) {
    const u8* d = ctl + off;
    ALEnvelope* env = (ALEnvelope*)malloc(sizeof(ALEnvelope));

    env->attackTime = read_s32_be(d + 0);
    env->decayTime = read_s32_be(d + 4);
    env->releaseTime = read_s32_be(d + 8);
    env->attackVolume = d[12];
    env->decayVolume = d[13];

    return env;
}

static ALKeyMap* parseKeyMap(const u8* ctl, u32 off) {
    const u8* d = ctl + off;
    ALKeyMap* km = (ALKeyMap*)malloc(sizeof(ALKeyMap));

    km->velocityMin = d[0];
    km->velocityMax = d[1];
    km->keyMin = d[2];
    km->keyMax = d[3];
    km->keyBase = d[4];
    km->detune = (s8)d[5];

    return km;
}

static ALADPCMBook* parseADPCMBook(const u8* ctl, u32 off) {
    const u8* d = ctl + off;

    s32 order = read_s32_be(d + 0);
    s32 npredictors = read_s32_be(d + 4);
    s32 bookSize = order * npredictors * 8; // number of s16 entries

    size_t allocSize = sizeof(ALADPCMBook) + (bookSize > 1 ? (bookSize - 1) * sizeof(s16) : 0);
    ALADPCMBook* book = (ALADPCMBook*)malloc(allocSize);

    book->order = order;
    book->npredictors = npredictors;

    for (s32 i = 0; i < bookSize; i++) {
        book->book[i] = read_s16_be(d + 8 + i * 2);
    }

    return book;
}

static ALADPCMloop* parseADPCMLoop(const u8* ctl, u32 off) {
    const u8* d = ctl + off;
    ALADPCMloop* loop = (ALADPCMloop*)malloc(sizeof(ALADPCMloop));

    loop->start = read_u32_be(d + 0);
    loop->end = read_u32_be(d + 4);
    loop->count = read_u32_be(d + 8);

    for (s32 i = 0; i < 16; i++) {
        loop->state[i] = read_s16_be(d + 12 + i * 2);
    }

    return loop;
}

static ALRawLoop* parseRawLoop(const u8* ctl, u32 off) {
    const u8* d = ctl + off;
    ALRawLoop* loop = (ALRawLoop*)malloc(sizeof(ALRawLoop));

    loop->start = read_u32_be(d + 0);
    loop->end = read_u32_be(d + 4);
    loop->count = read_u32_be(d + 8);

    return loop;
}

static ALWaveTable* parseWaveTable(const u8* ctl, u32 wtOff, u8* tbl) {
    const u8* d = ctl + wtOff;
    ALWaveTable* wt = (ALWaveTable*)malloc(sizeof(ALWaveTable));
    memset(wt, 0, sizeof(ALWaveTable));

    wt->base = tbl + read_u32_be(d + 0);
    wt->len = read_s32_be(d + 4);
    wt->type = d[8];
    wt->flags = d[9];

    if (wt->type == AL_ADPCM_WAVE) {
        u32 loopOff = read_u32_be(d + 12);
        u32 bookOff = read_u32_be(d + 16);
        wt->waveInfo.adpcmWave.loop = loopOff ? parseADPCMLoop(ctl, loopOff) : NULL;
        wt->waveInfo.adpcmWave.book = bookOff ? parseADPCMBook(ctl, bookOff) : NULL;
    } else if (wt->type == AL_RAW16_WAVE) {
        u32 loopOff = read_u32_be(d + 12);
        wt->waveInfo.rawWave.loop = loopOff ? parseRawLoop(ctl, loopOff) : NULL;
    }

    return wt;
}

static ALSound* parseSound(const u8* ctl, u32 sndOff, u8* tbl) {
    const u8* d = ctl + sndOff;
    ALSound* s = (ALSound*)malloc(sizeof(ALSound));
    memset(s, 0, sizeof(ALSound));

    u32 envOff = read_u32_be(d + 0);
    u32 kmOff = read_u32_be(d + 4);
    u32 wtOff = read_u32_be(d + 8);

    s->envelope = parseEnvelope(ctl, envOff);
    s->keyMap = parseKeyMap(ctl, kmOff);
    s->wavetable = parseWaveTable(ctl, wtOff, tbl);

    s->samplePan = d[12];
    s->sampleVolume = d[13];
    s->flags = d[14];

    return s;
}

static ALInstrument* parseInstrument(const u8* ctl, u32 instOff, u8* tbl) {
    const u8* d = ctl + instOff;

    s16 soundCount = read_s16_be(d + 14);
    size_t allocSize = sizeof(ALInstrument) + (soundCount > 1 ? (soundCount - 1) * sizeof(ALSound*) : 0);
    ALInstrument* inst = (ALInstrument*)malloc(allocSize);
    memset(inst, 0, allocSize);

    inst->volume = d[0];
    inst->pan = d[1];
    inst->priority = d[2];
    inst->flags = d[3];
    inst->tremType = d[4];
    inst->tremRate = d[5];
    inst->tremDepth = d[6];
    inst->tremDelay = d[7];
    inst->vibType = d[8];
    inst->vibRate = d[9];
    inst->vibDepth = d[10];
    inst->vibDelay = d[11];
    inst->bendRange = read_s16_be(d + 12);
    inst->soundCount = soundCount;

    for (s32 i = 0; i < soundCount; i++) {
        u32 sndOff = read_u32_be(d + 16 + i * 4);
        inst->soundArray[i] = parseSound(ctl, sndOff, tbl);
    }

    return inst;
}

static ALBank* parseBank(const u8* ctl, u32 bankOff, u8* tbl) {
    const u8* d = ctl + bankOff;

    s16 instCount = read_s16_be(d + 0);
    size_t allocSize = sizeof(ALBank) + (instCount > 1 ? (instCount - 1) * sizeof(ALInstrument*) : 0);
    ALBank* bank = (ALBank*)malloc(allocSize);
    memset(bank, 0, allocSize);

    bank->instCount = instCount;
    bank->flags = d[2];
    bank->pad = d[3];
    bank->sampleRate = read_s32_be(d + 4);

    u32 percOff = read_u32_be(d + 8);
    bank->percussion = percOff ? parseInstrument(ctl, percOff, tbl) : NULL;

    for (s32 i = 0; i < instCount; i++) {
        u32 instOff2 = read_u32_be(d + 12 + i * 4);
        bank->instArray[i] = instOff2 ? parseInstrument(ctl, instOff2, tbl) : NULL;
    }

    return bank;
}

/**
 * port_alBnkfNew — Parse an N64 .ctl binary (big-endian, 32-bit offsets)
 * into properly allocated 64-bit native-endian ALBankFile structs.
 *
 * @param ctlData  Raw .ctl binary data (big-endian, from OTR)
 * @param ctlSize  Size of ctl data in bytes
 * @param tblData  Raw .tbl sample data (from OTR)
 * @return         Newly allocated ALBankFile with all pointers resolved
 */
ALBankFile* port_alBnkfNew(u8* ctlData, s32 ctlSize, u8* tblData) {
    s16 revision = read_s16_be(ctlData + 0);
    s16 bankCount = read_s16_be(ctlData + 2);

    size_t allocSize = sizeof(ALBankFile) + (bankCount > 1 ? (bankCount - 1) * sizeof(ALBank*) : 0);
    ALBankFile* file = (ALBankFile*)malloc(allocSize);
    memset(file, 0, allocSize);

    file->revision = revision;
    file->bankCount = bankCount;

    for (s32 i = 0; i < bankCount; i++) {
        u32 bankOff = read_u32_be(ctlData + 4 + i * 4);
        file->bankArray[i] = bankOff ? parseBank(ctlData, bankOff, tblData) : NULL;
    }

    return file;
}

} // extern "C"
