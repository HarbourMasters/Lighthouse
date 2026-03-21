#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>
#include <string.h>

#include <libultra/convert.h>

Gfx D_8036C630[] =
{
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetCombineLERP(TEXEL0, 0, PRIMITIVE_ALPHA, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, PRIMITIVE_ALPHA, 0, 0, 0, 0, TEXEL0),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetPrimColor(0, 0, 0x00, 0x00, 0x00, 0xFF),
    gsDPSetColorDither(G_CD_DISABLE),
    gsSPEndDisplayList()
};

Gfx D_8036C690[] = 
{
    gsDPPipeSync(),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsDPSetColorDither(G_CD_MAGICSQ),
    gsSPEndDisplayList(),
};

/* .bss */
s32 D_803830A0;


/* .code */
void func_80314BB0(Gfx **gfx, Mtx **mtx, Vtx **vtx, void * frame_buffer_1, void *frame_buffer_2) {
    s32 x;
    s32 y;

    // [port] On N64, the RDP copies frame_buffer_2 → frame_buffer_1 via gDPSetColorImage
    // redirect. On PC, gDPSetColorImage doesn't redirect GPU output to CPU buffers,
    // so do the copy via CPU.
    memcpy(frame_buffer_1, frame_buffer_2, gFramebufferWidth * gFramebufferHeight * sizeof(u16));

    gSPDisplayList((*gfx)++, D_8036C630);
    __gSPInvalidateTexCache((*gfx)++, 0);
    gDPSetColorImage((*gfx)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth, OS_PHYSICAL_TO_K0(frame_buffer_1));
    // [port] Draw tiles to cover the viewport. At widescreen, expand X from center
    // to fill the full viewport. AdjXForAspectRatio in LUS squeezes X by (4/3)/windowAspect;
    // we pre-compensate by expanding X by vpW/320.
    {
        s32 vpW = port_getViewportWidth();
        if (vpW <= 320) {
            // 4:3: original tiles
            for(y = 0; y < gFramebufferHeight / 32 + 1; y++){
                for(x = 0; x < gFramebufferWidth / 32 + 1; x++){
                    gDPLoadTextureTile((*gfx)++, osVirtualToPhysical(frame_buffer_2), G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth, gFramebufferHeight,
                        0x20*x, 0x20*y, 0x20*(x + 1) - 1, 0x20*(y + 1) - 1,
                        0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, 0, 0
                    );
                    gSPScisTextureRectangle((*gfx)++, (0x20*x)*4, (0x20*y)*4, 0x20*(x + 1)*4, (0x20*(y + 1)*4),
                        G_TX_RENDERTILE, (0x20*x)<<5, (0x20*y)<<5, 0x400, 0x400
                    );
                }
            }
        } else {
            // Widescreen: center-relative X expansion
            s32 fbHalfW = gFramebufferWidth / 2;
            s32 xScale100 = vpW * 100 / 320;

            for(y = 0; y < gFramebufferHeight / 32 + 1; y++){
                for(x = 0; x < gFramebufferWidth / 32 + 1; x++){
                    s32 tx0 = 0x20 * x;
                    s32 ty0 = 0x20 * y;
                    s32 tx1 = 0x20 * (x + 1);
                    s32 ty1 = 0x20 * (y + 1);
                    s32 sx0 = fbHalfW + (tx0 - fbHalfW) * xScale100 / 100;
                    s32 sx1 = fbHalfW + (tx1 - fbHalfW) * xScale100 / 100;
                    s32 tileW = sx1 - sx0;
                    s32 sScale = (tileW > 0) ? (32 * 0x400 / tileW) : 0x400;

                    gDPLoadTextureTile((*gfx)++, osVirtualToPhysical(frame_buffer_2), G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth, gFramebufferHeight,
                        tx0, ty0, tx1 - 1, ty1 - 1,
                        0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, 0, 0
                    );
                    gSPWideTextureRectangle((*gfx)++,
                        sx0 * 4, ty0 * 4, sx1 * 4, ty1 * 4,
                        G_TX_RENDERTILE, tx0 << 5, ty0 << 5, sScale, 0x400
                    );
                }
            }
        }
    }
    gSPDisplayList((*gfx)++, D_8036C690);
    gDPSetColorImage((*gfx)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth, OS_PHYSICAL_TO_K0(gFramebuffers[getActiveFramebuffer()]));
}

void func_80315084(Gfx **gfx, Mtx **mtx, Vtx **vtx){
    func_80335128(0);
    D_803830A0 = 2;
    func_80314BB0(gfx, mtx, vtx, zBuffer_get(), gFramebuffers[getActiveFramebuffer()]);
}

void func_80315110(Gfx **gfx, Mtx **mtx, Vtx **vtx){
    if(!D_803830A0){
        if(map_get() != MAP_90_GL_BATTLEMENTS){
            func_803306C8(2);
            func_8032AD7C(2);
        }
    }
    else{
        D_803830A0--;
    }
    func_80314BB0(gfx, mtx, vtx, gFramebuffers[getActiveFramebuffer()], zBuffer_get());
}

void func_803151D0(Gfx **gfx, Mtx **mtx, Vtx **vtx){
    func_80335128(1);
}
