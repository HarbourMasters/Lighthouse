#include <ultra64.h>

#define SI_Q_BUF_LEN 1
u32 __osSiAccessQueueEnabled = 0;
OSMesg siAccessBuf[SI_Q_BUF_LEN];
OSMesgQueue __osSiAccessQueue;

void __osSiCreateAccessQueue(void) {
  __osSiAccessQueueEnabled = 1;
  osCreateMesgQueue(&__osSiAccessQueue, siAccessBuf, SI_Q_BUF_LEN);
  #ifndef LIGHTHOUSE_P
  osSendMesg(&__osSiAccessQueue, (OSMesg)NULL, OS_MESG_NOBLOCK);
  #else
  osSendMesg(&__osSiAccessQueue, OS_MESG_PTR(NULL), OS_MESG_NOBLOCK);
  #endif
  
}
void __osSiGetAccess(void) {
  OSMesg dummyMesg;
  if (!__osSiAccessQueueEnabled)
    __osSiCreateAccessQueue();
  osRecvMesg(&__osSiAccessQueue, &dummyMesg, OS_MESG_BLOCK);
}
void __osSiRelAccess(void) {
  #ifndef LIGHTHOUSE_P
  osSendMesg(&__osSiAccessQueue, (OSMesg)NULL, OS_MESG_NOBLOCK);
  #else
  osSendMesg(&__osSiAccessQueue, OS_MESG_PTR(NULL), OS_MESG_NOBLOCK);
  #endif
}
