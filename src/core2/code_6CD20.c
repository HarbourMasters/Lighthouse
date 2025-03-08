#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include "core2/particle.h"

extern ParticleEmitter *getOrCreateParticleEmitter(u8);

/* .data */
s16 D_80368D80[] = {
    ASSET_715_SPRITE_SPARKLE_RED,
    ASSET_716_SPRITE_SPARKLE_WHITE,
    ASSET_719_SPRITE_SPARKLE_GREEN_2,
};

ParticleSettingsVelocityAccelerationPosition D_80368D88 = {
    {{-300.0f, -300.0f, -300.0f}, {300.0f, 300.0f, 300.0f}},
    {{0.0f, -100.0f, 0.0f}, {0.0f, -100.0f, 0.0f}},
    {{-25.0f, -25.0f, -25.0f}, {25.0f, 25.0f, 25.0f}}};

/* .bss */
u8 D_80380A70;

/* .code */
void resetSparkleParticleEmitter(void) { freeParticleEmitter(D_80380A70); }

void initializeSparkleParticleEmitter(void) { D_80380A70 = allocateParticleEmitter(0xC); }

void createSparkleParticleEmitter(f32 arg0[3], s32 arg1, s32 arg2) {
  ParticleEmitter *pCtrl;
  if (arg2 < 3) {
    pCtrl = getOrCreateParticleEmitter(D_80380A70);
    particleEmitter_setSprite(pCtrl, D_80368D80[arg2]);
    particleEmitter_setFade(pCtrl, 0.4f, 0.8f);
    particleEmitter_setPosition(pCtrl, arg0);
    particleEmitter_setAngularVelocityRange(pCtrl, 0.0f, 0.0f, 200.0f, 0.0f,
                                            0.0f, 240.0f);
    particleEmitter_setCullDistance(pCtrl, 1.0f);
    particleEmitter_setFade(pCtrl, 0.1f, 0.5f);
    particleEmitter_setStartingScaleRange(pCtrl, 0.1f, 0.2f);
    particleEmitter_setFinalScaleRange(pCtrl, 0.4f, 0.5f);
    particleEmitter_setParticleLifeTimeRange(pCtrl, 0.75f, 0.75f);
    particleEmitter_setVelocityAccelerationAndPositionRanges(pCtrl,
                                                             &D_80368D88);
    if (arg1 == 0) {
      particleEmitter_setDrawMode(pCtrl, PART_EMIT_NO_DEPTH);
    }
    particleEmitter_emitN(pCtrl, 12);
  }
}
