#ifndef BANJO_KAZOOIE_CORE1_GLCRC_H
#define BANJO_KAZOOIE_CORE1_GLCRC_H

#include <ultra64.h>

s32 func_8025C29C(u64 *seed); // transform seed (in mips3 file)
void glcrc_calcChecksum(void *start, void *end, u32 checksum[2]);

#endif
