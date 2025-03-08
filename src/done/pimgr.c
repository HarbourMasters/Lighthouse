#include <ultra64.h>
#include "piint.h"

extern u32 __osPiAccessQueueEnabled;

OSMgrArgs __osPiDevMgr = {0};
OSPiHandle *__osPiTable = NULL;
OSPiHandle *__osCurrentHandle[2] = {&CartRomHandle, &LeoDiskHandle};
static OSThread piThread;
static char piThreadStack[OS_PIM_STACKSIZE];
static OSMesgQueue piEventQueue;
static OSMesg piEventBuf;

void osCreatePiManager(OSPri pri, OSMesgQueue *cmdQ, OSMesg *cmdBuf, s32 cmdMsgCnt)
{
	#ifndef LIGHTHOUSE_P
	u32 savedMask;
	OSPri oldPri;
	OSPri myPri;
	if (!__osPiDevMgr.initialized)
	{
		osCreateMesgQueue(cmdQ, cmdBuf, cmdMsgCnt);
		osCreateMesgQueue(&piEventQueue, (OSMesg*)&piEventBuf, 1);
		if (!__osPiAccessQueueEnabled)
			__osPiCreateAccessQueue();
		osSetEventMesg(OS_EVENT_PI, &piEventQueue, (OSMesg)0x22222222);
		oldPri = -1;
		myPri = osGetThreadPri(NULL);
		if (myPri < pri)
		{
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
		osCreateThread(&piThread, 0, __osDevMgrMain, &__osPiDevMgr, &piThreadStack[OS_PIM_STACKSIZE], pri);
		osStartThread(&piThread);
		__osRestoreInt(savedMask);
		if (oldPri != -1)
		{
			osSetThreadPri(NULL, oldPri);
		}
	}
	#endif
}
