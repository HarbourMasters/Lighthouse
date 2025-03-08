#include "functions.h"
#include "variables.h"
#include <ultra64.h>

//sparklesprite.c

typedef struct {
  s32 unk0;
} SparkleSpriteLocal;

void spawnSparkleSprite(Actor *this);

/* .data */
s32 sparkleSpriteFrameDurations[] = {1, 1, 2, 2, 3, 4, 5, 6, 7, 7, 8, 8};
ActorInfo sparkleSpriteActorInfo1 = {0x58,
                        0x4E,
                        ASSET_7DE_SPRITE_SHOE_SPARKLES,
                        0,
                        NULL,
                        spawnSparkleSprite,
                        actor_update_func_80326224,
                        func_80325934,
                        0,
                        0,
                        0.0f,
                        0};

ActorInfo sparkleSpriteActorInfo2 = {0x59,
                        0x4F,
                        ASSET_7DE_SPRITE_SHOE_SPARKLES,
                        0,
                        NULL,
                        spawnSparkleSprite,
                        actor_update_func_80326224,
                        func_80325934,
                        0,
                        0,
                        0.0f,
                        0};

/* .code */
void spawnSparkleSprite(Actor *this) {
  // Spawns a sparkle sprite when touching turbo trainers or wading boots
  SparkleSpriteLocal *local = (SparkleSpriteLocal *)&this->local;
  if (!this->initialized) {
    switch (this->marker->id) {
    case 0x58:
      func_8032AA58(this, 0.35f);
      break;
    case 0x59:
      func_8032AA58(this, 0.4f);
      break;
    }
    this->marker->propPtr->unk8_5 = 0.5 < randf();
    this->position_y += 100.0 * this->scale;
    actor_collisionOff(this);
    this->initialized = TRUE;
  } // L802C5B60
  this->marker->propPtr->unk8_15 = sparkleSpriteFrameDurations[local->unk0];
  local->unk0++;
  if (local->unk0 >= 0xC)
    marker_despawn(this->marker);
}
