#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include "version.h"

#include "core1/vimgr.h"

#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#endif

#include <ultra64.h>

#define VIMANAGER_THREAD_STACK_SIZE 0x400

// Used in US 1.0 NTSC
static OSViMode sViMode_US10_NTSC = {OS_VI_NTSC_LPN1,
                                     {
                                         VI_CTRL_TYPE_16 |
                                             VI_CTRL_GAMMA_DITHER_ON |
                                             VI_CTRL_GAMMA_ON | 0x3200, /*ctrl*/
                                         292,       /* width*/
                                         0x3E52239, /* burst*/
                                         0x20D,     /* vSync*/
                                         0xC15,     /* hSync*/
                                         0xC150C15, /* leap */
                                         0x8C02D5,  /* hstart */
                                         0x200,     /* xScale */
                                         0x0,       /* vCurrent */
                                     },
                                     {
                                         {0x248, 0x400, 0x3D01E8, 0xE0204, 2},
                                         {0x248, 0x400, 0x3D01E8, 0xE0204, 2},
                                     }};

// Used in US 1.0 MPAL (N64 SDK says: "mainly brazil")
static OSViMode sViMode_US10_MPAL = {OS_VI_MPAL_LPN1,
                                     {
                                         VI_CTRL_TYPE_16 |
                                             VI_CTRL_GAMMA_DITHER_ON |
                                             VI_CTRL_GAMMA_ON | 0x3200, /*ctrl*/
                                         292,       /* width*/
                                         0x4651E39, /* burst*/
                                         0x20D,     /* vSync*/
                                         0x40C11,   /* hSync*/
                                         0xC190C1A, /* leap */
                                         0x8C02D5,  /* hstart */
                                         0x200,     /* xScale */
                                         0x0,       /* vCurrent */
                                     },
                                     {
                                         {0x248, 0x400, 0x3D01E8, 0xE0204, 2},
                                         {0x248, 0x400, 0x3D01E8, 0xE0204, 2},
                                     }};

#if VERSION == VERSION_PAL
// Used in PAL
static OSViMode sViMode_PAL = {OS_VI_PAL_LPN1,
                               {
                                   VI_CTRL_TYPE_16 | VI_CTRL_GAMMA_DITHER_ON |
                                       VI_CTRL_GAMMA_ON | VI_CTRL_DIVOT_ON |
                                       0x3100, /*ctrl*/
                                   292,        /* width*/
                                   0x404233A,  /* burst*/
                                   0x271,      /* vSync*/
                                   0x150C69,   /* hSync*/
                                   0xC6F0C6E,  /* leap */
                                   0x9802E1,   /* hstart */
                                   0x200,      /* xScale */
                                   0x0,        /* vCurrent */
                               },
                               {
                                   {0x248, 0x350, 0x48024C, 0x9026B, 2},
                                   {0x248, 0x350, 0x48024C, 0x9026B, 2},
                               }};
#endif

// 42200000 3FAD097B 41F00000 457A0000
// C3A68832 DDC3A724 00000000 00000000

static u32 sActiveFramebuffer;
static u32 sFrameCount;
static u32 sFrameCounter;

#ifndef LIGHTHOUSE_P
typedef struct {
  /* 0x00 */ OSMesgQueue* queue;
  /* 0x04 */ OSMesg msg;
} __OSEventState;
#endif

static __OSEventState sMessageQueueArray[8];
static OSMesgQueue sMesgQueue1;
static OSMesg sMesgBuffer1[10];
static OSMesgQueue sMesgQueue2;
static OSMesg sMesgBuffer2[1];
static OSMesgQueue sMesgQueue3;
static OSMesg sMesgBuffer3[FRAMERATE];
volatile s32 vFrameCounterLimit;
static s32 sFrameLimit;
static OSThread sViManagerThread;
static u8 sViManagerThreadStack[VIMANAGER_THREAD_STACK_SIZE];

u32 getOtherFramebuffer(void) { return NOT(sActiveFramebuffer); }

s32 viMgr_getFrameCount(void) { return sFrameCount; }

s32 getActiveFramebuffer(void) { return sActiveFramebuffer; }

void viMgr_addMessageQueue(OSMesgQueue *mq, OSMesg msg) {
  s32 i;
  for (i = 0; i < 8; i++) {
    if (sMessageQueueArray[i].queue == NULL) {
      sMessageQueueArray[i].queue = mq;
      sMessageQueueArray[i].msg = msg;
      return;
    }
  }
}

void viMgr_init(void) {
  s32 i;

  viMgr_clearFramebuffers();

  osCreateViManager(OS_PRIORITY_VIMGR);
#if VERSION == VERSION_USA_1_0
  if (osTvType != OS_TV_NTSC)
    osViSetMode(&sViMode_US10_MPAL);
  else
    osViSetMode(&sViMode_US10_NTSC);
#elif VERSION == VERSION_PAL
  osViSetMode(&sViMode_PAL);
#endif

  osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON);
  osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
  osViSwapBuffer(&gFramebuffers[0]);

  osCreateMesgQueue(&sMesgQueue1, sMesgBuffer1, 10);
  osCreateMesgQueue(&sMesgQueue2, sMesgBuffer2, 1);
  osCreateMesgQueue(&sMesgQueue3, sMesgBuffer3, FRAMERATE);
  osViSetEvent(&sMesgQueue1, (OSMesg)NULL, 1);

  sActiveFramebuffer = 0;
  sFrameCount = 1;
  sFrameCounter = 0;

  for (i = 0; i < 8; i++) {
    sMessageQueueArray[i].queue = NULL;
  }

  vFrameCounterLimit = 0;
  viMgr_setFrameLimit(2);

  osCreateThread(&sViManagerThread, 0, viMgr_entry, NULL,
                 sViManagerThreadStack + VIMANAGER_THREAD_STACK_SIZE, 80);
  osStartThread(&sViManagerThread);
}

void viMgr_setFrameLimit(s32 arg0) { sFrameLimit = arg0; }

s32 viMgr_getFrameLimit(void) { return sFrameLimit; }

void viMgr_sendMessage(void) { osSendMesg(&sMesgQueue2, (OSMesg)NULL, OS_MESG_NOBLOCK); }

void viMgr_waitForFrame(s32 arg0) {
  static s32 vimgr_frameSkipCounter;

  osSetThreadPri(NULL, 0x7f);
  defragManager_setPriority(0x1E);
  defragManager_sendMessage();
  if (arg0) {
    osRecvMesg(&sMesgQueue2, NULL, OS_MESG_BLOCK);
  }

  while (vFrameCounterLimit < viMgr_getFrameLimit() - vimgr_frameSkipCounter) {
    osRecvMesg(&sMesgQueue3, NULL, OS_MESG_BLOCK);
  }

  while (sMesgQueue3.validCount) {
    osRecvMesg(&sMesgQueue3, NULL, OS_MESG_NOBLOCK);
  }

  osViSwapBuffer(gFramebuffers[sActiveFramebuffer = getOtherFramebuffer()]);
  vimgr_frameSkipCounter = 0;
  while (!(osDpGetStatus() & 2) &&
         osViGetCurrentFramebuffer() != osViGetNextFramebuffer()) {
    osRecvMesg(&sMesgQueue3, NULL, OS_MESG_BLOCK);
    vimgr_frameSkipCounter++;
  } // L8024C178
  sFrameCount = vFrameCounterLimit;
  vFrameCounterLimit = 0;
  defragManager_sendSecondaryMessage();
  osSetThreadPri(NULL, 0x14);
  defragManager_setPriority(0xA);
}

void viMgr_processFrame(void) {
  viMgr_waitForFrame(0);
  dummy_func_8025AFB8();
}

void viMgr_waitForFrameAndProcess(void) { viMgr_waitForFrame(1); }

//this isn't used anywhere as far as i know, so ill just ignore it entirely for now
void viMgr_removeMessageQueue(OSMesgQueue *mq, OSMesg msg) {
  s32 i;

  #ifndef LIGHTHOUSE_P
  for (i = 0; i < 8; i++) {
    if (sMessageQueueArray[i].queue == mq && sMessageQueueArray[i].msg == msg) {
      sMessageQueueArray[i].queue = NULL;
      return;
    }
  }
  #endif
}

void viMgr_setActiveFramebuffer(s32 fb_idx) {
  sActiveFramebuffer = fb_idx;
  osViSwapBuffer(gFramebuffers[sActiveFramebuffer]);
}

void viMgr_entry(void *arg0) {
  s32 i;
  OSMesg sp48;
  do {
    osRecvMesg(&sMesgQueue1, &sp48, OS_MESG_BLOCK);
    checkAndSetViMode();
    vFrameCounterLimit++;
    if (vFrameCounterLimit == 420) {
#if VERSION == VERSION_USA_1_0
      isHalted();
#endif
    }
    osSendMesg(&sMesgQueue3, (OSMesg)NULL, OS_MESG_NOBLOCK);

    for (i = 0; i < 8; i++) {
      if (sMessageQueueArray[i].queue != NULL) {
        osSendMesg(sMessageQueueArray[i].queue, sMessageQueueArray[i].msg,
                   OS_MESG_NOBLOCK);
      }
    }
  } while (1);
}

void viMgr_setScreenBlack(s32 active) { osViBlack(active); }

void viMgr_clearFramebuffers(void) {
  func_80253034(&gFramebuffers, 0,
                (s32)((f32)gFramebufferWidth * 2 * gFramebufferHeight *
                      2)); // this is defined in code_155f0.s
  osWritebackDCache(&gFramebuffers,
                    (s32)((f32)gFramebufferWidth * 2 * gFramebufferHeight * 2));
}

s32 viMgr_getFrameCounter(void) { return vFrameCounterLimit; }

void viMgr_setFrameCounter(s32 arg0) { vFrameCounterLimit = arg0; }
