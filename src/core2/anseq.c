#include "functions.h"
#include "variables.h"
#include <ultra64.h>

extern void baMotor_setEqualRumbleParams(f32, f32);
extern void func_802BB360(s32, f32);
extern void func_802BB378(s32, f32, f32);
extern void func_802BB3AC(s32, f32);
extern void func_8031B908(s32, s32, s32, f32);

/* .bss */
s32 s_activationFrameDelay;

/* .code */
void anSeq_updateSoundParams(s32 arg0, s32 arg1) {
  u8 sp1C[3];
  f32 f0;
  sp1C[0] = (arg0 >> 16) & 0xFF;
  sp1C[1] = (arg0 >> 8) & 0xFF;
  sp1C[2] = (arg0 >> 0) & 0xFF;
  f0 = reinterpret_cast(f32, arg1);
  func_8031B908(sp1C[0], sp1C[1], sp1C[2], f0);
}

void anSeq_updateRumble(s32 arg0, s32 arg1, s32 arg2) {
  f32 f0 = reinterpret_cast(f32, arg1);
  f32 f2 = reinterpret_cast(f32, arg2);
  func_802BB378(arg0, f0, f2);
}

void anSeq_updateRumbleWithParams(s32 arg0, s32 arg1, s32 arg2) {
  f32 f0 = reinterpret_cast(f32, arg1);
  f32 f2 = reinterpret_cast(f32, arg2);
  func_802BB3DC(arg0, f0, f2);
}

void anSeq_updateSound(s32 arg0, s32 arg1) {
  f32 f0 = reinterpret_cast(f32, arg1);
  func_802BB360(arg0, f0);
  func_802BB3AC(arg0, 1.0f);
  func_802BB3C4(arg0);
}

void anSeq_updateSoundWithVolume(s32 arg0, s32 arg1) {
  f32 f0 = reinterpret_cast(f32, arg1);
  func_802BB3AC(arg0, f0);
}

void anSeq_playSound(s32 arg0) {
  s32 sp18;
  s32 temp_t6;
  s32 phi_a3;

  phi_a3 = arg0 >> 0x10;
  if (!(phi_a3 == 0xF2 && map_get() == MAP_91_FILE_SELECT &&
        func_802C5A30() != 0) &&
      !((phi_a3 == 0x21 || phi_a3 == 0x3ED) &&
        map_get() == MAP_91_FILE_SELECT && (func_802C5A30() == 1))) {
    func_8030E6A4(phi_a3, (f32)((f64)((arg0 >> 8) & 0xFF) * 0.0078125),
                  (s32)((f64)(arg0 & 0xFF) * 128.0));
  }
}

void anSeq_playMusicTrack(s32 arg0) {
  comusic_playTrackWithVolumeOverride((u16)(arg0 >> 16), (u16)arg0 - 1);
}

void anSeq_stopMusicTrack(enum sfx_e arg0) { comusic_stopTrack(arg0); }

void anSeq_setRumbleParams(s32 arg0, s32 arg1) {
  f32 f12 = reinterpret_cast(f32, arg0);
  f32 f14 = reinterpret_cast(f32, arg1);
  baMotor_setEqualRumbleParams(f12, f14);
}

void anSeq_setRumbleParamsWithDuration(s32 arg0, s32 arg1, s32 arg2) {
  f32 f12 = reinterpret_cast(f32, arg0);
  f32 f14 = reinterpret_cast(f32, arg1);
  f32 f0 = reinterpret_cast(f32, arg2);
  baMotor_setRumbleParams(f12, f14, f0);
}

void anSeq_updateStep(bk_vector(AnSeqElement) * *ppAnSeq, AnSeqElement *pStep) {
  if (pStep->activationFrameDelay) {
    if (pStep->activationFrameDelay == 0xFF)
      return;

    pStep->activationFrameDelay -= 1;
    if (pStep->activationFrameDelay)
      return;

    pStep->activationFrameDelay = 0xFF;
  }

  switch (pStep->argCount) {
  case 0: // 80288BF8
    ((void (*)(void))pStep->funcPtr)();
    break;
  case 1: // 80288C0C
    ((void (*)(s32))pStep->funcPtr)(pStep->arg0);
    break;
  case 2: // 80288C24
    ((void (*)(s32, s32))pStep->funcPtr)(pStep->arg0, pStep->arg1);
    break;
  case 3: // 80288C40
    ((void (*)(s32, s32, s32))pStep->funcPtr)(pStep->arg0, pStep->arg1,
                                              pStep->arg2);
    break;
  case 4: // 80288C5C
    ((void (*)(void *))pStep->funcPtr)(&pStep->arg0);
    break;
  }
}

void anSeq_clear(bk_vector(AnSeqElement) * *ppAnSeq) { vector_clear(*ppAnSeq); }

AnSeqElement *__anSeq_pushStep(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                               s32 arg_cnt, void *funcPtr, s32 arg0, s32 arg1,
                               s32 arg2) {
  AnSeqElement *ptr = (AnSeqElement *)vector_pushBackNew(ppAnSeq);
  ptr->duration = duration;
  ptr->argCount = arg_cnt;
  ptr->funcPtr = funcPtr;
  ptr->arg0 = arg0;
  ptr->arg1 = arg1;
  ptr->arg2 = arg2;
  ptr->activationFrameDelay = s_activationFrameDelay;
  s_activationFrameDelay = 0;
  return ptr;
}

void anSeq_PushStep_0Arg(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         void *func_ptr) {
  __anSeq_pushStep(ppAnSeq, duration, 0, func_ptr, 0, 0, 0);
}

void anSeq_PushStep_1Arg(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         void *func_ptr, s32 arg0) {
  __anSeq_pushStep(ppAnSeq, duration, 1, func_ptr, arg0, 0, 0);
}

void anSeq_PushStep_2Arg(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         void *func_ptr, s32 arg0, s32 arg4) {
  __anSeq_pushStep(ppAnSeq, duration, 2, func_ptr, arg0, arg4, 0);
}

void anSeq_PushStep_3Arg(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         void *func_ptr, s32 arg0, s32 arg4, s32 arg5) {
  __anSeq_pushStep(ppAnSeq, duration, 3, func_ptr, arg0, arg4, arg5);
}

void anSeq_PushStep_ManyArg(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                            void *func_ptr, void *arg_ptr, s32 arg_size) {
  AnSeqElement *out = __anSeq_pushStep(ppAnSeq, duration, 4, func_ptr, 0, 0, 0);
  heap_memcpy(&out->arg0, arg_ptr, arg_size);
}

void anSeq_pushUpdateRumble(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         s32 arg2, s32 arg3, s32 arg4) {
  anSeq_PushStep_3Arg(ppAnSeq, duration, anSeq_updateRumble, arg2, arg3,
                      arg4);
}

void anSeq_pushUpdateRumbleWithParams(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         s32 arg2, s32 arg3, s32 arg4) {
  anSeq_PushStep_3Arg(ppAnSeq, duration, anSeq_updateRumbleWithParams, arg2, arg3,
                      arg4);
}

void anSeq_pushUpdateSound(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         s32 arg2, s32 arg3) {
  anSeq_PushStep_2Arg(ppAnSeq, duration, anSeq_updateSound, arg2, arg3);
}

void anSeq_pushUpdateSoundWithVolume(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         s32 arg2, s32 arg3) {
  anSeq_PushStep_2Arg(ppAnSeq, duration, anSeq_updateSoundWithVolume, arg2, arg3);
}

void anSeq_pushPlaySound(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         s32 arg2) {
  anSeq_PushStep_1Arg(ppAnSeq, duration, anSeq_playSound, arg2);
}

void anSeq_pushStopMusicTrack(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         enum sfx_e sfx_id) {
  anSeq_PushStep_1Arg(ppAnSeq, duration, anSeq_stopMusicTrack, sfx_id);
}

void anSeq_pushPlayMusicTrack(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         s32 arg2) {
  anSeq_PushStep_1Arg(ppAnSeq, duration, anSeq_playMusicTrack, arg2);
}

void anSeq_pushSetRumbleParams(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         s32 arg2, s32 arg3) {
  do {
    anSeq_PushStep_2Arg(ppAnSeq, duration, anSeq_setRumbleParams, arg2, arg3);
  } while (0);
}

void anSeq_pushSetRumbleParamsWithDuration(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         s32 arg2, s32 arg3, s32 arg4) {
  do {
    anSeq_PushStep_3Arg(ppAnSeq, duration, anSeq_setRumbleParamsWithDuration, arg2, arg3,
                        arg4);
  } while (0);
}

void anSeq_updateSoundParamsWithDuration(bk_vector(AnSeqElement) * *ppAnSeq, f32 duration,
                         s32 arg2, f32 arg3) {
  anSeq_PushStep_2Arg(ppAnSeq, duration, anSeq_updateSoundParams, arg2,
                      reinterpret_cast(s32, arg3));
}

void anSeq_free(bk_vector(AnSeqElement) * *ppAnSeq) {
  vector_free(*ppAnSeq);
  bk_free(ppAnSeq);
}

bk_vector(AnSeqElement) * *anSeq_new(void) {
  bk_vector(AnSeqElement) **ptr =
      (bk_vector(AnSeqElement) **)heap_malloc(sizeof(bk_vector(AnSeqElement) **));
  *ptr = vector_new(sizeof(AnSeqElement), 2);
  anSeq_clear(ptr);
  return ptr;
}

void anSeq_setActivationFrameDelay(bk_vector(AnSeqElement) * *ppAnSeq, s32 arg1) {
  s_activationFrameDelay = arg1;
}

void anSeq_update(bk_vector(AnSeqElement) * *ppAnSeq, AnimCtrl *pAnCtl) {
  AnSeqElement *iPtr;
  for (iPtr = vector_getBegin(*ppAnSeq);
       iPtr != (AnSeqElement *)vector_getEnd(*ppAnSeq); iPtr++) {
    if (anctrl_isAt(pAnCtl, iPtr->duration))
      anSeq_updateStep(ppAnSeq, iPtr);
  }
}
