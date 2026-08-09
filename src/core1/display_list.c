// BanjoDecomp: code_15B30.c
#include "core1/core1.h"
#include <ultra64.h>
#include "functions.h"
#include "port/ShipUtils.h" // BK_LOG_*, port_shapeControllerInput
#include "port/Engine.h"

#include <libultra/convert.h>
#include "port/Patches/Patches.h"

static Gfx *sGfxStack[2] = { NULL, NULL };
s32 gFramebufferWidth = DEFAULT_FRAMEBUFFER_WIDTH;
s32 gFramebufferHeight = DEFAULT_FRAMEBUFFER_HEIGHT;

static Mtx *sMtxStack[2];
static Vtx *sVtxStack[2];
static s32 sStackSelector;

// [port] Expand DL stack sizes to support draw distance enhancements
#define GFX_STACK_COUNT 11100
#define MTX_STACK_COUNT 2100
#define VTX_STACK_COUNT 1290

bool sUseTexturePointFilter;
struct ucode_task_data_s sUcodeTaskData[20];
s32 sCurrentUcodeTaskDataID;
OSMesgQueue sTaskDataListLockMesgQueue;
OSMesg sTaskDataListLockMesg;
u16  gScissorBoxLeft;
u16  gScissorBoxRight;
u16  gScissorBoxTop;
u16  gScissorBoxBottom;
Gfx *D_80283214; // never used

void core1_15B30_requestLockForTaskDataID(void) {
    osRecvMesg(&sTaskDataListLockMesgQueue, NULL, OS_MESG_BLOCK);
}

void core1_15B30_requestReleaseForTaskDataID(void) {
    osSendMesgPtr(&sTaskDataListLockMesgQueue, NULL, OS_MESG_BLOCK);
}

void core1_15B30_addAudioTaskData(Acmd *arg0, Acmd *arg1, OSMesgQueue *arg2, OSMesg arg3) {
    struct ucode_task_data_s *task_data;

    core1_15B30_requestLockForTaskDataID();
    task_data = &sUcodeTaskData[sCurrentUcodeTaskDataID];
    sCurrentUcodeTaskDataID = (sCurrentUcodeTaskDataID + 1) % 20;
    core1_15B30_requestReleaseForTaskDataID();
    task_data->task_type = UCODE_TASK_TYPE_AUDIO;
    task_data->data_ptr = arg0;
    task_data->data_ptr_end = arg1;
    task_data->audio_mesg_queue = arg2;
    task_data->audio_mesg = arg3;
    thread5_sendTaskToQueue(OS_MESG_PTR(task_data));
}

void setupFramebuffer(Gfx **gfx, void *color_buffer) {
    D_80283214 = *gfx;
    gSPSegment((*gfx)++, 0x00, 0x00000000);
    gDPSetRenderMode((*gfx)++, G_RM_NOOP, G_RM_NOOP2);
    gSPClearGeometryMode((*gfx)++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH);
    gDPPipeSync((*gfx)++);
    gDPPipelineMode((*gfx)++, G_PM_NPRIMITIVE);
    gDPSetAlphaCompare((*gfx)++, G_AC_NONE);
    gDPSetColorDither((*gfx)++, G_CD_MAGICSQ);
    gDPSetScissor((*gfx)++, G_SC_NON_INTERLACE, gScissorBoxLeft, gScissorBoxRight, gScissorBoxTop, gScissorBoxBottom);
    depthbuffer_clearRegion(gfx, 0, 0,  gFramebufferWidth, gFramebufferHeight, color_buffer);
    gDPSetColorImage((*gfx)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth, OS_K0_TO_PHYSICAL(color_buffer));
    gDPSetCycleType((*gfx)++, G_CYC_1CYCLE);
    gDPSetTextureConvert((*gfx)++, G_TC_FILT);
    gDPSetTextureDetail((*gfx)++, G_TD_CLAMP);
    if (sUseTexturePointFilter) {
        gDPSetTextureFilter((*gfx)++, G_TF_POINT);
    }else{
        gDPSetTextureFilter((*gfx)++, G_TF_BILERP);
    }
    gDPSetTextureLOD((*gfx)++, G_TL_TILE);
    gDPSetTextureLUT((*gfx)++, G_TT_NONE);
    gDPSetTexturePersp((*gfx)++, G_TP_PERSP);
    depthbuffer_set(gfx);
}

void setupFramebufferForGamemode(Gfx **gdl, s32 framebuffer_idx) {
    if(getGameMode() == GAME_MODE_8_BOTTLES_BONUS || getGameMode() == GAME_MODE_A_SNS_PICTURE)
    {
        EventSystem_Should(VB_PICTUREBOX_TARGET_FB, true, gdl);
        picturebox_setScissorBox();
        setupFramebuffer(gdl, picturebox_getColorBuffer());
    }
    else{
        scissorBox_setDefault();
        setupFramebuffer(gdl, gFramebuffers[framebuffer_idx]);
    }
}

void setupScissorBoxAndFramebuffer(Gfx **gfx, void *color_buffer) {
    gSPSegment((*gfx)++, 0x00, 0x00000000);
    gDPSetColorImage((*gfx)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth, OS_PHYSICAL_TO_K0(color_buffer));
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
    setupScissorBoxAndFramebuffer(gfx, gFramebuffers[framebuffer_idx]);
}

void core1_15B30_finishDList_renderThread(Gfx **gfx) {
    thread5_finishDList(gfx);
}

void core1_15B30_finishDList(Gfx **gfx) {
    gDPFullSync((*gfx)++);
    gSPEndDisplayList((*gfx)++);
}

void core1_15B30_addF3DEXTaskData(Gfx *start, Gfx *end, s32 flags) {
    struct ucode_task_data_s *task_data;
    core1_15B30_requestLockForTaskDataID();
    task_data = sUcodeTaskData + sCurrentUcodeTaskDataID;
    sCurrentUcodeTaskDataID = (sCurrentUcodeTaskDataID + 1) % 20;
    core1_15B30_requestReleaseForTaskDataID();
    task_data->task_type = UCODE_TASK_TYPE_F3DEX;
    task_data->unk4 = flags;
    task_data->data_ptr = start;
    task_data->data_ptr_end = end;
    thread5_sendTaskToQueue(OS_MESG_PTR(task_data));
}

void core1_15B30_addF3DEXTaskData_0(Gfx *start, Gfx *end) {
    core1_15B30_addF3DEXTaskData(start, end, 0);
}

void core1_15B30_addF3DEXTaskData_40000000(Gfx *start, Gfx *end) {
    core1_15B30_addF3DEXTaskData(start, end, 0x40000000);
}

void core1_15B30_addL3DEXTaskData(Gfx **start, Gfx **end, s32 flags) {
    struct ucode_task_data_s *task_data;

    core1_15B30_requestLockForTaskDataID();
    task_data = &sUcodeTaskData[sCurrentUcodeTaskDataID];
    sCurrentUcodeTaskDataID = (sCurrentUcodeTaskDataID + 1) % 20;
    core1_15B30_requestReleaseForTaskDataID();
    task_data->task_type = UCODE_TASK_TYPE_L3DEX;
    task_data->unk4 = flags;
    task_data->data_ptr = start;
    task_data->data_ptr_end = end;
    thread5_sendTaskToQueue(OS_MESG_PTR(task_data));
}

void core1_15B30_addL3DEXTaskData_0(Gfx **start, Gfx **end) {
    core1_15B30_addL3DEXTaskData(start, end, 0);
}

void core1_15B30_addL3DEXTaskData_40000000(Gfx **start, Gfx **end) {
    core1_15B30_addL3DEXTaskData(start, end, 0x40000000);
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

void core1_15B30_sendMesg3ToRenderThread(void) {
    thread5_sendTaskToQueue(OS_MESG_32(THREAD5_MESSAGE_EVENT_SYNC));
}

void core1_15B30_init(void) {
    sCurrentUcodeTaskDataID = 0;
    osCreateMesgQueue(&sTaskDataListLockMesgQueue, &sTaskDataListLockMesg, 1);
    osSendMesgPtr(&sTaskDataListLockMesgQueue, NULL, 1);
    thread5_create();
    scissorBox_setDefault();
}

void drawRectangle2D(Gfx **gfx, s32 x, s32 y, s32 w, s32 h, s32 r, s32 g, s32 b) {
    gDPPipeSync((*gfx)++);
    gDPPipelineMode((*gfx)++, G_PM_NPRIMITIVE);
    gDPSetCycleType((*gfx)++, G_CYC_FILL);
    gDPSetFillColor((*gfx)++, GPACK_RGBA5551(r, g, b, 1) << 16 | GPACK_RGBA5551(r, g, b, 1));
    gDPSetRenderMode((*gfx)++, G_RM_NOOP, G_RM_NOOP2);
    gDPScisFillRectangle((*gfx)++, x, y, x + w - 1, y + h - 1);
}

void graphicsCache_release(void) {
    if (sGfxStack[0]) {
        GameEngine_Free(sGfxStack[0]);
        GameEngine_Free(sGfxStack[1]);
        GameEngine_Free(sMtxStack[0]);
        GameEngine_Free(sMtxStack[1]);
        GameEngine_Free(sVtxStack[0]);
        GameEngine_Free(sVtxStack[1]);
        sGfxStack[0] = NULL;
    }
}

void graphicsCache_init(void) {
    if (sGfxStack[0] == NULL) {
#if 0
        sGfxStack[0] = (Gfx *)malloc(29600); // 3700 dlist commands
        sGfxStack[1] = (Gfx *)malloc(29600);
        sMtxStack[0] = (Mtx *)malloc(44800); // 700 matrices
        sMtxStack[1] = (Mtx *)malloc(44800);
        sVtxStack[0] = (Vtx *)malloc(6880); // 430 vertices
        sVtxStack[1] = (Vtx *)malloc(6880);
#endif
        sGfxStack[0] = (Gfx *)GameEngine_Malloc(GFX_STACK_COUNT * sizeof(Gfx));
        sGfxStack[1] = (Gfx *)GameEngine_Malloc(GFX_STACK_COUNT * sizeof(Gfx));
        sMtxStack[0] = (Mtx *)GameEngine_Malloc(MTX_STACK_COUNT * sizeof(Mtx));
        sMtxStack[1] = (Mtx *)GameEngine_Malloc(MTX_STACK_COUNT * sizeof(Mtx));
        sVtxStack[0] = (Vtx *)GameEngine_Malloc(VTX_STACK_COUNT * sizeof(Vtx));
        sVtxStack[1] = (Vtx *)GameEngine_Malloc(VTX_STACK_COUNT * sizeof(Vtx));
        dummy_func_80254464();
    }


    sStackSelector = 0;
    sUseTexturePointFilter = FALSE;
}

// [port] Verify the frame's dlist emission fit within the static stack capacities.
void graphicsCache_checkFrame(Gfx *gfxStart, Gfx *gfxEnd, Mtx *mtxStart, Mtx *mtxEnd, Vtx *vtxStart, Vtx *vtxEnd) {
    s32 gfxUsed = (s32)(gfxEnd - gfxStart);
    s32 mtxUsed = (s32)(mtxEnd - mtxStart);
    s32 vtxUsed = (s32)(vtxEnd - vtxStart);
    if (gfxUsed > GFX_STACK_COUNT) {
        BK_LOG_ERROR("Gfx stack overflow: %d used vs %d capacity", gfxUsed, GFX_STACK_COUNT);
    }
    if (mtxUsed > MTX_STACK_COUNT) {
        BK_LOG_ERROR("Mtx stack overflow: %d used vs %d capacity", mtxUsed, MTX_STACK_COUNT);
    }
    if (vtxUsed > VTX_STACK_COUNT) {
        BK_LOG_ERROR("Vtx stack overflow: %d used vs %d capacity", vtxUsed, VTX_STACK_COUNT);
    }
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

void core1_15B30_addTask7TaskData(s32 framebuffer_id) {
    struct ucode_task_data_s *task_data;

    core1_15B30_requestLockForTaskDataID();
    viMgr_setActiveFramebuffer(framebuffer_id);
    task_data = &sUcodeTaskData[sCurrentUcodeTaskDataID];
    sCurrentUcodeTaskDataID = (sCurrentUcodeTaskDataID + 1) % 20;
    core1_15B30_requestReleaseForTaskDataID();
    task_data->task_type = UCODE_TASK_TYPE_FRAMEBUFFER_CHANGED;
    thread5_sendTaskToQueue(OS_MESG_PTR(task_data));
}

void core1_15B30_toggleTexturePointFilter(void) {
    sUseTexturePointFilter = !sUseTexturePointFilter;
}

void graphicscache_swapAndGetStacks(Gfx **gfx, Mtx **mtx, Vtx **vtx) {
    sStackSelector = (1 - sStackSelector);
    *gfx = sGfxStack[sStackSelector];
    *mtx = sMtxStack[sStackSelector];
    *vtx = sVtxStack[sStackSelector];
}

void dummy_func_80254464(void) {}
