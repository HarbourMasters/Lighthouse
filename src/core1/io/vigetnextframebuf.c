#include "functions.h"
#include "variables.h"

#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "viint.h"
#include <ultra64.h>
#endif

void *osViGetNextFramebuffer(void) {
  register u32 saveMask;
  void *framep;
  saveMask = __osDisableInt();
  framep = __osViNext->framep;
  __osRestoreInt(saveMask);
  return framep;
}
