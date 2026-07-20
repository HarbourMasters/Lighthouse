// BanjoDecomp: defragmanager.c
#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"

#define DEFRAG_THREAD_STACK_SIZE 0x800

void defragThread_entry(void *arg);

OSMesgQueue sDefragThreadResumeSyncQueue;
OSMesg      sDefragThreadResumeSyncMesg;
OSMesgQueue sDefragThreadPauseSyncQueue;
OSMesg      sDefragThreadPauseSyncMesg;
OSThread    sDefragThread;
u8          sDefragThreadStack[0x800];

/* .code */
void defragManager_init(void){
    osCreateMesgQueue(&sDefragThreadResumeSyncQueue, &sDefragThreadResumeSyncMesg, 1);
    osCreateMesgQueue(&sDefragThreadPauseSyncQueue, &sDefragThreadPauseSyncMesg, 1);
    osCreateThread(&sDefragThread, 2, defragThread_entry, NULL, sDefragThreadStack + DEFRAG_THREAD_STACK_SIZE, 10);
    osStartThread(&sDefragThread);
}

void defragManager_free(void){
    osStopThread(&sDefragThread);
    osDestroyThread(&sDefragThread);
}

void defragManager_resume(void){
    if(func_8023E000() == 3){
        osSendMesgPtr(&sDefragThreadResumeSyncQueue, NULL, OS_MESG_BLOCK);
    }
}

void defragManager_pause(void){
    if(func_8023E000() == 3){
        osSendMesgPtr(&sDefragThreadPauseSyncQueue, NULL, OS_MESG_BLOCK);
    }
}

void defragManager_setPriority(OSPri pri){
    if(func_8023E000() == 3){
        osSetThreadPri(&sDefragThread, pri);
    }
}

void defragThread_entry(void *arg) {
    int tmp_v0;
    do{
        osRecvMesg(&sDefragThreadResumeSyncQueue, NULL, OS_MESG_BLOCK);
        if(!sDefragThreadPauseSyncQueue.validCount){
            do{
                tmp_v0 = game_defrag();
            }while(!sDefragThreadPauseSyncQueue.validCount && tmp_v0);
        }
        osRecvMesg(&sDefragThreadPauseSyncQueue, NULL, OS_MESG_BLOCK);
    }while(1);
}
