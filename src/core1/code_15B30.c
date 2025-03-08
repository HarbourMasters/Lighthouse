#include "core1/core1.h"
#include <ultra64.h>

//gfxtask.c


static Gfx *sGfxStack[2] = {NULL, NULL};
#ifndef LIGHTHOUSE_P
s32 gFramebufferWidth = DEFAULT_FRAMEBUFFER_WIDTH;
s32 gFramebufferHeight = DEFAULT_FRAMEBUFFER_HEIGHT;
#endif

static Mtx *sMtxStack[2];
static Vtx *sVtxStack[2];
static s32 sStackSelector;
s32 gTextureFilterPoint;
Struct_Core1_15B30 gfxTaskQueue[20];
s32 gfxTaskQueueIndex;
OSMesgQueue gfxTaskMesgQueue;
OSMesg gfxTaskMesg;
u16 gScissorBoxLeft;
u16 gScissorBoxRight;
u16 gScissorBoxTop;
u16 gScissorBoxBottom;
Gfx *currentGfxPtr;

void waitForGfxTask(void) {
  osRecvMesg(&gfxTaskMesgQueue, NULL, OS_MESG_BLOCK);
}

void signalGfxTask(void) {
#ifdef LIGHTHOUSE_P
  osSendMesg(&gfxTaskMesgQueue, (OSMesg)NULL, OS_MESG_BLOCK);
#else
  osSendMesg(&gfxTaskMesgQueue, NULL, OS_MESG_BLOCK);
#endif
}

void enqueueGfxTask(Gfx **arg0, Gfx **arg1, UNK_TYPE(s32) arg2,
                    UNK_TYPE(s32) arg3) {
  Struct_Core1_15B30 *sp1C;

  waitForGfxTask();
  sp1C = &gfxTaskQueue[gfxTaskQueueIndex];
  gfxTaskQueueIndex = (s32)(gfxTaskQueueIndex + 1) % 20;
  signalGfxTask();
  sp1C->unk0 = 0;
  sp1C->unk8 = arg0;
  sp1C->unkC = arg1;
  sp1C->unk10 = arg2;
  sp1C->unk14 = arg3;
  sendMesgToMainQueue(sp1C);
}

void setupGfxTask(Gfx **gdl, void *arg1) {
  currentGfxPtr = *gdl;
  __gSPSegment((*gdl)++, 0x00, 0x00000000);
  gDPSetRenderMode((*gdl)++, G_RM_NOOP, G_RM_NOOP2);
  gSPClearGeometryMode((*gdl)++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG |
                                     G_LIGHTING | G_TEXTURE_GEN |
                                     G_TEXTURE_GEN_LINEAR | G_LOD |
                                     G_SHADING_SMOOTH);
  gDPPipeSync((*gdl)++);
  gDPPipelineMode((*gdl)++, G_PM_NPRIMITIVE);
  gDPSetAlphaCompare((*gdl)++, G_AC_NONE);
  gDPSetColorDither((*gdl)++, G_CD_MAGICSQ);
  gDPSetScissor((*gdl)++, G_SC_NON_INTERLACE, gScissorBoxLeft, gScissorBoxRight,
                gScissorBoxTop, gScissorBoxBottom);
  depthBuffer_draw(gdl, 0, 0, gFramebufferWidth, gFramebufferHeight, arg1);
  gDPSetColorImage((*gdl)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth,
                   OS_K0_TO_PHYSICAL(arg1));
  gDPSetCycleType((*gdl)++, G_CYC_1CYCLE);
  gDPSetTextureConvert((*gdl)++, G_TC_FILT);
  gDPSetTextureDetail((*gdl)++, G_TD_CLAMP);
  if (gTextureFilterPoint) {
    gDPSetTextureFilter((*gdl)++, G_TF_POINT);
  } else {
    gDPSetTextureFilter((*gdl)++, G_TF_BILERP);
  }
  gDPSetTextureLOD((*gdl)++, G_TL_TILE);
  gDPSetTextureLUT((*gdl)++, G_TT_NONE);
  gDPSetTexturePersp((*gdl)++, G_TP_PERSP);
  depthBuffer_set(gdl);
}

void scissorBox_SetForGameMode(Gfx **gdl, s32 framebuffer_idx) {
  if (game_getMode() == GAME_MODE_8_BOTTLES_BONUS ||
      game_getMode() == GAME_MODE_A_SNS_PICTURE) {
    scissorBox_setSmall();
    setupGfxTask(gdl, func_8030C704());
  } else {
    scissorBox_setDefault();
    setupGfxTask(gdl, gFramebuffers[framebuffer_idx]);
  }
}

void setupScissorBoxAndFramebuffer(Gfx **gfx, s32 framebuffer_address) {
  __gSPSegment((*gfx)++, 0x00, 0x00000000);
  gDPSetColorImage((*gfx)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth,
                   OS_PHYSICAL_TO_K0(framebuffer_address));
  gSPClearGeometryMode((*gfx)++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG |
                                     G_LIGHTING | G_TEXTURE_GEN |
                                     G_TEXTURE_GEN_LINEAR | G_LOD |
                                     G_SHADING_SMOOTH);
  gSPTexture((*gfx)++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
  gSPSetGeometryMode((*gfx)++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);
  gDPSetCycleType((*gfx)++, G_CYC_1CYCLE);
  gDPPipelineMode((*gfx)++, G_PM_NPRIMITIVE);
  gDPSetCombineMode((*gfx)++, G_CC_SHADE, G_CC_SHADE);
  gDPSetAlphaCompare((*gfx)++, G_AC_NONE);
  gDPSetColorDither((*gfx)++, G_CD_DISABLE);
  gDPSetRenderMode((*gfx)++, G_RM_AA_ZB_XLU_LINE, G_RM_AA_ZB_XLU_LINE2);
  gSPClipRatio((*gfx)++, FRUSTRATIO_1);
  gDPSetScissor((*gfx)++, G_SC_NON_INTERLACE, gScissorBoxLeft, gScissorBoxRight,
                gScissorBoxTop, gScissorBoxBottom);
  gDPPipeSync((*gfx)++);
}

void setupDefaultScissorBoxAndFramebuffer(Gfx **gfx, s32 framebuffer_idx) {
  scissorBox_setDefault();
  setupScissorBoxAndFramebuffer(gfx, gFramebuffers[framebuffer_idx]);
}

void endGfxTask(Gfx **gfx) { endDisplayList(gfx); }

void finishFrame(Gfx **gdl) {
  gDPFullSync((*gdl)++);
  gSPEndDisplayList((*gdl)++);
}

void enqueueGfxTaskWithArg(Gfx *arg0, Gfx *arg1, s32 arg2) {
  Struct_Core1_15B30 *sp1C;
  waitForGfxTask();
  sp1C = gfxTaskQueue + gfxTaskQueueIndex;
  gfxTaskQueueIndex = (gfxTaskQueueIndex + 1) % 0x14;
  signalGfxTask();
  sp1C->unk0 = 1;
  sp1C->unk4 = arg2;
  sp1C->unk8 = arg0;
  sp1C->unkC = arg1;
  sendMesgToMainQueue(sp1C);
}

void enqueueGfxTaskNoArg(Gfx *arg0, Gfx *arg1) {
  enqueueGfxTaskWithArg(arg0, arg1, 0);
}

void enqueueGfxTaskWithFlag(Gfx *arg0, Gfx *arg1) {
  enqueueGfxTaskWithArg(arg0, arg1, 0x40000000);
}

void enqueueGfxTaskWithArgs(Gfx **arg0, Gfx **arg1, s32 arg2) {
  Struct_Core1_15B30 *sp1C;

  waitForGfxTask();
  sp1C = &gfxTaskQueue[gfxTaskQueueIndex];
  gfxTaskQueueIndex = (s32)(gfxTaskQueueIndex + 1) % 20;
  signalGfxTask();
  sp1C->unk0 = 2;
  sp1C->unk4 = arg2;
  sp1C->unk8 = arg0;
  sp1C->unkC = arg1;
  sendMesgToMainQueue(sp1C);
}

void enqueueGfxTaskNoArgs(Gfx **arg0, Gfx **arg1) {
  enqueueGfxTaskWithArgs(arg0, arg1, 0);
}

void enqueueGfxTaskWithFlags(Gfx **arg0, Gfx **arg1) {
  enqueueGfxTaskWithArgs(arg0, arg1, 0x40000000);
}

void scissorBox_get(u32 *left, u32 *top, u32 *right, u32 *bottom) {
  *left = gScissorBoxLeft;
  *top = gScissorBoxTop;
  *right = gScissorBoxRight;
  *bottom = gScissorBoxBottom;
}

void sendViMessage(void) { viMgr_sendMessage(); }

void sendGfxTaskMessage(void) { sendMesgToMainQueue(3); }

void initGfxTaskQueue(void) {
  gfxTaskQueueIndex = 0;
  osCreateMesgQueue(&gfxTaskMesgQueue, &gfxTaskMesg, 1);
  osSendMesg(&gfxTaskMesgQueue, (OSMesg)NULL, 1);
  createResetThread();
  scissorBox_setDefault();
}

void drawRectangle2D(Gfx **gfx, s32 x, s32 y, s32 w, s32 h, s32 r, s32 g,
                     s32 b) {
  gDPPipeSync((*gfx)++);
  gDPPipelineMode((*gfx)++, G_PM_NPRIMITIVE);
  gDPSetCycleType((*gfx)++, G_CYC_FILL);
  gDPSetFillColor((*gfx)++, GPACK_RGBA5551(r, g, b, 1) << 16 |
                                GPACK_RGBA5551(r, g, b, 1));
  gDPSetRenderMode((*gfx)++, G_RM_NOOP, G_RM_NOOP2);
  gDPScisFillRectangle((*gfx)++, x, y, x + w - 1, y + h - 1);
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

void graphicsCache_init(void) {
  if (sGfxStack[0] == NULL) {
    sGfxStack[0] = (Gfx *)heap_malloc(29600); // 3700 dlist commands
    sGfxStack[1] = (Gfx *)heap_malloc(29600);
    sMtxStack[0] = (Mtx *)heap_malloc(44800); // 700 matrices
    sMtxStack[1] = (Mtx *)heap_malloc(44800);
    sVtxStack[0] = (Vtx *)heap_malloc(6880); // 430 vertices
    sVtxStack[1] = (Vtx *)heap_malloc(6880);
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

void scissorBox_setDefault(void) { scissorBox_set(0, 292, 0, 216); }

void setActiveFramebuffer(s32 arg0) {
  Struct_Core1_15B30 *sp1C;

  waitForGfxTask();
  viMgr_setActiveFramebuffer(arg0);
  sp1C = &gfxTaskQueue[gfxTaskQueueIndex];
  gfxTaskQueueIndex = (s32)(gfxTaskQueueIndex + 1) % 20;
  signalGfxTask();
  sp1C->unk0 = 7;
  sendMesgToMainQueue(sp1C);
}

void toggleTextureFilterPoint(void) {
  u32 ret_val = gTextureFilterPoint;
  gTextureFilterPoint = ret_val < 1;
}

void getGraphicsStacks(Gfx **gfx, Mtx **mtx, Vtx **vtx) {
  sStackSelector = (1 - sStackSelector);
  *gfx = sGfxStack[sStackSelector];
  *mtx = sMtxStack[sStackSelector];
  *vtx = sVtxStack[sStackSelector];
}

void dummy_func_80254464(void) {}
