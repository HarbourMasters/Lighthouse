#include "core1/core1.h"
#include <ultra64.h>
#include "functions.h"
#include "port/Engine.h"

#include <libultra/convert.h>

static Gfx *sGfxStack[2] = { NULL, NULL };
s32 gFramebufferWidth = DEFAULT_FRAMEBUFFER_WIDTH;
s32 gFramebufferHeight = DEFAULT_FRAMEBUFFER_HEIGHT;

static Mtx *sMtxStack[2];
static Vtx *sVtxStack[2];
static s32 sStackSelector;

#define INITIAL_GFX_COUNT 3700
#define INITIAL_MTX_COUNT 700
#define INITIAL_VTX_COUNT 430

static s32 sGfxCapacity;
static s32 sMtxCapacity;
static s32 sVtxCapacity;
static s32 sGfxPeakUsed;
static s32 sMtxPeakUsed;
static s32 sVtxPeakUsed;
s32  gTextureFilterPoint;
Struct_Core1_15B30 D_80283008[20];
s32 D_802831E8;
OSMesgQueue D_802831F0;
OSMesg D_80283208;
u16  gScissorBoxLeft;
u16  gScissorBoxRight;
u16  gScissorBoxTop;
u16  gScissorBoxBottom;
Gfx *D_80283214;

void func_80253550(void){
    osRecvMesg(&D_802831F0, NULL, OS_MESG_BLOCK);
}

void func_8025357C(void){
    osSendMesgPtr(&D_802831F0, NULL, OS_MESG_BLOCK);
}

void func_802535A8(Gfx **arg0, Gfx **arg1, void *arg2, void *arg3) {
    Struct_Core1_15B30 *sp1C;

    func_80253550();
    sp1C = &D_80283008[D_802831E8];
    D_802831E8 = (s32) (D_802831E8 + 1) % 20;
    func_8025357C();
    sp1C->unk0 = 0;
    sp1C->unk8 = arg0;
    sp1C->unkC = arg1;
    sp1C->unk10 = arg2;
    sp1C->unk14 = arg3;
    func_80246670(OS_MESG_PTR(sp1C));
}


void func_80253640(Gfx ** gdl, void *arg1){
    D_80283214 = *gdl;
    gSPSegment((*gdl)++, 0x00, 0x00000000);
    gDPSetRenderMode((*gdl)++, G_RM_NOOP, G_RM_NOOP2);
    gSPClearGeometryMode((*gdl)++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH);
    gDPPipeSync((*gdl)++);
    gDPPipelineMode((*gdl)++, G_PM_NPRIMITIVE);
    gDPSetAlphaCompare((*gdl)++, G_AC_NONE);
    gDPSetColorDither((*gdl)++, G_CD_MAGICSQ);
    gDPSetScissor((*gdl)++, G_SC_NON_INTERLACE, gScissorBoxLeft, gScissorBoxRight, gScissorBoxTop, gScissorBoxBottom);
    func_80253208(gdl, 0, 0,  gFramebufferWidth, gFramebufferHeight, arg1);
    gDPSetColorImage((*gdl)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth, OS_K0_TO_PHYSICAL(arg1));
    gDPSetCycleType((*gdl)++, G_CYC_1CYCLE);
    gDPSetTextureConvert((*gdl)++, G_TC_FILT);
    gDPSetTextureDetail((*gdl)++, G_TD_CLAMP);
    if(gTextureFilterPoint){
        gDPSetTextureFilter((*gdl)++, G_TF_POINT);
    }else{
        gDPSetTextureFilter((*gdl)++, G_TF_BILERP);
    }
    gDPSetTextureLOD((*gdl)++, G_TL_TILE);
    gDPSetTextureLUT((*gdl)++, G_TT_NONE);
    gDPSetTexturePersp((*gdl)++, G_TP_PERSP);
    zBuffer_set(gdl);
}

extern s32 port_getAuxGpuFbId(void);
extern s32 port_getTransitionGpuFbId(void);
extern int port_shouldCaptureTransition(void);

void scissorBox_SetForGameMode(Gfx **gdl, s32 framebuffer_idx) {
    if(getGameMode() == GAME_MODE_8_BOTTLES_BONUS || getGameMode() == GAME_MODE_A_SNS_PICTURE)
    {
        // [port] Redirect rendering to GPU-side aux FB via gsSPSetFB.
        // On N64, gDPSetColorImage pointed to D_80382450 (CPU buffer).
        // gsSPResetFB is emitted in func_802E39D0 after the scene draw.
        s32 auxFb = port_getAuxGpuFbId();
        if (auxFb >= 0) {
            gsSPSetFB((*gdl)++, auxFb);
        }
        scissorBox_setSmall();
        func_80253640(gdl, func_8030C704());
    }
    else{
        scissorBox_setDefault();
        func_80253640(gdl, gFramebuffers[framebuffer_idx]);
        // [port] During transition capture, also redirect rendering to the
        // transition GPU FB so the scene is captured for jiggy piece textures.
        if (port_shouldCaptureTransition()) {
            s32 trFb = port_getTransitionGpuFbId();
            if (trFb >= 0) {
                gsSPSetFB((*gdl)++, trFb);
            }
        }
    }
}

void setupScissorBoxAndFramebuffer(Gfx **gfx, uintptr_t framebuffer_address){
    gSPSegment((*gfx)++, 0x00, 0x00000000);
    gDPSetColorImage((*gfx)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth, OS_PHYSICAL_TO_K0(framebuffer_address));
    gSPClearGeometryMode((*gfx)++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH);
    gSPTexture((*gfx)++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
    gSPSetGeometryMode((*gfx)++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);
    gDPSetCycleType((*gfx)++, G_CYC_1CYCLE);
    gDPPipelineMode((*gfx)++, G_PM_NPRIMITIVE);
    gDPSetCombineMode((*gfx)++, G_CC_SHADE, G_CC_SHADE);
    gDPSetAlphaCompare((*gfx)++, G_AC_NONE);
    gDPSetColorDither((*gfx)++, G_CD_DISABLE);
    gDPSetRenderMode((*gfx)++, G_RM_AA_ZB_XLU_LINE, G_RM_AA_ZB_XLU_LINE2);
    gSPClipRatio((*gfx)++, FRUSTRATIO_1);
    gDPSetScissor((*gfx)++, G_SC_NON_INTERLACE, gScissorBoxLeft, gScissorBoxRight, gScissorBoxTop, gScissorBoxBottom);
    gDPPipeSync((*gfx)++);
}

void setupDefaultScissorBoxAndFramebuffer(Gfx **gfx, s32 framebuffer_idx){
    scissorBox_setDefault();
    setupScissorBoxAndFramebuffer(gfx, (uintptr_t)gFramebuffers[framebuffer_idx]);
}

void func_80253DC0(Gfx **gfx){
    func_802476EC(gfx);
}

void finishFrame(Gfx **gdl) {
    gDPFullSync((*gdl)++);
    gSPEndDisplayList((*gdl)++);
}

void func_80253E14(Gfx *arg0, Gfx *arg1, s32 arg2){
    Struct_Core1_15B30 *sp1C;
    func_80253550();
    sp1C = D_80283008 + D_802831E8;
    D_802831E8 = (D_802831E8 + 1) % 0x14;
    func_8025357C();
    sp1C->unk0 = 1;
    sp1C->unk4 = arg2;
    sp1C->unk8 = arg0;
    sp1C->unkC = arg1;
    func_80246670(OS_MESG_PTR(sp1C));
}

void func_80253EA4(Gfx *arg0, Gfx *arg1){
    func_80253E14(arg0, arg1, 0);
}

void func_80253EC4(Gfx *arg0, Gfx *arg1){
    func_80253E14(arg0, arg1, 0x40000000);
}

void func_80253EE4(Gfx **arg0, Gfx **arg1, s32 arg2) {
    Struct_Core1_15B30 *sp1C;

    func_80253550();
    sp1C = &D_80283008[D_802831E8];
    D_802831E8 = (s32) (D_802831E8 + 1) % 20;
    func_8025357C();
    sp1C->unk0 = 2;
    sp1C->unk4 = arg2;
    sp1C->unk8 = arg0;
    sp1C->unkC = arg1;
    func_80246670(OS_MESG_PTR(sp1C));
}

void func_80253F74(Gfx **arg0, Gfx **arg1){
    func_80253EE4(arg0, arg1, 0);
}

void func_80253F94(Gfx **arg0, Gfx **arg1){
    func_80253EE4(arg0, arg1, 0x40000000);
}

void scissorBox_get(u32 *left, u32 *top, u32 *right, u32 *bottom){
   *left = gScissorBoxLeft;
   *top = gScissorBoxTop;
   *right = gScissorBoxRight;
   *bottom = gScissorBoxBottom;
}

void func_80253FE8(void){
    viMgr_func_8024BFAC();
}

void func_80254008(void){
    func_80246670(OS_MESG_32(3));
}

void func_80254028(void){
    D_802831E8 = 0;
    osCreateMesgQueue(&D_802831F0, &D_80283208, 1);
    osSendMesgPtr(&D_802831F0, NULL, 1);
    func_80247560();
    scissorBox_setDefault();
}

void drawRectangle2D(Gfx **gfx, s32 x, s32 y, s32 w, s32 h, s32 r, s32 g, s32 b){
    gDPPipeSync((*gfx)++);
    gDPPipelineMode((*gfx)++, G_PM_NPRIMITIVE);
    gDPSetCycleType((*gfx)++, G_CYC_FILL);
    gDPSetFillColor((*gfx)++, GPACK_RGBA5551(r, g, b, 1) << 16 | GPACK_RGBA5551(r, g, b, 1));
    gDPSetRenderMode((*gfx)++, G_RM_NOOP, G_RM_NOOP2);
    gDPScisFillRectangle((*gfx)++,  x, y, x + w -1, y + h -1);
}

void graphicsCache_release(void) {
    if (sGfxStack[0]) {
        bk_free(sGfxStack[0]);
        bk_free(sGfxStack[1]);
        bk_free(sMtxStack[0]);
        bk_free(sMtxStack[1]);
        bk_free(sVtxStack[0]);
        bk_free(sVtxStack[1]);
        sGfxStack[0] = NULL;
    }
}

void graphicsCache_init(void){
    if(sGfxStack[0] == NULL){
        sGfxCapacity = INITIAL_GFX_COUNT;
        sMtxCapacity = INITIAL_MTX_COUNT;
        sVtxCapacity = INITIAL_VTX_COUNT;
        sGfxPeakUsed = sMtxPeakUsed = sVtxPeakUsed = 0;
        sGfxStack[0] = (Gfx *)GameEngine_Malloc(sGfxCapacity * sizeof(Gfx));
        sGfxStack[1] = (Gfx *)GameEngine_Malloc(sGfxCapacity * sizeof(Gfx));
        sMtxStack[0] = (Mtx *)GameEngine_Malloc(sMtxCapacity * sizeof(Mtx));
        sMtxStack[1] = (Mtx *)GameEngine_Malloc(sMtxCapacity * sizeof(Mtx));
        sVtxStack[0] = (Vtx *)GameEngine_Malloc(sVtxCapacity * sizeof(Vtx));
        sVtxStack[1] = (Vtx *)GameEngine_Malloc(sVtxCapacity * sizeof(Vtx));
        dummy_func_80254464();
    }
    sStackSelector = 0;
    gTextureFilterPoint = 0;
}

void scissorBox_set(s32 left, s32 top, s32 right, s32 bottom) {
    gScissorBoxLeft = left;
    gScissorBoxTop = top;
    gScissorBoxRight = right;
    gScissorBoxBottom = bottom;
    gFramebufferWidth = top - left;
    gFramebufferHeight = bottom - right;
    viewport_pushFramebufferExtendsToVpStack();
}


void scissorBox_setDefault(void){
    scissorBox_set(0, DEFAULT_FRAMEBUFFER_WIDTH, 0, DEFAULT_FRAMEBUFFER_HEIGHT);
}

void func_80254374(s32 arg0) {
    Struct_Core1_15B30 *sp1C;

    func_80253550();
    viMgr_setActiveFramebuffer(arg0);
    sp1C = &D_80283008[D_802831E8];
    D_802831E8 = (s32) (D_802831E8 + 1) % 20;
    func_8025357C();
    sp1C->unk0 = 7;
    func_80246670(OS_MESG_PTR(sp1C));
}

void toggleTextureFilterPoint(void){
    u32 ret_val = gTextureFilterPoint;
    gTextureFilterPoint = ret_val < 1;
}

static void _graphicsCache_growIfNeeded(void **stack0, void **stack1, s32 *capacity, s32 peakUsed, size_t elemSize) {
    if (peakUsed > *capacity * 3 / 4) {
        s32 newCap = peakUsed * 2;
        GameEngine_Free(*stack0);
        GameEngine_Free(*stack1);
        *stack0 = GameEngine_Malloc(newCap * elemSize);
        *stack1 = GameEngine_Malloc(newCap * elemSize);
        *capacity = newCap;
    }
}

void graphicsCache_reportUsage(Gfx *gfxEnd, Gfx *gfxStart, Mtx *mtxEnd, Mtx *mtxStart, Vtx *vtxEnd, Vtx *vtxStart) {
    s32 gfxUsed = (s32)(gfxEnd - gfxStart);
    s32 mtxUsed = (s32)(mtxEnd - mtxStart);
    s32 vtxUsed = (s32)(vtxEnd - vtxStart);
    if (gfxUsed > sGfxPeakUsed) sGfxPeakUsed = gfxUsed;
    if (mtxUsed > sMtxPeakUsed) sMtxPeakUsed = mtxUsed;
    if (vtxUsed > sVtxPeakUsed) sVtxPeakUsed = vtxUsed;
}

void getGraphicsStacks(Gfx **gfx, Mtx **mtx, Vtx **vtx){
    _graphicsCache_growIfNeeded((void **)&sGfxStack[0], (void **)&sGfxStack[1], &sGfxCapacity, sGfxPeakUsed, sizeof(Gfx));
    _graphicsCache_growIfNeeded((void **)&sMtxStack[0], (void **)&sMtxStack[1], &sMtxCapacity, sMtxPeakUsed, sizeof(Mtx));
    _graphicsCache_growIfNeeded((void **)&sVtxStack[0], (void **)&sVtxStack[1], &sVtxCapacity, sVtxPeakUsed, sizeof(Vtx));

    sStackSelector = (1 - sStackSelector);
    *gfx = sGfxStack[sStackSelector];
    *mtx = sMtxStack[sStackSelector];
    *vtx = sVtxStack[sStackSelector];
}

void dummy_func_80254464(void) {}
