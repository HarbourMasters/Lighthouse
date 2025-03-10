#include "functions.h"
#include "variables.h"
#include <ultra64.h>

//purple_sparkle.c

void spawnPurpleSparkleParticles(Actor *this);

/* .data */
ActorInfo purpleSparkleActorInfo = {
    0x16,          0x6A,          0x0, 0x1, NULL, spawnPurpleSparkleParticles,
    func_80326244, func_80325340, 0,   0,   0.0f, 0};
s16 purpleSparkleAnimationFrames[] = {3, 2, 3, 4, 3, 5, 3, 0};
ParticleScaleAndLifetimeRanges purpleSparkleScaleAndLifetime = {
    {0.7f, 0.3f}, {0.01f, 0.01f}, {0.05f, 0.05f}, {0.2f, 0.2f}, 0.0f, 0.01f};
ParticleSettingsVelocityPosition purpleSparkleVelocityPosition = {
    {{-100.0f, 400.0f, -100.0f}, {100.0f, 800.0f, 100.0f}},
    {{-100.0f, 0.0f, -100.0f}, {100.0f, 0.0f, 100.0f}}};

/* .code */
void spawnPurpleSparkleParticles(Actor *this) {
  ParticleEmitter *pCtrl;
  f32 sp70[3];
  s32 phi_s6;
  s32 i;

  if (this->state == 2) {
    if (--this->unk38_31 == 0) {
      marker_despawn(this->marker);
    }
    pCtrl = partEmitMgr_newEmitter((s32)this->lifetime_value);
    for (phi_s6 = 0; phi_s6 < (s32)this->lifetime_value; phi_s6++) {
      for (i = 0; i < 3; i++) {
        sp70[i] = this->position[i] + randf() * 2 * this->unkF4_8 -
                  ((i == 1) ? 0 : this->unkF4_8);
      }
      particleEmitter_setSprite(pCtrl, ASSET_710_SPRITE_SPARKLE_PURPLE);
      particleEmitter_setStartingFrameRange(pCtrl, 0, 0);
      particleEmitter_setPosition(pCtrl, sp70);
      particleEmitter_setPositionAndVelocityRanges(
          pCtrl, &purpleSparkleVelocityPosition);
      particleEmitter_setScaleAndLifetimeRanges(pCtrl,
                                                &purpleSparkleScaleAndLifetime);
      particleEmitter_setSpawnInterval(pCtrl, 1.0f);
    }
  }
}
