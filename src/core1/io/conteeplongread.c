#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "controller.h"
#include <os_internal.h>
#endif

s32 osEepromLongRead(OSMesgQueue *mq, u8 address, u8 *buffer, int length) {
  s32 ret;
  ret = 0;
  while (length > 0) {
    ERRCK(osEepromRead(mq, address, buffer));
    length -= EEPROM_BLOCK_SIZE;
    address++;
    buffer += EEPROM_BLOCK_SIZE;
  }
  return ret;
}
