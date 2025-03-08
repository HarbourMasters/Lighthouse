#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#define DEFRAG_THREAD_STACK_SIZE 0x800

void defragThread_entry(void *arg);

OSMesgQueue sDefragMessageQueue;
OSMesg sDefragMessageBuffer;
OSMesgQueue sSecondaryDefragMessageQueue;
OSMesg sSecondaryDefragMessageBuffer;
OSThread sDefragThread;
u8 sDefragThreadStack[0x800];

/* .code */
void defragManager_init(void) {
  osCreateMesgQueue(&sDefragMessageQueue, &sDefragMessageBuffer, 1);
  osCreateMesgQueue(&sSecondaryDefragMessageQueue, &sSecondaryDefragMessageBuffer, 1);
  osCreateThread(&sDefragThread, 2, defragThread_entry, NULL,
                 sDefragThreadStack + DEFRAG_THREAD_STACK_SIZE, 10);
  osStartThread(&sDefragThread);
}

void defragManager_free(void) {
  osStopThread(&sDefragThread);
  osDestroyThread(&sDefragThread);
}

void defragManager_sendMessage(void) {
  #ifndef LIGHTHOUSE_P
  if (system_getCurrentMode() == 3) {
    osSendMesg(&sDefragMessageQueue, NULL, OS_MESG_BLOCK);
  }
  #else
  //pcstuff here
  #endif
}

void defragManager_sendSecondaryMessage(void) {
  #ifndef LIGHTHOUSE_P
  if (system_getCurrentMode() == 3) {
    osSendMesg(&sSecondaryDefragMessageQueue, NULL, OS_MESG_BLOCK);
  }
  #else
  //pcstuff here
  #endif
}

void defragManager_setPriority(OSPri pri) {
  if (system_getCurrentMode() == 3) {
    osSetThreadPri(&sDefragThread, pri);
  }
}

void defragThread_entry(void *arg) {
  #ifndef LIGHTHOUSE_P
  int tmp_v0;
  do {
    osRecvMesg(&sDefragMessageQueue, NULL, OS_MESG_BLOCK);
    if (!sSecondaryDefragMessageQueue.validCount) {
      do {
        tmp_v0 = game_defrag();
      } while (!sSecondaryDefragMessageQueue.validCount && tmp_v0);
    }
    osRecvMesg(&sSecondaryDefragMessageQueue, NULL, OS_MESG_BLOCK);
  } while (1);
  #else
  //pcstuff here
  #endif
}
