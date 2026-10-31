#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Config building
bool port_isRomhack(void);
const char* port_getRomhackName(void);

int port_getRomhackReturnToLair(int level_index, int* out_map, int* out_exit);
int port_getRomhackNewGameMap(void);
int port_getRomhackMumboCost(int transform_id);
int port_getRomhackMaxEggs(void);
int port_getRomhackMaxRedFeathers(void);
int port_getRomhackMaxGoldFeathers(void);
int port_getRomhackNotesMax(void);
int port_getRomhackKnowAllMoves(void);
int port_getRomhackSpecialLevel(void);
int port_getRomhackHideJiggiesLevel(void);
int port_getRomhackHideCollectiblesLevel(void);
int port_getRomhackJiggiesPerWorld(void);
void port_overrideRomhackJiggiesPerWorld(int value);
int port_getRomhackHoneycombsPerWorld(void);
void port_overrideRomhackHoneycombsPerWorld(int value);
int port_getRomhackExtraHcStart(void);
int port_getRomhackNoteDoor(int door_index);
int port_getRomhackJiggyPuzzleCost(int puzzle_index);
int port_getRomhackJiggyPuzzleSize(int puzzle_index);
int port_getRomhackJiggyPuzzleFlag(int puzzle_index);
const char* port_getRomhackLevelName(int level_index);
int port_getRomhackWarpDest(int warp_index);
void port_clearRomhackWarpDest(int warp_index);
void port_clearRomhackNamedWarps(void);
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
