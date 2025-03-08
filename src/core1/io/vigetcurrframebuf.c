#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "viint.h"
#include <ultra64.h>
#endif

void *osViGetCurrentFramebuffer(void) {
  register u32 saveMask;
  void *framep;
  saveMask = __osDisableInt();
  framep = __osViCurr->framep;
  __osRestoreInt(saveMask);
  return framep;
}
