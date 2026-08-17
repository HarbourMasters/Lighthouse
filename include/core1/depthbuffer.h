#ifndef BANJO_KAZOOIE_CORE1_DEPTHBUFFER_H
#define BANJO_KAZOOIE_CORE1_DEPTHBUFFER_H

#include <ultra64.h>

extern u8 D_8000E800[];

void depthbuffer_clear(Gfx **gfx);
void depthbuffer_clearRegion(Gfx **gfx, s32 x, s32 y, s32 w, s32 h, void *color_buffer);
bool depthbuffer_getUnk4(void);
bool depthbuffer_isDataPtrSet(void);
void depthbuffer_stub(void);
void depthbuffer_enable(bool enable);
void depthbuffer_setUnk4(bool value);
void depthbuffer_set(Gfx **gfx);
void *depthbuffer_getDataPtr(void);

#endif
