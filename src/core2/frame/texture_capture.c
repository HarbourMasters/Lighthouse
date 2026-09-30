// BanjoDecomp: core2/code_77E50.c
#include <ultra64.h>
#include "core1/core1.h"
#include "core2/model.h"

#include "port/Patches/Patches.h"

extern void gfx_texture_cache_clear(void);

// [port] The texture cache belongs to the renderer; marshal the clear.
static void __textureCacheClearFn(void* arg) {
    (void)arg;
    gfx_texture_cache_clear();
}

u8 *textureList_getDataPtr(BKTextureList *texture_list);

void model_copyFramebufferBlockToTexture(BKTextureList *texture_list, s32 index, s32 x_offset, s32 y_offset) {
    u16 *texture_data_ptr;
    u16 *framebuffer_ptr; 
    s32 y;
    s32 x;

    texture_data_ptr = (u16 *) textureList_getDataPtr(texture_list) + (32 * 32 * index);
    framebuffer_ptr = gFramebuffers[getActiveFramebuffer()];
    for (y = 0; y < 32; y++) {
        for (x = 0; x < 32; x++) {
            s32 fb_x = x_offset + x;
            s32 fb_y = y_offset + y;
            if (fb_x >= 0 && fb_x < gFramebufferWidth && fb_y >= 0 && fb_y < gFramebufferHeight) {
                texture_data_ptr[32 * (31 - y) + x] = framebuffer_ptr[fb_y * gFramebufferWidth + fb_x] | 0x01;
            } else {
                texture_data_ptr[32 * (31 - y) + x] = 0x01;
            }
        };
    };
}

void model_copyFramebufferToTextures(BKModelBin *this) {
    BKTextureList *texture_list;
    s32 x, y;

    texture_list = modelbin_getTextureList(this);
    // [port] The blocks below read the rendered frame from the CPU-side
    // buffer, so the frame this tick submitted has to be drawn first. One
    // sync covers the whole capture.
    port_pipelineSyncPoint();
    osInvalDCache(gFramebuffers[getActiveFramebuffer()], gFramebufferWidth * gFramebufferHeight * sizeof(u16));

    for (y = 0; y < 8; y++) {
        for (x = 0; x < 10; x++) {
            model_copyFramebufferBlockToTexture(texture_list, 10 * y + x, 32 * x + (gFramebufferWidth - 10 * 32) / 2, 32 * y + (gFramebufferHeight - 8 * 32) / 2);
        }
    };

    osWritebackDCacheAll();

    // [port] The CPU just modified texture data that LUS may have cached on the GPU.
    // Clear the texture cache so the next draw re-uploads from CPU memory.
    port_runOnRenderThread(__textureCacheClearFn, NULL);
}
