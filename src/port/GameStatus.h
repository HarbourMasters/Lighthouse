#pragma once

#include "libultraship/libultra/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// [port] Current level name from the pause menu totals table (romhack-patchable).
// Returns the patched string for the level containing map_id, or a fallback debug name.
const char* port_getLevelName(int map_id);

// [port] Per-level game stats (romhack-aware max values).
void port_getLevelStats(int map_id, s32* noteVal, s32* noteMax, s32* jiggyVal, s32* jiggyMax, s32* hcVal, s32* hcMax);

// [port] Per-level play time in seconds.
u16 port_getLevelTime(int map_id);

// [port] Update the window title with current world stats.
void port_setWindowTitle(int map_id);

#ifdef __cplusplus
}
#endif
