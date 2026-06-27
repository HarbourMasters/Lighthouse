// BanjoDecomp: code_9B70.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"


// [port] Tracks whether the valley pyramid is currently shown raised. Set on map entry by
// func_8038FF68; lets the live re-check in func_8038FFF4 fire exactly once.
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

    // [port] Raise the valley pyramid live when the water-pyramid jiggy becomes collected while
    // we're already in the valley — e.g. a networked teammate finishes the puzzle inside the
    // pyramid. func_8038FF68 only applies the raise on map entry, so without this the pyramid stays
    // sunk on the teammate's screen until the next reload. Restoring 0x130 to 0 undoes the sink the
    // entry path applied. Gated on LEVEL_FLAG_6 being clear so this never fights the local rise
    // cutscene (func_80390000), which owns that flag while it plays.
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
