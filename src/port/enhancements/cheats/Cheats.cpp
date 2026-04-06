// Cheats
//
// Cheat enhancement port functions.

#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"

// Infinite health — prevents health from being consumed.
extern "C" int port_isInfiniteHealth(void) {
    return CVarGetInteger(CVAR_ENHANCEMENT("Cheats.InfiniteHealth"), 0);
}
