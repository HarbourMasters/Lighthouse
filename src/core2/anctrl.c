#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include "animation.h"
#include "core2/anctrl.h"

/* .code */
void anctrl_updateSmoothTransition(AnimCtrl *actrl) {
  Animation *anim;
  f32 duration;

  if (actrl->smooth_transition) {
    anim = anctrl_getAnimPtr(actrl);
    duration = anim_getDuration(anim);
    if (duration < 1.0f) {
      anim_setDuration(
          anim,
          ml_min_f(1.0f, time_getDelta() / anctrl_getTransistionDuration(actrl) +
                             duration));
    }
  }
}

static void anctrl_update_looped(AnimCtrl *actrl) {
  Animation *anim;
  f32 delta;
  f32 tmp;

  anim = anctrl_getAnimPtr(actrl);
  anctrl_updateSmoothTransition(actrl);
  actrl->timer = anim_getTimer(anim);
  delta = time_getDelta() / anctrl_getDuration(actrl);
  if (actrl->playback_direction == 0) {
    delta = -delta;
  }
  tmp = actrl->timer + delta;
  if (tmp < 0.0f) {
    tmp += 1.0f;
  }
  tmp -= (f32)(s32)tmp; // 0.0f to 1.0f
  anim_setTimer(anim, tmp);
}

void anctrl_updateSubrangeLoop(AnimCtrl *actrl) {
  Animation *anim;
  f32 delta;
  f32 tmpf14;
  f32 f_range;
  f32 f_percent;

  anim = anctrl_getAnimPtr(actrl);
  anctrl_updateSmoothTransition(actrl);
  actrl->timer = anim_getTimer(anim);
  ;
  delta = time_getDelta() / anctrl_getDuration(actrl);
  if (actrl->playback_direction == 0) {
    delta = -delta;
  }
  tmpf14 = actrl->timer + delta;
  if (actrl->subrange_end <= tmpf14) {
    f_range = actrl->subrange_end - actrl->subrange_start;
    f_percent = (tmpf14 - actrl->subrange_start) / f_range;
    tmpf14 = actrl->subrange_start + (f_percent - (f32)(s32)f_percent) * f_range;
  }
  anim_setTimer(anim, tmpf14);
}

void anctrl_updateOnce(AnimCtrl *actrl) {
  Animation *anim;
  f32 phi_f0;
  f32 phi_f2;

  anim = anctrl_getAnimPtr(actrl);
  anctrl_updateSmoothTransition(actrl);
  actrl->timer = anim_getTimer(anim);
  phi_f2 = time_getDelta() / anctrl_getDuration(actrl);
  if (actrl->playback_direction == 0) {
    phi_f2 = -phi_f2;
  }
  phi_f0 = actrl->timer + phi_f2;

  if (phi_f0 < 0.0f) {
    phi_f0 = 0.0f;
    anctrl_setPlaybackType(actrl, ANIMCTRL_STOPPED);

  } else {
    if ((actrl->subrange_end < phi_f0) || (0.999999 < (f64)phi_f0)) {
      if (actrl->subrange_end < phi_f0)
        phi_f0 = actrl->subrange_end;
      if (0.999999 < (f64)phi_f0)
        phi_f0 = 0.9999989867210388f; // D_80373E00
      anctrl_setPlaybackType(actrl, ANIMCTRL_STOPPED);
    } else {
      phi_f0 = phi_f0 - (f32)(s32)phi_f0;
    }
  }
  anim_setTimer(anim, phi_f0);
}

AnimCtrl *anctrl_new(s32 arg0) { // new
  ActorAnimCtrl *actrl;

  actrl = (ActorAnimCtrl *)heap_malloc(anim_getSize() + 0x28);
  actrl->anctrl.animation = &actrl->animation;
  anim_new(&actrl->animation, 1);
  actrl->anctrl.playback_type = 0;
  actrl->anctrl.index = 0;
  actrl->anctrl.default_start = TRUE;
  actrl->anctrl.timer = 0.0f;
  actrl->anctrl.start = 0.0f;
  anctrl_setUnk23(&actrl->anctrl, func_8030C77C());
  anctrl_setSubRange(&actrl->anctrl, 0.0f, 1.0f);
  anctrl_setDuration(&actrl->anctrl, 2.0f);
  anctrl_setTransitionDuration(&actrl->anctrl, 0.2f);
  anctrl_setSmoothTransition(&actrl->anctrl, 1);
  anctrl_setDirection(&actrl->anctrl, mvmt_dir_forwards);
  return &actrl->anctrl;
}

void anctrl_free(AnimCtrl *actrl) { // free
  anim_release(actrl->animation);
  bk_free(actrl);
}

void anctrl_update(AnimCtrl *actrl) { // update
  switch (actrl->playback_type) {
  case 0:
    break;
  case ANIMCTRL_ONCE: // once
    anctrl_updateOnce(actrl);
    break;
  case ANIMCTRL_LOOP: // loop
    anctrl_update_looped(actrl);
    break;
  case ANIMCTRL_SUBRANGE_LOOP:
    anctrl_updateSubrangeLoop(actrl);
    break;
  case ANIMCTRL_STOPPED: // stopped
    anctrl_updateSmoothTransition(actrl);
    break;
  }
}

AnimCtrl *anctrl_defrag(AnimCtrl *actrl) { // bk_realloc
  ActorAnimCtrl *full_struct;
  full_struct = (ActorAnimCtrl *)defrag(actrl);
  full_struct->anctrl.animation = &full_struct->animation;
  return &full_struct->anctrl;
}

void anctrl_setIndex(AnimCtrl *actrl, enum asset_e index) {
  actrl->index = index;
}

Animation *anctrl_getAnimPtr(AnimCtrl *actrl) { return actrl->animation; }

void anctrl_setCallback(AnimCtrl *actrl, void (*arg1)(s32, s32)) {
  anim_80289790(actrl->animation, arg1);
}

void anctrl_setCallbackArg(AnimCtrl *actrl, s32 arg1) {
  anim_80289798(actrl->animation, arg1);
}

void anctrl_reset(AnimCtrl *actrl) {
  actrl->playback_type = ANIMCTRL_LOOP;
  actrl->default_start = TRUE;
  actrl->timer = 0.0;
  actrl->start = 0.0;
  anctrl_setSmoothTransition(actrl, 1);
  anctrl_setSubRange(actrl, 0.0, 1.0);
  anctrl_setDuration(actrl, 2.0);
  anctrl_setTransitionDuration(actrl, 0.2);
  anctrl_setDirection(actrl, mvmt_dir_forwards);
}

void anctrl_gotoStart(AnimCtrl *actrl) {
  if (actrl->default_start) {
    if (actrl->playback_direction)
      anim_setTimer(actrl->animation, 0.0f);
    else
      anim_setTimer(actrl->animation, 0.99999899f);
  } else {
    anim_setTimer(actrl->animation, actrl->start);
  }
  actrl->timer = anim_getTimer(actrl->animation);
}

void _anctrl_start(AnimCtrl *actrl, char *file, s32 line) {
  if (actrl->smooth_transition && anim_getIndex(actrl->animation) != 0) {
    anim_resetSmooth(actrl->animation);
    anim_setIndex(actrl->animation, actrl->index);
    anctrl_gotoStart(actrl);
    anim_setDuration(actrl->animation, 0.0f);
  } else {
    anim_resetNow(actrl->animation);
    anim_setIndex(actrl->animation, actrl->index);
    anctrl_gotoStart(actrl);
    anim_setDuration(actrl->animation, 1.0f);
  }
}

void anctrl_setAnimTimer(AnimCtrl *actrl, f32 timer) {
  anim_setTimer(actrl->animation, timer);
}

void anctrl_setPlaybackType(AnimCtrl *actrl, enum anctrl_playback_e arg1) {
  actrl->playback_type = arg1;
}

void anctrl_setDirection(AnimCtrl *actrl, s32 arg1) {
  actrl->playback_direction = arg1;
}

void anctrl_setSmoothTransition(AnimCtrl *actrl, s32 arg1) {
  actrl->smooth_transition = arg1;
}

void anctrl_setDuration(AnimCtrl *actrl, f32 arg1) {
  if (IO_READ(0x238) - 0x10000003) {
    arg1 += 3.0f;
  }
  actrl->animation_duration = arg1;
}

void anctrl_setTransitionDuration(AnimCtrl *actrl, f32 arg1) {
  actrl->transition_duration = arg1;
}

void anctrl_setSubRange(AnimCtrl *actrl, f32 start, f32 end) {
  actrl->subrange_start = start - (f32)(s32)start;
  actrl->subrange_end = (end != 1.0) ? end - (f32)(s32)end : end;
}

void anctrl_getSubRange(AnimCtrl *actrl, f32 *startPtr, f32 *endPtr) {
  *startPtr = actrl->subrange_start;
  *endPtr = actrl->subrange_end;
}

void anctrl_setStart(AnimCtrl *actrl, f32 start_position) {
  if (start_position == 1.0)
    start_position = 0.9999989867210388f; // D_80373E18

  actrl->start = start_position;
  actrl->default_start = FALSE;
}

void anctrl_setUnk23(AnimCtrl *actrl, s32 arg1) {
  actrl->unk23 = arg1;
  actrl->unk24 = 0;
}

enum asset_e anctrl_getIndex(AnimCtrl *actrl) {
  return anim_getIndex(actrl->animation);
}

enum anctrl_playback_e anctrl_getPlaybackType(AnimCtrl *actrl) {
  return actrl->playback_type;
}

s32 anctrl_isPlayedForwards(AnimCtrl *actrl) { return actrl->playback_direction; }

s32 anctrl_isSmoothTransistion(AnimCtrl *actrl) {
  return actrl->smooth_transition;
}

f32 anctrl_getDuration(AnimCtrl *actrl) { return actrl->animation_duration; }

f32 anctrl_getTransistionDuration(AnimCtrl *actrl) {
  return actrl->transition_duration;
}

f32 anctrl_getAnimTimer(AnimCtrl *actrl) {
  return anim_getTimer(actrl->animation);
}

f32 anctrl_getTimer(AnimCtrl *actrl) { return actrl->timer; }

void anctrl_setTimer(AnimCtrl *actrl, f32 arg1) { actrl->timer = arg1; }

s32 anctrl_8028780C(f32 position[3], s32 arg1) { return 0; }

void anctrl_drawSetup(AnimCtrl *actrl, f32 *position, s32 arg2) {
  s32 map;
  map = map_get();
  if (map != MAP_1E_CS_START_NINTENDO && map != MAP_1F_CS_START_RAREWARE &&
      map != MAP_20_CS_END_NOT_100 && actrl->unk23 != 0 && position != NULL) {
    actrl->unk24 = actrl->unk24 - 1;
    if (actrl->unk24 == 0xFF) {
      actrl->unk24 = anctrl_8028780C(position, arg2);
    } else {
      anim_drawSetup(actrl->animation);
      return;
    }
  }
  anim_update(actrl->animation);
}

s32 anctrl_isStopped(AnimCtrl *actrl) {
  return anctrl_getPlaybackType(actrl) == ANIMCTRL_STOPPED;
}

int anctrl_isAt(AnimCtrl *actrl, f32 arg1) {
  int retval;
  f32 f0 = anim_getTimer(actrl->animation);

  if (f0 == actrl->timer) {
    return 0;
  } else {
    if (actrl->playback_direction != 0) { // forward direction
      if (actrl->timer < f0) {
        return actrl->timer <= arg1 && arg1 < f0; // arg1 between last animation
                                                 // time and new animation time
      } else {
        return actrl->timer <= arg1 ||
               arg1 < f0; // animation just looped, arg1 outside of loop
      }
    } else {
      if (f0 < actrl->timer) {
        return arg1 <= actrl->timer && f0 < arg1;
      } else {
        return arg1 <= actrl->timer || f0 < arg1;
      }
    }
  }

  return retval;
}

s32 anctrl_isContiguous(AnimCtrl *actrl) {
  return (s32)actrl->animation - (s32)actrl == 0x28;
}
