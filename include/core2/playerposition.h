#ifndef BANJO_KAZOOIE_CORE2_PLAYERPOSITION_H
#define BANJO_KAZOOIE_CORE2_PLAYERPOSITION_H

#include <ultra64.h>

void playerPosition_init(void);
void playerPosition_func_8029842C(void);
void playerPosition_func_80298464(f32 position[3]);
void playerPosition_set(f32 position[3]);
void playerPosition_setY(f32 value);
void playerPosition_get(f32 position[3]);
f32 playerPosition_getY(void);
void playerPosition_func_80298504(f32 arg0[3]);
void playerPosition_addY(f32 value);
void playerPosition_getOffset(f32 offset[3]);
void playerPosition_setOffset(f32 offset[3]);
void playerPosition_applyOffset(void);

#endif
