#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#include "bk_bool.h"
#else

#include "controller.h"
#include "siint.h"
#include <os_internal.h>
#include <rcp.h>
#endif

#define HALF_A_SECOND OS_USEC_TO_CYCLES(500000)

u32 __osContinitialized = 0;
OSPifRam __osContPifRam;
u8 __osContLastCmd;
u8 __osMaxControllers;
OSTimer __osEepromTimer;
OSMesgQueue __osEepromTimerQ;
OSMesg __osEepromTimerMsg;
s32 osContInit(OSMesgQueue *mq, u8 *bitpattern, OSContStatus *data) {
  OSMesg dummy;
  s32 ret;
  OSTime t;
  OSTimer mytimer;
  OSMesgQueue timerMesgQueue;

  ret = 0;
  if (__osContinitialized)
    return ret;
  __osContinitialized = TRUE;
  t = osGetTime();
  if (500000 * osClockRate / 1000000 > t) {
    osCreateMesgQueue(&timerMesgQueue, &dummy, 1);
    #ifndef LIGHTHOUSE_P
    osSetTimer(&mytimer, 500000 * osClockRate / 1000000 - t, 0, &timerMesgQueue,
               &dummy);
    #else
    osSetTimer(&mytimer, 500000 * osClockRate / 1000000 - t, 0, &timerMesgQueue,
               dummy);
    #endif
    osRecvMesg(&timerMesgQueue, &dummy, OS_MESG_BLOCK);
  }
  __osMaxControllers = MAXCONTROLLERS;
  __osPackRequestData(CONT_CMD_REQUEST_STATUS);

  ret = __osSiRawStartDma(OS_WRITE, &__osContPifRam);
  osRecvMesg(mq, &dummy, OS_MESG_BLOCK);

  ret = __osSiRawStartDma(OS_READ, &__osContPifRam);
  osRecvMesg(mq, &dummy, OS_MESG_BLOCK);
  __osContGetInitData(bitpattern, data);
  __osContLastCmd = CONT_CMD_REQUEST_STATUS;
  __osSiCreateAccessQueue();
  osCreateMesgQueue(&__osEepromTimerQ, &__osEepromTimerMsg, 1);
  return ret;
}
void __osContGetInitData(u8 *pattern, OSContStatus *data) {
  u8 *ptr;
  __OSContRequestHeader requestformat;
  int i;
  u8 bits;
  bits = 0;
  ptr = (u8 *)&__osContPifRam;
  for (i = 0; i < __osMaxControllers;
       i++, ptr += sizeof(__OSContRequestHeader), data++) {
    requestformat = *(__OSContRequestHeader *)ptr;
    data->err_no = CHNL_ERR(requestformat);
    if (data->err_no == 0) {
      data->type = (requestformat.typel << 8) | requestformat.typeh;
      data->status = requestformat.status;
      bits |= 1 << i;
    }
  }
  *pattern = bits;
}
void __osPackRequestData(u8 cmd) {
  u8 *ptr;
  __OSContRequestHeader requestformat;
  int i;
  for (i = 0; i < ARRLEN(__osContPifRam.ram) + 1; i++) {
    __osContPifRam.ram[i] = 0;
  }
  __osContPifRam.status = CONT_CMD_EXE;
  ptr = (u8 *)&__osContPifRam.ram;
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
  ptr[0] = CONT_CMD_END;
}
