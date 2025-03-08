#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "osint.h"
#include <ultra64.h>
#endif

void osYieldThread(void) {
  register u32 saveMask = __osDisableInt();
  __osRunningThread->state = OS_STATE_RUNNABLE;
  __osEnqueueAndYield(&__osRunQueue);
  __osRestoreInt(saveMask);
}
