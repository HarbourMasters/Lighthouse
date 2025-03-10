#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "viint.h"
#endif

void osViSwapBuffer(void *frameBufPtr) {
  u32 saveMask = __osDisableInt();
  __osViNext->framep = frameBufPtr;
  __osViNext->state |= VI_STATE_10;
  __osRestoreInt(saveMask);
}
