// BanjoDecomp: core2/code_DC4B0.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

void func_80363500(Actor *this);

/* .data */
// Next To SM Bridge Bottles
// Gets removed when player learns all SM Moves
ActorInfo D_80373DC0 = {
    0x1EE, ACTOR_3BA_UNKNOWN, 0,
    0, NULL, 
    func_80363500, actor_update_func_80326224, func_80325340,
    0, 0, 0.0f, 0
}; 

// [port] Helper — safe dereference for &func_8034C528()->type_6D (NULL->member is UB)
static inline void safe_8034DEB4(s32 id, f32 val) {
    Struct70s *s = func_8034C528(id);
    if (s) func_8034DEB4(&s->type_6D, val);
}

/* .code */
void func_80363440(void){
    safe_8034DEB4(0x1F1, -5000.0f);
}

void func_80363470(void){
    safe_8034DEB4(0x1F2, -5000.0f);
    safe_8034DEB4(0x1F3, 0.0f);
    func_80363440();
}

void func_803634BC(void){
    safe_8034DEB4(0x1F3, -5000.0f);
    safe_8034DEB4(0x1F2, 0.0f);
}

void func_80363500(Actor *this){
    if(!this->volatile_initialized){
        if(!chmole_learnedAllSpiralMountainAbilities()){
            func_803634BC();
        }
        else{
            func_80363440();
            marker_despawn(this->marker);
        }
        this->volatile_initialized = true;
    }//L8036355C

    if(chmole_learnedAllSpiralMountainAbilities()){
        func_80363470();
        marker_despawn(this->marker);
    }
}
