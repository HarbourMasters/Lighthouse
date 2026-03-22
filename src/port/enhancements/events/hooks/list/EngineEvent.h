#pragma once

#include "port/enhancements/events/hooks/EventSystem.h"
#include <stdarg.h>

DEFINE_EVENT(GameFrameUpdate);

typedef enum VBehaviorID {
    VB_INIT_RETURN_TO_LAIR,
} VBehaviorID;

DEFINE_EVENT(VanillaBehavior, VBehaviorID id; bool* should; void* args;);
