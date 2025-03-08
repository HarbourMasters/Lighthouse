#include "functions.h"
#include "variables.h"
#include <ultra64.h>

extern ParticleEmitter *getOrCreateParticleEmitter(u8);

//fxSparkle?

/* .bss */
u8 D_80380A80;

/* .code */
void resetfxSparkleParticleEmitter(void) { freeParticleEmitter(D_80380A80); }

void initializefxSparkleParticleEmitter(void) { D_80380A80 = allocateParticleEmitter(0xF); }

ParticleEmitter *createfxSparkleParticleEmitter(f32 pos[3], enum asset_e sprite_id) {
  ParticleEmitter *this;
  this = getOrCreateParticleEmitter(D_80380A80);

  particleEmitter_setSprite(this, sprite_id);
  particleEmitter_setAccelerationRange(this, 0.0f, -250.0f, 0.0f, 0.0f, -250.0f,
                                       0.0f);
  particleEmitter_setFade(this, 0.4f, 0.8f);
  particleEmitter_setPosition(this, pos);
  particleEmitter_setStartingScaleRange(this, 0.22f, 0.22f);
  particleEmitter_setFinalScaleRange(this, 0.03f, 0.03f);
  particleEmitter_setAngularVelocityRange(this, 0.0f, 0.0f, 200.0f, 0.0f, 0.0f,
                                          240.0f);
  particleEmitter_setParticleLifeTimeRange(this, 0.7f, 0.7f);
  particleEmitter_setCullDistance(this, 1.0f);
  return this;
}
