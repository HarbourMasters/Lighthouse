#ifndef CUSTOM_OBJECT_H
#define CUSTOM_OBJECT_H

#include <stdint.h>
#include <vector>
#include <map>
#include "port/Rando/Types.h"

#include "prop.h"

extern std::map<actor_e, std::pair<ActorInfo, int32_t>> actorInfoMap;

extern std::vector<actor_e> junkItemList;

class CustomObject {
public:
    static void ResetRandoSpawnQueue();
    static void ClearRandoActorListEX();
    static bool CheckSpawnedIdList(RandoCheckId randoCheckId);
    static void RemoveSpawnedIdFromList(RandoCheckId randoCheckId);
    static Actor* SetCustomActorParametersEX(RandoCheckId randoCheckId, Actor* customActor);
    static Actor* SpawnCustomActorEX(RandoCheckId randoCheckId, int32_t position[3], ActorInfo* actorInfo,
                                     int32_t flags);
    static void FlushRandoSpawnQueue();
    static void AddPropToSpawnQueueEX(int32_t position[3], RandoCheckId randoCheckId);
    static Actor* ShouldCreateCustomActorEX(RandoCheckId randoCheckId, int32_t position[3], bool isProp,
                                            Actor* refActor = nullptr);
    static void ResolveCustomActorCollisionEX(RandoCheckId randoCheckId);
    static void CheckObtainedEX(RandoCheckId randoCheckId, bool isInit = false);
    static void ObjectCollectedEX(Prop* prop);
};

#endif // CUSTOM_OBJECT_H