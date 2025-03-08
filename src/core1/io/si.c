#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#endif

int __osSiDeviceBusy() {
  register u32 stat = IO_READ(SI_STATUS_REG);
  if (stat & (SI_STATUS_DMA_BUSY | SI_STATUS_RD_BUSY))
    return 1;
  return 0;
}
