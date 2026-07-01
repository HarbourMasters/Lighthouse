#pragma once

#include <libultraship/bridge/eventsbridge.h>
#include "port/Rando/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "enums.h"
#include "prop.h"

#ifdef __cplusplus
}
#endif

DEFINE_EVENT(InitRandoEvents)

DEFINE_EVENT(OnLoadFileSelect)

DEFINE_EVENT(OnSaveLoad, void* saveData;)

DEFINE_EVENT(OnSaveClear, void* result;)

DEFINE_EVENT(OnWarpDispatch, int32_t warpId; int32_t warpDest;)

DEFINE_EVENT(OnActorSpawn, int32_t actorId; int32_t posX; int32_t posY; int32_t posZ; int32_t rot; Actor * result;)

DEFINE_EVENT(OnLoadActorSaveState, Actor* actor; int32_t posX; int32_t posY; int32_t posZ;)

DEFINE_EVENT(OnSaveActorSaveState, Actor* actor;)

DEFINE_EVENT(OnActorCollision, Prop* propId;)

DEFINE_EVENT(OnFindActorFromActorId, int32_t actorId; Actor * result;)

DEFINE_EVENT(OnFindActorMarkerFromJiggyId, int32_t jiggyId; ActorMarker * result;)

DEFINE_EVENT(OnFindClosestActorFromActorId, int32_t actorId; Actor * result;)

DEFINE_EVENT(OnSetJiggyList, int32_t levelId;)

DEFINE_EVENT(OnIsJiggyScoreCollected, int32_t jiggyId; int32_t result;)

DEFINE_EVENT(OnIsJiggyScoreSpawned, int32_t jiggyId; int32_t result;)

DEFINE_EVENT(SetRandoInfFlag, int32_t flagId; int32_t flagState;)

DEFINE_EVENT(OnIsHoneycombScoreCollected, int32_t honeycombId; int32_t result;)

DEFINE_EVENT(ClearBundleDespawnQueue);

DEFINE_EVENT(OnIsMumboTokenScoreCollected, int32_t tokenId; int32_t result;)

DEFINE_EVENT(OnSnSItemState, int32_t snsItem; int32_t result;)
