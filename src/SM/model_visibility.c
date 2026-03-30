// BanjoDecomp: code_2900.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

/* .code */
void func_80388CF0(void){
    Struct70s *tmp_s70;
    if(getGameMode() != GAME_MODE_7_ATTRACT_DEMO){
        // [port] Split dereference from null check — &NULL->member is UB
        tmp_s70 = func_8034C528(0x1F2);
        if(tmp_s70) func_8034DEB4(&tmp_s70->type_6D, 0.0f);
        tmp_s70 = func_8034C528(0x1F3);
        if(tmp_s70) func_8034DEB4(&tmp_s70->type_6D, -5000.0f);
    }
}

void func_80388D48(void){
    // Lighthouse TODO is this an anti-piracy check?
    #if 0
    if(*(u32*)PHYS_TO_K1(0x200) - PHYS_TO_K1(0xC290000)){
        func_80388CF0();
    }
#endif
}
