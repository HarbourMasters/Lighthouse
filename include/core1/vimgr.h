#ifndef BANJO_KAZOOIE_CORE1_VIMGR_H
#define BANJO_KAZOOIE_CORE1_VIMGR_H

#ifdef __cplusplus
extern "C" {
#endif

/* typedef struct struct_1_s{
    OSMesgQueue *queue;
    OSMesg message;
} struct1; */

u32 getOtherFramebuffer(void);
s32 viMgr_getFrameCount(void);
s32 getActiveFramebuffer(void);
void viMgr_addMessageQueue(OSMesgQueue *mq, OSMesg msg);
void viMgr_init(void);
void viMgr_setFrameLimit(s32 arg0);
s32 viMgr_getFrameLimit(void);
void viMgr_sendMessage(void);
void viMgr_waitForFrame(s32 arg0);
void viMgr_processFrame(void);
void viMgr_waitForFrameAndProcess(void);
void viMgr_removeMessageQueue(OSMesgQueue *mq, OSMesg msg);
void viMgr_setActiveFramebuffer(s32 arg0);
void viMgr_entry(void *arg0);
void viMgr_setScreenBlack(s32 active);
void viMgr_clearFramebuffers(void);
s32 viMgr_getFrameCounter(void);
void viMgr_setFrameCounter(s32 arg0);

#ifdef __cplusplus
}
#endif


#endif
