#pragma once

#include <vector>

// Records the map/entrance pair of every map load so subareas can be catalogued
// in-game and promoted into the built-in warp table.
void WarpCatalog_Register(void);
void DrawEntranceRecorder(void);
void DrawWarpCatalog(void);

// Readable name for a map id, derived from the map_e enum.
const char* WarpCatalog_MapDisplayName(int map);

// An entrance prop found in the currently loaded map.
struct WarpEntranceInfo {
    int id;
    int pos[3];
    unsigned int yaw;
};

// Entrances present in the loaded map, rescanned once per map load.
const std::vector<WarpEntranceInfo>& WarpCatalog_GetMapEntrances(void);

// True when an entrance index maps to a marker actor at all. Indices outside the
// probeable set resolve through scripted spawn coords instead of props, so the
// scan can say nothing about them.
bool WarpCatalog_ExitIsProbeable(int exitId);

// True when a probeable entrance actually has a prop in the loaded map.
bool WarpCatalog_EntranceExists(int exitId);

void DrawWarpTab(void);

// Catalogued name for a map/entrance pair, or nullptr if it hasn't been named yet.
// Built-in names win over recorded ones when both cover the same pair.
const char* WarpCatalog_FindName(int map, int exit);
