#ifndef BANJO_KAZOOIE_CORE1_AUDIOMANAGER_H
#define BANJO_KAZOOIE_CORE1_AUDIOMANAGER_H

#include <ultra64.h>

#define AUDIOMANAGER_THREAD_ID 4
#define AUDIOMANAGER_THREAD_PRI 50

void audioManager_setupSeqp(ALSeqpConfig *config);
void audioManager_init(void);
ALHeap *audioManager_getALHeapInfo(void);
OSMesgQueue *audioManager_getDMANotifyMesgQueue(void);
OSIoMesg *audioManager_getExtraDMAMesg(void);
OSMesgQueue *audioManager_getFrameMesgQueue(void);
OSMesgQueue *audioManager_getReplyMesgQueue(void); // [port]

#endif
