#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include "core1/pfsmanager.h"

#include "version.h"

#define PFSMANAGER_THREAD_STACK_SIZE 0x200

extern struct {
  u8 pad0[4];
  s32 unk4;
  u8 pad8[4];
  s32 unkC;
} controllerState;

extern s32 previousControllerState;
extern s32 currentControllerState;

/* .data */
#if VERSION == VERSION_USA_1_0
s32 checksumValue1 = 0xC3A68832; // WHAT IS THIS?
s32 checksumValue2 = 0xDDC3A724; // WHAT IS THIS?
#elif VERSION == VERSION_PAL
s32 checksumValue1 = 0xED7BCDB7; // WHAT IS THIS?
s32 checksumValue2 = 0xF82DC7AC; // WHAT IS THIS?
#endif

static s32 isMessageQueueInitialized = 0;

/* .bss */
UNK_TYPE(s32) buttonPressCounts[4][5];
u8 controllerBitPattern;
PfsManagerControllerData controllerData[4];
PfsManagerControllerData firstControllerData;
Struct_core1_10A00_1 controllerStateData[4];
OSMesg contPollingMessageBuffer;
OSMesg contReplyMessageBuffer;
OSContPad contPadData[4];
OSContPad currentControllerData;
OSMesgQueue contPollingMessageQueue;
OSMesgQueue contReplyMessageQueue;
f32 joystickIdleTimes[4];
OSContStatus contStatus;
u8 pad_D_80281320[0x8];
volatile s32 isPfsManagerBusy;
OSThread pfsManagerThread;
u8 pfsManagerThreadStack[PFSMANAGER_THREAD_STACK_SIZE];
f32 demoPlaybackTime;
OSMesgQueue pfsManagerMessageQueue;
OSMesg pfsManagerMessageBuffer[4];
u8 pad_D_80281710[1];

f32 pfsManager_calculateStickValue(s32 arg0, s32 arg1, s32 arg2) {
  f32 phi_f2;

  phi_f2 = 0.0125f;
  if ((controllerState.unk4 != previousControllerState) || (controllerState.unkC != currentControllerState)) {
    phi_f2 = 0.00625f;
  }
  if (arg0 > 0) {
    arg0 = (arg2 < arg0) ? arg2 : (arg0 < arg1) ? arg1 : arg0;
    arg0 = (s32)((arg0 - arg1) * 0x50) / (s32)(arg2 - arg1);
  } else {
    if (arg0 < 0) {
      arg0 = (arg0 < -arg2) ? -arg2 : (-arg1 < arg0) ? -arg1 : arg0;
      arg0 = (s32)((arg0 + arg1) * 0x50) / (s32)(arg2 - arg1);
    }
  }
  return phi_f2 *= arg0;
}

void controller_copyFaceButtons(s32 controller_index, s32 dst[6]) {
  dst[FACE_BUTTON(BUTTON_A)] =
      controllerData[controller_index].face_button[FACE_BUTTON(BUTTON_A)];
  dst[FACE_BUTTON(BUTTON_B)] =
      controllerData[controller_index].face_button[FACE_BUTTON(BUTTON_B)];
  dst[FACE_BUTTON(BUTTON_C_LEFT)] =
      controllerData[controller_index].face_button[FACE_BUTTON(BUTTON_C_LEFT)];
  dst[FACE_BUTTON(BUTTON_C_DOWN)] =
      controllerData[controller_index].face_button[FACE_BUTTON(BUTTON_C_DOWN)];
  dst[FACE_BUTTON(BUTTON_C_UP)] =
      controllerData[controller_index].face_button[FACE_BUTTON(BUTTON_C_UP)];
  dst[FACE_BUTTON(BUTTON_C_RIGHT)] =
      controllerData[controller_index].face_button[FACE_BUTTON(BUTTON_C_RIGHT)];
}

void pfsManager_getFirstControllerFaceButtonState(s32 controller_index,
                                                  s32 dst[6]) {
  dst[FACE_BUTTON(BUTTON_A)] = firstControllerData.face_button[FACE_BUTTON(BUTTON_A)];
  dst[FACE_BUTTON(BUTTON_B)] = firstControllerData.face_button[FACE_BUTTON(BUTTON_B)];
  dst[FACE_BUTTON(BUTTON_C_LEFT)] =
      firstControllerData.face_button[FACE_BUTTON(BUTTON_C_LEFT)];
  dst[FACE_BUTTON(BUTTON_C_DOWN)] =
      firstControllerData.face_button[FACE_BUTTON(BUTTON_C_DOWN)];
  dst[FACE_BUTTON(BUTTON_C_UP)] =
      firstControllerData.face_button[FACE_BUTTON(BUTTON_C_UP)];
  dst[FACE_BUTTON(BUTTON_C_RIGHT)] =
      firstControllerData.face_button[FACE_BUTTON(BUTTON_C_RIGHT)];
}

s32 pfsManager_getButtonPressCount(s32 arg0, s32 arg1) {
  return buttonPressCounts[arg0][arg1];
}

s32 pfsManager_getSideButtonState(s32 controller_index, s32 dst[3]) {
  dst[SIDE_BUTTON(BUTTON_Z)] =
      controllerData[controller_index].side_button[SIDE_BUTTON(BUTTON_Z)];
  dst[SIDE_BUTTON(BUTTON_L)] =
      controllerData[controller_index].side_button[SIDE_BUTTON(BUTTON_L)];
  dst[SIDE_BUTTON(BUTTON_R)] =
      controllerData[controller_index].side_button[SIDE_BUTTON(BUTTON_R)];
}

s32 pfsManager_getFirstControllerSideButtonState(s32 controller_index,
                                                 s32 dst[3]) {
  dst[SIDE_BUTTON(BUTTON_Z)] = firstControllerData.side_button[SIDE_BUTTON(BUTTON_Z)];
  dst[SIDE_BUTTON(BUTTON_L)] = firstControllerData.side_button[SIDE_BUTTON(BUTTON_L)];
  dst[SIDE_BUTTON(BUTTON_R)] = firstControllerData.side_button[SIDE_BUTTON(BUTTON_R)];
}

f32 pfsManager_getJoystickIdleTime(s32 controller_index) {
  return joystickIdleTimes[controller_index];
}

s32 controller_getStartButton(s32 controller_index) {
  return controllerData[controller_index].start_button;
}

s32 pfsManager_getStartButtonState(s32 controller_index) {
  if (globalTimer_getTime() < 2) {
    return 0;
  }

  return controllerData[controller_index].start_button;
}

void pfsManager_getUnknownButtonState(s32 controller_index, s32 dst[4]) {
  dst[0] = controllerData[controller_index].unk24[0];
  dst[1] = controllerData[controller_index].unk24[1];
  dst[2] = controllerData[controller_index].unk24[2];
  dst[3] = controllerData[controller_index].unk24[3];
}

void controller_getJoystick(s32 controller_index, f32 dst[2]) {
  if (game_isSpecialMode()) {
    dst[0] = controllerStateData[controller_index].joystick[0];
    dst[1] = controllerStateData[controller_index].joystick[1];
  } else {
    dst[0] = pfsManager_calculateStickValue(
        contPadData[controller_index].stick_x, 7, 0x3B);
    dst[1] = pfsManager_calculateStickValue(
        contPadData[controller_index].stick_y, 7, 0x3D);
  }
}

void pfsManager_update(void) {
  int j;
  int i;
  u32 sp5C;
  u32 s0;
  u32 temp_t6;
  u32 temp_v0_3;
  u32 var_a2;
  if (system_getCurrentMode() == 3) {
    game_updateTime();
  }

  osSetThreadPri(0, 0x29);

  currentControllerData.stick_x = contPadData[0].stick_x;
  currentControllerData.stick_y = contPadData[0].stick_y;
  currentControllerData.button = contPadData[0].button;
  if ((game_getMode() == GAME_MODE_6_FILE_PLAYBACK) ||
      (game_getMode() == GAME_MODE_7_ATTRACT_DEMO) ||
      (game_getMode() == GAME_MODE_8_BOTTLES_BONUS) ||
      (game_getMode() == GAME_MODE_A_SNS_PICTURE) ||
      (game_getMode() == GAME_MODE_9_BANJO_AND_KAZOOIE)) {
    s0 = 0x1000;
    if (gctransition_done()) {
      demoPlaybackTime += time_getDelta();
    }
    if ((demoPlaybackTime < 1.0) ||
        (game_getMode() == GAME_MODE_9_BANJO_AND_KAZOOIE)) {
      s0 = 0;
    }
    temp_t6 = demo_readInput(&contPadData, &sp5C) == 0;
    if ((currentControllerData.button & s0) || temp_t6) {
      if (currentControllerData.button & s0) {
        volatileFlag_set(VOLATILE_FLAG_64, 1);
      } else {
        volatileFlag_set(VOLATILE_FLAG_63, 1);
      }
    }
    time_setDeltaReal_frames(sp5C);
  }
  sp5C = time_getDeltaReal_frames();
  randf();
  for (i = 0; i < 4; i++) {
    if ((contPadData[i].button & 0x20) &&
        (contPadData[i].button & 0x10)) {
      buttonPressCounts[i][0] = (contPadData[i].button & 0x0004)
                             ? (buttonPressCounts[i][0] + 1)
                             : (0);
      buttonPressCounts[i][1] = (contPadData[i].button & 0x2000)
                             ? (buttonPressCounts[i][1] + 1)
                             : (0);
      buttonPressCounts[i][2] = (contPadData[i].button & 0x8000)
                             ? (buttonPressCounts[i][2] + 1)
                             : (0);
      buttonPressCounts[i][3] = (contPadData[i].button & 0x4000)
                             ? (buttonPressCounts[i][3] + 1)
                             : (0);
      buttonPressCounts[i][4] =
          (currentControllerData.button & 0x4000) ? (buttonPressCounts[i][4] + 1) : (0);
      for (j = 0; j < 0xE; j++) {
        ((s32 *)(&controllerData[i]))[j] = 0;
      }

      for (j = 0; (j < 0xE) && (i == 0); j++) {
        ((s32 *)(&firstControllerData))[j] = 0;
      }

      controllerStateData[i].unk0 = 0;
      controllerStateData[i].unk2 = 0;
      controllerStateData[i].unk4 = 0;
      controllerStateData[i].unk6 = 0;
      controllerStateData[i].joystick[0] = 0.0f;
      controllerStateData[i].joystick[1] = 0.0f;
      controllerStateData[i].unk8[0] = 0.0f;
      controllerStateData[i].unk8[1] = 0.0f;
      continue;
    }

    for (j = 0; j < 5; j++) {
      buttonPressCounts[i][j] = 0;
    }

    controllerData[i].face_button[0] = (contPadData[i].button & 0x8000)
                                       ? (controllerData[i].face_button[0] + 1)
                                       : (0);
    controllerData[i].face_button[1] = (contPadData[i].button & 0x4000)
                                       ? (controllerData[i].face_button[1] + 1)
                                       : (0);
    controllerData[i].face_button[2] = (contPadData[i].button & 0x0002)
                                       ? (controllerData[i].face_button[2] + 1)
                                       : (0);
    controllerData[i].face_button[3] = (contPadData[i].button & 0x0004)
                                       ? (controllerData[i].face_button[3] + 1)
                                       : (0);
    controllerData[i].face_button[4] = (contPadData[i].button & 0x0008)
                                       ? (controllerData[i].face_button[4] + 1)
                                       : (0);
    controllerData[i].face_button[5] = (contPadData[i].button & 0x0001)
                                       ? (controllerData[i].face_button[5] + 1)
                                       : (0);
    controllerData[i].side_button[0] = (contPadData[i].button & 0x2000)
                                       ? (controllerData[i].side_button[0] + 1)
                                       : (0);
    controllerData[i].side_button[1] = (contPadData[i].button & 0x0020)
                                       ? (controllerData[i].side_button[1] + 1)
                                       : (0);
    controllerData[i].side_button[2] = (contPadData[i].button & 0x0010)
                                       ? (controllerData[i].side_button[2] + 1)
                                       : (0);
    controllerData[i].unk24[0] = (contPadData[i].button & 0x0800)
                                 ? (controllerData[i].unk24[0] + 1)
                                 : (0);
    controllerData[i].unk24[1] = (contPadData[i].button & 0x0400)
                                 ? (controllerData[i].unk24[1] + 1)
                                 : (0);
    controllerData[i].unk24[2] = (contPadData[i].button & 0x0200)
                                 ? (controllerData[i].unk24[2] + 1)
                                 : (0);
    controllerData[i].unk24[3] = (contPadData[i].button & 0x0100)
                                 ? (controllerData[i].unk24[3] + 1)
                                 : (0);
    controllerData[i].start_button = (contPadData[i].button & 0x1000)
                                     ? (controllerData[i].start_button + 1)
                                     : (0);
    if (i == 0) {
      firstControllerData.face_button[0] =
          (currentControllerData.button & 0x8000) ? (firstControllerData.face_button[0] + 1) : (0);
      firstControllerData.face_button[1] =
          (currentControllerData.button & 0x4000) ? (firstControllerData.face_button[1] + 1) : (0);
      firstControllerData.face_button[2] =
          (currentControllerData.button & 0x0002) ? (firstControllerData.face_button[2] + 1) : (0);
      firstControllerData.face_button[3] =
          (currentControllerData.button & 0x0004) ? (firstControllerData.face_button[3] + 1) : (0);
      firstControllerData.face_button[4] =
          (currentControllerData.button & 0x0008) ? (firstControllerData.face_button[4] + 1) : (0);
      firstControllerData.face_button[5] =
          (currentControllerData.button & 0x0001) ? (firstControllerData.face_button[5] + 1) : (0);
      firstControllerData.side_button[0] =
          (currentControllerData.button & 0x2000) ? (firstControllerData.side_button[0] + 1) : (0);
      firstControllerData.side_button[1] =
          (currentControllerData.button & 0x0020) ? (firstControllerData.side_button[1] + 1) : (0);
      firstControllerData.side_button[2] =
          (currentControllerData.button & 0x0010) ? (firstControllerData.side_button[2] + 1) : (0);
      firstControllerData.unk24[0] =
          (currentControllerData.button & 0x0800) ? (firstControllerData.unk24[0] + 1) : (0);
      firstControllerData.unk24[1] =
          (currentControllerData.button & 0x0400) ? (firstControllerData.unk24[1] + 1) : (0);
      firstControllerData.unk24[2] =
          (currentControllerData.button & 0x0200) ? (firstControllerData.unk24[2] + 1) : (0);
      firstControllerData.unk24[3] =
          (currentControllerData.button & 0x0100) ? (firstControllerData.unk24[3] + 1) : (0);
      firstControllerData.start_button = (currentControllerData.button & 0x1000)
                                    ? ((u64)firstControllerData.start_button + 1)
                                    : (0);
    }
    temp_v0_3 = (u16)controllerStateData[i].unk0;
    var_a2 = (u16)contPadData[i].button;
    controllerStateData[i].unk0 = var_a2;
    controllerStateData[i].unk2 = temp_v0_3;
    controllerStateData[i].unk4 = (~temp_v0_3) & var_a2;
    controllerStateData[i].unk6 = temp_v0_3 & (~var_a2);
    controllerStateData[i].unk8[0] = controllerStateData[i].joystick[0];
    controllerStateData[i].unk8[1] = controllerStateData[i].joystick[1];
    controllerStateData[i].joystick[0] = pfsManager_calculateStickValue(
        contPadData[i].stick_x, 7, 0x3B);
    controllerStateData[i].joystick[1] = pfsManager_calculateStickValue(
        contPadData[i].stick_y, 7, 0x3D);
    if ((controllerStateData[i].unk4 != 0) ||
        (controllerStateData[i].unk8[0] != controllerStateData[i].joystick[0]) ||
        (controllerStateData[i].unk8[1] != controllerStateData[i].joystick[1])) {
      joystickIdleTimes[i] = 0.0f;
    } else {
      joystickIdleTimes[i] += time_getDelta();
    }
  }

  osSetThreadPri(0, 0x14);
}

void pfsManager_readData() {
  pfsManager_setBusyState(0);
  if (!contStatus.err_no)
    osContGetReadData(contPadData);
}

void pfsManager_entry(void *arg) {
  do {
    osRecvMesg(&contPollingMessageQueue, 0, 1);
    if (isPfsManagerBusy == TRUE) {
      pfsManager_readData();
    } else {
      osSendMesg(&contReplyMessageQueue, (OSMesg)NULL, 0);
    }
  } while (1);
}

void pfsManager_init(void) {
  osCreateMesgQueue(&contPollingMessageQueue, &contPollingMessageBuffer,
                    1);
  osCreateMesgQueue(&contReplyMessageQueue, &contReplyMessageBuffer, 1);
  osCreateThread(&pfsManagerThread, 7, pfsManager_entry, NULL,
                 pfsManagerThreadStack + PFSMANAGER_THREAD_STACK_SIZE, 40);
                 #ifndef LIGHTHOUSE_P
  osSetEventMesg(OS_EVENT_SI, &contPollingMessageQueue,
                 &contPollingMessageBuffer);
                 #endif
  osContInit(&contPollingMessageQueue, &controllerBitPattern,
             &contStatus);
  osContSetCh(1);
  pfsManager_resetControllerData();
  enableControllerTimer();
  osStartThread(&pfsManagerThread);
}

bool pfsManager_contErr(void) { return BOOL(contStatus.err_no); }

void pfsManager_checkControllerError(void) {
  if (pfsManager_contErr())
    chOverlayNoController_spawn(0, 0);
}

void pfsManager_handleControllerError(void) {
  if (pfsManager_contErr())
    chOverlayNoController_func_802DD040(0, 0);
}

void pfsManager_getStartReadData(void) {
  if (isPfsManagerBusy == 0) {
    pfsManager_setBusyState(1);
    osContStartReadData(&contPollingMessageQueue);
  }
}

void pfsManager_receiveMesg(void) {
  osRecvMesg(&contPollingMessageQueue, NULL, 1);
  pfsManager_update();
}

void pfsManager_resetControllerData(void) {
  s32 iCont, j;

  // for(iCont = 0; iCont < 4; iCont++){
  //     controllerStateData[iCont].unk0 = 0;
  // }

  for (iCont = 0; iCont < 4; iCont++) {
    controllerStateData[iCont].unk0 = 0;
    controllerStateData[iCont].unk2 = 0;
    controllerStateData[iCont].unk4 = 0;
    controllerStateData[iCont].unk6 = 0;
    controllerStateData[iCont].joystick[0] = 0.0f;
    controllerStateData[iCont].joystick[1] = 0.0f;
    controllerStateData[iCont].unk8[0] = 0.0f;
    controllerStateData[iCont].unk8[1] = 0.0f;
    for (j = 0; j < 5; j++) {
      buttonPressCounts[iCont][j] = 0;
    }
    for (j = 0; j < 14; j++) {
      controllerData[iCont].face_button[j] = 0;
    }
    joystickIdleTimes[iCont] = 0.0f;
  }
}

void pfsManager_copyControllerData(s32 arg0, Struct_core1_10A00_1 *arg1) {
  heap_memcpy(arg1, controllerStateData + arg0, sizeof(Struct_core1_10A00_1));
}

void pfsManager_setZButtonState(s32 controller_index, s32 arg1) {
  controllerData[controller_index].side_button[SIDE_BUTTON(BUTTON_Z)] = arg1;
}

OSMesgQueue *pfsManager_getFrameReplyQ(void) {
  return &contReplyMessageQueue;
}

OSMesgQueue *pfsManager_getFrameMesgQ(void) {
  return &contPollingMessageQueue;
}

void pfsManager_setBusyState(s32 arg0) {
  if (!arg0)
    pfsManager_sendMesg();
  else
    pfsManager_waitForMesg();

  if (arg0 || pfsManagerMessageQueue.validCount == 1) {
    isPfsManagerBusy = arg0;
  }
}

bool pfsManager_isBusy(void) { return isPfsManagerBusy; }

int pfsManager_getControllerInputSum(int arg0) {
  return contPadData[arg0].button +
         contPadData[arg0].stick_x +
         contPadData[arg0].stick_y;
}

OSContPad *pfsManager_getControllerData(void) { return &currentControllerData; }

/* initilizes pfsManagerMessageQueue message queue */
void pfsManager_initMesgQueue(void) {
  isMessageQueueInitialized = TRUE;
  osCreateMesgQueue(&pfsManagerMessageQueue, &pfsManagerMessageBuffer, 5);
  osSendMesg(&pfsManagerMessageQueue, (OSMesg)NULL, OS_MESG_NOBLOCK);
}

void pfsManager_waitForMesg(void) {
  if (!isMessageQueueInitialized)
    pfsManager_initMesgQueue();
  osRecvMesg(&pfsManagerMessageQueue, NULL, OS_MESG_BLOCK);
#ifndef LIGHTHOUSE_P // todo: get this a pc version if needed

  osSetEventMesg(OS_EVENT_SI, &contPollingMessageQueue,
                 &contPollingMessageBuffer);
#endif
}

void pfsManager_sendMesg(void) {
  osSendMesg(&pfsManagerMessageQueue, (OSMesg)NULL, OS_MESG_NOBLOCK);
}
