#ifndef __ANIM_CTRL_H__
#define __ANIM_CTRL_H__

#include <ultra64.h>

#include "generic.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifndef NONMATCHING
#define anctrl_start(actrl, file, line) _anctrl_start(actrl, file, line)
#else
#define anctrl_start(actrl, file, line) _anctrl_start(actrl, __FILE__, __LINE__)
#endif

enum anctrl_playback_e{
    ANIMCTRL_ONCE = 1,
    ANIMCTRL_LOOP = 2,
    ANIMCTRL_STOPPED = 3,
    ANIMCTRL_SUBRANGE_LOOP = 4
};

typedef struct{
    MtxF mtx_0;
    s32 size_40;
    s32 capacity_44;
    MtxF data[];
}AnimMtxList;

typedef struct animation_s{
    GenFunction_2 matrices;
    s32     unk4;
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
void anctrl_free(AnimCtrl * actrl);
void anctrl_update(AnimCtrl *actrl);
AnimCtrl *anctrl_defrag(AnimCtrl *actrl);
void anctrl_setIndex(AnimCtrl *actrl, enum asset_e index);
Animation *anctrl_getAnimPtr(AnimCtrl *actrl);
void anctrl_setCallback(AnimCtrl *actrl,  void (* arg1)(s32,s32));
void anctrl_setCallbackArg(AnimCtrl *actrl, s32 arg1);
void anctrl_reset(AnimCtrl *actrl);
void anctrl_gotoStart(AnimCtrl *actrl);
void _anctrl_start(AnimCtrl * actrl, char *file, s32 line);
void anctrl_setAnimTimer(AnimCtrl *actrl, f32 timer);
void anctrl_setPlaybackType(AnimCtrl *actrl, enum anctrl_playback_e arg1);
void anctrl_setDirection(AnimCtrl *actrl, s32 arg1);
void anctrl_setSmoothTransition(AnimCtrl *actrl, s32 arg1);
void anctrl_setDuration(AnimCtrl *actrl, f32 arg1);
void anctrl_setTransitionDuration(AnimCtrl *actrl, f32 arg1);
void anctrl_setSubRange(AnimCtrl *actrl, f32 start, f32 end);
void anctrl_getSubRange(AnimCtrl *actrl, f32 *startPtr, f32 *endPtr);
void anctrl_setStart(AnimCtrl *actrl, f32 arg1);
void anctrl_setUnk23(AnimCtrl *actrl, s32 arg1);
enum asset_e anctrl_getIndex(AnimCtrl *actrl);
enum anctrl_playback_e anctrl_getPlaybackType(AnimCtrl *actrl);
s32 anctrl_isPlayedForwards(AnimCtrl *actrl);
s32 anctrl_isSmoothTransistion(AnimCtrl *actrl);
f32 anctrl_getDuration(AnimCtrl *actrl);
f32 anctrl_getTransistionDuration(AnimCtrl *actrl);
f32 anctrl_getAnimTimer(AnimCtrl *actrl);
f32 anctrl_getTimer(AnimCtrl *actrl);
void  anctrl_setTimer(AnimCtrl *actrl, f32 arg1);
s32  anctrl_8028780C(f32 position[3], s32 arg1);
void anctrl_drawSetup(AnimCtrl *actrl, f32 *arg1, s32 arg2);
s32 anctrl_isStopped(AnimCtrl *actrl);
int anctrl_isAt(AnimCtrl *actrl, f32 arg1);
s32 anctrl_isContiguous(AnimCtrl *actrl);

#ifdef __cplusplus
}
#endif

#endif
