#ifndef BANJO_KAZOOIE_CORE1_PARALLEL_H
#define BANJO_KAZOOIE_CORE1_PARALLEL_H

#include <ultra64.h>

void parallel_readDMA(void *vaddr, s32 devaddr, s32 size);
void parallel_init(void);

#endif
