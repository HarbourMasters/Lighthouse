#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Config building
bool RomhackConfig_IsActive(void);
const char* RomhackConfig_GetName(void);

int RomhackConfig_GetSceneRemap(int map_id);
int RomhackConfig_GetReturnToLair(int level_index, int* out_map, int* out_exit);
int RomhackConfig_GetMusic(int map_id, int* out_track1, int* out_track2);
int RomhackConfig_GetSkybox(int scene_id);
int RomhackConfig_GetSkyboxFull(int scene_id, int out_models[3], float out_scales[3], float out_rotations[3]);
int RomhackConfig_GetSceneDef(int scene_id, int* out_opa, int* out_xlu);
int RomhackConfig_GetSceneDefFull(int scene_id, int* out_opa, int* out_xlu, short out_min[3], short out_max[3],
                                  float* out_scale);
int RomhackConfig_GetNewGameMap(void);
int RomhackConfig_GetStartLevel1(void);
int RomhackConfig_GetStartLevel2(void);
int RomhackConfig_GetMumboCost(int transform_id);
int RomhackConfig_GetWarpExitBanjosHouse(void);
int RomhackConfig_GetWarpEnterLair(void);
int RomhackConfig_GetMaxEggs(void);
int RomhackConfig_GetMaxEggsCheato(void);
int RomhackConfig_GetMaxRedFeathers(void);
int RomhackConfig_GetMaxRedFeathersCheato(void);
int RomhackConfig_GetMaxGoldFeathers(void);
int RomhackConfig_GetMaxGoldFeathersCheato(void);
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
int RomhackConfig_GetJiggyPuzzleCost(int puzzle_index);
int RomhackConfig_GetJiggyPuzzleSize(int puzzle_index);
int RomhackConfig_GetJiggyPuzzleFlag(int puzzle_index);
const char* RomhackConfig_GetLevelName(int level_index);
int RomhackConfig_GetWarpDest(int warp_index);
void RomhackConfig_ClearWarpDest(int warp_index);
bool RomhackConfig_GetCustomCodeHash(char out_hex[41]);
int RomhackConfig_GetCustomCodeKind(void);
bool RomhackConfig_GetRomHash(char out_hex[41]);
const char* RomhackConfig_GetIdentifier(void);

#ifdef __cplusplus
}
#endif
