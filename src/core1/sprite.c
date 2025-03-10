#include "core1/core1.h"
#include <ultra64.h>

u32 sprite_getUnk8(BKSprite_s *sthis) { return sthis->unk8; }

u32 sprite_getUnkA(BKSprite_s *sthis) { return sthis->unkA; }

u32 sprite_getUnk6(BKSprite_s *sthis) { return sthis->unk6; }

u32 sprite_getUnk4(BKSprite_s *sthis) { return sthis->unk4; }

s32 sprite_getFrameCount(BKSprite_s *sthis) { return sthis->frameCnt; }

BKSpriteFrame *sprite_getFramePtr(BKSprite_s *sthis, u32 frame_id) {
  BKSpriteFrame *sprite_frame;
  sprite_frame = (BKSpriteFrame *)(sthis->offsets[frame_id] +
                                   ((s32)((*sthis).offsets + sthis->frameCnt)));
  return sprite_frame;
}
