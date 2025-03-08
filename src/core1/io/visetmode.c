#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "viint.h"
#include <os_internal.h>
#endif

void osViSetMode(OSViMode *modep) {
  register u32 saveMask;
  saveMask = __osDisableInt();
  __osViNext->modep = modep;
  __osViNext->state = VI_STATE_01;
  __osViNext->control = __osViNext->modep->comRegs.ctrl;
  __osRestoreInt(saveMask);
}
