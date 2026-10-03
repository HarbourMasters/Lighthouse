#pragma once

#include "libultraship/libultra/types.h"
#include "enums.h"

#ifdef __cplusplus
extern "C" {
#endif

// [port] Level name from the pause menu totals table (romhack-patchable), supplemented
// for the level_e values that table doesn't list. Upper case, as the game stores it.
const char* port_levelName(enum level_e level);

// [port] As above, for the level containing map_id; falls back to a debug map name.
const char* port_getLevelName(int map_id);

// [port] Per-level game stats (romhack-aware max values).
void port_getLevelStats(int map_id, s32* noteVal, s32* noteMax, s32* jiggyVal, s32* jiggyMax, s32* hcVal, s32* hcMax);

// [port] A level's note, jiggy and honeycomb maximums (romhack-aware).
void port_getLevelMaxima(enum level_e level, s32* noteMax, s32* jiggyMax, s32* hcMax);

// [port] Whether the pause totals screen shows a row for the level: 0 notes, 1 jiggies, 2 honeycombs, 3 time.
bool port_pauseTotalsRowVisible(enum level_e level, s32 row);

// [port] The level on a pause totals page, 1 to 11.
enum level_e port_pauseTotalsPageLevel(s32 page);

// [port] Per-level play time in seconds.
u16 port_getLevelTime(int map_id);

// [port] Update the window title with current world stats.
void port_setWindowTitle(int map_id);

#ifdef __cplusplus
}
#endif
