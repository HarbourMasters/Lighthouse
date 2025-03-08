#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include "version.h"
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "compat.h"
#else
extern u8 n_aspMainTextStart[];
extern u8 gSPF3DEX_fifoTextStart[];
extern u8 gSPL3DEX_fifoTextStart[];

extern u8 n_aspMainDataStart[];
extern u8 gSPF3DEX_fifoDataStart[];
extern u8 gSPL3DEX_fifoDataStart[];
#endif

//taskmanager, messagemgr, task_message.c

typedef struct {
  s32 unk0;
  s32 unk4;
  s32 unk8;
  s32 unkC;
} TaskQueueElement;

void processTaskQueue(void);

#define CORE1_8C50_EVENT_DP 4
#define CORE1_8C50_EVENT_SP 6
#define CORE1_8C50_EVENT_AUDIO_TIMER 8
#define CORE1_8C50_EVENT_FAULT 10
#define CORE1_8C50_EVENT_PRENMI 11
#define CORE1_8C50_EVENT_CONT_TIMER 13

/* .extern */


/* .data */
OSTask audioTask = {
    /* type */ M_AUDTASK,
    /* flags */ 0,
    NULL,
    0, /* ucode_boot */
    NULL,
    SP_UCODE_SIZE, /* ucode */
    NULL,
    SP_UCODE_DATA_SIZE, /* ucode_data */
    NULL,
    0, /* dram_stack */
    NULL,
    NULL, /* output_buff */
    NULL,
    0, /* data */
    NULL,
    0, /* yield_data */
};

OSTask gfxTask = {
    /* type */ M_GFXTASK,
    /* flags */ 0,
    NULL,
    0, /* ucode_boot */
    NULL,
    SP_UCODE_SIZE, /* ucode */
    NULL,
    SP_UCODE_DATA_SIZE, /* ucode_data */
    0x80000400,
    0x400, /* dram_stack */
    0x80000800,
    0x8000E800, /* output_buff */
    NULL,
    0, /* data */
    NULL,
    OS_YIELD_DATA_SIZE, /* yield_data */
};

s32 taskCounter = 0;
s32 taskDelay = 0;
s32 taskTimeout = 0;
s32 taskYieldCounter = 0;

/* .bss */
u64 yieldData[OS_YIELD_DATA_SIZE / sizeof(u64)];
static u8 pad[0x20]; // 8027FB40
OSMesgQueue mainQueue;
OSMesg mainQueueMessages[20];
OSMesgQueue secondaryQueue;
OSMesg secondaryQueueMessages[10];
TaskQueueElement *currentTask;
s32 taskOverflowCounter;
bool isTaskRunning;
s32 taskStatus;
s32 taskNextStatus;
s32 taskType;
s32 taskPreviousType;
s32 taskYielded;
u8 resetThreadStack[2048]; // stack for thread resetThread;
OSThread resetThread;
TaskQueueElement *primaryTaskQueue[20];
volatile s32 primaryTaskQueueHead;
volatile s32 primaryTaskQueueTail;
TaskQueueElement *secondaryTaskQueue[20];
volatile s32 secondaryTaskQueueHead;
volatile s32 secondaryTaskQueueTail;
void *currentFramebuffer;
OSTimer audioTimer;      // audio_timer
OSTimer controllerTimer; // controller_timer
s32 controllerTimerEnabled;

/* .code */
void sendMesgToMainQueue(OSMesg arg0) {
  #ifndef LIGHTHOUSE_P
  static s32 mainQueueFlag = 1;

  osSendMesg(&mainQueue, arg0, 1);
  if ((s32)arg0 == 3) {
    taskDelay = 0x1e;
    if (mainQueueFlag) {
      osDpSetStatus(DPC_CLR_FREEZE);
      mainQueueFlag = 0;
    }
    osRecvMesg(&secondaryQueue, NULL, 1);
    taskDelay = 0;
  }
  #else
  //pc thing here
  #endif
}

void enqueueToSecondaryQueue(OSMesg arg0) {
  #ifndef LIGHTHOUSE_P
  s32 tmp = (secondaryTaskQueueHead + 1) % 0x14;
  if (secondaryTaskQueueTail != tmp) {
    secondaryTaskQueue[secondaryTaskQueueHead] = arg0;
    secondaryTaskQueueHead = tmp;
  }
  #else
  //pc thing here
  #endif
}

void enqueueToPrimaryQueue(OSMesg arg0) {
  #ifndef LIGHTHOUSE_P
  s32 tmp = (primaryTaskQueueHead + 1) % 0x14;
  if (primaryTaskQueueTail != tmp) {
    primaryTaskQueue[primaryTaskQueueHead] = arg0;
    primaryTaskQueueHead = tmp;
  }
  #else
  //pc thing here
  #endif
}

void startAudioTask(TaskQueueElement *arg0) {
  ucode_getPtrAndSize(&audioTask.t.ucode_boot, &audioTask.t.ucode_boot_size);
  audioTask.t.ucode = n_aspMainTextStart;
  audioTask.t.ucode_data = n_aspMainDataStart;
  audioTask.t.data_ptr = (void *)arg0->unk8;
  audioTask.t.data_size = (arg0->unkC - arg0->unk8) >> 3 << 3;
  osWritebackDCache(audioTask.t.data_ptr, audioTask.t.data_size);
  osWritebackDCache(&audioTask, sizeof(OSTask));
  currentTask = arg0;
  osSpTaskLoad(&audioTask);
  osSpTaskStartGo(&audioTask);
  taskType = 4;
}

void startGfxTask(TaskQueueElement *arg0) {
  ucode_getPtrAndSize(&gfxTask.t.ucode_boot, &gfxTask.t.ucode_boot_size);
  gfxTask.t.ucode = gSPF3DEX_fifoTextStart;
  gfxTask.t.ucode_data = gSPF3DEX_fifoDataStart;
  gfxTask.t.data_ptr = (void *)arg0->unk8;
  gfxTask.t.data_size = (arg0->unkC - arg0->unk8) >> 3 << 3;
  osWritebackDCache(gfxTask.t.data_ptr, gfxTask.t.data_size);
  osWritebackDCache(&gfxTask, sizeof(OSTask));
  osSpTaskLoad(&gfxTask);
  osSpTaskStartGo(&gfxTask);
  taskType = arg0->unk4 | 0x8;
  taskNextStatus = arg0->unk4 | 0x1;
  if (!(osDpGetStatus() & DPC_STATUS_FREEZE)) {
    taskStatus = taskNextStatus;
    taskTimeout = 0x1e;
  }
}

void startGfxTaskL3DEX(TaskQueueElement *arg0) {
  ucode_getPtrAndSize(&gfxTask.t.ucode_boot, &gfxTask.t.ucode_boot_size);
  gfxTask.t.ucode = gSPL3DEX_fifoTextStart;
  gfxTask.t.ucode_data = gSPL3DEX_fifoDataStart;
  gfxTask.t.data_ptr = (void *)arg0->unk8;
  gfxTask.t.data_size = (arg0->unkC - arg0->unk8) >> 3 << 3;
  osWritebackDCache(gfxTask.t.data_ptr, gfxTask.t.data_size);
  osWritebackDCache(&gfxTask, sizeof(OSTask));
  osSpTaskLoad(&gfxTask);
  osSpTaskStartGo(&gfxTask);
  taskType = arg0->unk4 | 0x8;
  taskNextStatus = arg0->unk4 | 0x1;
  if (!(osDpGetStatus() & DPC_STATUS_FREEZE)) {
    taskStatus = taskNextStatus;
    taskTimeout = 0x1e;
  }
}

void startTask(TaskQueueElement *arg0) {
  switch (arg0->unk0) {
  case 1:
    startGfxTask(arg0);
    break;

  case 2:
    startGfxTaskL3DEX(arg0);
    break;
  }
}

void handleMesgType0(OSMesg msg) { enqueueToSecondaryQueue(msg); }

void handleMesgType1(OSMesg msg) {
  enqueueToPrimaryQueue(msg);
  if (taskType == 0x10 && !isTaskRunning) {
    startGfxTask(primaryTaskQueue[primaryTaskQueueTail]);
    primaryTaskQueueTail = (primaryTaskQueueTail + 1) % 0x14;
  }
}

void handleMesgType2(OSMesg msg) {
  enqueueToPrimaryQueue(msg);
  if (taskType == 0x10 && !isTaskRunning) {
    startGfxTaskL3DEX(primaryTaskQueue[primaryTaskQueueTail]);
    primaryTaskQueueTail = (primaryTaskQueueTail + 1) % 0x14;
  }
}

void checkAndSendMesg(void) {
  #ifndef LIGHTHOUSE_P
  if (taskType == 0x10 && taskStatus == 2 &&
      primaryTaskQueueTail == primaryTaskQueueHead &&
      !(osDpGetStatus() & DPC_STATUS_FREEZE)) {
    osSendMesg(&secondaryQueue, NULL, OS_MESG_NOBLOCK);
  } else {
    taskOverflowCounter++;
  }
  #else
  //pc thing here
  #endif 
}

void handleDpStatus(void) {
  #ifndef LIGHTHOUSE_P
  if ((taskStatus << 1) < 0) {
    osDpSetStatus(DPC_SET_FREEZE);
    currentFramebuffer = osViGetCurrentFramebuffer();
    viMgr_sendMessage();
  }
  taskStatus = taskNextStatus = 2;
  taskTimeout = 0;
  if (taskType == 0x10 && primaryTaskQueueTail != primaryTaskQueueHead &&
      !isTaskRunning) {
    startTask(primaryTaskQueue[primaryTaskQueueTail]);
    primaryTaskQueueTail = (primaryTaskQueueTail + 1) % 0x14;
  } else {
    if (taskOverflowCounter && primaryTaskQueueTail == primaryTaskQueueHead &&
        !(osDpGetStatus() & DPC_STATUS_FREEZE)) {
      osSendMesg(&secondaryQueue, NULL, 0);
      taskOverflowCounter--;
    }
  }
  #else
  //pc thing here
  #endif
}

void updateTimers(void) {
  #ifndef LIGHTHOUSE_P
  static s32 timerCounter = 0;
  s32 sp2C = (taskOverflowCounter != 0) &&
             (primaryTaskQueueTail == primaryTaskQueueHead) &&
             (taskNextStatus == 2) && (taskType == 0x10);
  volatile s32 sp30;

  sp30 = FALSE;
  if (osViGetCurrentFramebuffer() != currentFramebuffer || sp2C) {
    if (osDpGetStatus() & DPC_STATUS_FREEZE) {
      osDpSetStatus(DPC_CLR_FREEZE);

      taskStatus = taskNextStatus;
      dummy_func_8025AFB8();

      if (taskStatus & 1) {
        taskTimeout = 0x1E;
      }
    }

    if (sp2C) {
      osSendMesg(&secondaryQueue, NULL, OS_MESG_NOBLOCK);
      taskOverflowCounter--;
    }
  }

  taskCounter = 0;

  if (taskDelay != 0) {
    taskDelay--;
  }

  if (taskYieldCounter != 0) {
    taskYieldCounter--;
  }

  if (taskTimeout != 0) {
    taskTimeout--;
    if (taskTimeout == 0) {
      sp30 = TRUE;
    }
  }
  isTaskRunning = 0;
  timerCounter++;
  if (!(timerCounter & 1)) {
    osStopTimer(&audioTimer);
    osSetTimer(&audioTimer, 280000, 0, &mainQueue,
               CORE1_8C50_EVENT_AUDIO_TIMER);
  }

  if (controllerTimerEnabled) {
    osStopTimer(&controllerTimer);
#if VERSION == VERSION_USA_1_0
    osSetTimer(&controllerTimer, ((osClockRate / 60) * 2) / 3, 0, &mainQueue,
               CORE1_8C50_EVENT_CONT_TIMER);
#elif VERSION == VERSION_PAL
    osSetTimer(&controllerTimer, ((osClockRate / 60.0) * 2) / 3, 0, &mainQueue,
               CORE1_8C50_EVENT_CONT_TIMER);
#endif
  }
  #else
  //pc thing here
  #endif
}

void handleSpEvent(void) {
  #ifndef LIGHTHOUSE_P
  TaskQueueElement *sp1C;
  s32 temp_v1;
  TaskQueueElement *temp_v0;

  temp_v1 = taskType;
  if (taskType == 0x20) {
    sp1C = secondaryTaskQueue[secondaryTaskQueueTail];
    secondaryTaskQueueTail = (secondaryTaskQueueTail + 1) % 20;
    taskYielded = (osSpTaskYielded(&gfxTask) == 1);
    startAudioTask(sp1C);
    taskYieldCounter = 0;
    return;
  }

  if (taskType == 4) {
    osSendMesg(currentTask[1].unk0, currentTask[1].unk4, 0);
  }

  if ((taskType == 4) && (taskYielded != 0)) {
    osSpTaskLoad(&gfxTask);
    osSpTaskStartGo(&gfxTask);
    taskType = taskPreviousType;
    taskYielded = 0;
    return;
  }

  taskType = 0x10;
  if ((primaryTaskQueueTail != primaryTaskQueueHead) && (isTaskRunning == 0)) {
    startTask(primaryTaskQueue[primaryTaskQueueTail]);
    primaryTaskQueueTail = (primaryTaskQueueTail + 1) % 20;
    return;
  }

  if ((taskOverflowCounter != 0) && (taskStatus == 2) &&
      !(osDpGetStatus() & 2)) {
    osSendMesg(&secondaryQueue, NULL, 0);
    taskOverflowCounter -= 1;
  }
  #else
  //pc thing here
  #endif
}
//clang-format off
void setFlagTrue(OSMesg arg0){
    isTaskRunning = TRUE;
}
//clang-format on

void sendMesgToAudioManager(void) {
  #ifndef LIGHTHOUSE_P
  osSendMesg(audioManager_getFrameMesgQueue(), NULL, OS_MESG_NOBLOCK);
  processTaskQueue();
  #else
  //pc thing here
  #endif
}

void processTaskQueue(void) {
  TaskQueueElement *ptr;
  if ((taskType == 0x10) &&
      (secondaryTaskQueueTail != secondaryTaskQueueHead)) {
    ptr = secondaryTaskQueue[secondaryTaskQueueTail];
    secondaryTaskQueueTail = (secondaryTaskQueueTail + 1) % 0x14;
    startAudioTask(ptr);
  } else if ((taskType & 0x8) &&
             (secondaryTaskQueueTail != secondaryTaskQueueHead)) {
    osSpTaskYield();
    taskPreviousType = taskType;
    taskType = 0x20;
    taskYieldCounter = 0x1E;
  }
}

void noopFunction(void) {}

void setViMode(void) {
  #ifndef LIGHTHOUSE_P
  static OSViMode viModeNTSC = {
      OS_VI_MPAL_LPN1, /* type */
      {
          VI_CTRL_TYPE_16 | VI_CTRL_GAMMA_DITHER_ON | VI_CTRL_GAMMA_ON |
              0x3200, /*ctrl*/
          320,        /*width*/
          0x4651E39,  /*burst*/
          0x20D,      /*vSync*/
          0x40C11,    /* hSync*/
          0xC190C1A,  /* leap*/
          0x6C02EC,   /* hStart*/
          0,          /* xScale*/
          0,          /* vCurrent*/
      },
      {{640, 1024, 0x2501FF, 0xE0204, 2}, {640, 1024, 0x2501FF, 0xE0204, 2}}};
  static OSViMode viModeMPAL = {
      OS_VI_NTSC_LPN1, /* type */
      {
          VI_CTRL_TYPE_16 | VI_CTRL_GAMMA_DITHER_ON | VI_CTRL_GAMMA_ON |
              0x3200, /*ctrl*/
          320,        /*width*/
          0x3E52239,  /*burst*/
          0x20D,      /*vSync*/
          0xC15,      /* hSync*/
          0xC150C15,  /* leap*/
          0x6C02EC,   /* hStart*/
          0,          /* xScale*/
          0,          /* vCurrent*/
      },
      {{0x280, 1024, 0x2501FF, 0xE0204, 2}, {640, 1024, 0x2501FF, 0xE0204, 2}}};
#if VERSION == VERSION_PAL
  static OSViMode viModePAL = {
      OS_VI_PAL_LPN1, /* type */
      {
          VI_CTRL_TYPE_16 | VI_CTRL_GAMMA_DITHER_ON | VI_CTRL_GAMMA_ON |
              0x3200, /*ctrl*/
          320,        /*width*/
          0x404233A,  /*burst*/
          0x271,      /*vSync*/
          0x150C69,   /* hSync*/
          0xC6F0C6E,  /* leap*/
          0x800300,   /* hStart*/
          0,          /* xScale*/
          0,          /* vCurrent*/
      },
      {{640, 1024, 0x5F0239, 0x9026B, 2}, {640, 1024, 0x5F0239, 0x9026B, 2}}};
#endif
  static s32 viModeInitialized;

  if (!viModeInitialized) {
    viModeInitialized = TRUE;
#if VERSION == VERSION_USA_1_0
    if (osTvType != OS_TV_NTSC) {
      osViSetMode(&viModeNTSC);
    } else {
      osViSetMode(&viModeMPAL);
    }
#elif VERSION == VERSION_PAL
    // if(&viModeNTSC){}
    osViSetMode(&viModePAL);
#endif
    baMotor_resetMotor(); // stop controller motors
    do {
      osDpSetStatus(DPC_STATUS_FLUSH);
    } while (1);
  }
  #else
  //pc thing here
  #endif
}

void checkAndSetViMode(void) {
  if (!(___osGetSR() & SR_IBIT5)) {
    setViMode();
  }
  
}

// resetproc
void resetProc(void *arg0) {
  #ifndef LIGHTHOUSE_P
  OSMesg msg = NULL;
  do {
    osRecvMesg(&mainQueue, &msg, OS_MESG_BLOCK);
    checkAndSetViMode();
    if ((s32)msg == 3) {
      checkAndSendMesg();
    } else if ((u32)msg == 5) {
      updateTimers();
    } else if ((u32)msg == CORE1_8C50_EVENT_DP) {
      handleDpStatus();
    } else if ((u32)msg == CORE1_8C50_EVENT_SP) {
      handleSpEvent();
    } else if ((u32)msg == CORE1_8C50_EVENT_AUDIO_TIMER) {
      sendMesgToAudioManager();
    } else if ((u32)msg == CORE1_8C50_EVENT_FAULT) {
      do {
      } while (1);
    } else if ((u32)msg == CORE1_8C50_EVENT_PRENMI) {
      setViMode();
    } else if ((u32)msg == 12) {
    } else if ((u32)msg == CORE1_8C50_EVENT_CONT_TIMER) {
      pfsManager_getStartReadData();
    } else if ((u32)msg >= 100) {
      if (*(u32 *)msg == 0) {
        handleMesgType0(msg);
      } else if (*(u32 *)msg == 1) {
        handleMesgType1(msg);
      } else if (*(u32 *)msg == 2) {
        handleMesgType2(msg);
      } else if (*(u32 *)msg == 7) {
        setFlagTrue(msg);
      }
    }
  } while (1);
  #else
  //pc thing here
  #endif
}

//resetThreadCreate
void createResetThread(void) {
  #ifndef LIGHTHOUSE_P
  u64 *tmp_v0;
  osCreateMesgQueue(&mainQueue, &mainQueueMessages, 20);
  osCreateMesgQueue(&secondaryQueue, &secondaryQueueMessages, 10);
  osSetEventMesg(OS_EVENT_DP, &mainQueue, CORE1_8C50_EVENT_DP);
  osSetEventMesg(OS_EVENT_SP, &mainQueue, CORE1_8C50_EVENT_SP);
  osSetEventMesg(OS_EVENT_FAULT, &mainQueue, CORE1_8C50_EVENT_FAULT);
  osSetEventMesg(OS_EVENT_PRENMI, &mainQueue, CORE1_8C50_EVENT_PRENMI);
  viMgr_addMessageQueue(&mainQueue, 5);
  taskOverflowCounter = 0;
  isTaskRunning = 0;
  taskStatus = taskNextStatus = 2;
  taskType = taskPreviousType = 0x10;
  taskYielded = 0;
  primaryTaskQueueTail = 0;
  primaryTaskQueueHead = 0;
  secondaryTaskQueueTail = 0;
  secondaryTaskQueueHead = 0;
  tmp_v0 = yieldData;

  while ((u32)tmp_v0 % 0x10) {
    tmp_v0 = (u64 *)((u32)tmp_v0 + 1);
  }

  gfxTask.t.yield_data_ptr = tmp_v0;
  osCreateThread(&resetThread, 5, resetProc, NULL, &resetThreadStack[2048], 60);
  osStartThread(&resetThread);
  #else
  //pc thing here
  #endif
}

void enableControllerTimer(void) { controllerTimerEnabled = 1; }

void endDisplayList(Gfx **gfx) {
  gDPPipeSync((*gfx)++);
  gSPEndDisplayList((*gfx)++);
}

s32 getTaskState(void) { return taskType; }

OSMesgQueue *getMainQueue(void) { return &mainQueue; }

OSThread *getResetThread(void) { return &resetThread; }
