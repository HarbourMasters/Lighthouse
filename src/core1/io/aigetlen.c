#ifndef LIGHTHOUSE_P
#include <rcp.h>
#else
#include "pc_oscompat.h"
#endif

u32 osAiGetLength(void) { return IO_READ(AI_LEN_REG); }
