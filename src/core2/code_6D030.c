#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include "core2/particle.h"

//does this spawn bubbles or ripples
//TODO: fix bubbles or ripples

ParticleEmitter *createBubbleParticleEmitter(f32 pos[3], f32 arg1);

/* .bss */
u8 D_80380A90;

/* .code */
void createRippleEffect(ParticleEmitter *this, f32 pos[3]) {
  fxRipple_802F3554(0, pos);
}

void emitBubbleParticles(f32 pos[3]) {
  ParticleEmitter *pCtrl;
  pCtrl = createBubbleParticleEmitter(pos, 20.0f);
  particleEmitter_setParticleVelocityRange(pCtrl, -180.0f, 400.0f, -180.0f,
                                           180.0f, 700.0f, 180.0f);
  particleEmitter_emitN(pCtrl, 20);
}

void resetBubbleParticleEmitter(void) { freeParticleEmitter(D_80380A90); }

void initializeBubbleParticleEmitter(void) { D_80380A90 = allocateParticleEmitter(0x1e); }

ParticleEmitter *createBubbleParticleEmitter(f32 pos[3], f32 arg1) {
  ParticleEmitter *pCtrl;
  static s32 D_80368DD0[3] = {0xff, 0xff, 0xfe};

  pCtrl = getOrCreateParticleEmitter(D_80380A90);
  particleEmitter_setSprite(pCtrl, ASSET_70B_SPRITE_BUBBLE_2);
  particleEmitter_setAlpha(pCtrl, 180);
  particleEmitter_setAccelerationRange(pCtrl, 0.0f, -1300.0f, 0.0f, 0.0f,
                                       -1300.0f, 0.0f);
  particleEmitter_setCollisionCount(pCtrl, 1);
  particleEmitter_setMinHeight(pCtrl, pos[1]);
  particleEmitter_setParticleCallback(pCtrl, createRippleEffect);
  particleEmitter_setFade(pCtrl, 0.0f, 0.8f);
  particleEmitter_setSpawnPositionRange(pCtrl, -arg1, 0.0f, -arg1, arg1, 0.0f,
                                        arg1);
  particleEmitter_setPosition(pCtrl, pos);
  particleEmitter_setRGB(pCtrl, D_80368DD0);
  particleEmitter_setStartingScaleRange(pCtrl, 0.02f, 0.04f);
  particleEmitter_setFinalScaleRange(pCtrl, 0.01f, 0.01f);
  particleEmitter_setParticleLifeTimeRange(pCtrl, 2.0f, 2.0f);
  particleEmitter_setCullDistance(pCtrl, 10.0f);
  return pCtrl;
}
