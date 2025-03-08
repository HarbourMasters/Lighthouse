#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include <os_internal.h>
#include <rcp.h>
#endif


void osSpTaskYield(void) { __osSpSetStatus(SP_SET_YIELD); }
