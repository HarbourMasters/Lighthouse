// BanjoDecomp: code_9C10.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

/* .bss */
u8 GV_D_80391AD0;

/* .code */
void func_80390000(s32 arg0){
    Struct73s *tmp_v0;
    Struct6Ds *tmp_v0_2;
    Struct70s *tmp_s70;

    GV_D_80391AD0 = arg0;
    if(GV_D_80391AD0 == 1){
        ncStaticCamera_setToNode(0x14);
        func_80324E38(0.0f, 3);
        timed_setStaticCameraToNode(4.0f, 0x15);
        func_80324E38(8.0f, 0);

        // [port] Split dereference from null check — &NULL->member is UB
        tmp_s70 = func_8034C5AC(0x130);
        if(tmp_s70){
            tmp_v0 = &tmp_s70->type_73;
            func_8034E71C(tmp_v0, -1000, 0.0f);
            func_8034E71C(tmp_v0, 0, 10.0f);
        }

        tmp_s70 = func_8034C528(0x190);
        if(tmp_s70){
            tmp_v0_2 = &tmp_s70->type_6D;
            subaddie_positionMoveVertical(tmp_v0_2, 0.0f, 270.0f, 0.0f, 1);
        }
    }//L803900AC

    if(GV_D_80391AD0 == 2){
        levelSpecificFlags_set(LEVEL_FLAG_6_GV_UNKNOWN, false);
        musicKeepsPlaying();
        transitionToMap(MAP_15_GV_WATER_PYRAMID, 1, 0);
    }
}

void func_803900F8(void){}

void func_80390100(void){
    GV_D_80391AD0 = 0;

    if (levelSpecificFlags_get(LEVEL_FLAG_6_GV_UNKNOWN)) {
        func_80390000(1);
    }
}

void func_80390138(void){
    if(GV_D_80391AD0 && func_80334904() == 2){
        if(GV_D_80391AD0 == 1 && timedFuncQueue_is_empty()){
            func_80390000(2);
        }
    }
}
