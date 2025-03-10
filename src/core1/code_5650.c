#include "core1/core1.h"
#include "functions.h"
#ifndef LIGHTHOUSE_P
#include "n_libaudio.h"
#else
#include "pc_audio.h"

#endif
#include "n_synth.h"
#include "variables.h"
#include <ultra64.h>

//soundManager.c

void freeSound(N_AL_Struct81s *arg0);
void removeSoundEvents(ALEventQueue *arg0, N_AL_Struct81s *arg1, u16 arg2);
void handleSoundEvent(N_ALSndPlayer *arg0);
void processSoundEvent(N_ALSndPlayer *sndp, N_ALEvent *event);
void stopSound(N_AL_Struct81s *arg0);
void updateSoundPitch(N_AL_Struct81s *arg0);
s32 getSoundCounts(u16 *arg0, u16 *arg1);
void updateSoundVolumes(N_AL_Struct81s *arg0);
void *playSound(ALBank *bank, s16 arg1, struct46s *arg2);
void postSoundEvent(s32 arg0, s16 type, s32 arg2);
// extern ALEventUnknown;
/* .bss */
#define CODE_5650_ABS(s) (((s) >= 0) ? (s) : -(s))

typedef struct {
  s32 unk0; // sound state cnt
  s32 unk4;
  s32 unk8;     // maxSounds
  ALHeap *unkC; // heap
  u16 unk10;
} Struct87s;

N_ALSndPlayer soundPlayer;
s16 *soundVolumes;
struct {
  s32 volume[4];
} soundVolumeLevels;

/* .data */
struct {
  N_AL_Struct81s *unk0;
  N_AL_Struct81s *unk4;
  N_AL_Struct81s *unk8;
} soundState = {NULL, NULL, NULL};
N_ALSndPlayer *soundPlayerPtr = &soundPlayer;
s32 soundMode = 1;
s16 activeSoundCount = 0;

/* .code */
void initSoundPlayer(Struct87s *arg0) {
  N_AL_Struct81s *temp_a0;
  N_AL_Struct81s *var_v0;
  ALEvent sp40;
  u32 var_s0;

  soundPlayerPtr->maxSounds = (s32)arg0->unk8;
  soundPlayerPtr->target = NULL;
  soundPlayerPtr->frameTime = 33000;
  soundPlayerPtr->sndState = alHeapDBAlloc(NULL, 0, arg0->unkC, 1,
                                           arg0->unk0 * sizeof(N_AL_Struct81s));
  alEvtqNew(&soundPlayerPtr->evtq,
            alHeapDBAlloc(NULL, 0, arg0->unkC, 1, arg0->unk4 * 0x1C),
            arg0->unk4);
  soundState.unk8 = soundPlayerPtr->sndState;
  for (var_s0 = 1; var_s0 < arg0->unk0; var_s0++) {
    var_v0 = (N_AL_Struct81s *)soundPlayerPtr->sndState;
    temp_a0 = var_s0 + var_v0;
    alLink((ALLink *)temp_a0, (ALLink *)(temp_a0 - 1));
  }
  soundVolumes = alHeapDBAlloc(NULL, 0, arg0->unkC, 2, (s32)arg0->unk10);
  for (var_s0 = 0; var_s0 < arg0->unk10; var_s0++) {
    soundVolumes[var_s0] = 0x7FFF;
  }
  soundPlayerPtr->node.next = NULL;
  soundPlayerPtr->node.handler = (ALVoiceHandler)handleSoundEvent;
  soundPlayerPtr->node.clientData = soundPlayerPtr;
  n_alSynAddSndPlayer(soundPlayerPtr);
  sp40.type = 0x20;
  alEvtqPostEvent(&soundPlayerPtr->evtq, &sp40, soundPlayerPtr->frameTime);
  soundPlayerPtr->nextDelta =
      alEvtqNextEvent(&soundPlayerPtr->evtq, &soundPlayerPtr->nextEvent);
}

void handleSoundEvent(N_ALSndPlayer *arg0) {
  N_ALSndPlayer *new_var = (N_ALSndPlayer *)arg0;
  N_ALEvent2 sp3C;

  do {
    if ((s16)(new_var->nextEvent.type) == 0x20) {
      sp3C.type = 0x20;
      alEvtqPostEvent(&new_var->evtq, (ALEvent *)(&sp3C), new_var->frameTime);
      updateSoundVolumes(new_var);
    } else {
      processSoundEvent(new_var, &arg0->nextEvent);
    }
    new_var->nextDelta = alEvtqNextEvent(&new_var->evtq, &arg0->nextEvent);
  } while (arg0->nextDelta == 0);
  new_var->curTime += new_var->nextDelta;
}

void processSoundEvent(N_ALSndPlayer *sndp, N_ALEvent *event) {
  N_AL_Struct81s *temp_s0;
  ALVoiceConfig spDC;
  N_ALSndPlayer *temp_fp;
  N_ALEvent2 *var_s5;
  ALSound *temp_s1;

  N_ALEvent spC0;
  N_ALEvent spB0;
  s32 spAC;
  N_AL_Struct81s *temp_s6;
  s32 spA4;
  s32 temp_t6;
  s32 var_v0;
  bool var_s2;
  s32 sp94;
  s32 sp90;
  N_ALVoice *voice;
  ALKeyMap *keymap;

  u16 sp86;
  u16 sp84;
  N_AL_Struct81s *sp80;
  N_ALEvent sp70;
  s32 var_v1_4;
  s32 sp68;
  u8 clamped_pan;

  temp_fp = n_syn->n_sndp;
  var_s5 = event;
  sp94 = 1;
  sp90 = 0;
  temp_s0 = NULL;
  temp_s6 = NULL;
  do {
    if (temp_s6 != NULL) {
      spB0.msg.generic.data[0].i = temp_s0;
      spB0.type = var_s5->type;
      spB0.msg.generic.data[1].i = var_s5->msg.vol.delta;
      var_s5 = (N_ALEvent *)&spB0;
    }
    temp_s0 = (N_AL_Struct81s *)var_s5->msg.generic.data[0].i;
    temp_s1 = temp_s0->unk8;
    if (temp_s1 == NULL) {
      getSoundCounts(&sp86, &sp84);
      return;
    }
    keymap = temp_s1->keyMap;
    temp_s6 = temp_s0->node.next;
    switch (var_s5->type) { /* irregular */
    case 0x1:
      if ((temp_s0->unk40 != 5) && (temp_s0->unk40 != 4))
        return;

      spDC.fxBus = 0;
      spDC.priority = (u8)temp_s0->unk36;
      spDC.unityPitch = 0;
      var_s2 = (activeSoundCount >= temp_fp->maxSounds); // SHOULD BE ^1
      if (!(var_s2) || (temp_s0->unk3F & 0x10)) {
        sp90 = n_alSynAllocVoice(&temp_s0->voice, &spDC);
      }
      if (sp90 == 0) {
        if ((temp_s0->unk3F & 0x12) || (temp_s0->unk38 > 0)) {
          temp_s0->unk40 = 4U;
          temp_s0->unk38 = (s32)(temp_s0->unk38 - 1);
          alEvtqPostEvent(&temp_fp->evtq, (ALEvent *)var_s5, 0x8235);
          return;
        }

        if (var_s2) {
          sp80 = soundState.unk4;
          do {
            if (!(sp80->unk3F & 0x12) && (sp80->unk3F & 0x4) &&
                (sp80->unk40 != 3)) {
              sp70.type = 0x80;
              sp70.msg.generic.data[0].i = sp80;
              sp80->unk40 = 3U;
              var_s2 = 0;
              alEvtqPostEvent(&temp_fp->evtq, (ALEvent *)&sp70, 0x3E8);
              n_alSynSetVol(&sp80->voice, 0, 0x3E8);
            }
            sp80 = sp80->node.prev;
          } while (var_s2 && sp80 != NULL);
          if (!var_s2) {
            temp_s0->unk38 = 2;
            alEvtqPostEvent(&temp_fp->evtq, (ALEvent *)var_s5, 0x3E9);
          } else {
            stopSound(temp_s0);
          }
        } else {
          stopSound(temp_s0);
        }
        return;
      }
      temp_s0->unk3F |= 4;
      n_alSynStartVoice(&temp_s0->voice, temp_s1->wavetable);
      temp_s0->unk40 = 1;
      activeSoundCount += 1;
      spAC = (s32)(((f32)temp_s1->envelope->attackTime / temp_s0->unk2C) /
                   temp_s0->unk28);
      spA4 = MAX(0, ((soundVolumes[keymap->keyMin & 0x3F] *
                      ((temp_s1->envelope->attackVolume * temp_s0->unk34 *
                        temp_s1->sampleVolume) /
                       16129)) /
                     32767) -
                        1);
      n_alSynSetVol(&temp_s0->voice, 0, 0);
      n_alSynSetVol(&temp_s0->voice, (s16)spA4, spAC);
      var_v1_4 = (temp_s0->unk3D + temp_s1->samplePan) - 0x40;
      clamped_pan = MIN(MAX(var_v1_4, 0), 0x7F);
      n_alSynSetPan(&temp_s0->voice, clamped_pan);
      n_alSynSetPitch(&temp_s0->voice, temp_s0->unk2C * temp_s0->unk28);
      if ((keymap->keyMax & 0xF) >= 0xE) {
        var_v0 = (keymap->keyMax & 0xF) * 8;
      } else {
        var_v0 = temp_s0->unk3E;
        if (soundMode) {
          var_v0 += (keymap->keyMax & 0xF) * 8;
        }
      }
      var_v0 = MIN(0x7F, MAX(0, var_v0));
      n_alSynSetFXMix(&temp_s0->voice, var_v0);
      spC0.type = 0x40;
      spC0.msg.generic.data[0].i = temp_s0;
      spAC = (s32)(((f32)temp_s1->envelope->attackTime / temp_s0->unk2C) /
                   temp_s0->unk28);
      alEvtqPostEvent(&temp_fp->evtq, (ALEvent *)&spC0, spAC);

      break;

    case 0x2:
    case 0x400:
    case 0x1000:
      if ((var_s5->type != 0x1000) || (temp_s0->unk3F & 2)) {
        switch (temp_s0->unk40) { /* switch 1; irregular */
        case 1:                   /* switch 1 */
          removeSoundEvents(&temp_fp->evtq, temp_s0, 0x40);
          spAC = (s32)(((f32)temp_s1->envelope->releaseTime / temp_s0->unk28) /
                       temp_s0->unk2C);
          temp_s0->envPhase = 2;
          temp_s0->unk48 = 0;
          n_alSynSetVol(&temp_s0->voice, 0, spAC);
          if (spAC != 0) {
            spC0.type = 0x80;
            spC0.msg.generic.data[0].i = temp_s0;
            if (spAC <= 0x7D00) {
              spAC = 0x7D00;
            }
            alEvtqPostEvent(&temp_fp->evtq, (ALEvent *)&spC0, spAC);
            temp_s0->unk40 = 2U;
          } else {
            stopSound(temp_s0);
          }
          break;

        case 4: /* switch 1 */
        case 5: /* switch 1 */
          stopSound(temp_s0);
          break;
        }
        if (var_s5->type == 2) {
          var_s5->type = 0x1000;
        }
      }

      break;

    case 0x4:
      temp_s0->unk3D = var_s5->msg.generic.data[1].i;
      if (temp_s0->unk40 == 1) {
        clamped_pan =
            MIN(MAX((temp_s0->unk3D + temp_s1->samplePan) - 0x40, 0), 0x7F);
        n_alSynSetPan(&temp_s0->voice, clamped_pan);
      }

      break;

    case 0x10:
      temp_s0->unk2C = var_s5->msg.generic.data[1].f;
      if (temp_s0->unk40 == 1) {
        n_alSynSetPitch(&temp_s0->voice, temp_s0->unk2C * temp_s0->unk28);
        if (temp_s0->unk3F & 0x20) {
          updateSoundPitch(temp_s0);
        }
      }

      break;

    case 0x100:
      temp_s0->unk3E = (u8)var_s5->msg.vol.delta;
      if (temp_s0->unk40 == 1) {
        if ((keymap->keyMax & 0xF) >= 0xE) {
          var_v0 = (keymap->keyMax & 0xF) * 8;
        } else {
          var_v0 = temp_s0->unk3E;
          if (soundMode != 0) {
            var_v0 += (keymap->keyMax & 0xF) * 8;
          }
        }

        var_v0 = MIN(0x7F, MAX(0, var_v0));
        n_alSynSetFXMix(&temp_s0->voice, var_v0);
      }

      break;

    case 0x8:
      temp_t6 = MAX(0, (((f32)soundVolumes[keymap->keyMin & 0x3F] *
                         ((temp_s0->unk44 * (f32)temp_s0->unk34 *
                           (f32)temp_s1->sampleVolume) /
                          16129)) /
                        32767) -
                           1);
      temp_s0->unk34 = (s16)var_s5->msg.vol.delta;
      if (temp_s0->unk40 == 1) {
        spA4 = MAX(0, (((f32)soundVolumes[keymap->keyMin & 0x3F] *
                        ((temp_s0->unk44 * (f32)temp_s0->unk34 *
                          (f32)temp_s1->sampleVolume) /
                         16129)) /
                       32767) -
                          1);
        var_v0 = spA4 - temp_t6;
        n_alSynSetVol(&temp_s0->voice, (s16)spA4,
                      MAX(0x3E8, CODE_5650_ABS(var_v0) >> 2));
      }

      break;

    case 0x800:
      if (temp_s0->unk40 == 1) {
        spAC = ((temp_s1->envelope->releaseTime / temp_s0->unk28) /
                temp_s0->unk2C);
        spA4 = MAX(0, ((f32)(s16)soundVolumes[keymap->keyMin & 0x3F] *
                       ((temp_s0->unk44 * (f32)temp_s0->unk34 *
                         (f32)temp_s1->sampleVolume) /
                        16129)) /
                              32767 -
                          1);
        n_alSynSetVol(&temp_s0->voice, (s16)spA4, spAC);
      }

      break;
    case 0x40:
      if (!(temp_s0->unk3F & 2)) {
        spA4 = MAX(0, ((s32)((s16)soundVolumes[keymap->keyMin & 0x3F] *
                             ((temp_s1->envelope->decayVolume * temp_s0->unk34 *
                               temp_s1->sampleVolume) /
                              16129)) /
                       32767) -
                          1);
        spAC = (s32)(((f32)temp_s1->envelope->decayTime / temp_s0->unk28) /
                     temp_s0->unk2C);
        n_alSynSetVol(&temp_s0->voice, spA4, spAC);
        spC0.type = 2;
        spC0.msg.generic.data[0].i = temp_s0;
        alEvtqPostEvent(&temp_fp->evtq, (ALEvent *)&spC0, spAC);
        if (temp_s0->unk3F & 0x20) {
          updateSoundPitch(temp_s0);
        }
      }

      break;
    case 0x80:
      stopSound(temp_s0);

      break;

    case 0x200:
      if (temp_s0->unk3F & 0x10) {
        sp68 = playSound(var_s5->msg.midi.duration,
                         (s16)var_s5->msg.generic.data[1].i, temp_s0->unk30);
        postSoundEvent(sp68, AL_SEQP_PROG_EVT, temp_s0->unk34);
        postSoundEvent(sp68, AL_SEQ_END_EVT, temp_s0->unk3D);
        postSoundEvent(sp68, 0x100, temp_s0->unk3E);
        postSoundEvent(sp68, AL_SEQP_STOP_EVT,
                       reinterpret_cast(s32, temp_s0->unk2C));
      }

      break;
    default:
      break;
    }
    var_v0 = (u16)var_s5->type & 0x2D1;
    temp_s0 = temp_s6;
    if ((temp_s0 != NULL) && !var_v0) {
      sp94 = temp_s0->unk3F & 1;
    }
  } while ((sp94 == 0) && (temp_s0 != NULL) && !var_v0);
}

void stopSound(N_AL_Struct81s *arg0) {
  if (arg0->unk3F & 4) {
    n_alSynStopVoice(&arg0->voice);
    n_alSynFreeVoice(&arg0->voice);
  }
  freeSound(arg0);
  removeSoundEvents(&soundPlayerPtr->evtq, arg0, 0xFFFF);
}

void updateSoundPitch(N_AL_Struct81s *arg0) {
  ALEvent evt;
  f32 sp1C;

  sp1C = alCents2Ratio(arg0->unk8->keyMap->detune) * arg0->unk2C;
  evt.type = 0x10;
  evt.msg.unk3A70.unk0 = arg0;
  evt.msg.unk3A70.unk4 = reinterpret_cast(s32, sp1C);
  alEvtqPostEvent(&soundPlayerPtr->evtq, &evt, 33333);
}

void removeSoundEvents(ALEventQueue *arg0, N_AL_Struct81s *arg1, u16 arg2) {
  s32 pad[5];
  u32 mask;
  ALEventListItem *next_event_list;
  ALEventListItem *event_list;

  mask = osSetIntMask(OS_IM_NONE);
  for (event_list = (ALEventListItem *)arg0->allocList.next; event_list != NULL;
       event_list = next_event_list) {
    next_event_list = (ALEventListItem *)event_list->node.next;
    if ((arg1 == event_list->evt.msg.unk3A70.unk0) &&
        (reinterpret_cast(u16, event_list->evt.type) & arg2)) {
      if (next_event_list != NULL) {
        next_event_list->delta += event_list->delta;
      }
      alUnlink((ALLink *)event_list);
      alLink((ALLink *)event_list, (ALLink *)arg0);
    }
  }
  osSetIntMask(mask);
}

s32 getSoundCounts(u16 *arg0, u16 *arg1) {
  N_AL_Struct81s *var_v0;
  N_AL_Struct81s *var_v1_2;
  N_AL_Struct81s *var_a2;
  u16 var_a0;
  u16 var_a3;
  u16 var_v1;

  var_v0 = soundState.unk0;
  var_v1_2 = soundState.unk8;
  var_a2 = soundState.unk4;

  for (var_a3 = 0; var_v0 != NULL; var_a3++) {
    var_v0 = (N_AL_Struct81s *)var_v0->node.next;
  }

  for (var_a0 = 0; var_v1_2 != NULL; var_a0++) {
    var_v1_2 = (N_AL_Struct81s *)var_v1_2->node.next;
  }

  for (var_v1 = 0; var_a2 != NULL; var_v1++) {
    var_a2 = (N_AL_Struct81s *)var_a2->node.prev;
  }

  *arg0 = var_a0;
  *arg1 = var_a3;
  return var_v1;
}

void updateSoundVolumes(N_AL_Struct81s *arg0) {
  ALMicroTime *temp_a1;
  s32 prev_volume;
  s32 temp_t1;
  s32 temp_t5;
  s32 var_t0;
  ALMicroTime var_t1;
  u8 temp_t6;
  u8 var_a2;
  ALEnvelope *envelope;
  N_AL_Struct81s *var_v0;
  N_AL_Struct81s *var_v1;
  for (var_v0 = soundState.unk0; var_v0 != 0;
       var_v0 = (N_AL_Struct81s *)var_v0->node.next) {
    var_v1 = var_v0;
    envelope = var_v1->unk8->envelope;
    temp_a1 = &envelope->attackTime;
    soundVolumeLevels.volume[1] = (s32)envelope->attackVolume;
    soundVolumeLevels.volume[2] = (s32)envelope->decayVolume;
    while (((var_v1->envPhase < 3) &&
            (var_v1->unk48 >= temp_a1[var_v1->envPhase])) &&
           (temp_a1[var_v1->envPhase] != (-1))) {
      var_v1->unk48 -= temp_a1[var_v1->envPhase];
      var_v1->envPhase++;
    }

    if (var_v1->envPhase < 3) {
      if (temp_a1[var_v1->envPhase] != (-1)) {
        var_v1->unk44 = soundVolumeLevels.volume[var_v1->envPhase] +
                        (soundVolumeLevels.volume[var_v1->envPhase + 1] -
                         soundVolumeLevels.volume[var_v1->envPhase]) *
                            var_v0->unk48 / temp_a1[var_v1->envPhase];
      } else {
        var_v0->unk44 = soundVolumeLevels.volume[var_v1->envPhase];
      }
    }

    var_v1->unk48 += arg0->unk48;
  }
}

N_AL_Struct81s *allocateSound(ALBank *bank, ALSound *sound) {
  s32 sp24;
  ALKeyMap *sp30;
  N_AL_Struct81s *temp_s0;
  OSIntMask mask;

  temp_s0 = soundState.unk8;
  sp30 = sound->keyMap;
  if (temp_s0 != NULL) {
    mask = osSetIntMask(OS_IM_NONE);
    soundState.unk8 = (N_AL_Struct81s *)temp_s0->node.next;
    alUnlink((ALLink *)temp_s0);
    if (soundState.unk0 != NULL) {
      temp_s0->node.next = soundState.unk0;
      temp_s0->node.prev = NULL;
      soundState.unk0->node.prev = temp_s0;
      soundState.unk0 = temp_s0;
    } else {
      temp_s0->node.prev = NULL;
      temp_s0->node.next = NULL;
      soundState.unk0 = temp_s0;
      soundState.unk4 = temp_s0;
    }
    osSetIntMask(mask);
    sp24 = ((sound->envelope->decayTime + 1) == 0);
    // sp24 = sp20 + 0x40;
    temp_s0->unk36 = sp24 + 0x40;
    temp_s0->unk40 = 5;
    temp_s0->unk38 = 2;
    temp_s0->unk8 = sound;
    temp_s0->unk2C = 1.0f;
    temp_s0->unk3F = sp30->keyMax & 0xF0;
    temp_s0->unk30 = 0;
    temp_s0->envPhase = AL_PHASE_ATTACK;
    temp_s0->unk44 = 0.0f;
    temp_s0->unk48 = 0;
    if (temp_s0->unk3F & 0x20) {
      temp_s0->unk28 = alCents2Ratio((sp30->keyBase * 0x64) - 0x1770);
    } else {
      temp_s0->unk28 =
          alCents2Ratio(((sp30->keyBase * 0x64) + sp30->detune) - 0x1770);
    }
    if (sp24 != 0) {
      temp_s0->unk3F |= 2;
    }
    temp_s0->unk3E = 0;
    temp_s0->unk3D = 0x40;
    temp_s0->unk34 = 0x7FFF;
  }
  return temp_s0;
}

void freeSound(N_AL_Struct81s *arg0) {
  N_AL_Struct81s *var_v0;

  sizeof(ALVoice);

  var_v0 = soundState.unk0;
  if (arg0 == soundState.unk0) {
    soundState.unk0 = (N_AL_Struct81s *)arg0->node.next;
  }

  if (arg0 == soundState.unk4) {
    soundState.unk4 = (N_AL_Struct81s *)arg0->node.prev;
  }

  alUnlink((ALLink *)arg0);

  if (soundState.unk8 != NULL) {
    arg0->node.next = (ALLink *)soundState.unk8;
    arg0->node.prev = NULL;
    soundState.unk8->node.prev = (ALLink *)arg0;
    soundState.unk8 = arg0;
  } else {
    arg0->node.prev = NULL;
    arg0->node.next = NULL;
    soundState.unk8 = arg0;
  }

  if (arg0->unk3F & 0x4) {
    activeSoundCount--;
  }

  arg0->unk40 = 0;
  if (arg0->unk30 != NULL) {
    if (*arg0->unk30 == arg0) {
      *arg0->unk30 = NULL;
    }
    arg0->unk30 = NULL;
  }
}

void setSoundPriority(N_AL_Struct81s *arg0, u8 arg1) {
  if (arg0 != NULL)
    arg0->unk36 = arg1;
}

s32 getSoundState(N_AL_Struct81s *arg0) {
  if (arg0 != NULL)
    return arg0->unk40;
  return 0;
}

bool isSoundLooping(ALBank *bank, s16 arg1) {
  ALSound *snd = bank->instArray[0]->soundArray[arg1 - 1];
  if (snd->envelope->decayTime == -1)
    return TRUE;
  else
    return FALSE;
}

void *playSound(ALBank *bank, s16 arg1, struct46s *arg2) {
  ALKeyMap *temp_v0_2;
  ALSound *temp_s2;
  N_AL_Struct81s *temp_v0;
  s32 var_s3;
  s16 sp6E;
  s32 sp68;
  s32 var_s4;
  N_AL_Struct81s *var_fp;
  ALEvent sp50;
  ALEvent sp40;

  var_fp = NULL;
  sp6E = 0;
  var_s3 = 0;
  if (arg1 == 0) {
    return NULL;
  }

  do {
    temp_s2 = bank->instArray[0]->soundArray[arg1 - 1];
    temp_v0 = allocateSound(bank, temp_s2);
    if (temp_v0 != NULL) {
      temp_v0->unk4C = (s32)(arg1 - 1);
      soundPlayerPtr->target = temp_v0;
      sp50.type = AL_SEQ_MIDI_EVT;
      ((s32 *)&sp50.msg)[0] = temp_v0;
      var_s4 = temp_s2->keyMap->velocityMax * 0x8235;
      if (temp_v0->unk3F & 0x10) {
        temp_v0->unk3F &= ~(0x10);
        alEvtqPostEvent(&soundPlayerPtr->evtq, (ALEvent *)&sp50, var_s3 + 1);
        sp68 = var_s4 + 1;
        sp6E = arg1;
      } else {
        alEvtqPostEvent(&soundPlayerPtr->evtq, (ALEvent *)&sp50, var_s4 + 1);
      }
      var_fp = temp_v0;
    }
    temp_v0_2 = temp_s2->keyMap;
    var_s3 += var_s4;
    arg1 = temp_v0_2->velocityMin + ((temp_v0_2->keyMin & 0xC0) * 4);
  } while (arg1 != 0 && temp_v0 != NULL);

  if (var_fp != NULL) {
    var_fp->unk3F |= 1;
    var_fp->unk30 = arg2;
    if (sp6E != 0) {
      var_fp->unk3F |= 0x10;
      sp40.type = 0x200;
      ((s32 *)&sp40.msg)[0] = var_fp;
      ((s32 *)&sp40.msg)[1] = sp6E;
      ((s32 *)&sp40.msg)[2] = bank;
      alEvtqPostEvent(&soundPlayerPtr->evtq, &sp40, sp68);
    }
  }
  if (arg2 != NULL) {
    arg2->unk0 = (s32)var_fp;
  }
  return var_fp;
}

void stopSoundEvent(N_AL_Struct81s *arg0) {
  ALEvent evt;

  evt.type = 0x400;
  ((s32 *)&evt.msg)[0] = arg0;
  if (arg0 != NULL) {
    arg0->unk3F &= ~(0x10);
    alEvtqPostEvent(&soundPlayerPtr->evtq, &evt, 0);
  }
}

void stopAllSoundsWithMask(u8 arg0) {
  OSIntMask mask;
  ALEvent evt;
  N_AL_Struct81s *var_s0;

  mask = osSetIntMask(1U);
  for (var_s0 = soundState.unk0; var_s0 != NULL;
       var_s0 = (N_AL_Struct81s *)var_s0->node.next) {
    evt.type = 0x400;
    ((s32 *)&evt.msg)[0] = (s32)var_s0;
    if ((var_s0->unk3F & arg0) == arg0) {
      var_s0->unk3F &= ~(0x10);
      alEvtqPostEvent(&soundPlayerPtr->evtq, &evt, 0);
    }
  }
  osSetIntMask(mask);
}

void stopAllSounds(void) { stopAllSoundsWithMask(1); }

void stopAllSoundsWithPriority(void) { stopAllSoundsWithMask(0x11); }

void stopAllSoundsWithFlag(void) { stopAllSoundsWithMask(3); }

void postSoundEvent(s32 arg0, s16 type, s32 arg2) {
  ALEvent sp18;
  if (arg0) {
    sp18.type = type;
    ((s32 *)&sp18.msg)[0] = arg0;
    ((s32 *)&sp18.msg)[1] = arg2;

    alEvtqPostEvent(&soundPlayerPtr->evtq, &sp18, 0);
  }
}

s32 getSoundVolume(u8 arg0) { return (u16)soundVolumes[arg0]; }

void setSoundVolume(u8 arg0, u16 arg1) {
  N_AL_Struct81s *var_s0;
  s32 pad30;
  ALEvent evt;

  var_s0 = soundState.unk0;
  soundVolumes[arg0] = arg1;
  while (var_s0 != NULL) {
    if ((var_s0->unk8->keyMap->keyMin & 0x3F) == arg0) {
      evt.type = 0x800;
      ((s32 *)&evt.msg)[0] = var_s0;
      alEvtqPostEvent(&soundPlayerPtr->evtq, &evt, 0);
    }
    var_s0 = (N_AL_Struct81s *)var_s0->node.next;
  }
}

void setSoundMode(s32 arg0) { soundMode = arg0; }
