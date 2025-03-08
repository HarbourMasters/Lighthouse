#include "functions.h"
#include "variables.h"
#include <ultra64.h>
#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "viint.h"
#endif

void osViSetEvent(OSMesgQueue *mq, OSMesg m, u32 retraceCount) {
  register u32 saveMask;
  saveMask = __osDisableInt();
  __osViNext->msgq = mq;
  __osViNext->msg = m;
  __osViNext->retraceCount = retraceCount;
  __osRestoreInt(saveMask);
}
