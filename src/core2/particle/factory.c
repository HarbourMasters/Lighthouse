#include <ultra64.h>
#include "functions.h"
#include "variables.h"


extern GenFunction_0 commonParticleType_getInitMethod(enum common_particle_e); // [port] was missing — implicit int return truncated function pointer on 64-bit
extern GenFunction_0 commonParticleType_getFreeMethod(enum common_particle_e);
extern GenFunction_0 commonParticleType_getUpdateMethod(enum common_particle_e);

typedef struct {
    u8 unk0; //prev_particle_type
    u8 unk1; //current_particle_type
    u8 unk2; //next_particle_type
    u8 occupied;
}Struct_Core2_CB610_0;

/* .bss */
Struct_Core2_CB610_0 typeMaps[40];

void commonParticleTypeMap_freeAll(void){
    s32 i;
    for(i = 1; i < 40; i++){
        typeMaps[i].occupied = false;
    }
}

void commonParticleTypeMap_unused(void){ return; }

u8 commonParticleTypeMap_findFree(void){
    s32 i;
    for(i = 1; i < 40; i++){
        if(!typeMaps[i].occupied){
            typeMaps[i].occupied++;
            typeMaps[i].unk0 = 0;
            typeMaps[i].unk1 = 0;
            typeMaps[i].unk2 = 0;
            return i;
        }
    }
    return 0;
}

void commonParticleTypeMap_freeByIndex(u8 arg0) {
    if (commonParticleType_getFreeMethod(typeMaps[arg0].unk1) != NULL) {
        commonParticleType_getFreeMethod(typeMaps[arg0].unk1)();
    }
    typeMaps[arg0].occupied = 0;
}

void commonParticleTypeMap_advanceParticleType(u8 arg0, enum common_particle_e arg1){
    void (*funcPtr)(void);
    if(arg1){
        typeMaps[arg0].unk2 = arg1;
        if(commonParticleType_getFreeMethod(typeMaps[arg0].unk1)){
            funcPtr = commonParticleType_getFreeMethod(typeMaps[arg0].unk1);
            funcPtr();
        }

        typeMaps[arg0].unk0 = typeMaps[arg0].unk1;
        typeMaps[arg0].unk1 = typeMaps[arg0].unk2;
        typeMaps[arg0].unk2 = 0;
        if(commonParticleType_getInitMethod(typeMaps[arg0].unk1)){
            funcPtr = commonParticleType_getInitMethod(typeMaps[arg0].unk1);
            funcPtr();
        }

    }
}

s32 commonParticleTypeMap_getPreviousType(u8 arg0){
    return typeMaps[arg0].unk0;
}

s32 commonParticleTypeMap_getCurrentType(u8 arg0){
    return typeMaps[arg0].unk1;
}

s32 commonParticleTypeMap_getNextType(u8 arg0){
    return typeMaps[arg0].unk2;
}

void commonParticleTypeMap_updateByIndex(u8 arg0) {
    if (commonParticleType_getUpdateMethod(typeMaps[arg0].unk1) != NULL) {
        commonParticleType_getUpdateMethod(typeMaps[arg0].unk1)();
    }
}
