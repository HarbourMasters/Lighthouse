#ifndef BANJO_KAZOOIE_CORE1_MIDICHANNEL_H
#define BANJO_KAZOOIE_CORE1_MIDICHANNEL_H

#include <ultra64.h>

void midichannel_setChanMask(s32 chan_mask);
void midichannel_setChanMaskWithTransitionSpeed(s32 chan_mask, f32 transition_speed);
bool midichannel_isPlayerInRange(s32 x, s32 z, s32 distance);
f32 midichannel_getPlayerDistance(f32 x, f32 z);
bool midichannel_isPlayerInsideBoundingBox(s32 box_idx);
void midichannel_func_8024A9EC(s32 arg0);
void midichannel_func_8024AAB0(void);
void midichannel_func_8024ADF0(bool arg0);
void midichannel_func_8024AE74(void);
void midichannel_resetState(void);
void midichannel_setChanMaskFromWaterState(s32 chan_mask_underwater, s32 chan_mask_surface);
void midichannel_func_8024AF48(void);
void midichannel_incOrDecCounter(bool increment);
void midichannel_func_8024BD40(s32 arg0, s32 arg1);

#endif
