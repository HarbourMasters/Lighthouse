
#ifndef LIGHTHOUSE_P
#include <os_internal.h>
#include <rcp.h>
#else 
#include "pc_oscompat.h"
#endif

s32 __osAiDeviceBusy(void) {
  register s32 status = IO_READ(AI_STATUS_REG);
  if (status & AI_STATUS_FIFO_FULL)

    return 1;

  return 0;
}
