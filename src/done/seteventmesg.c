#ifndef LIGHTHOUSE_P

#include <os_internal.h>
#include "osint.h"
__OSEventState __osEventStateTab[OS_NUM_EVENTS];
void osSetEventMesg(OSEvent event, OSMesgQueue *mq, OSMesg msg)
{
	register u32 saveMask = __osDisableInt();
	__OSEventState *es;

	es = &__osEventStateTab[event];
	es->queue = mq;
	es->msg = msg;
	__osRestoreInt(saveMask);
}

#endif
