

#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include <os_internal.h>
#include <rcp.h>
#include "osint.h"
#endif

#include <sptask.h>

int __osSpDeviceBusy() {
  register u32 stat = IO_READ(SP_STATUS_REG);
  if (stat & (SP_STATUS_DMA_BUSY | SP_STATUS_DMA_FULL | SP_STATUS_IO_FULL))
    return 1;
  return 0;
}
