#include "piint.h"
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "os_pi.h"
#endif

extern u32 __osPiAccessQueueEnabled;

extern OSMgrArgs __osPiDevMgr = {0};

OSPiHandle *__osPiTable = NULL;
OSPiHandle *__osCurrentHandle[2] = {&CartRomHandle, &LeoDiskHandle};

/* .bss */
OSThread piThread;
char piThreadStack[OS_PIM_STACKSIZE];
OSMesgQueue piEventQueue;
OSMesg piEventBuf;

void osCreatePiManager(OSPri pri, OSMesgQueue *cmdQ, OSMesg *cmdBuf,
                       s32 cmdMsgCnt) {
  u32 savedMask;
  OSPri oldPri;
  OSPri myPri;
  if (!__osPiDevMgr.initialized) {
    osCreateMesgQueue(cmdQ, cmdBuf, cmdMsgCnt);
    osCreateMesgQueue(&piEventQueue, (OSMesg *)&piEventBuf, 1);
    if (!__osPiAccessQueueEnabled)
      __osPiCreateAccessQueue();
    
    #ifndef LIGHTHOUSE_P
    osSetEventMesg(OS_EVENT_PI, &piEventQueue, (OSMesg)0x22222222);
    #else
    //pc thing here
    #endif
    oldPri = -1;
    myPri = osGetThreadPri(NULL);
    if (myPri < pri) {
      oldPri = myPri;
      osSetThreadPri(NULL, pri);
    }
    savedMask = __osDisableInt();
    __osPiDevMgr.initialized = 1;
    __osPiDevMgr.mgrThread = &piThread;
    __osPiDevMgr.cmdQueue = cmdQ;
    __osPiDevMgr.eventQueue = &piEventQueue;
    __osPiDevMgr.accessQueue = &__osPiAccessQueue;
    __osPiDevMgr.piDmaCallback = osPiRawStartDma;
    __osPiDevMgr.epiDmaCallback = osEPiRawStartDma;
    osCreateThread(&piThread, 0, __osDevMgrMain, &__osPiDevMgr,
                   &piThreadStack[OS_PIM_STACKSIZE], pri);
    osStartThread(&piThread);
    __osRestoreInt(savedMask);
    if (oldPri != -1) {
      osSetThreadPri(NULL, oldPri);
    }
  }
}
