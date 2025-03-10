#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include "version.h"


//framebufferUtils.c

#if VERSION == VERSION_USA_1_0

/* .data */
u8 framebufferColors[8][3] = {{0xFF, 0x00, 0x00}, {0x00, 0xFF, 0x00},
                              {0x00, 0x00, 0xFF}, {0xFF, 0xFF, 0x00},
                              {0xFF, 0x00, 0xFF}, {0xFF, 0xFF, 0xFF},
                              {0xFF, 0x80, 0x00}, {0x00, 0xFF, 0xFF}};
s32 framebufferRed = 0xFF;
s32 framebufferGreen = 0xFF;
s32 framebufferBlue = 0xFF;
u8 colorIndices[] = {1, 7, 3, 4, 0, 6, 5, 0};
u32 patternData[] = {
    3, 0x0F6DE000, 1, 0x0F800000, 3, 0x0C4AE000, 3, 0x0E59E000, 4, 0x088AF200,
    3, 0x0F39E000, 3, 0x0F3DE000, 3, 0x0E4A4000, 3, 0x0F7DE000, 3, 0x0F79E000,
    3, 0x0F6FA000, 3, 0x0D75C000, 3, 0x0F24E000, 3, 0x0D6DC000, 3, 0x0F34E000,
    3, 0x0F348000, 3, 0x0F25E000, 3, 0x0B7DA000, 3, 0x0E92E000, 3, 0x0E928000,
    3, 0x0B75A000, 3, 0x0924E000, 5, 0x08EEB188, 4, 0x09DFB900, 4, 0x06999600,
    3, 0x0F7C8000, 4, 0x069DB700, 3, 0x0F7EA000, 3, 0x0F39E000, 3, 0x0E924000,
    4, 0x09999600, 5, 0x08C62A20, 5, 0x08C63550, 3, 0x0B55A000, 3, 0x0B79E000,
    4, 0x0F124F00, 1, 0x00800000, 1, 0x05000000, 3, 0x00380000, 2, 0x00000000};

s16 currentX = 0;
s16 initialY = 0;
s16 currentXOffset = 0;
s16 currentYOffset = 0;
s16 currentColorIndex = 0;
s16 haltFlag = 0;
s16 lineSpacing = 0;
s16 maxXOffset = 0;
s16 maxYOffset = 0;
u8 clearFlag = 0;
s32 previousTime = 0;
char fontCharacters[] = {'a', 'z', 'A', 'Z', '0', '9', '.', ':', '-', ' '};

/* .h */
void advanceLineWithSpacing(void);
void drawSpace(void);
void advanceLineAndUpdate(void);

/* code */
void setFramebufferColor(s32 r, s32 g, s32 b) {
  framebufferRed = r;
  framebufferGreen = g;
  framebufferBlue = b;
}

void drawPixel(s32 x, s32 y) {
  s32 rgba16;
  if (((x >= 0) && (x < gFramebufferWidth)) &&
      ((y >= 0) && (y < gFramebufferHeight))) {
    gFramebuffers[0][x + y * gFramebufferWidth] =
        _SHIFTL(framebufferRed >> 3, 11, 5) |
        _SHIFTL(framebufferGreen >> 3, 6, 5) |
        _SHIFTL(framebufferBlue >> 3, 1, 5) | _SHIFTL(1, 0, 1);
    gFramebuffers[1][x + y * gFramebufferWidth] =
        _SHIFTL(framebufferRed >> 3, 11, 5) |
        _SHIFTL(framebufferGreen >> 3, 6, 5) |
        _SHIFTL(framebufferBlue >> 3, 1, 5) | _SHIFTL(1, 0, 1);
  }
}

void drawRectangle(s32 x, s32 y, s32 w, s32 h) {
  s32 iy;
  s32 var_s1;
  s32 ix;

  for (ix = 0; ix < w; ix++) {
    for (iy = 0; iy < h; iy++) {
      drawPixel(x + ix, y + iy);
    }
  }
  osWritebackDCacheAll();
}

void fillScreenWithColor(s32 r, s32 g, s32 b) {
  setFramebufferColor(r, g, b);
  drawRectangle((gFramebufferWidth - 128) / 2, (gFramebufferHeight - 100) / 2,
                128, 100);
}

void noop(void) {}

void delayLoop(void) {
  s32 var_s0;

  for (var_s0 = 0; var_s0 < 2000000; var_s0++) {
    noop();
  }
}

void flashScreen(s32 arg0) {
  do {
    fillScreenWithColor(framebufferColors[arg0][0], framebufferColors[arg0][1],
                        framebufferColors[arg0][2]);
    delayLoop();
    fillScreenWithColor(0, 0, 0);
    delayLoop();
  } while (1);
}

void flashScreenOnce(s32 arg0) {
  fillScreenWithColor(framebufferColors[arg0][0], framebufferColors[arg0][1],
                      framebufferColors[arg0][2]);
  delayLoop();
  fillScreenWithColor(0, 0, 0);
  delayLoop();
}

void setScreenColor(s32 arg0) {
  fillScreenWithColor(framebufferColors[arg0][0], framebufferColors[arg0][1],
                      framebufferColors[arg0][2]);
}

void drawPattern(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 r, s32 g,
                 s32 b) {
  s32 var_s3;
  s32 sp48;
  u32 sp44;
  u32 sp40;
  s32 temp_lo;
  s32 temp_s1;
  s32 temp_t8;
  s32 var_s0;
  u32 temp_t4;
  u32 var_s4;
  u32 *ptr = patternData + (arg1 * 2);

  var_s4 = 0x10000000;
  sp44 = ptr[0];
  sp40 = ptr[1];
  for (sp48 = 0; sp48 < 5; sp48++) {
    temp_lo = sp48 * arg2;
    for (var_s3 = 0; var_s3 != sp44; var_s3++) {
      var_s4 >>= 1;
      if (sp40 & var_s4) {
        setFramebufferColor(r, g, b);
        drawRectangle((currentXOffset + arg2 * var_s3) - arg3,
                      (currentYOffset + temp_lo) - arg3, (arg2 + arg3) + arg3,
                      (arg2 + arg3) + arg3);
      } else {
        if (arg4 != 0) {
          setFramebufferColor(0, 0, 0);
        }
        drawRectangle((currentXOffset + arg2 * var_s3) - arg3,
                      (currentYOffset + temp_lo) - arg3, (arg2 + arg3) + arg3,
                      (arg2 + arg3) + arg3);
      }
    }
  }
  if (arg3 == 0) {
    currentXOffset += (sp44 * arg2) + 2;
  }
}

void clearFramebuffers(void) {
  s16 *var_v0;
  s16 *var_v1;
  s32 temp_lo;
  void *temp_a0;
  s32 i;

  if (previousTime == 0) {
    previousTime = globalTimer_getTime();
    return;
  }
  if (globalTimer_getTime() != previousTime) {
    clearFlag = FALSE;
    previousTime = globalTimer_getTime();
  }

  if (clearFlag) {
    for (i = 0; i < gFramebufferWidth * gFramebufferHeight; i++) {
      gFramebuffers[0][i] = 0;
      gFramebuffers[1][i] = 0;
    }
    osWritebackDCacheAll();
  }
}

void drawCharacter(s32 arg0, s32 arg1, s32 arg2) {
  drawPattern(arg0, arg1, arg2, 1, 1, 0, 0, 0);
  drawPattern(arg0, arg1, arg2, 0, 1, framebufferColors[colorIndices[arg0]][0],
              framebufferColors[colorIndices[arg0]][1],
              framebufferColors[colorIndices[arg0]][2]);
}

void drawNumber(s32 arg0, s32 arg1, s32 arg2) {
  s16 temp_v0;
  s32 var_a1;
  s32 var_s0;
  s32 var_s1;

  currentXOffset = currentX;
  lineSpacing = arg2;
  if (arg1 < 0) {
    drawCharacter(arg0, 0x26, arg2);
    arg1 *= -1;
  }

  for (var_s0 = 1000000000; var_s0 >= 10 && arg1 < var_s0; var_s0 /= 10) {
    continue;
  }

  for (var_s0 = var_s0; var_s0 >= 10; var_s0 /= 10) {
    for (var_a1 = 0; arg1 >= var_s0; var_a1++) {
      arg1 -= var_s0;
    }
    drawCharacter(arg0, var_a1, arg2);
  }

  drawCharacter(arg0, arg1, arg2);
  drawSpace();
  if (arg2 == 7) {
    maxXOffset = currentXOffset;
    maxYOffset = 0;
  } else if (currentXOffset >= maxYOffset) {
    maxYOffset = currentXOffset;
  }
}

void drawScore(s32 arg0, s32 arg1) {
  haltFlag = 0;
  currentColorIndex = arg0;
  currentX = 0xE;
  currentYOffset = 0xA;
  drawNumber(arg0, arg1, 7);
  advanceLineWithSpacing();
  initialY = currentYOffset;
  maxYOffset = currentX;
  clearFlag = 1;
}

void updateScore(s32 arg0) {
  drawNumber(currentColorIndex, arg0, 2);
  advanceLineWithSpacing();
}

void drawHex(u32 arg0) {
  s32 var_s0;

  currentXOffset = currentX;
  lineSpacing = 2;
  drawCharacter(currentColorIndex, 0x25, 2);
  var_s0 = (arg0 >= 0x01000000U) ? 0x1C : (arg0 >= 0x10000U) ? 0x14 : 0xC;
  if (var_s0 >= 0) {
    do {
      drawCharacter(currentColorIndex, ((s32)arg0 >> var_s0) & 0xF, 2);
      var_s0 -= 4;
    } while (var_s0 >= 0);
  }
  advanceLineAndUpdate();
}

void drawFloat(f32 arg0) {
  f32 var_f22;
  s32 var_s0;

  currentXOffset = currentX;
  lineSpacing = 2;
  if (arg0 < 0.0f) {
    drawCharacter(currentColorIndex, 0x26, 2);
    arg0 *= -1.0f;
  }
  var_f22 = 1e+09;
  while ((var_f22 >= 10.0f) && (arg0 < var_f22)) {
    var_f22 /= 10.0f;
  }
  while (1e-09 <= var_f22) {
    if ((0.09 < var_f22) && (var_f22 < 0.11)) {
      drawCharacter(currentColorIndex, 0x24, 2);
    }

    var_s0 = 0;
    while (var_f22 <= arg0) {
      arg0 -= var_f22;
      var_s0 += 1;
    }
    drawCharacter(currentColorIndex, var_s0, 2);
    var_f22 /= 10.0f;
  }
  advanceLineAndUpdate();
}

// letter to font index???
s32 charToFontIndex(s32 arg0) {
  // lowercase_letter
  if ((arg0 >= (s32)fontCharacters[0]) && ((s32)fontCharacters[1] >= arg0)) {
    return (arg0 - fontCharacters[0]) + 0xA;
  }

  // uppercase_letter
  if ((arg0 >= (s32)fontCharacters[2]) && ((s32)fontCharacters[3] >= arg0)) {
    return (arg0 - fontCharacters[2]) + 0xA;
  }

  // number
  if ((arg0 >= (s32)fontCharacters[4]) && ((s32)fontCharacters[5] >= arg0)) {
    return (arg0 - fontCharacters[4]);
  }

  //.
  if (arg0 == fontCharacters[6]) {
    return 0x24;
  }

  //:
  if (arg0 == fontCharacters[7]) {
    return 0x25;
  }

  //-
  if (arg0 == fontCharacters[8]) {
    return 0x26;
  }
  //
  if (arg0 == fontCharacters[9]) {
    return 0x27;
  }
  return 0x27;
}

void drawString(u8 *arg0) {
  s32 i;
  s32 var_v0;

  currentXOffset = currentX;
  lineSpacing = 2;
  for (i = 0; arg0[i] != 0; i++) {
    var_v0 = arg0[i];
    drawCharacter(currentColorIndex, charToFontIndex(var_v0), 2);
  }
  advanceLineAndUpdate();
}

void halt(void) {
  haltFlag = 1;
  do {
  } while (1);
}

void haltWithDelay(void) {
  s32 i;
  haltFlag = 1;
  for (i = 30000000; i != 0; i--) {
  }
  haltFlag = 0;
}

void haltForSeconds(s32 arg0) {
  s32 i;

  haltFlag = 1;
  while (arg0 != 0) {
    for (i = 30000000; i != 0; i--) {
    }
    arg0--;
  }
  haltFlag = 0;
}

void advanceLine(s32 arg0) {
  currentYOffset += arg0;
  if (currentYOffset >= 0xCD) {
    currentX = maxYOffset + 4;
    currentXOffset = currentX;
    currentYOffset = (maxXOffset < currentXOffset) ? 0xA : initialY;
    maxYOffset = 0;
  }
}

void advanceLineWithSpacing(void) { advanceLine(lineSpacing * 5 + 2); }

void advanceLineByTwo(void) { advanceLine(2); }

void advanceLineAndUpdate(void) {
  drawSpace();
  if (currentXOffset >= maxYOffset) {
    maxYOffset = currentXOffset;
  }
  advanceLineWithSpacing();
}

void drawSpace(void) {
  s16 sp1E;
  s16 temp_v0;

  sp1E = currentXOffset--;
  drawCharacter(currentColorIndex, 0x27, lineSpacing);
  currentXOffset = sp1E;
}

s32 isHalted(void) { return haltFlag; }

void noopWithArg(UNK_TYPE(s32) arg0) {}
#endif
