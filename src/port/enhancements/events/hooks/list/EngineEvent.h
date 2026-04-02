#pragma once

#include "port/enhancements/events/hooks/EventSystem.h"
#include <stdarg.h>

DEFINE_EVENT(GameFrameUpdate);

typedef enum VBehaviorID {
    VB_INIT_RETURN_TO_LAIR,
    VB_STATIC_CAMERA_SET,
    VB_STATIC_CAMERA_EXIT,
} VBehaviorID;

DEFINE_EVENT(VanillaBehavior, VBehaviorID id; bool* should; void* args;);

DEFINE_EVENT(OnMapLoad, int32_t mapId;);
