// BanjoDecomp: framebufferdraw.c
#include <ultra64.h>
#include "core1/core1.h"
#include "core2/core2.h"
#include "port/Patches/Patches.h"

#define IA8_I(ia) ((ia) >> 4)
#define IA8_A(ia) ((ia) & 0xF)
#define I4_2_RGBA16(i,a) ((i << 12) | (i << 7) | (i << 2) | (a))

#define RGBA16_R5(rgba) ((rgba) >> 11)
#define RGBA16_G5(rgba) (((rgba) >> 6) & 0x1f)
#define RGBA16_B5(rgba) (((rgba) >> 1) & 0x1f)
#define RGBA16_RED5(rgba) ((rgba) >> 11)
#define C5_TO_C8(c5) ((c5) << 3)
#define C8_TO_C5(c5) ((c5) >> 3)

static s32 sPrimColor_r;
static s32 sPrimColor_g;
static s32 sPrimColor_b;
static s32 sBufferIndex;

void framebufferdraw_drawSprite_CI4(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled) {
    BKSpriteFrame *sprite_frame;
    BKSpriteTextureBlock *chunk;
    u16 *framebuffer, *fb_pixel;
    int fb_y, fb_x, iy, ix, i_chunk;
    u8 *tmem;
    u32 ci4_1, ci4_2;
    u8 *palette_unaligned;
    u16 *palette;

    framebuffer = gFramebuffers[sBufferIndex];
    sprite_frame = sprite_getFramePtr(sprite, frame);

    if (!alpha_enabled) {
        framebufferdraw_setPrimColor(0, 0, 128);
        framebufferdraw_drawRect(x, y, sprite_frame->w, sprite_frame->h);
    }

    for (palette_unaligned = (u8 *) sprite_frame->data; (uintptr_t) palette_unaligned % 8; palette_unaligned++);
    palette = (u16 *) palette_unaligned;

    chunk = (BKSpriteTextureBlock *) (palette + 16);

    for (i_chunk = 0; i_chunk < sprite_frame->chunkCnt; i_chunk++) {
        palette_unaligned = (u8 *) sprite_frame->data; // TODO: Check if this can be removed
        for (tmem = chunk->data; (uintptr_t) tmem % 8; tmem++);
        
        for (iy = 0; iy < chunk->h; iy++) {
            for (ix = 0; ix < chunk->w; ix += 2) {
                ci4_1 = (*tmem >> 4) & 0xF;
                ci4_2 = *tmem & 0xF;

                fb_x = x + chunk->x + ix;

                if ((0 <= fb_x) && (fb_x < gFramebufferWidth)) {
                    fb_y = y + chunk->y + iy;

                    if ((0 <= fb_y) && (fb_y < gFramebufferHeight)) {
                        fb_pixel = framebuffer + fb_x + fb_y * gFramebufferWidth;

                        if (palette[ci4_1] & 1) {
                            fb_pixel[0] = palette[ci4_1];
                        } else {
                            if (!alpha_enabled) {
                                fb_pixel[0] = 0x0001;
                            }
                        }

                        if (palette[ci4_2] & 1) { 
                            fb_pixel[1] = palette[ci4_2];
                        } else {
                            if (!alpha_enabled) {
                                fb_pixel[1] = 0x0001;
                            }
                        }
                    }
                }

                tmem++;
            }
        }

        chunk = (BKSpriteTextureBlock *) tmem;
    }
}

void framebufferdraw_drawSprite_CI8(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled) {
    static s32 D_80275C00 = 0;
    u16 *palette;
    int fb_y, fb_x, iy, ix, i_chunk;
    u8 *tmem;
    BKSpriteTextureBlock *chunk;
    u16 *fb_pixel;
    int new_var;
    BKSpriteFrame *sprite_frame;
    u16 *framebuffer;
    u8 *palette_unaligned;

    if (++D_80275C00 == 10) {
        D_80275C00 = 0;
    }

    framebuffer = gFramebuffers[sBufferIndex];
    sprite_frame = sprite_getFramePtr(sprite, frame);

    if (!alpha_enabled) {
        framebufferdraw_setPrimColor(0, 0, 128);
        framebufferdraw_drawRect(x, y, sprite_frame->w, sprite_frame->h);
    }

    for (palette_unaligned = (u8 *) sprite_frame->data; (uintptr_t) palette_unaligned % 8; palette_unaligned++);
    palette = (u16 *) palette_unaligned;

    chunk = (BKSpriteTextureBlock *) (palette + 256);

    for (i_chunk = 0; i_chunk < sprite_frame->chunkCnt; i_chunk++) {
        palette_unaligned = chunk->data; // TODO: Check if this can be removed
        for (tmem = chunk->data; (uintptr_t) tmem % 8; tmem++);

        for (iy = 0; iy < chunk->h; iy++) {
            for (ix = 0; ix < chunk->w; ix++) {
                fb_x = x + chunk->x + ix;

                if ((0 <= fb_x) && (fb_x < gFramebufferWidth)) {
                    fb_y = y + chunk->y + iy;

                    if ((0 <= fb_y) && (fb_y < gFramebufferHeight)) {
                        fb_pixel = framebuffer + fb_x + fb_y * gFramebufferWidth;

                        if (palette[*tmem] & 1) {
                            *fb_pixel = palette[*tmem];
                        } else {
                            if (!alpha_enabled) {
                                if (palette[*tmem] & 0xFFFE) {
                                    *fb_pixel = D_80275C00 < 5 ? 0xFFFF : palette[*tmem] | 1;
                                } else {
                                    *fb_pixel = 1;
                                }
                            }
                        }
                    }
                }

                tmem++;
            }
        }

        chunk = (BKSpriteTextureBlock *) tmem;
    }
}

void framebufferdraw_drawSprite_RGBA16(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled) {
    BKSpriteFrame *sprite_frame;
    BKSpriteTextureBlock *chunk;
    u16 *framebuffer, *fb_pixel;
    int fb_y, fb_x, iy, ix, i_chunk;
    u16 *tmem;

    framebuffer = gFramebuffers[sBufferIndex];
    sprite_frame = sprite_getFramePtr(sprite, frame);

    if (!alpha_enabled) {
        framebufferdraw_setPrimColor(0, 0, 128);
        framebufferdraw_drawRect(x, y, sprite_frame->w, sprite_frame->h);
    }

    chunk = (BKSpriteTextureBlock *) sprite_frame->data;

    for (i_chunk = 0; i_chunk < sprite_frame->chunkCnt; i_chunk++) {
        for (tmem = (u16 *) chunk->data; (uintptr_t) tmem % 8; tmem++);

        for (iy = 0; iy < chunk->h; iy++) {
            for (ix = 0; ix < chunk->w; ix++) {
                fb_x = x + chunk->x + ix;

                if ((0 <= fb_x) && (fb_x < gFramebufferWidth)) {
                    fb_y = y + chunk->y + iy;

                    if ((0 <= fb_y) && (fb_y < gFramebufferHeight)) {
                        fb_pixel = framebuffer + fb_x + fb_y * gFramebufferWidth;

                        if (*tmem & 1) {
                            *fb_pixel = *tmem;
                        } else {
                            if (!alpha_enabled) {
                                *fb_pixel = *tmem | 1;
                            }
                        }
                    }
                }

                tmem++;
            }
        }

        chunk = (BKSpriteTextureBlock *) tmem;
    }
}

void framebufferdraw_drawSprite_I4(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled) {
    BKSpriteFrame *sprite_frame;
    BKSpriteTextureBlock *chunk;
    u16 *framebuffer, *fb_pixel;
    int fb_y, fb_x, iy, ix, i_chunk;
    u8 *tmem;
    u32 i4_1, i4_2;

    framebuffer = gFramebuffers[sBufferIndex];
    sprite_frame = sprite_getFramePtr(sprite, frame);

    if (!alpha_enabled) {
        framebufferdraw_setPrimColor(0, 0, 128);
        framebufferdraw_drawRect(x, y, sprite_frame->w, sprite_frame->h);
    }

    chunk = (BKSpriteTextureBlock *) sprite_frame->data;

    for (i_chunk = 0; i_chunk < sprite_frame->chunkCnt; i_chunk++) {
        for (tmem = chunk->data; (uintptr_t) tmem % 8; tmem++);
        
        for (iy = 0; iy < chunk->h; iy++) {
            for (ix = 0; ix < chunk->w; ix += 2) {
                fb_x = x + chunk->x + ix;

                if ((0 <= fb_x) && (fb_x < gFramebufferWidth)) {
                    fb_y = y + chunk->y + iy;

                    if ((0 <= fb_y) && (fb_y < gFramebufferHeight)) {
                        i4_1 = *tmem >> 4;
                        i4_2 = *tmem & 0xF;
                        fb_pixel = framebuffer + fb_x + fb_y * gFramebufferWidth;

                        if (i4_1) {
                            fb_pixel[0] = (i4_1 << 12) | (i4_1 << 7) | (i4_1 << 2) | 1;
                        } else {
                            if (!alpha_enabled) {
                                fb_pixel[0] = 0x0001;
                            }
                        }

                        if (i4_2) {
                            fb_pixel[1] = (i4_2 << 12) | (i4_2 << 7) | (i4_2 << 2) | 1;
                        } else {
                            if (!alpha_enabled) {
                                fb_pixel[1] = 0x0001;
                            }
                        }
                    }
                }
                tmem++;
            }
        }
        chunk = (BKSpriteTextureBlock *) tmem;
    }
}

void framebufferdraw_drawSprite_IA4(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled) {
    BKSpriteFrame *sprite_frame;
    BKSpriteTextureBlock *chunk;
    u16 *framebuffer, *fb_pixel;
    int fb_y, fb_x, iy, ix, i_chunk;
    u8 *tmem;
    u32 ia4_1, ia4_2, p1_i, p1_a, p2_i, p2_a;

    framebuffer = gFramebuffers[sBufferIndex];
    sprite_frame = sprite_getFramePtr(sprite, frame);

    if (!alpha_enabled) {
        framebufferdraw_setPrimColor(0, 0, 128);
        framebufferdraw_drawRect(x, y, sprite_frame->w, sprite_frame->h);
    }

    chunk = (BKSpriteTextureBlock *) sprite_frame->data;

    for (i_chunk = 0; i_chunk < sprite_frame->chunkCnt; i_chunk++) {
        for (tmem = chunk->data; (uintptr_t) tmem % 8; tmem++);
        
        for (iy = 0; iy < chunk->h; iy++) {
            for (ix = 0; ix < chunk->w; ix += 2) {
                fb_x = x + chunk->x + ix;

                if ((0 <= fb_x) && (fb_x < gFramebufferWidth)) {
                    fb_y = y + chunk->y + iy;

                    if ((0 <= fb_y) && (fb_y < gFramebufferHeight)) {
                        fb_pixel = framebuffer + fb_x + fb_y * gFramebufferWidth;
                        ia4_1 = *tmem >> 4;
                        ia4_2 = *tmem & 0xF;
                        p1_i = ia4_1 & 0x0E;
                        p1_a = ia4_1 & 0x01;
                        p2_i = ia4_2 & 0x0E;
                        p2_a = ia4_2 & 0x01;

                        if (p1_a) { \
                            fb_pixel[0] = (p1_i << 12) | (p1_i << 7) | (p1_i << 2) | p1_a;
                        } else {
                            if (!alpha_enabled) {
                                fb_pixel[0] = 0x0001;
                            }
                        }

                        if (p2_a) {
                            fb_pixel[1] = (p2_i << 12) | (p2_i << 7) | (p2_i << 2) | p2_a;
                        } else {
                            if (!alpha_enabled) {
                                fb_pixel[1] = 0x0001;
                            }
                        }
                    }
                }

                tmem++;
            }
        }

        chunk = (BKSpriteTextureBlock *) tmem;
    }
}

void framebufferdraw_drawSprite_I8(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled) {
    BKSpriteFrame *sprite_frame;
    BKSpriteTextureBlock *chunk;
    u16 *framebuffer, *fb_pixel;
    int fb_y, fb_x, iy, ix, i_chunk;
    u8 *tmem;
    u32 i8;

    framebuffer = gFramebuffers[sBufferIndex];
    sprite_frame = sprite_getFramePtr(sprite, frame);

    if (!alpha_enabled) {
        framebufferdraw_setPrimColor(0, 0, 128);
        framebufferdraw_drawRect(x, y, sprite_frame->w, sprite_frame->h);
    }

    chunk = (BKSpriteTextureBlock *) sprite_frame->data;

    for (i_chunk = 0; i_chunk < sprite_frame->chunkCnt; i_chunk++) {
        for (tmem = chunk->data; (uintptr_t) tmem % 8; tmem++);

        for (iy = 0; iy < chunk->h; iy++) {
            for (ix = 0; ix < chunk->w; ix++) {
                fb_x = x + chunk->x + ix;

                if ((0 <= fb_x) && (fb_x < gFramebufferWidth)) {
                    fb_y = y + chunk->y + iy;

                    if ((0 <= fb_y) && (fb_y < gFramebufferHeight)) {
                        fb_pixel = framebuffer + fb_x + fb_y * gFramebufferWidth;
                        i8 = *tmem >> 3;
                        if (i8) {
                            *fb_pixel = (i8 << 11) | (i8 << 6) | (i8 << 1) | 1;
                        } else {
                            if (!alpha_enabled) {
                                *fb_pixel = 1;
                            }
                        }
                    }
                }

                tmem++;
            }
        }

        chunk = (BKSpriteTextureBlock *) tmem;
    }
}

void framebufferdraw_drawSprite_IA8(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled) {
    BKSpriteFrame *sprite_frame;
    BKSpriteTextureBlock *chunk;
    u16 *framebuffer, *fb_pixel;
    int fb_y, fb_x, iy, ix, i_chunk;
    u8 *tmem;
    u32 ia8;

    framebuffer = gFramebuffers[sBufferIndex];
    sprite_frame = sprite_getFramePtr(sprite, frame);

    if (!alpha_enabled) {
        framebufferdraw_setPrimColor(0, 0, 128);
        framebufferdraw_drawRect(x, y, sprite_frame->w, sprite_frame->h);
    }

    chunk = (BKSpriteTextureBlock *) sprite_frame->data;

    for (i_chunk = 0; i_chunk < sprite_frame->chunkCnt; i_chunk++) {
        for (tmem = chunk->data; (uintptr_t) tmem % 8; tmem++);
        
        for (iy = 0; iy < chunk->h; iy++) {
            for (ix = 0; ix < chunk->w; ix++) {
                fb_x = x + chunk->x + ix;

                if ((0 <= fb_x) && (fb_x < gFramebufferWidth)) {
                    fb_y = y + chunk->y + iy;

                    if ((0 <= fb_y) && (fb_y < gFramebufferHeight)) {
                        fb_pixel = framebuffer + fb_x + fb_y * gFramebufferWidth;
                        ia8 = *tmem;

                        if (IA8_A(ia8)) {
                            *fb_pixel = I4_2_RGBA16(IA8_I(ia8), 1);
                        } else {
                            if (!alpha_enabled) {
                                *fb_pixel = 0x0001;
                            }
                        }
                    }
                }

                tmem++;
            }
        }

        chunk = (BKSpriteTextureBlock *) tmem;
    }
}

void framebufferdraw_drawSprite_RGBA32(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled){
    BKSpriteTextureBlock *chunk;
    u32 *tmem;
    u32 dest_rgba32;
    s16 var_v1;
    int fb_y, fb_x;
    u16 *framebuffer;
    s32 temp_t1;
    s32 alpha;
    int i_chunk, iy, ix;
    u16 *fb_pixel;
    BKSpriteFrame *sprite_frame;
    u16 p1_a;
    u32 rgba32;
    s32 r8, g8, b8;

    framebuffer = gFramebuffers[sBufferIndex];
    sprite_frame = sprite_getFramePtr(sprite, frame);

    if (!alpha_enabled) {
        framebufferdraw_setPrimColor(0, 0, 128);
        framebufferdraw_drawRect(x, y, sprite_frame->w, sprite_frame->h);
    }

    chunk = (BKSpriteTextureBlock *) sprite_frame->data;

    for (i_chunk = 0; i_chunk < sprite_frame->chunkCnt; i_chunk++) {
        if ((-5 <= chunk->x) && (chunk->x < 41) && (-5 <= chunk->y) && (chunk->y < 41)) {
            for (tmem = (u32 *) chunk->data; (uintptr_t) tmem % 8; tmem++);

            for (iy = 0; iy < chunk->h; iy++) {
                for (ix = 0; ix < chunk->w; ix++) {
                    fb_x = x + chunk->x + ix;

                    if ((0 <= fb_x) && (fb_x < gFramebufferWidth)) {
                        fb_y = y + chunk->y + iy;

                        if ((0 <= fb_y) && (fb_y < gFramebufferHeight)) {
                            fb_pixel = framebuffer + fb_x + fb_y * gFramebufferWidth;
                            rgba32 = *tmem;
                            dest_rgba32 = *fb_pixel;
                            alpha = _SHIFTR(rgba32, 0, 8);

                            if (alpha) { //blend texture with existing pixel color
                                r8 = ((_SHIFTR(rgba32, 24, 8) * alpha) + ((       (dest_rgba32 >> 11) * 8) * (255 - alpha))) / 255;
                                g8 = ((_SHIFTR(rgba32, 16, 8) * alpha) + ((_SHIFTR(dest_rgba32, 6, 5) * 8) * (255 - alpha))) / 255;
                                b8 = ((_SHIFTR(rgba32,  8, 8) * alpha) + ((_SHIFTR(dest_rgba32, 1, 5) * 8) * (255 - alpha))) / 255;

                                *fb_pixel = ((_SHIFTL((r8 >> 3), 11, 5) | _SHIFTL((g8 >> 3), 6, 5)) | _SHIFTL((b8 >> 3), 1, 5)) | 1;
                            }
                            else {
                                if (!alpha_enabled) {
                                    *fb_pixel = 0x0001;
                                }
                            }
                        }
                    }

                    tmem++;
                }
            }

            chunk = (BKSpriteTextureBlock *) tmem;
        }
    }
}

void framebufferdraw_stub1(s32 x, s32 y, s32 w, s32 h) {} //signature may have more variables passed in
void framebufferdraw_stub2(s32 x, s32 y, s32 w, s32 h) {} //signature may have more variables passed in

void framebufferdraw_drawTexture_CI4(s32 x, s32 y, void *tmem, s32 w, s32 h, bool alpha_enabled) {
    s32 ix;
    s32 iy;
    s32 ci4;
    u16 *fb_pixel;
    u16 *palette_ptr;
    u8 *pixel_ptr;

    palette_ptr = (u16 *) tmem;
    pixel_ptr = (u8 *) (palette_ptr + 16);
    fb_pixel = &gFramebuffers[sBufferIndex][x + y * gFramebufferWidth];

    for (iy = 0; iy < h; iy++) {
        for (ix = 0; ix < w; ix++) {
            ci4 = (ix & 1) ? pixel_ptr[ix / 2 + (iy * w) / 2] & 0xF : pixel_ptr[ix / 2 + (iy * w) / 2] >> 4;
            *fb_pixel = palette_ptr[ci4];
            fb_pixel++;
        }

        fb_pixel += gFramebufferWidth - w;
    }
}

void framebufferdraw_drawSprite(s32 x, s32 y, BKSprite *sprite, s32 frame, bool alpha_enabled) {
    s16 type = sprite->type;

    if (type & SPRITE_TYPE_I4) {
        framebufferdraw_drawSprite_I4(x, y, sprite, frame, alpha_enabled);
    } else if (type & SPRITE_TYPE_IA4) {
        framebufferdraw_drawSprite_IA4(x, y, sprite, frame, alpha_enabled);
    } else if (type & SPRITE_TYPE_I8) {
        framebufferdraw_drawSprite_I8(x, y, sprite, frame, alpha_enabled);
    } else if (type & SPRITE_TYPE_IA8) {
        framebufferdraw_drawSprite_IA8(x, y, sprite, frame, alpha_enabled);    
    } else if (type & SPRITE_TYPE_RGBA16) {
        framebufferdraw_drawSprite_RGBA16(x, y, sprite, frame, alpha_enabled);
    } else if (type & SPRITE_TYPE_RGBA32) {
        framebufferdraw_drawSprite_RGBA32(x, y, sprite, frame, alpha_enabled);
    } else if (type & SPRITE_TYPE_CI4) {
        framebufferdraw_drawSprite_CI4(x, y, sprite, frame, alpha_enabled);
    } else if (type & SPRITE_TYPE_CI8) {
        framebufferdraw_drawSprite_CI8(x, y, sprite, frame, alpha_enabled);
    }
}

void framebufferdraw_drawSpriteMask(s32 x, s32 y, BKSpriteMask *sprite_mask, s32 chunk_id, bool alpha_enabled) {
    BKSpriteTextureBlock *chunk;
    int iy, ix;
    u16 *fb_pixel, *tmem;

    chunk = spritemask_getChunk(sprite_mask, chunk_id);
    tmem = (u16 *) chunk->data;

    if (sprite_mask->texture_type == SPRITE_TYPE_CI4) {
        framebufferdraw_drawTexture_CI4(x, y, chunk->data, chunk->w, chunk->h, alpha_enabled);
        return;
    }
    
    fb_pixel = &gFramebuffers[sBufferIndex][x + y * gFramebufferWidth];

    for (iy = 0; iy < chunk->h; iy++) {
        for (ix = 0; ix < chunk->w; ix++) {
            if (*tmem & 1) {
                *fb_pixel = *tmem;
            } else {
                if (!alpha_enabled) {
                    *fb_pixel = 0x0001;
                }
            }

            tmem++;
            fb_pixel++;
        }

        fb_pixel += gFramebufferWidth - chunk->w;
    }
}

void framebufferdraw_drawSpriteMaskRegion(s32 x, s32 y, BKSpriteMask* sprite_mask, s32 chunk_id, s32 mX, s32 mY, s32 mW, s32 mH, s32 mask_color, s32 mStride, bool dim, s32 replacement_color) {
    BKSpriteTextureBlock *chunk;
    int iy, ix;
    u16 *fb_pixel, *tmem;

    chunk = spritemask_getChunk(sprite_mask, chunk_id);
    tmem = (u16*) chunk->data + mX + mY * chunk->w;
    fb_pixel = &gFramebuffers[sBufferIndex][x + y * gFramebufferWidth];

    for (iy = 0; iy < mH; iy += mStride) {
        for (ix = 0; ix < mW; ix++){

            if (mask_color != *tmem) {
                *fb_pixel = (replacement_color != 0) ? replacement_color : *tmem;
            } else {
                if (dim) {
                    *fb_pixel = _SHIFTL(C8_TO_C5(C5_TO_C8(RGBA16_R5(*fb_pixel)) / 2), 11, 5) 
                            | _SHIFTL(C8_TO_C5(C5_TO_C8(RGBA16_G5(*fb_pixel)) / 2), 6, 5) 
                            | _SHIFTL(C8_TO_C5(C5_TO_C8(RGBA16_B5(*fb_pixel)) / 2), 1, 5) 
                            | 1;
                }
            }

            fb_pixel++;
            tmem++;
        }

        tmem += mStride * chunk->w - mW;
        fb_pixel += gFramebufferWidth - mW;
    }
}

void framebufferdraw_drawGrid(s32 x, s32 y, s32 w, s32 h, s32 horz_spacing, s32 vert_spacing) {
    s32 ix, iy;

    // BUG: Grid is always drawn from [x,y] to [gFramebufferWidth,gFramebufferHeight] instead of [w,h]

    for (ix = 0; ix < gFramebufferWidth; ix += horz_spacing) {
        for (iy = 0; iy < gFramebufferHeight; iy++) {
            framebufferdraw_setPixel(x + ix, y + iy);
        }
    }

    for (iy = 0; iy < gFramebufferHeight; iy += vert_spacing) {
        for (ix = 0; ix < gFramebufferWidth; ix++) {
            framebufferdraw_setPixel(x + ix, y + iy);
        }
    }
}

void framebufferdraw_setPixel(s32 x, s32 y) {
    if ((0 <= x) && (x < gFramebufferWidth) && (0 <= y) && (y < gFramebufferHeight)) {
        gFramebuffers[sBufferIndex][x + y * gFramebufferWidth] =_SHIFTL(sPrimColor_r >> 3, 11, 5) | _SHIFTL(sPrimColor_g >> 3, 6, 5) | _SHIFTL(sPrimColor_b >> 3, 1, 5) | _SHIFTL(1, 0, 1);
    }
}

void framebufferdraw_drawOutline(s32 x, s32 y, s32 w, s32 h) {
    s32 ix, iy;

    for (ix = 0; ix < w; ix++) {
        framebufferdraw_setPixel(x + ix, y);
        framebufferdraw_setPixel(x + ix, y + h - 1);
    }

    for (iy = 1; iy < h - 1; iy++) {
        framebufferdraw_setPixel(x, y + iy);
        framebufferdraw_setPixel(x + w - 1, y + iy);
    }
}

void framebufferdraw_func_8024A564(s32 x, s32 y, u16 *palette, s32 arg3, s32 arg4, f32 arg5, f32 arg6) {
    s32 ix_start;
    s32 var_a2;
    s32 new_var, new_var2;
    s32 iy, ix;
    s32 var_v0;
    s32 fb_width;
    u16 *fb_pixel_start, *fb_pixel;

    var_v0 = 0;
    fb_width = gFramebufferWidth;
    fb_pixel_start = &gFramebuffers[sBufferIndex][x + (y * gFramebufferWidth)];
    new_var = (s32) (((f64) (256.0f / arg6)) + 0.5);
    new_var2 = (s32) (((f64) (256.0f / arg5)) + 0.5);

    for (iy = arg4 * arg6 + 0.5; iy != 0; iy--) {
        ix_start = arg3 * arg5 + 0.5;
        var_a2 = ((var_v0 >> 8) * arg3) << 8;
        fb_pixel = fb_pixel_start;

        for (ix = arg3 * arg5 + 0.5; ix != 0; ix--) {
            *fb_pixel = palette[var_a2 >> 8];
            var_a2 += new_var2;
            fb_pixel++;
        }

        fb_pixel_start += gFramebufferWidth;
        var_v0 += new_var;
    }
}

void framebufferdraw_drawRect(s32 x, s32 y, s32 w, s32 h) {
    s32 iy, ix;

    for (ix = 0; ix < w; ix++) {
        for (iy = 0; iy < h; iy++) {
                framebufferdraw_setPixel(x + ix, y + iy);
            }
    }
}

void framebufferdraw_resetPrimColorAndBuffer(void) {
    framebufferdraw_setPrimColor(0, 128, 0);
    framebufferdraw_setBufferIndex(0);
}

void framebufferdraw_setPrimColor(s32 r, s32 g, s32 b) {
    sPrimColor_r = r;
    sPrimColor_g = g;
    sPrimColor_b = b;
}

void framebufferdraw_setBufferIndex(s32 buffer_indx) {
    sBufferIndex = buffer_indx;
}

s32 framebufferdraw_getBufferIndex(void) {
    return sBufferIndex;
}
