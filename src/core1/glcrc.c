// BanjoDecomp: glcrc.c
#include <ultra64.h>
#include "core1/core1.h"

// The decomp links this from the ROM's mips3 segment; recovered here by
// disassembling core1 and verified against real save/global checksums.
// Updates the 64-bit seed in place and returns its low word.
s32 func_8025C29C(u64 *seed) {
    u64 s = *seed;
    u64 a2 = (s << 63) >> 31;
    u64 a1 = (s << 31) >> 32;
    u64 a3 = (s << 44) >> 32;
    a2 = (a2 | a1) ^ a3;
    a3 = ((a2 >> 20) & 0xfff) ^ a2;
    *seed = a3;
    return (s32)a3;
}

void glcrc_calcChecksum(void *start, void *end, u32 checksum[2]) {
    u8 *p;
    u32 shift = 0;
    u64 seed = 0x8F809F473108B3C1;
    u32 crc1 = 0;
    u32 crc2 = 0;
    s32 tmp;

    // CRC1: Iterate forwards over bytes
    for (p = (u8*)start; p < (u8*)end; p++) {
        seed += *p << (shift & 15);
        tmp = func_8025C29C(&seed);
        shift += 7;
        crc1 ^= tmp;
    }

    // CRC2: Iterate backwards over bytes
    for (p = (u8 *)end - 1; p >= (u8*)start; p--) {
        seed += *p << (shift & 15);
        tmp = func_8025C29C(&seed);
        shift += 3;
        crc2 ^= tmp;
    }

    checksum[0] = crc1;
    checksum[1] = crc2;
}
