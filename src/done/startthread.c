#include <ultra64.h>
#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "osint.h"
#endif

void osStartThread(OSThread *t)
{
    register u32 saveMask = __osDisableInt();
    switch (t->state)
    {
    case OS_STATE_WAITING:
        t->state = OS_STATE_RUNNABLE;
        __osEnqueueThread(&__osRunQueue, t);
        break;
    case OS_STATE_STOPPED:
        if (t->queue == NULL || t->queue == &__osRunQueue)
        {
            t->state = OS_STATE_RUNNABLE;
            __osEnqueueThread(&__osRunQueue, t);
        }
        else
        {
            t->state = OS_STATE_WAITING;
            __osEnqueueThread(t->queue, t);
            __osEnqueueThread(&__osRunQueue, __osPopThread(t->queue));
        }
        break;
    }
    if (__osRunningThread == NULL)
    {
        __osDispatchThread();
    }
    else
    {
        if (__osRunningThread->priority < __osRunQueue->priority)
        {
            __osRunningThread->state = OS_STATE_RUNNABLE;
            __osEnqueueAndYield(&__osRunQueue);
        }
    }
    __osRestoreInt(saveMask);
}
