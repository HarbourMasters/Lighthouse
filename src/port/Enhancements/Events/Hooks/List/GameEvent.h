#pragma once

#include <libultraship/bridge/eventsbridge.h>

DEFINE_EVENT(OnActorDestroy, Actor* actor;)
DEFINE_EVENT(OnGameSave, int32_t fileNum;)
DEFINE_EVENT(OnGameLoad, int32_t fileNum;)
DEFINE_EVENT(OnPropInit, Prop* propPtr;)
DEFINE_EVENT(OnBottlesBonusComplete, int32_t index;)
DEFINE_EVENT(OnSaveFileLoad, int32_t fileNum; void* saveBuffer; int32_t result;)
DEFINE_EVENT(OnSaveFileSave, void* saveBuffer; int32_t fileNum; int32_t * result;)
// Identifies which warp_* dispatcher is firing OnWarpResolveDest. Keep values
// stable so listener case statements keep matching across refactors.
typedef enum WarpId {
    WARP_ID_SM_EXIT_BANJOS_HOUSE = 1,
    WARP_ID_LAIR_ENTER_MM_LOBBY_FROM_SM_LEVEL = 2,
} WarpId;

DEFINE_EVENT(OnWarpResolveDest, int32_t warpId; int32_t defaultDest; int32_t bkcfOverride; int32_t * dest;)
DEFINE_EVENT(OnNewGame, int32_t* skipIntro;)
DEFINE_EVENT(EggHeadSpawn, float* pitch; float* spawnHeight; float* minVerticalVelocity; float* yawBias;
             int32_t * flattenTrajectory;)

DEFINE_EVENT(OnGetLevelSpecificFlag, int32_t flagId; int32_t result;)

DEFINE_EVENT(OnCheckSpiralMountainAbilities, int32_t result;)
