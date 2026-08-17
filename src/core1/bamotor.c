// BanjoDecomp: bamotor.c
#include <ultra64.h>
#include "core1/core1.h"
#include "core2/core2.h"
#include "bk_time.h"
#include "version.h"
#include "port/DevTools/ThreadWatchdog.h"
#include "port/OS/OS.h"
#include "port/Patches/Patches.h"

#define BAMOTOR_THREAD_STACK_SIZE 0x200

OSMesgQueue sbaMotorThreadMesgQ;
OSMesg sbaMotorThreadMesgBuf;
bool sbaMotorAvialable;
bool sbaMotorGood;
OSMesgQueue *sbaMotorPfsMesgQ;
OSPfs sbaMotorPfsHandle;
f32 D_80282420;
f32 D_80282424;
f32 D_80282428;
f32 D_8028242C;
OSThread sbaMotorThread;
u8 sbaMotorThreadStrack[BAMOTOR_THREAD_STACK_SIZE];

void baMotor_startMotor(void) {
    s32 status;

    if (sbaMotorGood) {
        controller_func_8024F35C(4);
        status = osMotorStart(&sbaMotorPfsHandle);
        sbaMotorGood = (status == 0);
        controller_func_8024F35C(0);
    }
}

void baMotor_stopMotor(void) {
    s32 status;

    if (sbaMotorGood) {
        controller_func_8024F35C(4);
        status = osMotorStop(&sbaMotorPfsHandle);
        sbaMotorGood = (status == 0);
        controller_func_8024F35C(0);
    }
}

void baMotor_reinitialize(void) {
    s32 status;

    if (!sbaMotorGood) {
        controller_func_8024F35C(4);
        status = osMotorInit(sbaMotorPfsMesgQ, &sbaMotorPfsHandle, 0);
        sbaMotorGood = (status == 0);
        controller_func_8024F35C(0);
    }
}

void baMotor_entry(void *arg) {
    static s32 frame_count;
    static s32 rumble_state;
    f32 temp_f2;
    f32 temp_f0;
    f32 temp_f12;
    f32 temp_f14;
    s32 prev_state;
    s32 var_v0;
    s32 var_v1;

    do {
        osRecvMesg(&sbaMotorThreadMesgQ, NULL, OS_MESG_BLOCK);
        if (OS_ThreadShouldExit()) { // [port] cooperative shutdown
            return;
        }
        ThreadWatchdog_Beat(WATCHDOG_RUMBLE); // [port] one beat per retrace signal
        frame_count++;

        if (!sbaMotorGood && ((frame_count % FRAMERATE) == 0)) {
            baMotor_reinitialize();
        }

        prev_state = rumble_state;

        if (D_80282424 != D_80282420) {
            temp_f2 = D_80282428 + ((D_8028242C - D_80282428) * D_80282424 / D_80282420);
            var_v0 = (s32) (((1.0 - temp_f2) * 8.0) + 1);
            if (var_v0 < 2) {
                rumble_state = var_v0;
            } else {
                rumble_state = (frame_count % var_v0) == 0;
            }
        } else {
            rumble_state = 0;
        }

        if (rumble_state != prev_state) {
            if (rumble_state) {
                baMotor_startMotor();
            } else {
                baMotor_stopMotor();
            }
        }
    } while (TRUE);
}

void baMotor_80250BA4(s32 arg0, s32 arg1, s32 arg2) {
    f64 f0 = 524288.0;
    baMotor_80250D94(arg0 / f0, arg1 / f0, arg2 / f0);
}

void baMotor_80250C08(void) {
    if (sbaMotorAvialable) {
        D_80282424 = MIN(D_80282420, D_80282424 + time_getDelta());
    }
}

void baMotor_init(void) {
    s32 status;

    controller_func_8024F35C(4);
    sbaMotorPfsMesgQ = pfsManager_getFrameReplyQ();
    status = osPfsInit(sbaMotorPfsMesgQ, &sbaMotorPfsHandle, 0);
    if ((status == PFS_ERR_ID_FATAL) || (status == PFS_ERR_DEVICE)) {
        // The rumble pack is only initialized, if no Pfs (Transfer Pak) is inserted into the Controller
        status = osMotorInit(sbaMotorPfsMesgQ, &sbaMotorPfsHandle, 0);
    }
    controller_func_8024F35C(0);

    sbaMotorAvialable = (status == 0);
    sbaMotorGood = sbaMotorAvialable;

    if (sbaMotorAvialable) {
        osCreateMesgQueue(&sbaMotorThreadMesgQ, &sbaMotorThreadMesgBuf, 1);
        // [port] The thread parks on that queue between retraces, so it has to
        // block on it, and its entry needs allowlisting before osStartThread
        // will really launch it.
        OS_SetQueueBlocking(&sbaMotorThreadMesgQ, 1);
        OS_EnableThreadEntry((void*)baMotor_entry);
        osCreateThread(&sbaMotorThread, BAMOTOR_THREAD_ID, baMotor_entry, NULL, sbaMotorThreadStrack + BAMOTOR_THREAD_STACK_SIZE, BAMOTOR_THREAD_PRI);
        osStartThread(&sbaMotorThread);
        viMgr_registerSignalMesg(&sbaMotorThreadMesgQ, OS_MESG_32(0));
    }
}

// [port] Watchdog diagnostics
OSMesgQueue *baMotor_getRetraceQueue(void){
    return &sbaMotorThreadMesgQ;
}

void baMotor_stub(void) {}

void baMotor_80250D94(f32 arg0, f32 arg1, f32 arg2) {
    f32 f4;
    if ((arg2 != 0.0f) && sbaMotorAvialable) {
        if(func_802E4A08() == 0){
            if(!(0.1 < D_80282420 - D_80282424) || !(arg0 + arg1 < D_80282428 + D_8028242C)){
                f32 scale = port_getRumbleScale();
                D_80282420 = arg2;
                D_80282424 = 0.0f;
                D_80282428 = MIN(arg0 * scale, 1.0f);
                D_8028242C = MIN(arg1 * scale, 1.0f);
            }
        }
    }
}

void baMotor_80250E6C(f32 arg0, f32 arg1) {
    baMotor_80250D94(arg0, arg0, arg1);
}

void baMotor_80250E94(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5) {
    if (sbaMotorAvialable) {
        timedFunc_set_3(0.0f, (GenFunction_3) baMotor_80250BA4, 0, (s32) (arg0 * 524288.0f), (s32) (arg2*524288.0f));
        timedFunc_set_3(arg2, (GenFunction_3) baMotor_80250BA4, (s32) (arg0 * 524288.0f), (s32) (arg1 * 524288.0f), (s32) (arg3*524288.0f));
        timedFunc_set_3(arg2 + arg3, (GenFunction_3) baMotor_80250BA4, (s32) (arg1 * 524288.0f), (s32) (arg1 * 524288.0f), (s32) (arg4*524288.0f));
        timedFunc_set_3(arg2 + arg3 + arg4, (GenFunction_3) baMotor_80250BA4, (s32) (arg1 * 524288.0f), 0, (s32) (arg5*524288.0f));
    }
}

void baMotor_forceStop(void) {
    int i;
    s32 status;

    if (sbaMotorGood) {
        controller_func_8024F35C(4);
        status = osMotorInit(sbaMotorPfsMesgQ, &sbaMotorPfsHandle, 0);
        sbaMotorGood = (status == 0);
        for (i = 0; (i < 3) && sbaMotorGood; i++) {
            status = osMotorStop(&sbaMotorPfsHandle);
            sbaMotorGood = (status == 0);
        }
        controller_func_8024F35C(0);
    }
}
