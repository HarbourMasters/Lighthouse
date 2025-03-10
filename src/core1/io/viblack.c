#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else

#include "functions.h"
#include "variables.h"
#include "viint.h"
#include <ultra64.h>
#endif

void osViBlack(u8 active) {
  register u32 saveMask = __osDisableInt();
  if (active)
    __osViNext->state |= VI_STATE_BLACK;
  else
    __osViNext->state &= ~VI_STATE_BLACK;
  __osRestoreInt(saveMask);
}
