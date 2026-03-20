#pragma once

#include "port/enhancements/events/hooks/EventSystem.h"
#include <stdarg.h>

DEFINE_EVENT(GameFrameUpdate);

typedef enum VBehaviorID { VB_DUMMY } VBehaviorID;

DEFINE_EVENT(VanillaBehavior, VBehaviorID id; bool* should; va_list args;);
