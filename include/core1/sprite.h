#ifndef BANJO_KAZOOIE_CORE1_SPRITE_H
#define BANJO_KAZOOIE_CORE1_SPRITE_H

#ifdef __cplusplus
extern "C" {
#endif

u32 sprite_getUnk8(BKSprite_s *sthis);
u32 sprite_getUnkA(BKSprite_s *sthis);
u32 sprite_getUnk6(BKSprite_s *sthis);
u32 sprite_getUnk4(BKSprite_s *sthis);
s32 sprite_getFrameCount(BKSprite_s *sthis);
BKSpriteFrame *sprite_getFramePtr(BKSprite_s *sthis, u32 frame_id);

#ifdef __cplusplus
}
#endif

#endif
