#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Config building
bool port_isRomhack(void);
const char* port_getRomhackName(void);

int port_getRomhackSceneRemap(int map_id);
int port_getRomhackReturnToLair(int level_index, int* out_map, int* out_exit);
int port_getRomhackMusic(int map_id, int* out_track1, int* out_track2);
int port_getRomhackSkybox(int scene_id);
int port_getRomhackSkyboxFull(int scene_id, int out_models[3], float out_scales[3], float out_rotations[3]);
int port_getRomhackSceneDef(int scene_id, int* out_opa, int* out_xlu);
int port_getRomhackSceneDefFull(int scene_id, int* out_opa, int* out_xlu, short out_min[3], short out_max[3],
                                float* out_scale);
int port_getRomhackNewGameMap(void);
int port_getRomhackStartLevel1(void);
int port_getRomhackStartLevel2(void);
int port_getRomhackMumboCost(int transform_id);
int port_getRomhackWarpExitBanjosHouse(void);
int port_getRomhackWarpEnterLair(void);
int port_getRomhackMaxEggs(void);
int port_getRomhackMaxEggsCheato(void);
int port_getRomhackMaxRedFeathers(void);
int port_getRomhackMaxRedFeathersCheato(void);
int port_getRomhackMaxGoldFeathers(void);
int port_getRomhackMaxGoldFeathersCheato(void);
int port_getRomhackNotesMax(void);
int port_getRomhackKnowAllMoves(void);
int port_getRomhackSpecialLevel(void);
int port_getRomhackHideJiggiesLevel(void);
int port_getRomhackHideCollectiblesLevel(void);
int port_getRomhackJiggiesPerWorld(void);
int port_getRomhackHoneycombsPerWorld(void);
int port_getRomhackExtraHcStart(void);
int port_getRomhackNoteDoor(int door_index);
int port_getRomhackJiggyPuzzleCost(int puzzle_index);
int port_getRomhackJiggyPuzzleSize(int puzzle_index);
int port_getRomhackJiggyPuzzleFlag(int puzzle_index);
const char* port_getRomhackLevelName(int level_index);
int port_getRomhackWarpDest(int warp_index);
void port_clearRomhackWarpDest(int warp_index);
bool port_getRomhackCustomCodeHash(char out_hex[41]);
int port_getRomhackCustomCodeKind(void);
bool port_getRomhackRomHash(char out_hex[41]);
const char* port_getRomhackIdentifier(void);

// Hack specific functions
int romhack_mumboTransform(int transformId);
int romhack_mumboWishwashyId(void);
int romhack_mumboRandomEventsAllowed(void);

#ifdef __cplusplus
}
#endif
