#pragma once

#include <libultraship/bridge/eventsbridge.h>

#include "enums.h"
#include "prop.h"

// clang-format off
DEFINE_EVENT(OnSaveLoad,
	void* saveData;
)

DEFINE_EVENT(OnActorSpawn,
	int32_t actorId;
	int32_t posX;
	int32_t posY;
	int32_t posZ;
	Actor* result;
)

DEFINE_EVENT(OnActorSaveState)

DEFINE_EVENT(OnActorCollision,
	Prop* propId;
)
// clang-format on