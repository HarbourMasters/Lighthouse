#ifndef BANJO_KAZOOIE_CORE2_LEVELOVERLAY_H
#define BANJO_KAZOOIE_CORE2_LEVELOVERLAY_H

#include <ultra64.h>
#include "bool.h"
#include "enums.h"

void leveloverlay_drawCallback(Gfx **gfx, Mtx **mtx, Vtx **vtx);
enum overlay_e leveloverlay_getOverlayFromLevel(enum level_e lvl);
void leveloverlay_releaseCallback(bool flag);
void leveloverlay_initCallback(bool flag);
void leveloverlay_releaseCallback_NotFP(void); // calls release callback only, when release flag is not set, which is always the case, except for FP
void leveloverlay_releaseCallback_OnlyFP(void); // calls release callback only, when release flag is set, which is only the case for FP
void leveloverlay_initCallback_OnlyFP(void); // calls release callback only, when init flag is not set, which is only the case for FP
void leveloverlay_initCallback_NotFP(void); // calls release callback only, when init flag is set, which is always the case, except for FP
void leveloverlay_debug(void);
void leveloverlay_init(void);
void leveloverlay_unk14Callback(s32 arg0, s32 arg1);
void leveloverlay_updateCallback(void);

#endif
