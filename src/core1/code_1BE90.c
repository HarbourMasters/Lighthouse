#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include "version.h"

//comusic.c

extern musicTrack_setTempo(u8, s32);

void comusic_fadeMainTrackWithDelay(s32, f32);
bool musicTrack_isStopped(u8);
void musicTrack_setVolume(u8, s32);
void musicTrack_setIndexCopy(u8, s32);
void comusic_fadeTrackWithArgs(enum comusic_e, s32, s32, f32, char *, s32);
void comusic_fadeTrackWithDetails(enum comusic_e, s32, s32, f32, s32 *, char *,
                                  s32);
void comusicPlayer_free(void);
void comusic_updateTrackWithArgs(s32, s32, s32);
void comusic_stopTrack(enum comusic_e);
void comusic_fadeTrack(enum comusic_e, s32, s32, s32);
void *freelist_next(FLA_s **, s32 *);

/* .bss */
CoMusic *activeTrackPtr = NULL; // active track ptr
int trackUpdateFlag = 0;

/* .code */
CoMusic *comusic_find_track(enum comusic_e track_id) {
  CoMusic *iMusPtr;
  CoMusic *freeSlotPtr;

  freeSlotPtr = NULL;
  for (iMusPtr = activeTrackPtr + 1; iMusPtr < activeTrackPtr + 5; iMusPtr++) {
    if (track_id == iMusPtr->track_id) {
      return iMusPtr;
    }
    if (freeSlotPtr == 0) {
      if ((s32)iMusPtr->track_id < 0) {
        freeSlotPtr = iMusPtr;
      }
    }
  }
  return freeSlotPtr;
}

void comusic_initTrack(CoMusic *this, s32 arg1, s32 arg2) {
  s32 sp2C;
  s32 i;
  struct12s *tmp;

  freelist_clear(this->unk18);
  for (i = 0; i < 0xE; i++) {
    this->unk1C[i] = 0;
  }
  tmp = (struct12s *)freelist_next(&this->unk18, &sp2C);
  tmp->unk0 = arg1;
  tmp->unk1 = arg2;
}

void comusic_resetTrack(CoMusic *this, s32 arg1) {
  comusic_initTrack(this, arg1, arg1);
}

void comusic_clearTrack(CoMusic *this) {
  comusic_resetTrack(this, musicTrack_getVolume(this->track_id));
  this->track_id = -1;
  this->unk14 = 0;
  this->unk15 = 0;
  musicTrack_setIndexCopy(this - activeTrackPtr, -1);
}

void comusicPlayer_init(void) {
  CoMusic *iPtr;
  s32 i;

  if (activeTrackPtr != NULL)
    comusicPlayer_free();

  activeTrackPtr = (CoMusic *)heap_malloc(6 * sizeof(CoMusic));
  for (iPtr = activeTrackPtr; iPtr < activeTrackPtr + 6; iPtr++) {
    iPtr->track_id = -1;
    iPtr->unk8 = 0;
    iPtr->unk12 = 0;
    iPtr->unkC = 0;
    iPtr->unk4 = 0.0f;
    iPtr->unk14 = 0;
    iPtr->unk15 = 0;
    iPtr->unk0 = 0.0f;
    iPtr->unk18 = (FREE_LIST(struct12s) *)freelist_new(sizeof(struct12s), 4);
    for (i = 0; i < 0xE; i++) {
      iPtr->unk1C[i] = 0;
    }
  }
}

// comusic_freeAll
void comusicPlayer_free(void) {
  CoMusic *iPtr;
  musicTrack_stopAll();
  musicTrack_unloadAll();

  for (iPtr = activeTrackPtr; iPtr < activeTrackPtr + 6; iPtr++) {
    freelist_free(iPtr->unk18);
  }
  bk_free(activeTrackPtr);
  activeTrackPtr = NULL;
}

s32 comusic_active_track_count(void) {
  CoMusic *iPtr;
  s32 cnt = 0;
  for (iPtr = activeTrackPtr; iPtr < activeTrackPtr + 6; iPtr++) {
    if (iPtr->track_id >= 0)
      cnt++;
  }
  return cnt;
}

void comusicPlayer_update(void) {
  s32 temp_lo;
  CoMusic *var_s0;
  f32 dt;

  dt = time_getDelta();
  for (var_s0 = activeTrackPtr; var_s0 < &activeTrackPtr[6]; var_s0++) {
    if (var_s0->track_id >= 0) {
      temp_lo = var_s0 - activeTrackPtr;
      var_s0->unk4 = ml_min_f(var_s0->unk4 + dt, 600.0f);
      if ((var_s0->unk4 > 1.0f) && musicTrack_isStopped(temp_lo)) {
        comusic_stopTrack(var_s0->track_id);
      }
    }
  }
  musicTrack_update();
  if (!trackUpdateFlag)
    return;

  trackUpdateFlag = FALSE;
  for (var_s0 = activeTrackPtr; var_s0 < &activeTrackPtr[6]; var_s0++) {
    if (var_s0->track_id >= 0) {
      if (var_s0->unk12 != 0) {
        temp_lo = var_s0 - activeTrackPtr;
        if (var_s0->unk0 > 0.0f) {
          var_s0->unk0 -= time_getDelta();
          trackUpdateFlag = TRUE;
        } else if (var_s0->unk12 < 0) {
          var_s0->unk8 += var_s0->unk12;
          if (var_s0->unk15 && (var_s0->unkC == 0) && (var_s0->unk8 <= 0)) {
            comusic_clearTrack(var_s0);
            continue;
          } else {
            if (var_s0->unkC >= var_s0->unk8) {
              var_s0->unk8 = var_s0->unkC;
              var_s0->unk12 = 0;
            } else {
              trackUpdateFlag = TRUE;
            }
            musicTrack_setVolume(temp_lo, (s16)var_s0->unk8);
          }
        } else if (var_s0->unk8 < var_s0->unkC) {
          if (var_s0->unk8 == 0) {
            var_s0->unk4 = 0.0f;
          }
          var_s0->unk8 += var_s0->unk12;
          if (var_s0->unk8 >= var_s0->unkC) {
            var_s0->unk8 = var_s0->unkC;
            var_s0->unk12 = 0;
          } else {
            trackUpdateFlag = TRUE;
          }
          musicTrack_setVolume(temp_lo, (s16)var_s0->unk8);
        } else {
          var_s0->unk12 = 0;
        }
      }
    }
  }
}

void comusic_findNextTrack(CoMusic *this, s32 *arg1, s32 *arg2) {
  int i;
  int cnt = freelist_size(this->unk18);
  s32 tmp_s1 = 0x7FFF;
  s32 tmp_s2 = 0x40000000;
  struct12s *tmp_ptr;

  for (i = 1; i < cnt; i++) {
    if (freelist_elementIsAlive(this->unk18, i)) {
      tmp_ptr = (struct12s *)freelist_at(this->unk18, i);
      if (tmp_ptr->unk0 < tmp_s1 ||
          (tmp_s1 == tmp_ptr->unk0 && tmp_ptr->unk1 < tmp_s2)) {
        tmp_s1 = tmp_ptr->unk0;
        tmp_s2 = tmp_ptr->unk1;
      } // L80259F40
    }
  }
  *arg1 = tmp_s1;
  *arg2 = tmp_s2;
}

void comusic_updateTrack(CoMusic *self, s32 *arg1, s32 *arg2, s32 *arg3) {
  struct12s *temp_v0;
  f32 pad;
  s32 sp34;
  s32 var_s2;

  var_s2 = *arg1;
  sp34 = *arg2;
  if ((*arg3 != 0) && !freelist_elementIsAlive(self->unk18, *arg3)) {
    *arg3 = 0;
  }

  if (var_s2 < 0) {
    temp_v0 = (struct12s *)freelist_at(self->unk18, 1);
    if (temp_v0->unk0 < musicTrack_getVolume(self->track_id)) {
      var_s2 = musicTrack_getVolume(self->track_id);
    } else {
      var_s2 = temp_v0->unk0;
    }
    if (*arg3 != 0) {
      temp_v0 = (struct12s *)freelist_at(self->unk18, *arg3);
      *arg2 = temp_v0->unk1;
      freelist_freeElement(self->unk18, *arg3);
      *arg3 = 0;
      comusic_findNextTrack(self, arg1, &sp34);
      return;
    }
  }

  if (*arg3 == 0) {
    temp_v0 = (struct12s *)freelist_at(self->unk18, 1);
    if ((temp_v0->unk0 < var_s2) ||
        ((var_s2 == temp_v0->unk0) && (sp34 >= temp_v0->unk1))) {
      comusic_initTrack(self, var_s2, sp34);
    } else {
      freelist_next(&self->unk18, arg3);
    }
  }
  if (*arg3 != 0) {
    temp_v0 = (struct12s *)freelist_at(self->unk18, *arg3);
    temp_v0->unk0 = var_s2;
    temp_v0->unk1 = sp34;
  }
  comusic_findNextTrack(self, arg1, arg2);
}

void comusic_setMainTrack(enum comusic_e arg0, s32 arg1) {
  if (arg0 != activeTrackPtr[0].track_id) {
    musicTrack_setIndexCopy(0, arg0);
  }
  musicTrack_setVolume(0, (s16)arg1);
  activeTrackPtr[0].track_id = (s16)arg0;
  activeTrackPtr[0].unk8 = arg1;
  activeTrackPtr[0].unk0 = 0.0f;
  activeTrackPtr[0].unk12 = 0;
  activeTrackPtr[0].unk4 = 0.0f;
  activeTrackPtr[0].unk15 = 0;
  comusic_resetTrack(&activeTrackPtr[0], arg1);
}

void comusic_setMainTrackWithVolume(enum comusic_e arg0) {

  if (arg0 != activeTrackPtr[0].track_id) {
    musicTrack_setIndexCopy(0, arg0);
    activeTrackPtr[0].track_id = (s16)arg0;
    activeTrackPtr[0].unk8 = musicTrack_getVolume(arg0);
    activeTrackPtr[0].unk0 = 0.0f;
    activeTrackPtr[0].unk12 = 0;
    activeTrackPtr[0].unk4 = 0.0f;
    activeTrackPtr[0].unk15 = 0;
    comusic_resetTrack(&activeTrackPtr[0], activeTrackPtr[0].unk8);
  }
}

void comusic_setSecondaryTrack(s32 arg0) {
  CoMusic *music = &activeTrackPtr[5];
  s32 temp_v0;

  if (arg0 != music->track_id) {
    musicTrack_setIndexCopy(5, arg0);
    music->track_id = (s16)arg0;
    temp_v0 = musicTrack_getVolume(arg0);
    music->unk8 = temp_v0;
    music->unk12 = 0;
    music->unk15 = 0;
    music->unk0 = 0.0f;
    music->unk4 = 0.0f;
    comusic_resetTrack(music, temp_v0);
  }
}

void comusic_clearSecondaryTrack(void) { comusic_clearTrack(&activeTrackPtr[5]); }

void comusic_clearMainTrack(void) { comusic_clearTrack(&activeTrackPtr[0]); }

void comusic_fadeAllTracks(s32 arg0, s32 arg1) {
  s32 i;

  comusic_updateTrackWithArgs(arg0, arg1, 1);
  for (i = 1; i < 5; i++) {
    s16 val = (i + activeTrackPtr)->track_id; // Doesn't match with activeTrackPtr[i]
    if (val >= 0) {
      comusic_fadeTrack(val, arg0, arg1, 1);
    }
  }
}

void comusic_fadeAllTracksIfNotPlaying(s32 arg0, s32 arg1) {
  s32 i;

  if (activeTrackPtr->unk14 == 0) {
    comusic_updateTrackWithArgs(arg0, arg1, 1);
  }
  for (i = 1; i < 5; i++) {
    CoMusic *current = (i + activeTrackPtr); // Doesn't match with activeTrackPtr[i]
    if (current->track_id >= 0 && current->unk14 == 0) {
      comusic_fadeTrack(current->track_id, arg0, arg1, 1);
    }
  }
}

void comusic_fadeAllTracksWithArgs(s32 arg0, s32 arg1, s32 arg2) {
  s32 i;

  comusic_updateTrackWithArgs(arg0, arg1, arg2);
  for (i = 1; i < 5; i++) {
    s16 val = (i + activeTrackPtr)->track_id; // Doesn't match with activeTrackPtr[i]
    if (val >= 0) {
      comusic_fadeTrack(val, arg0, arg1, arg2);
    }
  }
}

void comusic_updateMainTrack(s32 arg0, s32 arg1, s32 *arg2) {
  if (activeTrackPtr[0].track_id >= 0) {
    comusic_updateTrack(&activeTrackPtr[0], &arg0, &arg1, arg2);
    if (arg0 != activeTrackPtr[0].unk8) {
      if (activeTrackPtr[0].unk8 < arg0) {
        activeTrackPtr[0].unk12 = arg1;
      } else {
        activeTrackPtr[0].unk12 = -arg1;
      }
      activeTrackPtr[0].unkC = arg0;
      trackUpdateFlag = 1;
    }
  }
}

void comusic_updateTrackWithArgs(s32 arg0, s32 arg1, s32 arg2) {
  comusic_updateMainTrack(arg0, arg1, &activeTrackPtr->unk1C[arg2]);
}

void playMusicWithFade(s32 arg0, s32 arg1) {
  comusic_updateTrackWithArgs(arg0, arg1, 0);
}

void comusic_playTrackWithVolume(enum comusic_e comusic_id, s32 volume,
                                 s32 arg2) {
  CoMusic *tmp_a2;
  s32 sp20;

  if (volume == -1) {
    volume = musicTrack_getVolume(comusic_id);
  }

  tmp_a2 = comusic_find_track(comusic_id);
  if (tmp_a2 == NULL)
    return;

  sp20 = (tmp_a2 - activeTrackPtr);
  if (tmp_a2->track_id < 0 || arg2) {
    switch (comusic_id) {
    case COMUSIC_15_EXTRA_LIFE_COLLECTED:
      if (map_get() == MAP_10_BGS_MR_VILE) {
        break;
      }
    case COMUSIC_3B_MINIGAME_VICTORY:
    case COMUSIC_3C_MINIGAME_LOSS:
      comusic_fadeMainTrackWithDelay(4000, 2.0f);
    }
    tmp_a2->track_id = comusic_id;
    tmp_a2->unk12 = 0;
    tmp_a2->unk15 = 0;
    tmp_a2->unk4 = 0.0f;
    comusic_resetTrack(tmp_a2, volume);
    musicTrack_setIndexCopy(sp20, comusic_id);
  }
  musicTrack_setVolume(sp20, (s16)volume);
  tmp_a2->unk8 = volume;
}

void comusic_playTrackWithDefaultVolume(enum comusic_e track_id, s32 volume) {
  comusic_playTrackWithVolume(track_id, volume, 0);
}

void comusic_playTrackWithVolumeOverride(enum comusic_e track_id, s32 volume) {
  comusic_playTrackWithVolume(track_id, volume, 1);
}

// comusic_queueTrack
void comusic_playTrack(enum comusic_e track_id) {
  CoMusic *trackPtr;
  s32 indx;

  trackPtr = comusic_find_track(track_id);
  if (trackPtr == NULL)
    return;

  indx = trackPtr - activeTrackPtr;
  if (trackPtr->track_id < 0) {
    trackPtr->track_id = track_id;
    trackPtr->unk12 = 0;
    trackPtr->unk4 = 0.0f;
    musicTrack_setIndexCopy(indx, track_id);
    comusic_resetTrack(trackPtr,
                       trackPtr->unk8 = musicTrack_getVolume(track_id));
  }
}

void comusic_playTrackWithDelay(enum comusic_e comusic_id, f32 delay1,
                                f32 delay2) {
  timedFunc_set_1(delay1, (GenFunction_1)comusic_playTrack, comusic_id);
  timedFunc_set_1(delay1 + delay2, (GenFunction_1)comusic_stopTrack,
                  comusic_id);
}

void comusic_stopTrack(enum comusic_e track_id) {
  CoMusic *trackPtr;

  trackPtr = comusic_find_track(track_id);
  if (trackPtr != NULL && trackPtr->track_id >= 0) {
    comusic_clearTrack(trackPtr);
  }
}

s32 comusic_stopMainTrackIfDone(void) {
  if (activeTrackPtr[0].unkC == 0 && activeTrackPtr[0].unk8 <= 0) {
    comusic_clearTrack(&activeTrackPtr[0]);
    return 1;
  }
  return 0;
}

s32 comusic_stopTrackIfDone(enum comusic_e track_id) {
  CoMusic *trackPtr;

  trackPtr = comusic_find_track(track_id);
  if (trackPtr != NULL && trackPtr->unkC == 0 && trackPtr->unk8 <= 0) {
    comusic_clearTrack(trackPtr);
    return 1;
  }
  return 0;
}

void comusic_setTrackFlag(enum comusic_e track_id, s32 arg1) {
  CoMusic *trackPtr;

  trackPtr = comusic_find_track(track_id);
  if (trackPtr != NULL) {
    trackPtr->unk14 = arg1;
  }
}

void comusic_setMainTrackFlag(s32 arg0) {
  if (activeTrackPtr[0].track_id >= 0) {
    activeTrackPtr[0].unk14 = arg0;
  }
}

void comusic_stopAllTracks(void) {
  CoMusic *trackPtr = &activeTrackPtr[0];

  while (trackPtr < &activeTrackPtr[6]) {
    if (trackPtr->track_id >= 0) {
      comusic_clearTrack(trackPtr);
    }
    trackPtr++;
  }
}

// dequeue_allTracks
void comusic_stopAllSecondaryTracks(void) {
  CoMusic *iPtr;

  for (iPtr = &activeTrackPtr[1]; iPtr < &activeTrackPtr[6]; iPtr++) {
    if (iPtr->track_id >= 0) {
      comusic_clearTrack(iPtr);
    }
  }
}

// dequeue_allTracks
void comusic_stopAllTracksIfNotPlaying(void) {
  CoMusic *iPtr;

  for (iPtr = &activeTrackPtr[0]; iPtr < &activeTrackPtr[6]; iPtr++) {
    if (iPtr->track_id >= 0 && !iPtr->unk14) {
      comusic_clearTrack(iPtr);
    }
  }
}

// dequeue_nonmainTracks
void comusic_stopAllSecondaryTracksIfNotPlaying(void) {
  CoMusic *iPtr;

  for (iPtr = &activeTrackPtr[1]; iPtr < &activeTrackPtr[6]; iPtr++) {
    if (iPtr->track_id >= 0 && !iPtr->unk14) {
      comusic_clearTrack(iPtr);
    }
  }
}

// dequeue_track?
void comusic_stopTrackById(enum comusic_e track_id) {
  CoMusic *trackPtr;

  if (trackPtr = comusic_find_track(track_id)) {
    trackPtr->unk15 = 1;
    if (!trackPtr->unk8)
      comusic_clearTrack(trackPtr);
  }
}

void comusic_stopMainTrackById(void) {
  activeTrackPtr[0].unk15 = 1;
  if (!activeTrackPtr[0].unk8) {
    comusic_clearTrack(&activeTrackPtr[0]);
  }
}

void comusic_fadeTrackWithArgsNoDelay(enum comusic_e comusic_id, s32 arg1,
                                      s32 arg2) {
  comusic_fadeTrackWithArgs(comusic_id, arg1, arg2, 0.0f, "comusic.c",
                            VER_SELECT(0x39e, 0x39f, 0, 0));
}

void comusic_fadeTrackWithDetailsNoDelay(enum comusic_e comusic_id, s32 arg1,
                                         s32 arg2, s32 arg3) {
  comusic_fadeTrackWithDetails(comusic_id, arg1, arg2, 0.0f, arg3, "comusic.c",
                               VER_SELECT(0x3a3, 0x3a4, 0, 0));
}

void comusic_fadeTrack(enum comusic_e comusic_id, s32 arg1, s32 arg2,
                       s32 arg3) {
  comusic_fadeTrackWithDetails(
      comusic_id, arg1, arg2, 0.0f,
      (s32) & (comusic_find_track(comusic_id)->unk1C[arg3]), "comusic.c",
      VER_SELECT(0x3aa, 0x3ab, 0, 0));
}

void comusic_fadeTrackWithArgs(enum comusic_e comusic_id, s32 arg1, s32 arg2,
                               f32 arg3, char *arg4, s32 char5) {
  comusic_fadeTrackWithDetails(comusic_id, arg1, arg2, 0.0f,
                               (s32)comusic_find_track(comusic_id)->unk1C,
                               "comusic.c", VER_SELECT(0x3b1, 0x3b2, 0, 0));
}

void comusic_fadeTrackWithDetails(enum comusic_e comusic_id, s32 arg1, s32 arg2,
                                  f32 arg3, s32 *arg4, char *arg5, s32 arg6) {
  CoMusic *trackPtr;
  u32 slot_index;

  // get track location
  trackPtr = comusic_find_track(comusic_id);
  if (trackPtr == NULL)
    return;

  // check if track is loaded in slot
  if (trackPtr->track_id < 0) { // Track not loaded
    if (arg1 == 0)
      return;
    slot_index = (trackPtr - activeTrackPtr);
    musicTrack_setIndexCopy(slot_index, comusic_id);
    trackPtr->track_id = comusic_id;
    trackPtr->unk8 = 0;
    trackPtr->unk15 = 0;
    trackPtr->unk4 = 0.0f;
    comusic_resetTrack(trackPtr, 0);
    musicTrack_setVolume(slot_index, 0);
  }
  comusic_updateTrack(trackPtr, &arg1, &arg2, arg4);
  trackPtr->unk0 = arg3;
  trackPtr->unk12 = (trackPtr->unk8 < arg1) ? arg2 : -arg2;
  trackPtr->unkC = arg1;
  trackUpdateFlag = 1;
}

// comusic_trackQueued
int comusic_isTrackQueued(enum comusic_e arg0) {
  CoMusic *trackPtr = comusic_find_track(arg0);
  return (trackPtr == NULL || trackPtr->track_id == -1) ? 0 : 1;
}

// comusic_isPrimaryTrack
int comusic_isMainTrack(enum comusic_e arg0) {
  return activeTrackPtr[0].track_id == arg0;
}

s32 comusic_getTrackIndex(enum comusic_e id) {
  CoMusic *ptr = comusic_find_track(id);
  return ptr - activeTrackPtr;
}

void comusic_fadeMainTrack(s32 arg0, f32 arg1) {
  playMusicWithFade(0, arg0);
  timedFunc_set_2(arg1, (GenFunction_2)playMusicWithFade, -1, arg0);
}

void comusic_fadeMainTrackWithDelay(s32 arg0, f32 arg1) {
  comusic_fadeAllTracksWithArgs(0, arg0, 6);
  timedFunc_set_3(arg1, (GenFunction_3)comusic_fadeAllTracksWithArgs, -1, arg0,
                  6);
}

void comusic_setTrackVolume(enum comusic_e track_id, s32 arg1) {
  CoMusic *ptr = comusic_find_track(track_id);

  if (!ptr)
    return;
  musicTrack_setTempo(ptr - activeTrackPtr, arg1);
}

int comusic_isTrackPlaying(void) {
  s32 out = musicTrack_getEvent(0, 0x6A, 0);
  if (out)
    musicTrack_setEventById(0, 0x6A, 0);
  return out;
}

void comusic_defrag(void) {
  CoMusic *iPtr;

  if (!activeTrackPtr)
    return;

  for (iPtr = &activeTrackPtr[0]; iPtr < &activeTrackPtr[6]; iPtr++) {
    iPtr->unk18 = (FREE_LIST(struct12s) *)freelist_defrag(iPtr->unk18);
  }
  activeTrackPtr = (CoMusic *)defrag(activeTrackPtr);
}
