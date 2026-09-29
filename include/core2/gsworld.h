#ifndef BANJO_KAZOOIE_CORE2_GSWORLD_H
#define BANJO_KAZOOIE_CORE2_GSWORLD_H

#include <ultra64.h>
#include "bool.h"
#include "enums.h"

void gsworld_draw(Gfx** gfx, Mtx **mtx, Vtx **vtx);
void gsworld_stub1(s32 arg0, s32 arg1, s32 arg2);
enum map_e gsworld_getMap(void);
s32 gsworld_getExit();
void gsworld_transitionToExit(s32 exit);
s32 gsworld_getUnk0();
void gsworld_free(void);
void gsworld_set(enum map_e map, s32 exit, bool reload);
void gsworld_reload(void);
void gsworld_stub2(void);
void gsworld_setUnk0(s32 value);
s32 gsworld_update(void);
void gsworld_setEnableUpdate(bool value);
bool gsworld_getEnableUpdate();
void gsworld_setEnableDraw(bool value);
bool gsworld_getEnableDraw();
void gsworld_load(enum map_e map_id);
void gsworld_stub3(enum map_e map);

#endif
