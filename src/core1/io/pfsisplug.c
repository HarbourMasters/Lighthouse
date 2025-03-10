#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "controller.h"
#include <os_internal.h>
#endif

#include "siint.h"

OSPifRam __osPfsPifRam; // TODO bss

s32 osPfsIsPlug(OSMesgQueue *queue, u8 *pattern) {
  s32 ret;
  OSMesg dummy;
  u8 bitpattern;
  OSContStatus data[MAXCONTROLLERS];
  int channel;
  u8 bits;
  int crc_error_cnt;
  ret = 0;
  bits = 0;
  crc_error_cnt = 3;
  __osSiGetAccess();
  while (TRUE) {
    __osPfsRequestData(CONT_CMD_REQUEST_STATUS);
    ret = __osSiRawStartDma(OS_WRITE, &__osPfsPifRam);
    osRecvMesg(queue, &dummy, OS_MESG_BLOCK);
    ret = __osSiRawStartDma(OS_READ, &__osPfsPifRam);
    osRecvMesg(queue, &dummy, OS_MESG_BLOCK);
    __osPfsGetInitData(&bitpattern, data);
    for (channel = 0; channel < __osMaxControllers; channel++) {
      if ((data[channel].status & CONT_ADDR_CRC_ER) == 0) {
        crc_error_cnt--;
        break;
      }
    }
    if (__osMaxControllers == channel)
      crc_error_cnt = 0;
    if (crc_error_cnt < 1) {
      for (channel = 0; channel < __osMaxControllers; channel++) {
        if (data[channel].err_no == 0 &&
            (data[channel].status & CONT_CARD_ON) != 0)
          bits |= 1 << channel;
      }
      __osSiRelAccess();
      *pattern = bits;
      return ret;
    }
  }
}
void __osPfsRequestData(u8 cmd) {
  u8 *ptr;
  __OSContRequestHeader requestformat;
  int i;
  __osContLastCmd = cmd;

  for (i = 0; i < ARRLEN(__osPfsPifRam.ram) + 1;
       i++) { // also clear status
    __osPfsPifRam.ram[i] = 0;
  }

  __osPfsPifRam.status = CONT_CMD_EXE;
  ptr = (u8 *)&__osPfsPifRam;
  requestformat.align = CONT_CMD_NOP;
  requestformat.txsize = CONT_CMD_REQUEST_STATUS_TX;
  requestformat.rxsize = CONT_CMD_REQUEST_STATUS_RX;
  requestformat.poll = cmd;
  requestformat.typeh = CONT_CMD_NOP;
  requestformat.typel = CONT_CMD_NOP;
  requestformat.status = CONT_CMD_NOP;
  requestformat.align1 = CONT_CMD_NOP;
  for (i = 0; i < __osMaxControllers; i++) {
    *(__OSContRequestHeader *)ptr = requestformat;
    ptr += sizeof(__OSContRequestHeader);
  }
  *ptr = CONT_CMD_END;
}
void __osPfsGetInitData(u8 *pattern, OSContStatus *data) {
  u8 *ptr;
  __OSContRequestHeader requestformat;
  int i;
  u8 bits;
  bits = 0;
  ptr = (u8 *)&__osPfsPifRam;
  for (i = 0; i < __osMaxControllers;
       i++, ptr += sizeof(__OSContRequestHeader)) {
    requestformat = *(__OSContRequestHeader *)ptr;
    data->err_no = CHNL_ERR(requestformat);
    if (data->err_no == 0) {
      data->type = (requestformat.typel << 8) | (requestformat.typeh);
      data->status = requestformat.status;
      bits |= 1 << i;
    }
    data++;
  }
  *pattern = bits;
}
