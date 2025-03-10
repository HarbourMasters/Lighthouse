#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include "version.h"

#define RUMBLE_THREAD_STACK_SIZE 0x200

OSMesgQueue rumbleMesgQueue;
OSMesg rumbleMesg;
s32 rumbleEnabled;
s32 motorInitialized;
OSMesgQueue *pfsMesgQueue;
OSPfs sMotorPfs;
f32 rumbleDuration;
f32 rumbleElapsed;
f32 rumbleStartIntensity;
f32 rumbleEndIntensity;
OSThread sRumbleThread;
u8 sRumbleThreadStack[RUMBLE_THREAD_STACK_SIZE];

/* .code */
void baMotor_setRumbleParams(f32, f32, f32);
void pfsManager_setBusyState(s32);

void baMotor_startRumble(void) {
  u32 motor_status;

  if (motorInitialized) {
    pfsManager_setBusyState(4);
    motor_status = osMotorStart(&sMotorPfs);
    motorInitialized = (motor_status == 0);
    pfsManager_setBusyState(0);
  }
}

void baMotor_stopRumble(void) {
  u32 motor_status;

  if (motorInitialized) {
    pfsManager_setBusyState(4);
    motor_status = osMotorStop(&sMotorPfs);
    motorInitialized = (motor_status == 0);
    pfsManager_setBusyState(0);
  }
}

void baMotor_initMotor(void) {
  u32 motor_status;

  if (!motorInitialized) {
    pfsManager_setBusyState(4);
    motor_status = osMotorInit(pfsMesgQueue, &sMotorPfs, 0);
    motorInitialized = (motor_status == 0);
    pfsManager_setBusyState(0);
  }
}

void rumbleThread_entry(void *arg) {
  static s32 sRumbleCounter;
  static s32 rumble_state;
  f32 temp_f2;
  f32 temp_f0;
  f32 temp_f12;
  f32 temp_f14;
  s32 prev_state;
  s32 var_v0;
  s32 var_v1;

  do {
    osRecvMesg(&rumbleMesgQueue, NULL, 1);
    sRumbleCounter++;
    if (!motorInitialized && ((sRumbleCounter % FRAMERATE) == 0)) {
      baMotor_initMotor();
    }
    prev_state = rumble_state;
    if (rumbleElapsed != rumbleDuration) {
      temp_f2 =
          rumbleStartIntensity + ((rumbleEndIntensity - rumbleStartIntensity) * rumbleElapsed / rumbleDuration);
      var_v0 = (s32)(((1.0 - temp_f2) * 8.0) + 1);
      if (var_v0 < 2) {
        rumble_state = var_v0;
      } else {
        rumble_state = (sRumbleCounter % var_v0) == 0;
      }
    } else {
      rumble_state = 0;
    }

    if (rumble_state != prev_state) {
      if (rumble_state) {
        baMotor_startRumble(); // start_motor
      } else {
        baMotor_stopRumble(); // stop_motor
      }
    }
  } while (1);
}

void baMotor_convertRumbleParams(s32 arg0, s32 arg1, s32 arg2) {
  f64 f0 = 524288.0;
  baMotor_setRumbleParams(arg0 / f0, arg1 / f0, arg2 / f0);
}

void baMotor_updateRumbleTimer(void) {
  if (rumbleEnabled != 0) {
    rumbleElapsed = MIN(rumbleDuration, rumbleElapsed + time_getDelta());
  }
}

void baMotor_init(void) {
  s32 pfs_status;
  #ifndef LIGHTHOUSE_P
  pfsManager_setBusyState(4);
  pfsMesgQueue = pfsManager_getFrameReplyQ();
  pfs_status = osPfsInit(pfsMesgQueue, &sMotorPfs, 0);
  if (pfs_status == PFS_ERR_ID_FATAL || pfs_status == PFS_ERR_DEVICE) {
    pfs_status = osMotorInit(pfsMesgQueue, &sMotorPfs, 0);
  }
  pfsManager_setBusyState(0);
  rumbleEnabled = (pfs_status == 0);
  motorInitialized = rumbleEnabled;
  if (rumbleEnabled) {
    osCreateMesgQueue(&rumbleMesgQueue, &rumbleMesg, 1);
    osCreateThread(&sRumbleThread, 8, rumbleThread_entry, NULL,
                   sRumbleThreadStack + RUMBLE_THREAD_STACK_SIZE, 25);
    osStartThread(&sRumbleThread);
    viMgr_addMessageQueue(&rumbleMesgQueue, 0);
  }
  #else
  //rumble here
  #endif
}

//dont move this
void baMotor_stub1(void) {}

void baMotor_setRumbleParams(f32 arg0, f32 arg1, f32 arg2) {
  f32 f4;
  if (arg2 != 0.0f && rumbleEnabled) {
    if (game_isSpecialMode() == 0) {
      if (!(0.1 < rumbleDuration - rumbleElapsed) ||
          !(arg0 + arg1 < rumbleStartIntensity + rumbleEndIntensity)) {
        rumbleDuration = arg2;
        rumbleElapsed = 0.0f;
        rumbleStartIntensity = arg0;
        rumbleEndIntensity = arg1;
      }
    }
  }
}

void baMotor_setEqualRumbleParams(f32 arg0, f32 arg1) {
  baMotor_setRumbleParams(arg0, arg0, arg1);
}

void baMotor_scheduleRumblePattern(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4,
                      f32 arg5) {
  if (rumbleEnabled) {
    timedFunc_set_3(0.0f, (GenFunction_3)baMotor_convertRumbleParams, 0,
                    (s32)(arg0 * 524288.0f), (s32)(arg2 * 524288.0f));
    timedFunc_set_3(arg2, (GenFunction_3)baMotor_convertRumbleParams,
                    (s32)(arg0 * 524288.0f), (s32)(arg1 * 524288.0f),
                    (s32)(arg3 * 524288.0f));
    timedFunc_set_3(arg2 + arg3, (GenFunction_3)baMotor_convertRumbleParams,
                    (s32)(arg1 * 524288.0f), (s32)(arg1 * 524288.0f),
                    (s32)(arg4 * 524288.0f));
    timedFunc_set_3(arg2 + arg3 + arg4, (GenFunction_3)baMotor_convertRumbleParams,
                    (s32)(arg1 * 524288.0f), 0, (s32)(arg5 * 524288.0f));
  }
}

void baMotor_resetMotor(void) {
  int i;
  u32 motor_status;
  if (motorInitialized) {
    pfsManager_setBusyState(4);
    motor_status = osMotorInit(pfsMesgQueue, &sMotorPfs, 0);
    motorInitialized = (motor_status == 0);
    for (i = 0; i < 3 && motorInitialized; i++) {
      motor_status = osMotorStop(&sMotorPfs);
      motorInitialized = (motor_status == 0);
    }
    pfsManager_setBusyState(0);
  }
}
