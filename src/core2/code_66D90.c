#include <ultra64.h>
#include "functions.h"
#include "variables.h"

extern void particleEmitter_setMaxHeight(ParticleEmitter *, f32);
extern void particleEmitter_setWaveAmplitude(ParticleEmitter *, f32, f32, f32);
extern void particleEmitter_setWaveFrequency(ParticleEmitter *, f32, f32, f32);
extern void particleEmitter_setWaveLength(ParticleEmitter *, f32);
extern ParticleEmitter *getOrCreateParticleEmitter(u8);

/* .data */
s32 D_80368850[3] = {0xFF, 0xFF, 0xFE};

/* .bss */
u8 D_803808F0;

/* .code */
void func_802EDD20(void){
    freeParticleEmitter(D_803808F0);
}

void func_802EDD44(void){
    D_803808F0 = allocateParticleEmitter(0x1e);
}

void func_802EDD68(ParticleEmitter *caller, f32 pos[3]){
    fxRipple_802F3554(0, pos);
}

ParticleEmitter *func_802EDD8C(f32 pos[3], f32 xz_range, f32 arg2){
    ParticleEmitter *pCtrl = getOrCreateParticleEmitter(D_803808F0);
    particleEmitter_setSprite(pCtrl, ASSET_70A_SPRITE_BUBBLE_1);
    particleEmitter_setDrawMode(pCtrl, 4);
    particleEmitter_setAlpha(pCtrl, 0xff);
    particleEmitter_setAccelerationRange( pCtrl,
        0.0f, 150.0f, 0.0f,
        0.0f, 150.0f, 0.0f
    );
    particleEmitter_setCollisionCount(pCtrl, 1);
    particleEmitter_setMaxHeight(pCtrl, arg2);
    particleEmitter_setParticleCallback(pCtrl, func_802EDD68);
    particleEmitter_setFade(pCtrl, 0.0f, 0.8f);
    particleEmitter_setParticleLifeTimeRange(pCtrl, 4.0f, 4.0f);
    particleEmitter_setSpawnPositionRange( pCtrl,
        -xz_range, 0.0f, -xz_range,
        xz_range, 0.0f, xz_range
    );
    particleEmitter_setPosition(pCtrl, pos);
    particleEmitter_setRGB(pCtrl, D_80368850);
    particleEmitter_setStartingScaleRange(pCtrl, 0.05f, 0.09f);
    particleEmitter_setFinalScaleRange(pCtrl, 0.05f, 0.09f);
    particleEmitter_setCullDistance(pCtrl, 10.0f);
    particleEmitter_setWaveAmplitude(pCtrl, 0.25f, 0.25f, 0.0f);
    particleEmitter_setWaveFrequency(pCtrl, 0.0f, 0.1f, 0.0f);
    particleEmitter_setWaveLength(pCtrl, 0.2f);

    return pCtrl;
}
