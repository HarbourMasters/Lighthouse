#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Config building
bool RomhackConfig_IsActive(void);
const char* RomhackConfig_GetName(void);

int RomhackConfig_GetNewGameMap(void);
int RomhackConfig_GetMumboCost(int transform_id);
int RomhackConfig_GetMaxEggs(void);
int RomhackConfig_GetMaxRedFeathers(void);
int RomhackConfig_GetMaxGoldFeathers(void);
int RomhackConfig_GetNotesMax(void);
int RomhackConfig_GetKnowAllMoves(void);
int RomhackConfig_GetSpecialLevel(void);
int RomhackConfig_GetHideJiggiesLevel(void);
int RomhackConfig_GetHideCollectiblesLevel(void);
int RomhackConfig_GetJiggiesPerWorld(void);
void RomhackConfig_OverrideJiggiesPerWorld(int value);
int RomhackConfig_GetHoneycombsPerWorld(void);
void RomhackConfig_OverrideHoneycombsPerWorld(int value);
int RomhackConfig_GetExtraHcStart(void);
int RomhackConfig_GetNoteDoor(int door_index);
int RomhackConfig_GetJiggyPuzzleSize(int puzzle_index);
int RomhackConfig_GetJiggyPuzzleFlag(int puzzle_index);
const char* RomhackConfig_GetLevelName(int level_index);
int RomhackConfig_GetWarpDest(int warp_index);
void RomhackConfig_ClearWarpDest(int warp_index);
void RomhackConfig_ClearNamedWarps(void);
const char* RomhackConfig_GetIdentifier(void);

#ifdef __cplusplus
}
#endif
