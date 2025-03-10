#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

struct {
  void *data;
  int unk4;
} DepthBuffer_S;

void depthBuffer_clear(Gfx **gfx) {
  depthBuffer_draw(gfx, 0, 0, gFramebufferWidth, gFramebufferHeight,
                gFramebuffers[getActiveFramebuffer()]);
}

void depthBuffer_draw(Gfx **gfx, s32 x, s32 y, s32 w, s32 h, void *color_buffer) {
  if (DepthBuffer_S.data != NULL &&
      (game_getMode() != GAME_MODE_4_PAUSED || func_80335134())) {
    // draw z_buffer
    gDPPipeSync((*gfx)++);
    gDPSetColorImage((*gfx)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth,
                     OS_K0_TO_PHYSICAL(DepthBuffer_S.data));
    gDPSetCycleType((*gfx)++, G_CYC_FILL);
    gDPSetRenderMode((*gfx)++, G_RM_NOOP, G_RM_NOOP2);
    gDPSetFillColor((*gfx)++, 0xFFFCFFFC);
    gDPScisFillRectangle((*gfx)++, x, y, x + w - 1, y + h - 1);

    // draw color_buffer
    gDPPipeSync((*gfx)++);
    gDPSetColorImage((*gfx)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, gFramebufferWidth,
                     OS_K0_TO_PHYSICAL(color_buffer));
  }
}

int depthBuffer_getStatus(void) { return DepthBuffer_S.unk4; }

bool depthBuffer_isPointerSet(void) { return DepthBuffer_S.data != NULL; }

void depthBuffer_stub(void) {}

void depthBuffer_init(int arg0) {
  u16 *var_v0;
  int new_var;
  if (arg0) {
    var_v0 = (DepthBuffer_S.data = &D_8000E800);
    while (((s32)DepthBuffer_S.data) % 0x40) {
      var_v0 = (DepthBuffer_S.data = var_v0 + 1);
    }

    do {
    } while ((&D_8000E800 && 1) * 0); // remove this
  } else {
    DepthBuffer_S.data = NULL;
  }
  DepthBuffer_S.unk4 = FALSE;
}

void depthBuffer_setStatus(int arg0) {
  DepthBuffer_S.unk4 = (DepthBuffer_S.data != NULL && arg0);
}

void depthBuffer_set(Gfx **gfx) {
  if (DepthBuffer_S.data && game_getMode() != GAME_MODE_4_PAUSED) {
    gDPPipeSync((*gfx)++);
    gDPSetDepthImage((*gfx)++, DepthBuffer_S.data);
  }
}

void *depthBuffer_get(void) { return DepthBuffer_S.data; }
