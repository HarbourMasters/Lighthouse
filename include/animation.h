#ifndef ANIMATION_H
#define ANIMATION_H

#include "prop.h"
size_t  anim_getSize(void);
enum asset_e  anim_getIndex(Animation *self);
f32  anim_getTimer(Animation *self);
f32  anim_getDuration(Animation *self);
void anim_new(Animation *self, bool arg1);
void anim_setTimer(Animation *self, f32 arg1);
void anim_80289790(Animation* self, GenFunction_2 arg1);
void anim_80289798(Animation *self, uintptr_t arg1);
void anim_setDuration(Animation *self, f32 arg1);

//represents the transformation on a given model bone



#endif
