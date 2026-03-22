#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "core2/statetimer.h"

/* .bss */
u8 iFrameState;

/* .code */
void baiFrame_setState(s32 arg0){
    iFrameState = arg0;
}

s32 baiFrame_getState(void){
    return iFrameState;
}

void baiFrame_reset(void){
    iFrameState = 0;
    baiFrame_setState(1);
    stateTimer_clear(STATE_TIMER_4_UNKNOWN);
}

void baiFrame_start(void){
    baiFrame_startWithValue(0.6f);
}

void baiFrame_startWithValue(f32 value){
    stateTimer_set(STATE_TIMER_4_UNKNOWN, value);
    baiFrame_setState(3);
}

void baiFrame_update(void){
    if(stateTimer_isDone(STATE_TIMER_4_UNKNOWN)){
        baiFrame_setState(1);
    }
}
