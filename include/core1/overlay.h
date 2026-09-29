#ifndef BANJO_KAZOOIE_CORE1_OVERLAY_H
#define BANJO_KAZOOIE_CORE1_OVERLAY_H

#include <ultra64.h>

void overlay_load(s32 id, u8 *ram_start, u8 *ram_end, u32 rom_start, u32 rom_end, u8 *code_start, u8 *code_end, u8 *data_start, u8 *data_end, u8 *bss_start, u8 *bss_end);

#endif
