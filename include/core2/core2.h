#ifndef __CORE_2_H__
#define __CORE_2_H__

#include "bool.h"
#include "core2/animmtxlist.h"
#include "core2/timedfunc.h"
#include "gc/gc.h"
#include "core2/print.h"
#include "core2/anctrl.h"
#include "core2/modelRender.h"
#include "core2/code_C31A0.h"
#include "core2/animtexturecache.h"
#include "core2/fla.h"
#include "model.h"

void code35520_getDistanceVectors(s32 id, s32 *vec11, s32 *vec12, s32 *vec13, s32 *vec21, s32 *vec22, s32 *vec23, s32 *vec31, s32 *vec32, s32 *vec33);
void code35520_selectTable(void);

void leveloverlay_drawCallback(Gfx **gfx, Mtx **mtx, Vtx **vtx);
enum overlay_e leveloverlay_getOverlayFromLevel(enum level_e lvl);
void leveloverlay_releaseCallback(bool flag);
void leveloverlay_initCallback(bool flag);
void leveloverlay_releaseCallback_NotFP(void);
void leveloverlay_releaseCallback_OnlyFP(void);
void leveloverlay_initCallback_OnlyFP(void);
void leveloverlay_initCallback_NotFP(void);
void leveloverlay_debug(void);
void leveloverlay_init(void);
void leveloverlay_unk14Callback(s32 arg0, s32 arg1);
void leveloverlay_updateCallback(void);

void actors_applyFromSavestate(void *savestate_ptr, ActorListSaveState *savestate_actorlist_ptr);

s32 cubeList_getOrSetNextProp2Flags(s32 op);
void cubeList_sort(bool absolute_positon);

u32 mapSpecificFlags_getAll(void);
void mapSpecificFlags_setAll(u32 flags);

void mapSavestate_init(void);
void mapSavestate_clearAll(void);
void mapSavestate_defrag(void);
void mapSavestate_save(enum map_e map);
void mapSavestate_apply(enum map_e map);

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
