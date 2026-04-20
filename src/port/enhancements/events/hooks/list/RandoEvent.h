#pragma once

#include <libultraship/bridge/eventsbridge.h>

#include "enums.h"
#include "prop.h"

DEFINE_EVENT(OnActorSpawn, enum actor_e actorId; int32_t posX; int32_t posY; int32_t posZ; int32_t rot; Actor * result;)

DEFINE_EVENT(OnActorCollision, Prop* propId;)