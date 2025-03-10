#include "piint.h"

#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include <os_internal.h>
#endif

s32 osPiReadIo(u32 devAddr, u32 *data) {
  register s32 ret;
  __osPiGetAccess();
  ret = osPiRawReadIo(devAddr, data);
  __osPiRelAccess();
  return ret;
}
