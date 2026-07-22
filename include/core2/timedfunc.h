#ifndef __TIMED_FUNC_H__
#define __TIMED_FUNC_H__
#include <ultra64.h>

#include "generic.h"

#ifndef __cplusplus
#define reinterpret_cast(type, var) (*((type *)&var))
#endif


void timedFunc_set_0(f32 time, GenFunction_0 funcPtr);
void timedFunc_set_1(f32 time, GenFunction_1 funcPtr, uintptr_t arg0);
void timedFunc_set_2(f32 time, GenFunction_2 funcPtr, uintptr_t arg0, uintptr_t arg1);
void timedFunc_set_3(f32 time, GenFunction_3 funcPtr, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2);
void timedFunc_set_4(f32 time, GenFunction_4 funcPtr, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);
void timedFunc_set_5(f32 time, GenFunction_5 funcPtr, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4);
void timedFunc_set_6(f32 time, GenFunction_6 funcPtr, void* argPtr, size_t argSize);
void timedJiggySpawn(f32 time, s32 jiggyId, f32 *position);

#endif
