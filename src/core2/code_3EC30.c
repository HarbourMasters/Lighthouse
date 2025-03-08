#include "functions.h"
#include "variables.h"
#include <ultra64.h>

//rippleeffect.c

void actor_initRippleEffect(Actor *this);

/* .data */
ActorInfo rippleEffectActorInfo = {0x65,
                        0xF3,
                        0x0,
                        0x0,
                        0x0,
                        actor_initRippleEffect,
                        actor_update_func_80326224,
                        func_80325340,
                        0,
                        0,
                        0.0f,
                        0};

/* .code */
void actor_initRippleEffect(Actor *this) {
  if (!this->initialized) {
    actor_collisionOff(this);
    this->lifetime_value = 0.0f;
    this->pitch = 90.0f;
    fxRipple_802F3554(4, this->position);
    this->initialized = TRUE;
  } else {
    this->lifetime_value += time_getDelta();
    if (2.0 < this->lifetime_value)
      marker_despawn(this->marker);
  }
}
