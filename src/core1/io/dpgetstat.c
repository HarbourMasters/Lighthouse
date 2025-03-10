#ifdef LIGHTHOUSE_P
#include "libultraship.h"
#else
#include <os_internal.h>
#include <rcp.h>
#endif


u32 osDpGetStatus() { return IO_READ(DPC_STATUS_REG); }
