#ifndef BANJO_KAZOOIE_CORE2_GC_TRANSITION_H
#define BANJO_KAZOOIE_CORE2_GC_TRANSITION_H

#include <ultra64.h>

#ifdef __cplusplus
extern "C" {
#endif

void gctransition_defrag(void);
void gctransition_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void gctransition_8030BD4C(void);
f32 gctransition_8030BD88(void);
int gctransition_done(void);
int gctransition_active(void);
int  gctransition_8030BDC0(void);
void gctransition_8030BE3C(void);
void gctransition_8030BE60(void);
void gctransition_8030BEA4(s32 arg0);
void gctransition_update(void);
void gctransition_reset(void);

#ifdef __cplusplus
}
#endif

#endif
