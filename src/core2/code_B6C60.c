#include <ultra64.h>
#include "functions.h"
#include "variables.h"


/* .bss */
s32 D_80384470;

/* .code */
f32 func_8033DBF0(void){
    return (f32)D_80384470;
}

void resetFrameCount(void){
    D_80384470 = 0;
}

void resetGameFlags(void){}

void clearGameStateVariables(void){}

f32 resetGameStateVariables(void){
    f32 out;
    D_80384470 = viMgr_getFrameCount();
    out = 0.016666666666666666*D_80384470;
    return out;
}
