#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "controller.h"
#include "siint.h"
#include <os_internal.h>
#include <rcp.h>
#endif

static void __osPackReadData(void);
s32 osContStartReadData(OSMesgQueue *mq) {
  s32 ret;
  int i;

  ret = 0;
  __osSiGetAccess();
  if (__osContLastCmd != CONT_CMD_READ_BUTTON) {
    __osPackReadData();
    ret = __osSiRawStartDma(OS_WRITE, &__osContPifRam);
    osRecvMesg(mq, NULL, OS_MESG_BLOCK);
  }
  for (i = 0; i < ARRLEN(__osContPifRam.ram) + 1; i++) {
    __osContPifRam.ram[i] = 0xFF;
  }
  __osContPifRam.status = 0;
  ret = __osSiRawStartDma(OS_READ, &__osContPifRam);

  __osContLastCmd = CONT_CMD_READ_BUTTON;
  __osSiRelAccess();
  return ret;
}

void osContGetReadData(OSContPad *data) {
  u8 *ptr;
  __OSContReadHeader readformat;
  int i;
  ptr = (u8 *)&__osContPifRam.ram;
  for (i = 0; i < __osMaxControllers;
       i++, ptr += sizeof(__OSContReadHeader), data++) {
    readformat = *(__OSContReadHeader *)ptr;
    data->err_no = CHNL_ERR(readformat);
    if (data->err_no == 0) {
      data->button = readformat.button;
      data->stick_x = readformat.joyX;
      data->stick_y = readformat.joyY;
    }
  }
}

static void __osPackReadData(void) {
  u8 *ptr;
  __OSContReadHeader readformat;
  int i;

  ptr = (u8 *)&__osContPifRam.ram;
  for (i = 0; i < ARRLEN(__osContPifRam.ram) + 1; i++) {
    __osContPifRam.ram[i] = 0;
  }
  __osContPifRam.status = CONT_CMD_EXE;
  readformat.align = CONT_CMD_NOP;
  readformat.txsize = CONT_CMD_READ_BUTTON_TX;
  readformat.rxsize = CONT_CMD_READ_BUTTON_RX;
  readformat.poll = CONT_CMD_READ_BUTTON;
  readformat.button = -1;
  readformat.joyX = -1;
  readformat.joyY = -1;
  for (i = 0; i < __osMaxControllers; i++) {
    *(__OSContReadHeader *)ptr = readformat;
    ptr += sizeof(__OSContReadHeader);
  }
  *ptr = CONT_CMD_END;
}
