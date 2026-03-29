#pragma once

#include "port/enhancements/events/hooks/EventSystem.h"
#include <stdarg.h>

#include "enums.h"
#include "prop.h"

DEFINE_EVENT(OnActorSpawn,
	enum actor_e actorId;
	int32_t posX;
	int32_t posY;
	int32_t posZ;
	int32_t rot;
	Actor* spawnedActor;
)