#pragma once

#include "port/enhancements/events/hooks/EventSystem.h"
#include <stdarg.h>
#include "prop.h"

DEFINE_EVENT(GameFrameUpdate);
DEFINE_EVENT(FrameDrawEnd);

DEFINE_EVENT(VanillaBehavior, VBehaviorID id; bool* should; va_list * originalArgs;);

DEFINE_EVENT(OnMapLoad, int32_t mapId;);

DEFINE_EVENT(OnActorTick, Actor* actor;);
DEFINE_EVENT(OnPropTick, ActorMarker* marker; float* position;);
DEFINE_EVENT(OnSpritePropTick, int32_t assetId; float* position;);
DEFINE_EVENT(OnNametagDraw, Actor* actor; const char* label; float yOffset;);