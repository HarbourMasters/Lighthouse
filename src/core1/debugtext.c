#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"

#include "version.h"

#if VERSION == VERSION_USA_1_0

/* .data */
u8 RGB_VALUES[8][3] = {
    {0xFF, 0x00, 0x00}, // Red
    {0x00, 0xFF, 0x00}, // Green
    {0x00, 0x00, 0xFF}, // Blue
    {0xFF, 0xFF, 0x00}, // Yellow
    {0xFF, 0x00, 0xFF}, // Pink
    {0xFF, 0xFF, 0xFF}, // White
    {0xFF, 0x80, 0x00}, // Orange
    {0x00, 0xFF, 0xFF}  // Cyan
};
s32 red = 0xFF;
s32 green = 0xFF;
s32 blue = 0xFF;
u8  COLOR_SELECTOR[] = {1, 7, 3, 4, 0, 6, 5, 0};
u32 CHARACTER_ENCODING[] = {
    3, 0x0F6DE000,
    1, 0x0F800000,
    3, 0x0C4AE000,
    3, 0x0E59E000, 
    4, 0x088AF200,
    3, 0x0F39E000, 
    3, 0x0F3DE000,
    3, 0x0E4A4000, 
    3, 0x0F7DE000,
    3, 0x0F79E000, 
    3, 0x0F6FA000,
    3, 0x0D75C000, 
    3, 0x0F24E000,
    3, 0x0D6DC000, 
    3, 0x0F34E000,
    3, 0x0F348000, 
    3, 0x0F25E000,
    3, 0x0B7DA000, 
    3, 0x0E92E000,
    3, 0x0E928000, 
    3, 0x0B75A000,
    3, 0x0924E000, 
    5, 0x08EEB188,
    4, 0x09DFB900, 
    4, 0x06999600,
    3, 0x0F7C8000, 
    4, 0x069DB700,
    3, 0x0F7EA000, 
    3, 0x0F39E000,
    3, 0x0E924000, 
    4, 0x09999600,
    5, 0x08C62A20, 
    5, 0x08C63550,
    3, 0x0B55A000, 
    3, 0x0B79E000, 
    4, 0x0F124F00,
    1, 0x00800000,
    1, 0x05000000,
    3, 0x00380000,
    2, 0x00000000
};

s16 startingXCoordinate = 0;
s16 largeYCoordinate = 0;
s16 cursorPosition = 0;
s16 startingYCoordinate = 0;
s16 currentColor = 0;
s16 isThreadLocked = 0;
s16 currentFontSize = 0;
s16 largeValueCursorPosition = 0;
s16 longestLineLengthPosition = 0;
 u8 shouldClearText = 0;
s32 clearTime = 0;
char CHARACTER_RANGE[] = {'a','z','A','Z','0','9','.',':', '-', ' '};


/* .h */
void gcdebugText_wrapToTop(void);
void gcdebugText_printSpace(void);
void gcdebugText_endLine(void);

/* code */
void setRGB(s32 r, s32 g, s32 b){
    red = r;
    green = g;
    blue = b;
}

void setPixelInFrameBuffer(s32 x, s32 y) {
    s32 rgba16;
    if( ((x >= 0) && (x < gFramebufferWidth))
        && ((y >= 0) && (y < gFramebufferHeight))
    ) {
        gFramebuffers[0][x + y*gFramebufferWidth] = _SHIFTL(red >> 3, 11, 5) | _SHIFTL(green >> 3, 6, 5) | _SHIFTL(blue >> 3, 1, 5) | _SHIFTL(1, 0, 1);
        gFramebuffers[1][x + y*gFramebufferWidth] = _SHIFTL(red >> 3, 11, 5) | _SHIFTL(green >> 3, 6, 5) | _SHIFTL(blue >> 3, 1, 5) | _SHIFTL(1, 0, 1);
        
    }
}

void drawPixel(s32 x, s32 y, s32 w, s32 h) {
    s32 iy;
    s32 var_s1;
    s32 ix;

    for(ix = 0; ix < w; ix++){
        for(iy = 0; iy < h; iy++){
            setPixelInFrameBuffer(x + ix, y + iy);
        }
    }
    osWritebackDCacheAll();
}

void drawSquare(s32 r, s32 g, s32 b) {
    setRGB(r, g, b);
    drawPixel((gFramebufferWidth - 128) / 2, (gFramebufferHeight - 100) / 2, 128, 100);
}

void gcdebugText_empty(void){}

void gcdebugText_stallOnThread(void) {
    s32 var_s0;

    for(var_s0 = 0; var_s0 < 2000000; var_s0++){
        gcdebugText_empty();
    }
}

void gcdebugText_flashSquare(s32 arg0) {
    do{
        drawSquare(RGB_VALUES[arg0][0], RGB_VALUES[arg0][1], RGB_VALUES[arg0][2]);
        gcdebugText_stallOnThread();
        drawSquare(0, 0, 0);
        gcdebugText_stallOnThread();
    }
    while(1);
}

void gcdebugText_drawSquare(s32 arg0) {
    drawSquare(RGB_VALUES[arg0][0], RGB_VALUES[arg0][1], RGB_VALUES[arg0][2]);
    gcdebugText_stallOnThread();
    drawSquare(0, 0, 0);
    gcdebugText_stallOnThread();
}

void gcdebugText_drawSquareOnly(s32 arg0) {
    drawSquare(RGB_VALUES[arg0][0], RGB_VALUES[arg0][1], RGB_VALUES[arg0][2]);
}

void drawCharacter(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 r, s32 g, s32 b) {
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
    u32 *ptr = CHARACTER_ENCODING + (arg1*2);
    
    var_s4 = 0x10000000;
    sp44 = ptr[0];
    sp40 = ptr[1];
    for(sp48 = 0; sp48 < 5; sp48++){
        temp_lo = sp48 * arg2;
        for(var_s3 = 0; var_s3 != sp44; var_s3++){
            var_s4 >>= 1;
            if (sp40 & var_s4) {
                setRGB(r, g, b);
                drawPixel((cursorPosition + arg2*var_s3) - arg3, (startingYCoordinate + temp_lo) - arg3, (arg2 + arg3) + arg3, (arg2 + arg3) + arg3);
            } else {
                if (arg4 != 0) {
                    setRGB(0, 0, 0);
                }
                drawPixel((cursorPosition + arg2*var_s3) - arg3, (startingYCoordinate + temp_lo) - arg3, (arg2 + arg3) + arg3, (arg2 + arg3) + arg3);
            }
        }
    }
    if (arg3 == 0) {
        cursorPosition += (sp44 * arg2) + 2;
    }
}

void gcdebugText_clearText(void) {
    s16 *var_v0;
    s16 *var_v1;
    s32 temp_lo;
    void *temp_a0;
    s32 i;

    if (clearTime == 0) {
        clearTime = globalTimer_getTime();
        return;
    }
    if (globalTimer_getTime() != clearTime) {
        shouldClearText = false;
        clearTime = globalTimer_getTime();
    }

    if (shouldClearText) {
        for(i = 0; i < gFramebufferWidth * gFramebufferHeight; i++){
                gFramebuffers[0][i] = 0;
                gFramebuffers[1][i] = 0;
        }
        osWritebackDCacheAll();
    }
}

void printCharacter(s32 arg0, s32 arg1, s32 arg2) {
    drawCharacter(arg0, arg1, arg2, 1, 1, 0, 0, 0);
    drawCharacter(arg0, arg1, arg2, 0, 1, RGB_VALUES[COLOR_SELECTOR[arg0]][0], RGB_VALUES[COLOR_SELECTOR[arg0]][1], RGB_VALUES[COLOR_SELECTOR[arg0]][2]);
}

void printValue(s32 arg0, s32 arg1, s32 arg2) {
    s16 temp_v0;
    s32 var_a1;
    s32 var_s0;
    s32 var_s1;

    cursorPosition = startingXCoordinate;
    currentFontSize = arg2;
    if (arg1 < 0) {
        printCharacter(arg0, 0x26, arg2);
        arg1 *= -1;
    }
    
    for(var_s0 = 1000000000; var_s0 >= 10 && arg1 < var_s0; var_s0 /= 10){
        continue;
    }

    for(var_s0 = var_s0; var_s0 >= 10; var_s0 /= 10){
        for(var_a1 = 0; arg1 >= var_s0; var_a1++){
            arg1 -= var_s0;
        }
        printCharacter(arg0, var_a1, arg2);
    }

    printCharacter(arg0, arg1, arg2);
    gcdebugText_printSpace();
    if (arg2 == 7) {
        largeValueCursorPosition = cursorPosition;
        longestLineLengthPosition = 0;
    }
    else if (cursorPosition >= longestLineLengthPosition) {
        longestLineLengthPosition = cursorPosition;
    }
}

void gcdebugText_showLargeValue(s32 arg0, s32 arg1) {
    isThreadLocked = 0;
    currentColor = arg0;
    startingXCoordinate = 0xE;
    startingYCoordinate = 0xA;
    printValue(arg0, arg1, 7);
    gcdebugText_wrapToTop();
    largeYCoordinate = startingYCoordinate;
    longestLineLengthPosition = startingXCoordinate;
    shouldClearText = 1;
}


void gcdebugText_showValue(s32 arg0){
    printValue(currentColor, arg0, 2);
    gcdebugText_wrapToTop();
}

void gcdebugText_showHexValue(u32 arg0) {
    s32 var_s0;

    cursorPosition = startingXCoordinate;
    currentFontSize = 2;
    printCharacter(currentColor, 0x25, 2);
     var_s0 = (arg0 >= 0x01000000U) ? 0x1C
            : (arg0 >= 0x10000U) ? 0x14
            :0xC;
    if (var_s0 >= 0) {
        do {
            printCharacter(currentColor, ((s32) arg0 >> var_s0) & 0xF, 2);
            var_s0 -= 4;
        } while (var_s0 >= 0);
    }
    gcdebugText_endLine();
}

void gcdebugText_showFloat(f32 arg0) {
    f32 var_f22;
    s32 var_s0;

    cursorPosition = startingXCoordinate;
    currentFontSize = 2;
    if (arg0 < 0.0f) {
        printCharacter(currentColor, 0x26, 2);
        arg0 *= -1.0f;
    }
    var_f22 = 1e+09;
    while((var_f22 >= 10.0f) && (arg0 < var_f22)) {
        var_f22 /= 10.0f;
    }
    while (1e-09 <= var_f22) {
            if ((0.09 < var_f22) && (var_f22 < 0.11)) {
                printCharacter(currentColor, 0x24, 2);
            }
            
            var_s0 = 0;
            while (var_f22 <= arg0) {
                    arg0 -= var_f22;
                    var_s0 += 1;
            }
            printCharacter(currentColor, var_s0, 2);
            var_f22 /= 10.0f;
    }
    gcdebugText_endLine();
}

//letter to font index???
s32 encodeCharacter(s32 arg0) {
    //lowercase_letter
    if ((arg0 >= (s32) CHARACTER_RANGE[0]) && ((s32) CHARACTER_RANGE[1] >= arg0)) {
        return (arg0 - CHARACTER_RANGE[0]) + 0xA;
    }

    //uppercase_letter
    if ((arg0 >= (s32) CHARACTER_RANGE[2]) && ((s32) CHARACTER_RANGE[3] >= arg0)) {
        return (arg0 - CHARACTER_RANGE[2]) + 0xA;
    }

    //number
    if ((arg0 >= (s32) CHARACTER_RANGE[4]) && ((s32) CHARACTER_RANGE[5] >= arg0)) {
        return (arg0 - CHARACTER_RANGE[4]);
    }

    //.
    if (arg0 == CHARACTER_RANGE[6]) {
        return 0x24;
    }

    //:
    if (arg0 == CHARACTER_RANGE[7]) {
        return 0x25;
    }

    //-
    if (arg0 == CHARACTER_RANGE[8]) {
        return 0x26;
    }
    // 
    if (arg0 == CHARACTER_RANGE[9]) {
        return 0x27;
    }
    return 0x27;
}


void gcdebugText_showText(u8 *arg0){
    s32 i;
    s32 var_v0;

    cursorPosition = startingXCoordinate;
    currentFontSize = 2;
    for(i = 0; arg0[i] != 0; i++){
        var_v0 = arg0[i];
        printCharacter(currentColor, encodeCharacter(var_v0), 2);
    }
    gcdebugText_endLine();
}


void gcdebugText_lockScreen(void){
    isThreadLocked = 1;
    do{}while(1);
}

void gcdebugText_pauseThread(void){
    s32 i;
    isThreadLocked = 1;
    for(i = 30000000; i != 0; i--){}
    isThreadLocked = 0;
}

void gcdebugText_pauseThreadForTime(s32 arg0){
    s32 i;

    isThreadLocked = 1;
    while(arg0 != 0){
        for(i = 30000000; i != 0; i--){}
        arg0--;
    }
    isThreadLocked = 0;
}

void checkYAndgcdebugText_wrapToTop(s32 arg0) {
    startingYCoordinate += arg0;
    if (startingYCoordinate >= 0xCD) {
        startingXCoordinate = longestLineLengthPosition + 4;
        cursorPosition = startingXCoordinate;
        startingYCoordinate = (largeValueCursorPosition < cursorPosition) ? 0xA : largeYCoordinate;
        longestLineLengthPosition = 0;
    }
}

void gcdebugText_wrapToTop(void){
    checkYAndgcdebugText_wrapToTop( currentFontSize*5 + 2);
}

void gcdebugText_wrapToTopSmall(void){
    checkYAndgcdebugText_wrapToTop(2);
}

void gcdebugText_endLine(void){
    gcdebugText_printSpace();
    if(cursorPosition >= longestLineLengthPosition){
        longestLineLengthPosition = cursorPosition;
    }
    gcdebugText_wrapToTop();
}

void gcdebugText_printSpace(void) {
    s16 sp1E;
    s16 temp_v0;

    sp1E = cursorPosition--;
    printCharacter(currentColor, 0x27, currentFontSize);
    cursorPosition = sp1E;
}


s32 gcdebugText_isThreadLocked(void){
    return isThreadLocked;
}

void gcdebugText_unused(s32 arg0){ // [port] was UNK_TYPE(s32) — empty stub

}
#endif
