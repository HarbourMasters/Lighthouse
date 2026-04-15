#include <ultra64.h>
#include "functions.h"
#include "variables.h"


extern GenFunction_0 commonParticleType_getInitMethod(enum common_particle_e);
extern GenFunction_0 commonParticleType_getFreeMethod(enum common_particle_e);
extern GenFunction_0 commonParticleType_getUpdateMethod(enum common_particle_e);

typedef struct {
    u8 unk0; //prev_particle_type
    u8 unk1; //current_particle_type
    u8 unk2; //next_particle_type
    u8 occupied;
}Struct_Core2_CB610_0;

/* .bss */
Struct_Core2_CB610_0 D_803861C0[40];

void commonParticleTypeMap_freeAll(void){
    s32 i;
    for(i = 1; i < 40; i++){
        D_803861C0[i].occupied = false;
    }
}

void commonParticleTypeMap_unused(void){ return; }

u8 commonParticleTypeMap_findFree(void){
    s32 i;
    for(i = 1; i < 40; i++){
        if(!D_803861C0[i].occupied){
            D_803861C0[i].occupied++;
            D_803861C0[i].unk0 = 0;
            D_803861C0[i].unk1 = 0;
            D_803861C0[i].unk2 = 0;
            return i;
        }
    }
    return 0;
}

void commonParticleTypeMap_freeByIndex(u8 arg0) {
    if (commonParticleType_getFreeMethod(D_803861C0[arg0].unk1) != NULL) {
        commonParticleType_getFreeMethod(D_803861C0[arg0].unk1)();
    }
    D_803861C0[arg0].occupied = 0;
}

void commonParticleTypeMap_advanceParticleType(u8 arg0, enum common_particle_e arg1){
    void (*funcPtr)(void);
    if(arg1){
        D_803861C0[arg0].unk2 = arg1;
        if(commonParticleType_getFreeMethod(D_803861C0[arg0].unk1)){
            funcPtr = commonParticleType_getFreeMethod(D_803861C0[arg0].unk1);
            funcPtr();
        }

        D_803861C0[arg0].unk0 = D_803861C0[arg0].unk1;
        D_803861C0[arg0].unk1 = D_803861C0[arg0].unk2;
        D_803861C0[arg0].unk2 = 0;
        if(commonParticleType_getInitMethod(D_803861C0[arg0].unk1)){
            funcPtr = commonParticleType_getInitMethod(D_803861C0[arg0].unk1);
            funcPtr();
        }

    }
}

s32 commonParticleTypeMap_getPreviousType(u8 arg0){
    return D_803861C0[arg0].unk0;
}

s32 commonParticleTypeMap_getCurrentType(u8 arg0){
    return D_803861C0[arg0].unk1;
}

s32 commonParticleTypeMap_getNextType(u8 arg0){
    return D_803861C0[arg0].unk2;
}

void commonParticleTypeMap_updateByIndex(u8 arg0) {
    if (commonParticleType_getUpdateMethod(D_803861C0[arg0].unk1) != NULL) {
        commonParticleType_getUpdateMethod(D_803861C0[arg0].unk1)();
    }
}
