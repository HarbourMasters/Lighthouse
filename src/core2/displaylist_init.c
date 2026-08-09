// BanjoDecomp: core2/code_8DC20.c
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>
#include <string.h>

#include <libultra/convert.h>
#include "port/Engine.h"
#include "port/Patches/Patches.h"

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
    // [port] GPU-side pause snapshot
    s32 pauseFb = port_getPauseFramebufferId();
    bool isCapture = (frame_buffer_1 == depthbuffer_getDataPtr());

    gSPDisplayList((*gfx)++, D_8036C630);

    if (isCapture) {
        // First frame: size the pause FB to the current render res (only changes on a
        // resolution/aspect toggle between pauses), then copy the backbuffer into it.
        pauseFb = port_capturePauseFramebuffer();
        gDPCopyFB((*gfx)++, pauseFb, 0, 0, NULL);
    }

    // Draw the saved pause FB as a full-screen background, edge-to-edge in widescreen
    s32 renderW;
    s32 renderH;
    port_getPauseFramebufferSize(&renderW, &renderH);
    s32 x0 = OTRGetRectDimensionFromLeftEdge(0);
    s32 x1 = OTRGetRectDimensionFromRightEdge((f32)gFramebufferWidth);

    gDPSetTextureImageFB((*gfx)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth, pauseFb);
    {
        int16_t uls = port_mirror_shouldFlipPauseBg() ? renderW : 0;
        int16_t lrs = port_mirror_shouldFlipPauseBg() ? 0 : renderW;
        gDPImageRectangle((*gfx)++,
            x0 << 2, 0,
            uls, 0,
            x1 << 2, gFramebufferHeight << 2,
            lrs, renderH,
            G_TX_RENDERTILE,
            renderW, renderH);
    }

    gSPDisplayList((*gfx)++, D_8036C690);
    gDPSetColorImage((*gfx)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth, OS_PHYSICAL_TO_K0(gFramebuffers[getActiveFramebuffer()]));
}

void func_80315084(Gfx **gfx, Mtx **mtx, Vtx **vtx){
    gsworld_setEnableDraw(0);
    D_803830A0 = 2;
    func_80314BB0(gfx, mtx, vtx, depthbuffer_getDataPtr(), gFramebuffers[getActiveFramebuffer()]);
    port_mirror_markCapture();
}

void func_80315110(Gfx **gfx, Mtx **mtx, Vtx **vtx){
    if(!D_803830A0){
        if(gsworld_getMap() != MAP_90_GL_BATTLEMENTS){
            func_803306C8(2);
            func_8032AD7C(2);
        }
    }
    else{
        D_803830A0--;
    }
    func_80314BB0(gfx, mtx, vtx, gFramebuffers[getActiveFramebuffer()], depthbuffer_getDataPtr());
}

void func_803151D0(Gfx **gfx, Mtx **mtx, Vtx **vtx){
    gsworld_setEnableDraw(1);
}
