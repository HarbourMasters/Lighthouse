#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"

extern void port_registerAuxColorImage(void* cpuAddr, unsigned int width, unsigned int height);
extern void port_unregisterAuxColorImage(void* cpuAddr);
extern int port_getAuxFramebufferId(void* cpuAddr);


#define TILE_SIZE 32
#define TILE_COUNT_X 5
#define TILE_COUNT_Y 4
#define IMAGE_WIDTH (TILE_SIZE * TILE_COUNT_X)
#define IMAGE_HEIGHT (TILE_SIZE * TILE_COUNT_Y)
#define SCREEN_WIDTH 292
#define SCREEN_HEIGHT 216
#define HORIZONTAL_MARGIN (((SCREEN_WIDTH) - IMAGE_WIDTH) / 2)
#define VERTICAL_MARGIN (((SCREEN_HEIGHT) - IMAGE_HEIGHT) / 2)

/* .data */
Gfx D_8036C450[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetCombineLERP(TEXEL0, 0, PRIMITIVE_ALPHA, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, PRIMITIVE_ALPHA, 0, 0, 0, 0, TEXEL0),
    gsDPSetTextureFilter(G_TF_POINT),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetPrimColor(0, 0, 0x00, 0x00, 0x00, 0x78),
    gsSPEndDisplayList()
};

Gfx D_8036C4A8[] = {
    gsDPPipeSync(),
    gsDPSetTextureFilter(G_TF_BILERP),
    gsDPSetTexturePersp(G_TP_PERSP),
    gsSPEndDisplayList()
};

/* .bss */
s16 *D_80382450;
void *D_80382454;


/* .code */
void func_8030C160(void){
    func_8024F150();
}

void func_8030C180(void){
    func_8024F180();
}

void func_8030C1A0(void){
    if(D_80382454 == NULL){
        D_80382454 = D_80382450 = bk_malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(u16) + 64);

        while((uintptr_t)D_80382450 & 0x3F){
            D_80382450++;
        }
        // [port] Register this buffer as an auxiliary render target so the interpreter
        // redirects scene rendering into a dedicated FBO and reads it back to CPU.
        port_registerAuxColorImage(D_80382450, IMAGE_WIDTH, IMAGE_HEIGHT);
    }
}

void func_8030C204(void){
    if(D_80382454){
        port_unregisterAuxColorImage(D_80382450);
        bk_free(D_80382454);
        D_80382454 = NULL;
    }

    switch(getGameMode()){
        case GAME_MODE_8_BOTTLES_BONUS:
            chBottlesBonus_func_802DEA8C(0, 0);
            break;
        case GAME_MODE_A_SNS_PICTURE:
            func_802DF11C(0, 0);
            break;
    }
}

void func_8030C27C(void){
    switch(getGameMode()){
        case GAME_MODE_8_BOTTLES_BONUS:
            chBottlesBonus_spawn(0, 0);
            break;
        case GAME_MODE_A_SNS_PICTURE:
            func_802DF090(0, 0);
            break;

    }
}

void func_8030C2D4(Gfx **gdl, Mtx **mptr, Vtx **vptr){
    scissorBox_setDefault();
    func_80253640(gdl, gFramebuffers[getActiveFramebuffer()]);
}

// Draws the aux framebuffer content into the center of the screen.
// [port] Uses GPU-side FB directly via gDPSetTextureImageFB instead of CPU readback.
// Grayscale is applied via LUS grayscale mode (replaces N64's G_IM_FMT_IA reinterpretation).
void func_8030C33C(Gfx **gfx, Mtx **mtx, Vtx **vtx) {
    s32 auxFbId = port_getAuxFramebufferId(D_80382450);
    if (auxFbId < 0) {
        return; // GPU FB not registered yet
    }

    // Set up rendering state
    gSPDisplayList((*gfx)++, D_8036C450);
    // [port] Enable grayscale — on N64 this was achieved by loading RGBA16 as IA16 format.
    // LUS grayscale mode applies the same effect in the shader.
    gSPGrayscale((*gfx)++, 1);
    // Draw the GPU FB directly as a texture, scaled to the picture frame area
    gDPSetTextureImageFB((*gfx)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, IMAGE_WIDTH, auxFbId);
    gDPImageRectangle((*gfx)++,
        HORIZONTAL_MARGIN << 2, VERTICAL_MARGIN << 2,   // dest x0, y0 (10.2 format)
        0, 0,                                            // source x0, y0
        (HORIZONTAL_MARGIN + IMAGE_WIDTH) << 2, (VERTICAL_MARGIN + IMAGE_HEIGHT) << 2, // dest x1, y1
        IMAGE_WIDTH, IMAGE_HEIGHT,                       // source x1, y1
        G_TX_RENDERTILE,
        IMAGE_WIDTH, IMAGE_HEIGHT);                      // source image dimensions
    gSPGrayscale((*gfx)++, 0);
    // Reset rendering state
    gSPDisplayList((*gfx)++, D_8036C4A8);
}


s16 *func_8030C704(void){
    return D_80382450;
}

void scissorBox_setSmall(void){
    scissorBox_set(0, 160, 0, 128);
}
