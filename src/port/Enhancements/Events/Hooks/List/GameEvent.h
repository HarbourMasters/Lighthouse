#pragma once

#include <libultraship/bridge/eventsbridge.h>

DEFINE_EVENT(SetAnimSpeedMult, int32_t* mult; int32_t id;)
DEFINE_EVENT(OnActorUpdate, Actor* actor;)
DEFINE_EVENT(OnActorDestroy, Actor* actor;)
DEFINE_EVENT(OnActorArrayFree)
DEFINE_EVENT(OnMapFileLoad, int32_t map;)
DEFINE_EVENT(OnPlayerDeath)
DEFINE_EVENT(OnGameFileErase, int32_t gamenum;)
DEFINE_EVENT(OnGameSave, int32_t fileNum;)
DEFINE_EVENT(OnGameLoad, int32_t fileNum;)
DEFINE_EVENT(OnGameErase, int32_t gameNum;)
// flagSpace = AnchorFlagSpace; length = 1 for single sets, bit count for setN.
DEFINE_EVENT(OnGameFlagSet, int32_t flagSpace; int32_t index; int32_t value; int32_t length;)
// count is the resulting absolute value.
DEFINE_EVENT(OnItemCountChanged, int32_t item; int32_t count;)
// kind = AnchorCollectibleSpace.
DEFINE_EVENT(OnCollectibleCollected, int32_t kind; int32_t id;)
// prop = Prop*, an egg, feather or extra life picked up.
DEFINE_EVENT(OnConsumableCollected, void* prop;)
DEFINE_EVENT(OnNoteCollected, ActorMarker* marker;)
// markerId = marker_e.
DEFINE_EVENT(OnJinjoCollected, int32_t markerId;)
// A carried actor leaving the player's hands; markerId = marker_e, start/target are f32[3].
DEFINE_EVENT(OnCarryThrow, int32_t markerId; float* start; float* target;)
// move = ability_e; value = 1 learned, 0 cleared.
DEFINE_EVENT(OnAbilityLearned, int32_t move; int32_t value;)
DEFINE_EVENT(OnJiggySpawned, int32_t jiggyId; float x; float y; float z;)
DEFINE_EVENT(OnHoneycombDropSpawn, int32_t honeycombId; int32_t bundleId; float x; float y; float z;)
DEFINE_EVENT(OnTimedJiggyExpired, int32_t jiggyId;)
DEFINE_EVENT(OnPropInit, Prop* propPtr;)
DEFINE_EVENT(OnBottlesBonusComplete, int32_t index;)
DEFINE_EVENT(OnSaveFileLoad, int32_t fileNum; void* saveBuffer; int32_t result;)
DEFINE_EVENT(OnSaveFileSave, void* saveBuffer; int32_t fileNum; int32_t * result;)
// The warp_* functions a romhack's config can send elsewhere.
typedef enum WarpId {
    WARP_ID_SM_EXIT_BANJOS_HOUSE = 1,
    WARP_ID_LAIR_ENTER_LAIR_FROM_SM_LEVEL = 3,
} WarpId;

// nodeProp = NodeProp*.
DEFINE_EVENT(OnWarpDispatch, void* nodeProp; int32_t warpIndex;)
DEFINE_EVENT(OnNamedWarp, void* nodeProp; int32_t warpId;)
DEFINE_EVENT(OnWarpMusic, int32_t dest;)
DEFINE_EVENT(OnMapChangeRequest, int32_t* map; int32_t * exit;)
DEFINE_EVENT(OnMapTransitionIn, int32_t map; int32_t * index;)
DEFINE_EVENT(OnMapTransitionOut, int32_t from; int32_t to; int32_t * index;)
DEFINE_EVENT(OnNewGame)
DEFINE_EVENT(OnDefaultBootMap, int32_t* map;)
// result = MapSkyInfo* or MapModelDescription* for the first two.
DEFINE_EVENT(OnGetMapSkyInfo, int32_t map; void* result;)
DEFINE_EVENT(OnGetMapModelDescription, int32_t map; void* result;)
DEFINE_EVENT(OnGetMapMusicTrack, int32_t map; bool secondary; int32_t result;)
DEFINE_EVENT(OnGetMapLevel, int32_t map; int32_t result;)
// The file-select commit, for both new game and continue. Unlike OnGameLoad this
// does not fire for the zoombox preview.
DEFINE_EVENT(OnGameStart)
DEFINE_EVENT(EggHeadSpawn, float* pitch; float* spawnHeight; float* minVerticalVelocity; float* yawBias;
             int32_t * flattenTrajectory;)

DEFINE_EVENT(OnLevelReset, int32_t levelId;)

DEFINE_EVENT(OnGetLevelSpecificFlag, int32_t flagId; int32_t result;)

DEFINE_EVENT(OnCheckSpiralMountainAbilities, int32_t result;)
