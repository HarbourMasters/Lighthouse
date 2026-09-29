#ifndef BANJO_KAZOOIE_CORE2_FILEPROGRESS_H
#define BANJO_KAZOOIE_CORE2_FILEPROGRESS_H

#include <ultra64.h>
#include "bool.h"
#include "enums.h"

void fileprogressflag_func_8031FC40(void);
u32 fileprogressflag_func_8031FE40(void);
void fileprogressflag_func_8031FEC0(void);
bool fileProgressFlag_get(enum file_progress_e index);
u32 fileProgressFlag_getN(enum file_progress_e start_index, s32 count);
bool fileProgressFlag_getAndSet(enum file_progress_e index, bool value);
void fileprogressflag_func_8031FFAC(void);
void fileProgressFlag_set(enum file_progress_e index, bool value);
void fileProgressFlag_setN(enum file_progress_e start_index, u32 values, s32 count);
void fileProgressFlag_getSizeAndPtr(s32 *size, u8 **addr);

bool bitfieldarray_getBit(u8 *array, s32 index);
u32 bitfieldarray_getNBits(u8 *array, s32 start_index, s32 count);
void bitfieldarray_setBit(u8 *array, s32 index, bool value);
void bitfieldarray_setNBits(u8 *array, s32 start_index, u32 values, s32 count);

bool volatileflag_stub1(void);
bool volatileflag_stub2(void);
u32 volatileflag_func_80320250(void);
void volatileflag_func_803202D0(void);
s32 volatileflag_func_80320320(void);
void volatileflag_func_803203A0(void);
bool volatileFlag_get(enum volatile_flags_e index);
u32 volatileFlag_getN(enum volatile_flags_e start_index, s32 count);
bool volatileFlag_getAndSet(enum volatile_flags_e index, bool value);
void volatileFlag_clear(void);
void volatileFlag_set(enum volatile_flags_e index, bool value);
void volatileFlag_setN(enum volatile_flags_e start_index, s32 values, s32 count);
s32 volatileflag_func_8032056C(void);
s32 volatileflag_func_80320708(void);
void volatileFlag_backupAll(void);
void volatileFlag_restoreAll(void);

#endif
