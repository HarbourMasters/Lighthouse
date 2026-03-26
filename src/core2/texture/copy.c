#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"

extern void gfx_texture_cache_clear(void);
extern u16 port_sampleHiresReadback(int x, int y);

u8 *func_802EA620(BKTextureList *texture_list);

// [port] Samples from full-res GPU readback instead of 292x216 gFramebuffers.
void func_802FEDE0(BKTextureList *texture_list, s32 indx, s32 x_offset, s32 y_offset){
    u16 *sp24;
    // u16 *frame_buffer_ptr; // [port] replaced by high-res readback sampling
    s32 y;
    s32 x;

    sp24 = (u16*)func_802EA620(texture_list) + indx*32*32;
    // frame_buffer_ptr = gFramebuffers[getActiveFramebuffer()]; // [port] replaced
    for(y = 0; y < 32; y++){
        for(x = 0; x < 32; x++){
            // [port] Alpha bit is at position 8 in byte-swapped RGBA16
            sp24[32*(31 - y) + x] = port_sampleHiresReadback(x_offset + x, y_offset + y) | 0x100;
        };
    };
}

//framebuffer_to_model_texture
void func_802FEF48(BKModelBin *model_bin){
    BKTextureList *texture_list;
    s32 x, y;

    texture_list = model_getTextureList(model_bin);
    osInvalDCache((void *)gFramebuffers[getActiveFramebuffer()], gFramebufferWidth * gFramebufferHeight*2);

    for(y = 0; y < 8; y++){
        for(x = 0; x < 10; x++){
            func_802FEDE0(texture_list, 10*y + x, 32*x + (gFramebufferWidth - 10*32)/2, (s32)32*y + (gFramebufferHeight - 8*32)/2);
        }
    };

    osWritebackDCacheAll();

    // [port] The CPU just modified texture data that LUS may have cached on the GPU.
    // Clear the texture cache so the next draw re-uploads from CPU memory.
    gfx_texture_cache_clear();
}
