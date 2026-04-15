#ifndef __CORE_2_H__
#define __CORE_2_H__

#include "core2/timedfunc.h"
#include "gc/gc.h"
#include "core2/print.h"
#include "core2/anctrl.h"

void func_80351A04(Struct68s *arg0, s32 arg1);
void func_80351A14(Struct68s *arg0, Struct68DrawMethod arg1);
void func_8035179C_copyPosition(Struct68s* arg0, f32 arg1[3]);
void func_80351814(Struct68s *arg0, f32 arg1[3]);
f32  func_80351830(Struct68s *arg0);


extern void sfxsource_setSampleRate(u8, s32);

void gsworld_draw(Gfx **gdl, Mtx **mptr, Vtx **vptr);
void gsworld_stub1(s32 arg0, s32 arg1, s32 arg2);
enum map_e gsworld_getMap(void);
s32  gsworld_getExit(void);
void gsworld_transitionToExit(s32 exit);
s32  gsworld_getUnk0(void);
void gsworld_free(void);
void gsworld_set(enum map_e map, s32 exit, s32 reload);
void gsworld_reload(void);
void gsworld_stub2(void);
void gsworld_setUnk0(s32 value);
s32  gsworld_update(void);
void gsworld_setEnableUpdate(s32 value);
s32  gsworld_getEnableUpdate(void);
void gsworld_setEnableDraw(s32 value);
s32  gsworld_getEnableDraw(void);
void gsworld_load(enum map_e map_id);
void gsworld_stub3(s32 arg0);

#endif
