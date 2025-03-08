#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

typedef struct {
  u32 unk0;
  u32 unk4;
} struct49s;

extern struct49s D_803FFE10[];

extern u8 heapBlocks;
extern u32 compressionCRC1;
extern u32 compressionCRC2;

void overlay_load(s32 overlay_id, u32 ram_start, u32 ram_end, u32 rom_start,
                  u32 rom_end, u32 code_start, u32 code_end, u32 data_start,
                  u32 data_end, u32 bss_start, u32 bss_end) {
  #ifndef LIGHTHOUSE_P

  u32 sp34;
  u32 sp30;
  u32 sp2C;
  u32 *tmp;

  osWritebackDCacheAll();
  osInvalDCache(ram_start, ram_end - ram_start);
  osInvalICache(ram_start, ram_end - ram_start);

  if (bss_start) {
    osInvalDCache(bss_start, bss_end - bss_start);
  }

  rom_start = D_803FFE10[overlay_id].unk0;
  rom_end = D_803FFE10[overlay_id].unk4;

  if (overlay_id) {
    sendGfxTaskMessage();
    sp34 = &D_8000E800;
  } else {
    sp34 = &heapBlocks;
  }
  piMgr_read(sp34, rom_start, rom_end - rom_start);
  rarezip_uncompress(&sp34, &ram_start);
  sp2C = compressionCRC1;
  sp30 = compressionCRC2;
  rarezip_uncompress(&sp34, &ram_start);

  if (bss_start) {
    bzero(bss_start, bss_end - bss_start);
    osWritebackDCacheAll();
    tmp = (u32 *)bss_start;
    tmp[0] = sp2C;
    tmp[1] = sp30;
    tmp[2] = compressionCRC1;
    tmp[3] = compressionCRC2;
  }
  #endif
}
