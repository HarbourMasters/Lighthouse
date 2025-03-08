#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include "bk_time.h"

//frame_manager.c

extern void system_scheduleModeChange(s32);
extern void comusicPlayer_update(void);
extern void rotateModel(void *, f32);
extern void resetGameFlags(void);
extern void clearGameStateVariables(void);
extern f32 resetGameStateVariables(void);

/* .bss */
struct {
  f32 unk0;
  f32 unk4;
  s32 unk8;
  s32 unkC;
  void *unk10;
  s32 unk14;
  f32 unk18;
  // u8 pad1C[0x4];
} gameStateData;

/* .code */
void drawFramebuffer(s32 arg0) {
  s32 temp_v0;

  temp_v0 = gameStateData.unk14;
  if ((temp_v0 == 0) || (temp_v0 == 3)) {
    framebufferdraw_setBufferIndex(arg0);
    framebufferdraw_drawTexture_RGBA16(0, 0, gameStateData.unkC, 0, 0);
    osWritebackDCache(
        gFramebuffers[arg0],
        (s32)((f32)gFramebufferWidth * (f32)gFramebufferHeight * sizeof(s16)));
  }
}

void renderGraphics(s32 arg0, Gfx **gfx_begin, Gfx **gfx_end) {
  Gfx *gfx;
  Gfx *gfx_start;
  Mtx *mtx;
  Mtx *mtx_start;
  Vtx *vtx;
  Vtx *vtx_start;

  getGraphicsStacks(&gfx, &mtx, &vtx);
  gfx_start = gfx;
  mtx_start = mtx;
  vtx_start = vtx;
  scissorBox_SetForGameMode(&gfx, arg0);
  if (gameStateData.unk14 == 2) {
    drawRectangle2D(&gfx, 0, 0, (s32)(f32)gFramebufferWidth,
                    (s32)(f32)gFramebufferHeight, 0, 0, 0);
  }
  if ((gameStateData.unk14 == 0) || (gameStateData.unk14 == 3)) {
    viewport_setRenderViewportAndPerspectiveMatrix(&gfx, &mtx);
    gcbound_draw(&gfx);
  }
  if (gameStateData.unk14 == 1) {
    drawRectangle2D(&gfx, 0, 0, (s32)(f32)gFramebufferWidth,
                    (s32)(f32)gFramebufferHeight, 0, 0, 0);
    viewport_setRenderViewportAndPerspectiveMatrix(&gfx, &mtx);
    executeDrawMethod(gameStateData.unk10, &gfx, &mtx, &vtx);
  }
  finishFrame(&gfx);
  osWritebackDCache(mtx_start, (mtx - mtx_start) * sizeof(Mtx));
  osWritebackDCache(vtx_start, (vtx - vtx_start) * sizeof(Vtx));
  *gfx_begin = gfx_start;
  *gfx_end = gfx;
}

void setGameState(s32 arg0) {
  gameStateData.unk14 = arg0;
  if (gameStateData.unk14 == 0) {
    gameStateData.unk8 = 0xFF;
    gameStateData.unk0 = 0.0f;
    comusic_playTrackWithVolumeOverride(COMUSIC_31_GAME_OVER, -1);
    resetFrameCount();
  } else if (gameStateData.unk14 == 1) {
    gameStateData.unk4 = 0.0f;
    if (comusic_isTrackQueued(COMUSIC_31_GAME_OVER)) {
      comusic_fadeTrackWithArgsNoDelay(COMUSIC_31_GAME_OVER, 0, 200);
    }
    loadTextureMemory(gameStateData.unk10, getActiveFramebuffer());
  } else if (gameStateData.unk14 == 3) {
    gameStateData.unk0 = 0.0f;
  }
}

void processFrame(s32 arg0) {
  Gfx *gfx_begin;
  Gfx *gfx_end;

  drawFramebuffer(getOtherFramebuffer());
  renderGraphics(getOtherFramebuffer(), &gfx_begin, &gfx_end);
  enqueueGfxTaskNoArg(gfx_begin, gfx_end);
  sendGfxTaskMessage();
  viMgr_processFrame();
}

void freeResources(void) {
  assetcache_release(gameStateData.unkC);
  freeModel(gameStateData.unk10);
  cleanupTextRenderer();
  comusicPlayer_free();
  depthBuffer_stub();
  viMgr_setFrameLimit(2);
}

void dummy_func_802E35D0(void) {}


void updateGameStateData(void) {
  s32 sp40[6];
  s32 i;
  s32 sp38;

  if (gameStateData.unk18 == 0.0f) {
    time_setDeltaReal_sec(0.0f);
  } else {
    clearGameStateVariables();
    time_setDeltaReal_sec(resetGameStateVariables());
  }
  resetGameFlags();
  gameStateData.unk18 += time_getDelta();
  gameStateData.unk0 += time_getDelta();
  if (0.83333333333333337 <= gameStateData.unk0) {
    gameStateData.unk0 -= 0.83333333333333337;
  }
  if (gameStateData.unk14 == 0) {
    gameStateData.unk8 -= 0x10;
    if (gameStateData.unk8 <= 0) {
      gameStateData.unk8 = 0;
      setGameState(3);
    }
  } else if (gameStateData.unk14 == 3) {
    sp38 = 0;
    controller_copyFaceButtons(0, &sp40);
    for (i = 0; i < 6; i++) {
      if (sp40[i] == 1) {
        sp38++;
      }
    }
    if (gameStateData.unk18 > 15.0f) {
      sp38++;
    }
    if (controller_getStartButton(0) == 1) {
      sp38++;
    }
    if (sp38 != 0) {
      setGameState(1);
    }
  } else if (gameStateData.unk14 == 1) {
    gameStateData.unk4 += 0.01;
    if (gameStateData.unk4 >= 1.0f) {
      system_scheduleModeChange(1);
      setGameState(2);
      return;
    }
  }
  comusicPlayer_update();
  if (gameStateData.unk14 == 0) {
    gcbound_alpha(gameStateData.unk8);
  }
  if (gameStateData.unk14 == 1) {
    rotateModel(gameStateData.unk10, gameStateData.unk4);
  }
  processFrame(0);
}
