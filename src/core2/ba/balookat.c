// BanjoDecomp: core2/balookat.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

/* .bss */
struct50s D_8037C5F0[8];
u8 D_8037C670;
u8 D_8037C671;

/* .code */
int balookat_try_get_position(f32 arg0[3]){
    if(!D_8037C5F0[D_8037C670 - 1].lookat)
        return 0;

    ml_vec3f_copy(arg0, D_8037C5F0[D_8037C670 - 1].position);
    return 1;
}

s32 balookat_getState(void){
    if(!D_8037C670){
        return 0;
    }
    return D_8037C5F0[D_8037C670 - 1].state;
}

void balookat_init(void){
    D_8037C671 = 0;
    D_8037C670 = 0;
}

void balookat_pop(void){
    if(D_8037C670){
        D_8037C670--;
        if(D_8037C670 == 0){
            baiFrame_startWithValue(1.2f);
            D_8037C671 = 0;
        }
    }
}

void balookat_push(s32 arg0){
    D_8037C5F0[D_8037C670].state = arg0;
    D_8037C5F0[D_8037C670].lookat = 0;
    ml_vec3f_clear(D_8037C5F0[D_8037C670].position);
    D_8037C670++;
}

void balookat_set_position(f32 arg0[3]){
    D_8037C5F0[D_8037C670-1].lookat = 1;
    ml_vec3f_copy(D_8037C5F0[D_8037C670-1].position, arg0);
}

void balookat_update_state(s32 arg0){
    int val = 0;
    switch(arg0){
        case 1:
            if(bs_checkInterrupt(BS_INTR_1E) == 2)
                val = 1;
            break;
        case 2:
            if(bs_checkInterrupt(BS_INTR_1C) == 2)
                val = 1;
            break;
        case 3:
            if(bs_checkInterrupt(BS_INTR_1D) == 2)
                val = 1;
            break;
        case 4:
            if(bs_checkInterrupt(BS_INTR_25) == 2)
                val = 1;
            break;
        default:
            val = 1;
            break;
    }
    if(val)
        D_8037C671 = arg0;
}

void balookat_end(void){
    if(D_8037C670)
        D_8037C671 = 0;
}

void balookat_update(void){
    s32 tmp;
    if(D_8037C670){
        tmp = D_8037C5F0[D_8037C670-1].state;
        if(D_8037C671 != tmp){
            balookat_update_state(tmp);
        }
    }
}
