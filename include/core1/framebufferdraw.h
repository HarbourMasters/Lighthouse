#ifndef BANJO_KAZOOIE_CORE1_FRAMEBUFFERDRAW_H
#define BANJO_KAZOOIE_CORE1_FRAMEBUFFERDRAW_H

#include <ultra64.h>

void framebufferdraw_drawSprite_CI4(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled);
void framebufferdraw_drawSprite_CI8(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled);
void framebufferdraw_drawSprite_RGBA16(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled);
void framebufferdraw_drawSprite_I4(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled);
void framebufferdraw_drawSprite_IA4(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled);
void framebufferdraw_drawSprite_I8(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled);
void framebufferdraw_drawSprite_IA8(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled);
void framebufferdraw_drawSprite_RGBA32(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled);
void framebufferdraw_stub1(s32 x, s32 y, s32 w, s32 h);
void framebufferdraw_stub2(s32 x, s32 y, s32 w, s32 h);
void framebufferdraw_drawTexture_CI4(s32 x, s32 y, void *tmem, s32 w, s32 h, bool alpha_enabled);
void framebufferdraw_drawSprite(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled);
void framebufferdraw_drawSpriteMask(s32 x, s32 y, BKSpriteMask *sprite_mask, s32 chunk_id, bool alpha_enabled);
void framebufferdraw_drawSpriteMaskRegion(s32 x, s32 y, BKSpriteMask* sprite_mask, s32 chunk_id, s32 mX, s32 mY, s32 mW, s32 mH, s32 mask_color, s32 mStride, bool dim, s32 replacement_color);
void framebufferdraw_drawGrid(s32 x, s32 y, s32 w, s32 h, s32 horz_spacing, s32 vert_spacing);
void framebufferdraw_setPixel(s32 x, s32 y);
void framebufferdraw_drawOutline(s32 x, s32 y, s32 w, s32 h);
void framebufferdraw_func_8024A564(s32 x, s32 y, u16 *arg2, s32 arg3, s32 arg4, f32 arg5, f32 arg6);
void framebufferdraw_drawRect(s32 x, s32 y, s32 w, s32 h);
void framebufferdraw_resetPrimColorAndBuffer(void);
void framebufferdraw_setPrimColor(s32 r, s32 g, s32 b);
void framebufferdraw_setBufferIndex(s32 buffer_indx);
s32 framebufferdraw_getBufferIndex(void);

#endif
