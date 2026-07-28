#ifndef PORT_OS_H
#define PORT_OS_H

// N64 OS SDK
//
// libultraship already implements most of the libultra surface, and where its
// version is correct for a PC port we use it as-is. This layer exists only for
// the calls libultraship cannot serve, so that the decomp's own threading code
// can run instead of being replaced by port stand-ins.

#ifdef __cplusplus
extern "C" {
#endif

#include "libultraship/libultra/thread.h"

void OS_CreateThread(OSThread* thread, OSId id, void* entry, void* arg, void* sp, OSPri p);
void OS_StartThread(OSThread* thread);
void OS_StopThread(OSThread* thread);
void OS_DestroyThread(OSThread* thread);
void OS_SetThreadPri(OSThread* thread, OSPri p);

// Allowlist one decomp thread entry point. Until its entry is passed here, an
// osCreateThread/osStartThread pair is recorded but never launched, so threads
// are revived deliberately, one consumer at a time.
void OS_EnableThreadEntry(void* entry);

#include "libultraship/libultra/message.h"

// OS_MESG_BLOCK only blocks on queues opted in here; the rest stay as LUS had
// them, non-blocking.
void OS_SetQueueBlocking(OSMesgQueue* mq, int enabled);

#ifdef __cplusplus
}
#endif

#endif // PORT_OS_H
