#ifndef BANJO_KAZOOIE_CORE1_DEBUGTEXT_H
#define BANJO_KAZOOIE_CORE1_DEBUGTEXT_H

#include <ultra64.h>
#include "version.h"

#if VERSION == VERSION_USA_1_0
void gcdebugtext_setRGB(s32 r, s32 g, s32 b);
void gcdebugtext_setPixelInFrameBuffer(s32 x, s32 y);
void gcdebugtext_drawPixel(s32 x, s32 y, s32 w, s32 h);
void gcdebugtext_drawSquare(s32 r, s32 g, s32 b);
void gcdebugtext_stub1(void);
void gcdebugtext_stallOnThread(void);
void gcdebugtext_flashSquare(s32 color);
void gcdebugtext_drawSquare2(s32 color);
void gcdebugtext_drawSquareOnly(s32 color);
void gcdebugtext_drawCharacter(s32 colorSelect, s32 displayCharacter, s32 fontSize, s32 keepCursor, bool setBackgroundColorBlack, s32 r, s32 g, s32 b);
void gcdebugtext_clearText(void);
void gcdebugtext_printCharacter(s32 colorSelect, s32 displayCharacter, s32 fontSize);
void gcdebugtext_printValue(s32 colorSelect, s32 displayValue, s32 fontSize);
void gcdebugtext_showLargeValue(s32 colorSelect, s32 displayValue);
void gcdebugtext_showValue(s32 displayValue);
void gcdebugtext_showHexValue(u32 displayValue);
void gcdebugtext_showFloat(f32 displayValue);
s32 gcdebugtext_encodeCharacter(s32 character);
void gcdebugtext_showText(u8 *text);
void gcdebugtext_lockScreen(void);
void gcdebugtext_pauseThread(void);
void gcdebugtext_pauseThreadForTime(s32 time);
void gcdebugtext_checkYAndgcdebugtext_wrapToTop(s32 lineHeight);
void gcdebugtext_wrapToTop(void);
void gcdebugtext_wrapToTopSmall(void);
void gcdebugtext_endLine(void);
void gcdebugtext_printSpace(void);
bool gcdebugtext_isThreadLocked(void);
void gcdebugtext_stub2(s32 arg0);
#endif

#endif
