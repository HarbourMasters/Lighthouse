// BanjoDecomp: core1/code_8C50.c
// Needs to be included here because interrupt.h contains:
// typedef u32 OSIntMask; and u32 isn't defined before that.

#include "libultraship/libultra/types.h"
//#include <PR/os_system.h>
#include <PR/ucode.h>
#include "core1/core1.h"
#include "functions.h"
#include "port/DevTools/ThreadWatchdog.h"
#include "port/OS/OS.h"
#include "port/Patches/Patches.h"
#include "libultraship/libultra/rcp.h"
#include "libultraship/libultra/sptask.h"
#include "libultraship/libultra/types.h"
#include "variables.h"
#include "version.h"
#include <libultra/r4300.h>
#include <ultra64.h>

#include <libultra/rdp.h>

#define UNKFLAG1_AUDIO_TASK     0x04
#define UNKFLAG1_GFX_TASK       0x08
#define UNKFLAG1_NO_TASK        0x10
#define UNKFLAG1_TASK_YIELDED   0x20

/* .extern */
extern u8 n_aspMainTextStart[];
extern u8 gSPF3DEX_fifoTextStart[];
extern u8 gSPL3DEX_fifoTextStart[];

extern u8 n_aspMainDataStart[];
extern u8 gSPF3DEX_fifoDataStart[];
extern u8 gSPL3DEX_fifoDataStart[];

/* .data */
static OSTask sAudTask = {
    /* type */ M_AUDTASK,
    /* flags */ 0,
    NULL, 0,                  /* ucode_boot */
    NULL, SP_UCODE_SIZE,      /* ucode */
    NULL, SP_UCODE_DATA_SIZE, /* ucode_data */
    NULL, 0,                  /* dram_stack */
    NULL, NULL,               /* output_buff */
    NULL, 0,                  /* data */
    NULL, 0,                  /* yield_data */
};

static OSTask sGfxTask = {
    /* type */ M_GFXTASK,
    /* flags */ 0,
    NULL, 0,                  /* ucode_boot */
    NULL, SP_UCODE_SIZE,      /* ucode */
    NULL, SP_UCODE_DATA_SIZE, /* ucode_data */
    (u64 *)(uintptr_t)0x80000400, 0x400,        /* dram_stack */
    (u64 *)(uintptr_t)0x80000800, (u64 *)(uintptr_t)0x8000E800,   /* output_buff */
    NULL, 0,                  /* data */
    NULL, OS_YIELD_DATA_SIZE, /* yield_data */
};

static s32 sUnkCounter1 = 0; // Unused counter, probably for debugging
static s32 sUnkCounter2 = 0; // Unused counter, probably for debugging
static s32 sUnkCounter3 = 0; // Unused counter, probably for debugging
static s32 sUnkCounter4 = 0; // Unused counter, probably for debugging

/* .bss */
static u64 sYieldData[OS_YIELD_DATA_SIZE / sizeof(u64)];
static u8 pad[0x20];
static OSMesgQueue sThread5MesgQueue;
static OSMesg      sThread5MesgBuffer[20];
static OSMesgQueue sThread5SyncMesgQueue;
static OSMesg      sThread5SyncMesgBufer[10];
static struct ucode_task_data_s *sActiveAudioTaskDataPtr;
static s32 sSyncCounter;
static bool sTask7Handled;
static s32 sUnkFlag2_Saved;
static s32 sUnkFlag2;
static s32 sUnkFlag1;
static s32 sUnkFlag1_Saved;
static s32 sGfxTaskYielded;
static u8 sThread5Stack[2048]; // stack for thread sThread5
static OSThread sThread5;
static struct ucode_task_data_s *sGfxTaskDataList[20];
static volatile s32 sSelectedGfxTaskDataID;
static volatile s32 sActiveGfxTaskDataID;
static struct ucode_task_data_s *sAudioTaskDataList[20];
static volatile s32 sSelectedAudioTaskDataID;
static volatile s32 sActiveAudioTaskDataID;
static void *sCurrentFramebuffer;
static OSTimer sAudioTimer;
static OSTimer sControllerTimer;
static bool sEnableControllerTimer;

void thread5_startNextAudioTask(void);
void osSpTaskLoad(OSTask* task);
void osSpTaskStartGo(OSTask* task);

/* .code */

// [port] The queues are static; Game.cpp opts them into blocking before
// thread5_create runs.
OSMesgQueue* thread5_getTaskQueue(void) {
    return &sThread5MesgQueue;
}
OSMesgQueue* thread5_getSyncQueue(void) {
    return &sThread5SyncMesgQueue;
}

void thread5_sendTaskToQueue(OSMesg arg0) {
    static bool clear_freeze = true;

    port_thread5_onSubmit(arg0.ptr); // [port] carry the interpolation pair with the task
    osSendMesg(&sThread5MesgQueue, arg0, 1);
    // Lighthouse [port] Adjustment here to account for our OSMesg union definition
    if (arg0.data32 == THREAD5_MESSAGE_EVENT_SYNC) {
        sUnkCounter2 = 30;
        if (clear_freeze) {
            osDpSetStatus(DPC_CLR_FREEZE);
            clear_freeze = false;
        }
        osRecvMesg(&sThread5SyncMesgQueue, NULL, 1);
        sUnkCounter2 = 0;
    }
}

void thread5_insertAudioTaskData(OSMesg arg0) {
    s32 new_id = (sSelectedAudioTaskDataID + 1) % 20;
    if (sActiveAudioTaskDataID != new_id) {
        sAudioTaskDataList[sSelectedAudioTaskDataID] = arg0.ptr;
        sSelectedAudioTaskDataID = new_id;
    }
}

void thread5_insertGfxTaskData(OSMesg arg0) {
    s32 new_id = (sSelectedGfxTaskDataID + 1) % 20;
    if (sActiveGfxTaskDataID != new_id) {
        sGfxTaskDataList[sSelectedGfxTaskDataID] = arg0.ptr;
        sSelectedGfxTaskDataID = new_id;
    }
}

void thread5_startAudioTask(struct ucode_task_data_s *task_data) {
#if 0 // [port] microcode boot pointers, there is no RSP to load them into
    ucode_getPtrAndSize(&sAudTask.t.ucode_boot, &sAudTask.t.ucode_boot_size);
    sAudTask.t.ucode = n_aspMainTextStart;
    sAudTask.t.ucode_data = n_aspMainDataStart;
#endif
    sAudTask.t.data_ptr = (void*) task_data->data_ptr;
    sAudTask.t.data_size = ((u8 *)task_data->data_ptr_end - (u8 *)task_data->data_ptr) >> 3 << 3;
    osWritebackDCache(sAudTask.t.data_ptr , sAudTask.t.data_size);
    osWritebackDCache(&sAudTask, sizeof(OSTask));
    sActiveAudioTaskDataPtr = task_data;
    osSpTaskLoad(&sAudTask);
    osSpTaskStartGo(&sAudTask);
    sUnkFlag1 = UNKFLAG1_AUDIO_TASK;
}

void thread5_startF3DEXTask(struct ucode_task_data_s *task_data) {
#if 0 // [port] microcode boot pointers, there is no RSP to load them into
    ucode_getPtrAndSize(&sGfxTask.t.ucode_boot, &sGfxTask.t.ucode_boot_size);
    sGfxTask.t.ucode = gSPF3DEX_fifoTextStart;
    sGfxTask.t.ucode_data = gSPF3DEX_fifoDataStart;
#endif
    sGfxTask.t.data_ptr = (void*) task_data->data_ptr;
    sGfxTask.t.data_size = ((u8 *)task_data->data_ptr_end - (u8 *)task_data->data_ptr) >> 3 << 3;
    osWritebackDCache(sGfxTask.t.data_ptr , sGfxTask.t.data_size);
    osWritebackDCache(&sGfxTask, sizeof(OSTask));
    osSpTaskLoad(&sGfxTask);
    osSpTaskStartGo(&sGfxTask);
    sUnkFlag1 = task_data->unk4 | UNKFLAG1_GFX_TASK;
    sUnkFlag2 = task_data->unk4 | 0x1;
    if (!(osDpGetStatus() & DPC_STATUS_FREEZE)) {
        sUnkFlag2_Saved = sUnkFlag2;
        sUnkCounter3 = 30;
    }
}

void thread5_startL3DEXTask(struct ucode_task_data_s *task_data) {
#if 0 // [port] microcode boot pointers, there is no RSP to load them into
    ucode_getPtrAndSize(&sGfxTask.t.ucode_boot, &sGfxTask.t.ucode_boot_size);
    sGfxTask.t.ucode = gSPL3DEX_fifoTextStart;
    sGfxTask.t.ucode_data = gSPL3DEX_fifoDataStart;
#endif
    sGfxTask.t.data_ptr = (void*) task_data->data_ptr;
    sGfxTask.t.data_size = ((u8 *)task_data->data_ptr_end - (u8 *)task_data->data_ptr) >> 3 << 3;
    osWritebackDCache(sGfxTask.t.data_ptr , sGfxTask.t.data_size);
    osWritebackDCache(&sGfxTask, sizeof(OSTask));
    osSpTaskLoad(&sGfxTask);
    osSpTaskStartGo(&sGfxTask);
    sUnkFlag1 = task_data->unk4 | UNKFLAG1_GFX_TASK;
    sUnkFlag2 = task_data->unk4 | 0x1;
    if (!(osDpGetStatus() & DPC_STATUS_FREEZE)) {
        sUnkFlag2_Saved = sUnkFlag2;
        sUnkCounter3 = 30;
    }
}

void thread5_startGfxTask(struct ucode_task_data_s *task_data) {
    switch (task_data->task_type) {
        case 1:
            thread5_startF3DEXTask(task_data);
            break;

        case 2:
            thread5_startL3DEXTask(task_data);
            break;
    }
}

void thread5_handleAudioTaskMesg(OSMesg msg) {
    thread5_insertAudioTaskData(msg);
    if ((sUnkFlag1 == UNKFLAG1_NO_TASK) && (sActiveAudioTaskDataID != sSelectedAudioTaskDataID)) {
        struct ucode_task_data_s *ptr = sAudioTaskDataList[sActiveAudioTaskDataID];
        sActiveAudioTaskDataID = (sActiveAudioTaskDataID + 1) % 20;
        thread5_startAudioTask(ptr);
    }
}

void thread5_handleF3DEXTaskMesg(OSMesg msg) {
    thread5_insertGfxTaskData(msg);
    if (sUnkFlag1 == UNKFLAG1_NO_TASK && !sTask7Handled) {
        thread5_startF3DEXTask(sGfxTaskDataList[sActiveGfxTaskDataID]);
        sActiveGfxTaskDataID = (sActiveGfxTaskDataID + 1) % 20;
    }
}

void thread5_handleL3DEXTaskMesg(OSMesg msg) {
    thread5_insertGfxTaskData(msg);
    if (sUnkFlag1 == UNKFLAG1_NO_TASK && !sTask7Handled) {
        thread5_startL3DEXTask(sGfxTaskDataList[sActiveGfxTaskDataID]);
        sActiveGfxTaskDataID = (sActiveGfxTaskDataID + 1) % 20;
    }
}

void thread5_handleSyncEvent(void) {
    if ((sUnkFlag1 == UNKFLAG1_NO_TASK)
        && (sUnkFlag2_Saved == 2)
        && (sActiveGfxTaskDataID == sSelectedGfxTaskDataID)
        && !(osDpGetStatus() & DPC_STATUS_FREEZE)
    ) {
        osSendMesgPtr(&sThread5SyncMesgQueue, NULL, OS_MESG_NOBLOCK);
    }
    else {
        sSyncCounter++;
    }
}

extern u64 osClockRate;

void thread5_handleDPEvent(void) {
    if ((sUnkFlag2 << 1) < 0) {
        osDpSetStatus(DPC_SET_FREEZE);
        sCurrentFramebuffer = osViGetCurrentFramebuffer();
        viMgr_func_8024BFAC();
    }
    sUnkFlag2 = sUnkFlag2_Saved = 2;
    sUnkCounter3 = 0;
    if (sUnkFlag1 == UNKFLAG1_NO_TASK && sActiveGfxTaskDataID != sSelectedGfxTaskDataID && !sTask7Handled) {
        thread5_startGfxTask(sGfxTaskDataList[sActiveGfxTaskDataID]);
        sActiveGfxTaskDataID = (sActiveGfxTaskDataID + 1) % 20;
    }
    else {
        if (sSyncCounter && sActiveGfxTaskDataID == sSelectedGfxTaskDataID && !(osDpGetStatus() & DPC_STATUS_FREEZE)) {
            osSendMesgPtr(&sThread5SyncMesgQueue, NULL, 0);
            sSyncCounter--;
        }
    }
}

void thread5_handleVIRetraceEvent(void) {
    s32 sp2C = (sSyncCounter != 0) && (sActiveGfxTaskDataID == sSelectedGfxTaskDataID) && (sUnkFlag2 == 2) && (sUnkFlag1 == UNKFLAG1_NO_TASK);
    volatile s32 sp30;

    sp30 = false;
    if (osViGetCurrentFramebuffer() != sCurrentFramebuffer || sp2C) {
        if (osDpGetStatus() & DPC_STATUS_FREEZE) {
            osDpSetStatus(DPC_CLR_FREEZE);

            sUnkFlag2_Saved = sUnkFlag2;
            // dummy_func_8025AFB8();

            if (sUnkFlag2_Saved & 1) {
                sUnkCounter3 = 30;
            }
        }

        if (sp2C) {
            osSendMesgPtr(&sThread5SyncMesgQueue, NULL, OS_MESG_NOBLOCK);
            sSyncCounter--;
        }
    }

    sUnkCounter1 = 0;

    if (sUnkCounter2 != 0) {
        sUnkCounter2--;
    }

    if (sUnkCounter4 != 0) {
        sUnkCounter4--;
    }

    if (sUnkCounter3 != 0) {
        sUnkCounter3--;
        if (sUnkCounter3 == 0) {
            sp30 = true;
        }
    }
    sTask7Handled = false;
    static s32 audiotimer_trigger = 0;
    audiotimer_trigger++;
    if (!(audiotimer_trigger & 1)) {
        osStopTimer(&sAudioTimer);
        osSetTimer(&sAudioTimer, 280000, 0, &sThread5MesgQueue, OS_MESG_32(THREAD5_MESSAGE_EVENT_AUDIO_TIMER));
    }
    if (sEnableControllerTimer && OS_SiPumpLive()) {
        osStopTimer(&sControllerTimer);
#if VERSION == VERSION_USA_1_0
        osSetTimer(&sControllerTimer, ((osClockRate / 60)* 2) / 3, 0, &sThread5MesgQueue, OS_MESG_32(THREAD5_MESSAGE_EVENT_CONT_TIMER));
#elif VERSION == VERSION_PAL
        osSetTimer(&sControllerTimer, ((osClockRate / 60.0)* 2) / 3, 0, &sThread5MesgQueue, OS_MESG_32(THREAD5_MESSAGE_EVENT_CONT_TIMER));
#endif
    }
}

void thread5_handleSPEvent(void) {
    struct ucode_task_data_s *active_audio_task;
    s32 temp_v1;

    temp_v1 = sUnkFlag1;
    if (sUnkFlag1 == UNKFLAG1_TASK_YIELDED) {
        active_audio_task = sAudioTaskDataList[sActiveAudioTaskDataID];
        sActiveAudioTaskDataID = (sActiveAudioTaskDataID + 1) % 20;
        sGfxTaskYielded = (osSpTaskYielded(&sGfxTask) == 1);
        thread5_startAudioTask(active_audio_task);
        sUnkCounter4 = 0;
        return;
    }

    if (sUnkFlag1 == UNKFLAG1_AUDIO_TASK) {
        // [port] The raw decomp sends through the first two struct fields;
        // with the struct typed, these are the reply queue and message set by
        // core1_15B30_addAudioTaskData.
        osSendMesg(sActiveAudioTaskDataPtr->audio_mesg_queue, sActiveAudioTaskDataPtr->audio_mesg, 0);
    }

    if ((sUnkFlag1 == UNKFLAG1_AUDIO_TASK) && (sGfxTaskYielded != 0)) {
        osSpTaskLoad(&sGfxTask);
        osSpTaskStartGo(&sGfxTask);
        sUnkFlag1 = sUnkFlag1_Saved;
        sGfxTaskYielded = 0;
        return;
    }

    sUnkFlag1 = UNKFLAG1_NO_TASK;
    if ((sActiveGfxTaskDataID != sSelectedGfxTaskDataID) && (sTask7Handled == 0)) {
        thread5_startGfxTask(sGfxTaskDataList[sActiveGfxTaskDataID]);
        sActiveGfxTaskDataID = (sActiveGfxTaskDataID + 1) % 20;
        return;
    }

    if ((sSyncCounter != 0) && (sUnkFlag2_Saved == 2) && !(osDpGetStatus() & 2)) {
        osSendMesgPtr(&sThread5SyncMesgQueue, NULL, 0);
        sSyncCounter--;
    }
}

void thread5_handleTask7Mesg(OSMesg arg0) {
    sTask7Handled = true;
}

void thread5_handleAudioTimerEvent(void) {
    // [port] While the demo audio hold is up, skip the frame message so the
    // engine does not consume sfx cues queued for the demo's first frame.
    if (port_audioHeld() || port_audioStallHold()) {
        return;
    }
    osSendMesgPtr(audioManager_getFrameMesgQueue(), NULL, OS_MESG_NOBLOCK);
    thread5_startNextAudioTask();
}

void thread5_startNextAudioTask(void) {
    struct ucode_task_data_s *ptr;
    if ((sUnkFlag1 == UNKFLAG1_NO_TASK) && (sActiveAudioTaskDataID != sSelectedAudioTaskDataID)) {
        ptr = sAudioTaskDataList[sActiveAudioTaskDataID];
        sActiveAudioTaskDataID = (sActiveAudioTaskDataID + 1) % 20;
        thread5_startAudioTask(ptr);
    } else if ((sUnkFlag1 & UNKFLAG1_GFX_TASK) && (sActiveAudioTaskDataID != sSelectedAudioTaskDataID)) {
        osSpTaskYield();
        sUnkFlag1_Saved = sUnkFlag1;
        sUnkFlag1 = UNKFLAG1_TASK_YIELDED;
        sUnkCounter4 = 30;
    }
}

void thread5_stub(void) {}

extern s32 osTvType;

void thread5_handlePreNMIEvent(void) {
    static OSViMode osViModeMpalLpn1 = {
        OS_VI_MPAL_LPN1, /* type */
        {
          VI_CTRL_TYPE_16 | VI_CTRL_GAMMA_DITHER_ON | VI_CTRL_GAMMA_ON | 0x3200,       /*ctrl*/
          320,          /*width*/
          0x4651E39,    /*burst*/
          0x20D,        /*vSync*/
          0x40C11,      /* hSync*/
          0xC190C1A,    /* leap*/
          0x6C02EC,     /* hStart*/
          0, /* xScale*/
          0, /* vCurrent*/
        },
        {
            {640, 1024, 0x2501FF, 0xE0204, 2},
            {640, 1024, 0x2501FF, 0xE0204, 2}
        }
    };
    static OSViMode osViModeNtscLpn1 = {
        OS_VI_NTSC_LPN1, /* type */
        {
          VI_CTRL_TYPE_16 | VI_CTRL_GAMMA_DITHER_ON | VI_CTRL_GAMMA_ON | 0x3200,       /*ctrl*/
          320,          /*width*/
          0x3E52239,    /*burst*/
          0x20D,        /*vSync*/
          0xC15,        /* hSync*/
          0xC150C15,    /* leap*/
          0x6C02EC,     /* hStart*/
          0, /* xScale*/
          0, /* vCurrent*/
        },
        {
            {0x280, 1024, 0x2501FF, 0xE0204, 2},
            {640, 1024, 0x2501FF, 0xE0204, 2}
        }
    };
#if VERSION == VERSION_PAL
    static OSViMode osViModePalLpn1 = {
        OS_VI_PAL_LPN1, /* type */
        {
          VI_CTRL_TYPE_16 | VI_CTRL_GAMMA_DITHER_ON | VI_CTRL_GAMMA_ON | 0x3200,       /*ctrl*/
          320,          /*width*/
          0x404233A,    /*burst*/
          0x271,        /*vSync*/
          0x150C69,        /* hSync*/
          0xC6F0C6E,    /* leap*/
          0x800300,     /* hStart*/
          0, /* xScale*/
          0, /* vCurrent*/
        },
        {
            {640, 1024, 0x5F0239, 0x9026B, 2},
            {640, 1024, 0x5F0239, 0x9026B, 2}
        }
    };
#endif
    static bool mode_set;

    if (!mode_set) {
        mode_set = true;
#if VERSION == VERSION_USA_1_0
        if (osTvType != OS_TV_NTSC) {
            osViSetMode(&osViModeMpalLpn1);
        } else {
            osViSetMode(&osViModeNtscLpn1);
        }
#elif VERSION == VERSION_PAL
        // if(&osViModeMpalLpn1){}
        osViSetMode(&osViModePalLpn1);
#endif
        baMotor_80250FC0(); //stop controller motors
        do {
            osDpSetStatus(DPC_STATUS_FLUSH);
        } while (1);
    }
}

void thread5_checkAndExecutePreNMI(void) {
    if (!(bkGetSR() & SR_IBIT5)) {
        thread5_handlePreNMIEvent();
    }
}

//thread5 entry
void thread5_entry(void *arg) {
    // [port] Only adjustment: OSMesg is a union here,
    // so event words read through .data32 and tasks through .ptr
    OSMesg msg;
    msg.ptr = NULL;
    do {
        osRecvMesg(&sThread5MesgQueue, &msg, OS_MESG_BLOCK);
        // [port] Shutdown released the queue rather than delivering anything, so msg
        // holds nothing worth dispatching. Leave before the engine goes away.
        if (OS_ThreadShouldExit()) {
            return;
        }
        ThreadWatchdog_Beat(WATCHDOG_THREAD5); // [port] one beat per serviced message
        thread5_checkAndExecutePreNMI();
        if ((uintptr_t)msg.ptr < 100) {
            if (msg.data32 == THREAD5_MESSAGE_EVENT_SYNC) { thread5_handleSyncEvent(); }
            else if (msg.data32 == THREAD5_MESSAGE_EVENT_VI_RETRACE)  { thread5_handleVIRetraceEvent(); }
            else if (msg.data32 == THREAD5_MESSAGE_EVENT_DP)          { thread5_handleDPEvent(); }
            else if (msg.data32 == THREAD5_MESSAGE_EVENT_SP)          { thread5_handleSPEvent(); }
            else if (msg.data32 == THREAD5_MESSAGE_EVENT_AUDIO_TIMER) { thread5_handleAudioTimerEvent(); }
            else if (msg.data32 == THREAD5_MESSAGE_EVENT_FAULT)       { do{}while(1); }
            else if (msg.data32 == THREAD5_MESSAGE_EVENT_PRENMI)      { thread5_handlePreNMIEvent(); }
            else if (msg.data32 == THREAD5_MESSAGE_EVENT_DEBUG) {  }
            else if (msg.data32 == THREAD5_MESSAGE_EVENT_CONT_TIMER)  { pfsManager_getStartReadData(); }
        }
        else {
            if (((struct ucode_task_data_s *)msg.ptr)->task_type == UCODE_TASK_TYPE_AUDIO) { thread5_handleAudioTaskMesg(msg); }
            else if (((struct ucode_task_data_s *)msg.ptr)->task_type == UCODE_TASK_TYPE_F3DEX) { thread5_handleF3DEXTaskMesg(msg); }
            else if (((struct ucode_task_data_s *)msg.ptr)->task_type == UCODE_TASK_TYPE_L3DEX) { thread5_handleL3DEXTaskMesg(msg); }
            else if (((struct ucode_task_data_s *)msg.ptr)->task_type == UCODE_TASK_TYPE_FRAMEBUFFER_CHANGED) { thread5_handleTask7Mesg(msg); }
        }
    } while (1);
}

//thread5 create
void thread5_create(void) {
    u8 *yield_data_ptr;
    osCreateMesgQueue(&sThread5MesgQueue, sThread5MesgBuffer, 20);
    osCreateMesgQueue(&sThread5SyncMesgQueue, sThread5SyncMesgBufer, 10);
    osSetEventMesg(OS_EVENT_DP, &sThread5MesgQueue, OS_MESG_32(THREAD5_MESSAGE_EVENT_DP));
    osSetEventMesg(OS_EVENT_SP, &sThread5MesgQueue, OS_MESG_32(THREAD5_MESSAGE_EVENT_SP));
    osSetEventMesg(OS_EVENT_FAULT, &sThread5MesgQueue, OS_MESG_32(THREAD5_MESSAGE_EVENT_FAULT));
    osSetEventMesg(OS_EVENT_PRENMI, &sThread5MesgQueue, OS_MESG_32(THREAD5_MESSAGE_EVENT_PRENMI));
    viMgr_registerSignalMesg(&sThread5MesgQueue, OS_MESG_32(THREAD5_MESSAGE_EVENT_VI_RETRACE));
    sSyncCounter = 0;
    sTask7Handled = 0;
    sUnkFlag2 = sUnkFlag2_Saved = 2;
    sUnkFlag1 = sUnkFlag1_Saved = UNKFLAG1_NO_TASK;
    sGfxTaskYielded = 0;
    sSelectedGfxTaskDataID = 0;
    sActiveGfxTaskDataID = 0;
    sSelectedAudioTaskDataID = 0;
    sActiveAudioTaskDataID = 0;

    for (yield_data_ptr = (u8 *) sYieldData; (uintptr_t) yield_data_ptr % 0x10; yield_data_ptr++);

    sGfxTask.t.yield_data_ptr = (u64 *) yield_data_ptr;
    osCreateThread(&sThread5, THREAD5_ID, thread5_entry, NULL, &sThread5Stack[2048], THREAD5_PRI);
    osStartThread(&sThread5);
}

void thread5_enableControllerTimer(void) {
    sEnableControllerTimer = 1;
}

void thread5_finishDList(Gfx **gfx) {
    gDPPipeSync((*gfx)++);
    gSPEndDisplayList((*gfx)++);
}

s32 __thread5_getUnkFlag1(void) {
    return sUnkFlag1;
}

OSMesgQueue *__thread5_getMessageQueue(void) {
    return &sThread5MesgQueue;
}

OSThread *__thread5_getThreadObject(void) {
    return &sThread5;
}

// [port] Watchdog diagnostics: unsynchronized snapshot of the pipeline state.
void thread5_getWatchdogState(Thread5WatchdogState *out) {
    out->unkFlag1 = sUnkFlag1;
    out->unkFlag2 = sUnkFlag2;
    out->unkFlag2Saved = sUnkFlag2_Saved;
    out->syncCounter = sSyncCounter;
    out->task7Handled = sTask7Handled;
    out->gfxActiveId = sActiveGfxTaskDataID;
    out->gfxSelectedId = sSelectedGfxTaskDataID;
    out->audioActiveId = sActiveAudioTaskDataID;
    out->audioSelectedId = sSelectedAudioTaskDataID;
    out->taskQueueCount = sThread5MesgQueue.validCount;
    out->taskQueueCap = sThread5MesgQueue.msgCount;
    out->syncQueueCount = sThread5SyncMesgQueue.validCount;
    out->syncQueueCap = sThread5SyncMesgQueue.msgCount;
}
