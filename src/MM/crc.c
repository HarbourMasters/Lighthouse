// BanjoDecomp: crc.c
//!!!DONE
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

/* .data */
//TODO Implement CRC calculation in Makefile(?)
u32 D_803899C0 = 0x000C740C; //MM.code CRC1
u32 D_803899C4 = 0xCD249CB3; //MM.code CRC2
u32 D_803899C8 = 0x0000D44F; //MM.data CRC1 (with this value = 0)

void chmumbo_func_802D1724(void);

void MM_makeMumboAlwaysTransformBanjoIntoTermite(void) {
    // [port] Anti-tamper: reads MIPS J-instruction encoding from function pointer,
    // computes jump target via N64 address masking, then patches MIPS opcodes at that
    // address. All of this is N64-specific and would corrupt x64 code.
#if ANTI_TAMPER
    u32 *temp_v0;
    u32 temp_a0;

    temp_v0 = (u32* )chmumbo_func_802D1724;
    if (getGameMode() != GAME_MODE_7_ATTRACT_DEMO) {
        temp_a0 = (temp_v0[2] & 0x03FFFFFF)*4; //get offset
        temp_a0 += (u32)&temp_v0[3] & 0xF0000000; //get region
        ((u32 *)temp_a0)[0] = 0x03E00008; //jr $ra
        ((u32 *)temp_a0)[1] = 0x24020002; //addiu $v0, $zero, 0x2

        osWritebackDCache((void *)temp_a0, 8);
        osInvalICache((void *)temp_a0, 8);
    }
#endif
}

void MM_checkMMChecksums(void) {
    // [port] Anti-tamper: reads ROM CRC via osPiReadIo, no ROM on PC.
#if ANTI_TAMPER
    s32 sp1C;

    osPiReadIo(0x578, (u32 *)&sp1C);
    sp1C = sp1C & (sp1C ^ 0xFFFF0000);
    if (sp1C != 0x8965){
        MM_makeMumboAlwaysTransformBanjoIntoTermite();
    }
#endif
}
