#ifndef __ANIM_CTRL_H__
#define __ANIM_CTRL_H__

#include <ultra64.h>
#include "generic.h"

#ifndef NONMATCHING
#define anctrl_start(ctrl, file, line) _anctrl_start(ctrl, file, line)
#else
#define anctrl_start(ctrl, file, line) _anctrl_start(ctrl, __FILE__, __LINE__)
#endif

enum anctrl_playback_e{
    ANIMCTRL_ONCE = 1,
    ANIMCTRL_LOOP = 2,
    ANIMCTRL_STOPPED = 3,
    ANIMCTRL_SUBRANGE_LOOP = 4
};

typedef struct animation_s{
    GenFunction_2 matrices;
    uintptr_t unk4;
    u8      unk8;
    u8      unk9;
    s16     animcache_index[3];
    u32     index;
    f32     timer;
    f32     duration;
    u8      reset;
    u8      triple_buffer;
    u8      unk1E;
    u8      unk1F;
} Animation;

typedef struct anctrl_s{
    Animation *animation;
    f32     timer;
    f32     subrange_start;
    f32     subrange_end;
    f32     animation_duration;
    f32     transition_duration;
    float   start;
    s32     index;
    u8      playback_type;
    u8      playback_direction;
    u8      smooth_transition;
    u8      unk23;
    u8      unk24;
    u8      default_start;
    u8      pad26[2];
} AnimCtrl;

typedef struct actorAnimCtrl_s{
    AnimCtrl anctrl;
    Animation animation;
} ActorAnimCtrl;

AnimCtrl *anctrl_new(s32 arg0);
void anctrl_free(AnimCtrl * ctrl);
void anctrl_update(AnimCtrl *ctrl);
AnimCtrl *anctrl_defrag(AnimCtrl *ctrl);
void anctrl_setIndex(AnimCtrl *ctrl, enum asset_e index);
Animation *anctrl_getAnimPtr(AnimCtrl *ctrl);
void func_8028746C(AnimCtrl *ctrl, GenFunction_2 arg1);
void func_8028748C(AnimCtrl *ctrl, uintptr_t arg1);
void anctrl_reset(AnimCtrl *ctrl);
void __anctrl_gotoStart(AnimCtrl *ctrl);
void _anctrl_start(AnimCtrl * ctrl, char *file, s32 line);
void anctrl_setAnimTimer(AnimCtrl *ctrl, f32 timer);
void anctrl_setPlaybackType(AnimCtrl *ctrl, enum anctrl_playback_e arg1);
void anctrl_setDirection(AnimCtrl *ctrl, s32 arg1);
void anctrl_setSmoothTransition(AnimCtrl *ctrl, s32 arg1);
void anctrl_setDuration(AnimCtrl *ctrl, f32 arg1);
void anctrl_setTransitionDuration(AnimCtrl *ctrl, f32 arg1);
void anctrl_setSubRange(AnimCtrl *ctrl, f32 start, f32 end);
void anctrl_getSubRange(AnimCtrl *ctrl, f32 *startPtr, f32 *endPtr);
void anctrl_setStart(AnimCtrl *ctrl, f32 arg1);
void func_80287784(AnimCtrl *ctrl, s32 arg1);
enum asset_e anctrl_getIndex(AnimCtrl *ctrl);
enum anctrl_playback_e anctrl_getPlaybackType(AnimCtrl *ctrl);
s32 anctrl_isPlayedForwards(AnimCtrl *ctrl);
s32 anctrl_isSmoothTransistion(AnimCtrl *ctrl);
f32 anctrl_getDuration(AnimCtrl *ctrl);
f32 anctrl_getTransistionDuration(AnimCtrl *ctrl);
f32 anctrl_getAnimTimer(AnimCtrl *ctrl);
f32 anctrl_getTimer(AnimCtrl *ctrl);
void  anctrl_setTimer(AnimCtrl *ctrl, f32 arg1);
s32  anctrl_8028780C(f32 position[3], s32 arg1);
void anctrl_drawSetup(AnimCtrl *ctrl, f32 *arg1, s32 arg2);
s32 anctrl_isStopped(AnimCtrl *ctrl);
int anctrl_isAt(AnimCtrl *ctrl, f32 arg1);
s32 anctrl_isContiguous(AnimCtrl *ctrl);
#endif
