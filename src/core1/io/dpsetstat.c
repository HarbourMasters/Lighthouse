#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include <os_internal.h>
#include <rcp.h>
#endif

void osDpSetStatus(u32 data) { IO_WRITE(DPC_STATUS_REG, data); }
