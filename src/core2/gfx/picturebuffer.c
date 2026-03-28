#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"

int gfx_create_framebuffer(unsigned int width, unsigned int height, unsigned int native_width,
                           unsigned int native_height, unsigned char resize);

extern BKGfxList *model_getDisplayList(BKModelBin *arg0);

// [port] GPU-side framebuffer for aux picture (SNS/Bottles Bonus).
// Created via gfx_create_framebuffer (standard LUS path) so it integrates
// properly with gsSPSetFB and gDPSetTextureImageFB.
static s32 sAuxGpuFbId = -1;


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
    // [port] Create GPU FB for aux picture rendering. The CPU buffer is kept
    // as a dummy for legacy code paths but no data is read back into it.
    if(D_80382454 == NULL){
        D_80382454 = D_80382450 = bk_malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(u16) + 64);
        while((uintptr_t)D_80382450 & 0x3F){
            D_80382450++;
        }
    }
    if (sAuxGpuFbId < 0) {
        sAuxGpuFbId = gfx_create_framebuffer(IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_HEIGHT, 0);
    }
}

void func_8030C204(void){
    if(D_80382454){
        bk_free(D_80382454);
        D_80382454 = NULL;
    }
    // [port] GPU FB persists — don't delete, just leave it for reuse.

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

s32 port_getAuxGpuFbId(void) {
    return sAuxGpuFbId;
}

// [port] Read the GPU aux FB into D_80382450 so model textures can load
// tiles from it. The texture cache clear forces re-import each frame.
void port_readAuxFbToCpu(Gfx **gfx) {
    if (sAuxGpuFbId >= 0 && D_80382450 != NULL) {
        gDPReadFB((*gfx)++, sAuxGpuFbId, (u16 *)D_80382450, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
        __gSPInvalidateTexCache((*gfx)++, 0);
    }
}

// [port] Patch a picture model's display list to load the entire 160x128 image
// as a single texture instead of individual 32x32 tiles. This eliminates seams
// when bilinear filtering is used. Also recalculates vertex UVs to map the full
// image. Based on BanjoRecomp's picture_patches.c approach.
// from: 0 = map X,Z to U,V (FROM_XZ), 1 = map Y,Z to U,V (FROM_YZ)
#define FROM_XZ 0
#define FROM_YZ 1

void port_patchPictureModel(BKModelBin *model_bin, s32 min_xy, s32 max_xy, s32 min_z, s32 max_z, u32 from) {
    if (model_bin->pad0[0] == 0xBA) {
        return;
    }
    model_bin->pad0[0] = 0xBA;

    BKGfxList *gfx_list = model_getDisplayList(model_bin);
    Vtx *vtx_list = model_getVtxList(model_bin)->vtx_18;
    Gfx *cur_gfx = &gfx_list->list[0];
    Gfx *end_gfx = &gfx_list->list[*(s32 *)&gfx_list->pad0[0]];
    s32 in_framebuffer_tile = 0;

    // [port] The model importer tags segmented addresses with bit 0 (w1 |= 1)
    // so the interpreter can identify them. Match against the tagged value.
    #define SEG4_TAGGED ((uintptr_t)0x04000000 | 1)

    while (cur_gfx < end_gfx) {
        if ((cur_gfx->words.w0 >> 24) == G_SETTIMG) {
            in_framebuffer_tile = ((cur_gfx->words.w1 & ~(uintptr_t)1) == 0x04000000);
            if (in_framebuffer_tile) {
                Gfx *g = cur_gfx;
                Gfx *ng = cur_gfx + 1;
                u8 fmt = (ng->words.w0 >> 21) & 0x7;
                gDPLoadTextureTile(g++, SEG4_TAGGED, fmt, G_IM_SIZ_16b,
                    IMAGE_WIDTH, IMAGE_HEIGHT, 0, 0, IMAGE_WIDTH - 1, IMAGE_HEIGHT - 1,
                    0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP,
                    G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            }
        } else if ((cur_gfx->words.w0 >> 24) == G_SETOTHERMODE_H) {
            // [port] If the model's DL sets G_TF_POINT for framebuffer tiles,
            // override to G_TF_BILERP so the full-image texture is smoothly filtered.
            u32 sft = (cur_gfx->words.w0 >> 8) & 0xFF;
            u32 len = (cur_gfx->words.w0 & 0xFF) + 1;
            if (sft == (32 - G_MDSFT_TEXTFILT - 2) && len == 2) {
                if (cur_gfx->words.w1 == G_TF_POINT) {
                    cur_gfx->words.w1 = G_TF_BILERP;
                }
            }
        } else if ((cur_gfx->words.w0 >> 24) == G_VTX) {
            u32 cur_vtx_count = (cur_gfx->words.w0 >> 10) & 0x3F;
            Vtx *cur_vtx;
            if ((cur_gfx->words.w1 >> 24) == 0x01) {
                // Mask out bit 0 (segment tag) from the offset
                cur_vtx = (Vtx *)((cur_gfx->words.w1 & 0x00FFFFFE) + (u8 *)vtx_list);
            } else {
                cur_vtx = (Vtx *)(uintptr_t)cur_gfx->words.w1;
            }

            if (in_framebuffer_tile) {
                u32 i;
                for (i = 0; i < cur_vtx_count; i++) {
                    f32 xy_frac, z_frac;
                    if (from == FROM_YZ) {
                        xy_frac = (cur_vtx[i].v.ob[1] - (f32)min_xy) / (max_xy - min_xy);
                        z_frac  = (cur_vtx[i].v.ob[2] - (f32)min_z)  / (max_z - min_z);
                        cur_vtx[i].v.tc[0] = (s16)(z_frac * IMAGE_WIDTH * 64.0f);
                        cur_vtx[i].v.tc[1] = (s16)((1.0f - xy_frac) * IMAGE_HEIGHT * 64.0f);
                    } else {
                        xy_frac = (cur_vtx[i].v.ob[0] - (f32)min_xy) / (max_xy - min_xy);
                        z_frac  = (cur_vtx[i].v.ob[2] - (f32)min_z)  / (max_z - min_z);
                        cur_vtx[i].v.tc[0] = (s16)(xy_frac * IMAGE_WIDTH * 64.0f);
                        cur_vtx[i].v.tc[1] = (s16)((1.0f - z_frac) * IMAGE_HEIGHT * 64.0f);
                    }
                }
            }
        }
        cur_gfx++;
    }
}


s16 *func_8030C704(void){
    return D_80382450;
}

void scissorBox_setSmall(void){
    scissorBox_set(0, 160, 0, 128);
}
