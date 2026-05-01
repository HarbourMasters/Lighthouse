#include <ultra64.h>
#include "boot/overlaytable.h"
#include "boot/rarezip.h"
#include "core1/main.h"

#define ENTRY_STACK_LEN 0x2000

u8 gEntryStack[ENTRY_STACK_LEN];

extern u8 gHeapBase[];
extern u8 core1_VRAM[];
extern u32 gCore1CRCs[4];

/// @brief Entry point of the game
/// @param arg0 unused/unknown parameter
void func_80000450(s32 arg0) {
    // [port] Stubbed on PC. This is the N64 ROM entry point that boots core1
    // by DMA-reading the core1_rzip segment from ROM, decompressing it into
    // VRAM via rarezip, recording its CRCs into gCore1CRCs, and jumping into
    // core1_main. None of that applies to Lighthouse: there is no ROM to DMA,
    // no linker-provided core1_rzip_ROM_START/END segment, no core1_VRAM, and
    // no need for CRCs. The PC entry point is port/Engine.cpp, which hands off
    // to core1_main through its own initialization path. Nothing in the
    // codebase calls func_80000450, so the body is left out on PC.
#if 0
    u8 *in = gHeapBase;
    u8 *out = core1_VRAM;

    osInitialize();

    osPiRawStartDma(OS_READ, (u32)(uintptr_t)core1_rzip_ROM_START, in, (u32)((uintptr_t)core1_rzip_ROM_END - (uintptr_t)core1_rzip_ROM_START));
    while (osPiGetStatus() & PI_STATUS_DMA_BUSY);

    rarezip_init();

    rarezip_uncompress_file_and_update_pointers(&in, &out);
    gCore1CRCs[0] = inflate_crc1;
    gCore1CRCs[1] = inflate_crc2;

    rarezip_uncompress_file_and_update_pointers(&in, &out);
    gCore1CRCs[2] = inflate_crc1;
    gCore1CRCs[3] = inflate_crc2;

    overlaytable_init();

    (&core1_main)(arg0);
#endif
}
