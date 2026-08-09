// BanjoDecomp: core2/particleemittermanager.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

typedef struct {
    f32 freeTime;
    ParticleEmitter *p_emitter;
    u32 isActive:1;
    u32 capacity:10;
    u32 padding:21;
} Struct_Core2_69F60_0;

void pem_free(u8 arg0);

/* .bss*/
u8 D_80380930;
Struct_Core2_69F60_0 D_80380938[16];

/* .code */
ParticleEmitter *pem_getEmitterByIndex(u8 arg0){
    if(D_80380938[arg0].p_emitter == NULL){
        D_80380930 = arg0;
        D_80380938[arg0].p_emitter = partEmitMgr_newEmitter(D_80380938[arg0].capacity);
        particleEmitter_manualFree(D_80380938[arg0].p_emitter);
        D_80380930 = 0;
    }
    D_80380938[arg0].freeTime = 1.0f;
    return D_80380938[arg0].p_emitter;
}

u8 pem_newEmitter(s32 cnt){
    int i;
    for(i = 1; i < 16; i++){
        if(D_80380938[i].isActive == 0){
            D_80380938[i].isActive++;
            D_80380938[i].p_emitter = NULL;
            D_80380938[i].capacity = cnt;
            return i;
        }
    }
    return 0;
}

void pem_freeAll(void){
    int i;
    for(i = 1; i < 16; i++){
        if(D_80380938[i].isActive != 0){
            pem_free(i);
        }
    }
}

void pem_setAllInactive(void){
    int i;
    for(i = 1; i < 16; i++){
        D_80380938[i].isActive = 0;
    }
}

void pem_free(u8 arg0){
    if(D_80380938[arg0].p_emitter){
        partEmitMgr_freeEmitter(D_80380938[arg0].p_emitter);
    }
    D_80380938[arg0].isActive = 0;
}

void pem_updateAll(void){
    int i;
    for(i = 1; i < 16; i++){
        if( D_80380938[i].isActive != 0
            && D_80380938[i].p_emitter != NULL
            && particleEmitter_isDone(D_80380938[i].p_emitter)
        ){
           D_80380938[i].freeTime -= time_getDelta();
           if(D_80380938[i].freeTime <= 0.0f){
                partEmitMgr_freeEmitter(D_80380938[i].p_emitter);
                D_80380938[i].p_emitter = NULL;
           }
        }
    }
}

void pem_freeEmitters(void){
    int i;
    for(i = 1; i < 16; i++){
        if( D_80380938[i].isActive != 0
            && D_80380938[i].p_emitter != NULL
            && i != D_80380930
        ){
           partEmitMgr_freeEmitter(D_80380938[i].p_emitter);
           D_80380938[i].p_emitter = NULL;
        }
    }
}

void pem_defragAll(void){
    int i;
    for(i = 1; i < 16; i++){
        if( D_80380938[i].isActive != 0
            && D_80380938[i].p_emitter != NULL
        ){
           D_80380938[i].p_emitter = partEmitMgr_defragEmitter(D_80380938[i].p_emitter);
        }
    }
}

void pem_freeDependencies(void){
    func_802EDD20();
    fxRipple_free();
    func_802F1E80();
    fxSparkle_free();
    func_802F404C();
    func_802F422C();
    dustEmitter_free();
    func_802F3CB0();
}

void pem_initDependencies(void){
    func_802EDD44();
    fxRipple_init();
    func_802F1EA4();
    fxSparkle_init();
    func_802F4070();
    func_802F4250();
    dustEmitter_init();
    func_802F3CD4();
}
