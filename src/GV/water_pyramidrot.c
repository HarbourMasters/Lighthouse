// BanjoDecomp: code_9B70.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"


static s32 sGvPyramidRaised = 0;

void func_8038FF60(void){}

void func_8038FF68(void){
    Struct70s *tmp_s70;

    if(gsworld_getMap() != MAP_12_GV_GOBIS_VALLEY) return;

    // [port] Split dereference from null check — &NULL->member is UB
    if(jiggyscore_isCollected(JIGGY_42_GV_WATER_PYRAMID)){
        tmp_s70 = func_8034C528(0x190);
        if(tmp_s70) subaddie_positionMoveVertical(&tmp_s70->type_6D, 0.0f, 270.0f, 0.0f, 1);
        sGvPyramidRaised = 1;
    }
    else{
        tmp_s70 = func_8034C5AC(0x130);
        if(tmp_s70) func_8034E71C(&tmp_s70->type_73, -1500, 0.0f);
        sGvPyramidRaised = 0;
    }
}

void func_8038FFF4(void){
    Struct70s *tmp_s70;

    // LEVEL_FLAG_6 = local rise cutscene in progress.
    if(sGvPyramidRaised) return;
    if(gsworld_getMap() != MAP_12_GV_GOBIS_VALLEY) return;
    if(levelSpecificFlags_get(LEVEL_FLAG_6_GV_UNKNOWN)) return;
    if(!jiggyscore_isCollected(JIGGY_42_GV_WATER_PYRAMID)) return;

    tmp_s70 = func_8034C5AC(0x130);
    if(tmp_s70) func_8034E71C(&tmp_s70->type_73, 0, 2.5f);
    tmp_s70 = func_8034C528(0x190);
    if(tmp_s70) subaddie_positionMoveVertical(&tmp_s70->type_6D, 0.0f, 270.0f, 2.5f, 1);
    sGvPyramidRaised = 1;
}
