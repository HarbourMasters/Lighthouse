#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#endif
extern OSMgrArgs __osPiDevMgr;

OSMesgQueue *osPiGetCmdQueue(void) {
  if (!__osPiDevMgr.initialized)
    return NULL;
  return __osPiDevMgr.cmdQueue;

}
