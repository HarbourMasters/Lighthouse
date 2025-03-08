#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "viint.h"
#include <os_internal.h>
#endif

__OSViContext *__osViGetCurrentContext(void) { return __osViCurr; }
