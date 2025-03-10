#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "osint.h"
#endif

OSPri osGetThreadPri(OSThread *thread)
{
    if (thread == NULL)
        thread = __osRunningThread;
    return thread->priority;
}
