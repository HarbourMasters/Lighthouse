#include <ultra64.h>
#include "functions.h"
#include "variables.h"

typedef struct {
    f32 unk0;
    ParticleEmitter *p_emitter;
    u32 unk8_31:1;
    u32 capacity:10;
    u32 pad8_20:21;
} Struct_Core2_69F60_0;

void pem_free(u8 arg0);

/* .bss*/
u8 initializingIndex;
Struct_Core2_69F60_0 emitterData[16];

/* .code */
ParticleEmitter *pem_getEmitterByIndex(u8 arg0){
    if(emitterData[arg0].p_emitter == NULL){
        initializingIndex = arg0;
        emitterData[arg0].p_emitter = partEmitMgr_newEmitter(emitterData[arg0].capacity);
        particleEmitter_manualFree(emitterData[arg0].p_emitter);
        initializingIndex = 0;
    }
    emitterData[arg0].unk0 = 1.0f;
    return emitterData[arg0].p_emitter;
}

u8 pem_newEmitter(s32 cnt){
    int i;
    for(i = 1; i < 16; i++){
        if(emitterData[i].unk8_31 == 0){
            emitterData[i].unk8_31++;
            emitterData[i].p_emitter = NULL;
            emitterData[i].capacity = cnt;
            return i;
        }
    }
    return 0;
}

void pem_freeAll(void){
    int i;
    for(i = 1; i < 16; i++){
        if(emitterData[i].unk8_31 != 0){
            pem_free(i);
        }
    }
}

void pem_setAllInactive(void){
    int i;
    for(i = 1; i < 16; i++){
        emitterData[i].unk8_31 = 0;
    }
}

void pem_free(u8 arg0){
    if(emitterData[arg0].p_emitter){
        partEmitMgr_freeEmitter(emitterData[arg0].p_emitter);
    }
    emitterData[arg0].unk8_31 = 0;
}

void pem_updateAll(void){
    int i;
    for(i = 1; i < 16; i++){
        if( emitterData[i].unk8_31 != 0
            && emitterData[i].p_emitter != NULL
            && particleEmitter_isDone(emitterData[i].p_emitter)
        ){
           emitterData[i].unk0 -= time_getDelta();
           if(emitterData[i].unk0 <= 0.0f){
                partEmitMgr_freeEmitter(emitterData[i].p_emitter);
                emitterData[i].p_emitter = NULL;
           }
        }
    }
}

void pem_freeEmitters(void){
    int i;
    for(i = 1; i < 16; i++){
        if( emitterData[i].unk8_31 != 0
            && emitterData[i].p_emitter != NULL
            && i != initializingIndex
        ){
           partEmitMgr_freeEmitter(emitterData[i].p_emitter);
           emitterData[i].p_emitter = NULL;
        }
    }
}

void pem_defragAll(void){
    int i;
    for(i = 1; i < 16; i++){
        if( emitterData[i].unk8_31 != 0
            && emitterData[i].p_emitter != NULL
        ){
           emitterData[i].p_emitter = partEmitMgr_defragEmitter(emitterData[i].p_emitter);
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
