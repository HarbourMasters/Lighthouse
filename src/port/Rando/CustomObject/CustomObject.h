#ifndef CUSTOM_OBJECT_H
#define CUSTOM_OBJECT_H

#include <stdint.h>
#include <vector>
#include "port/Rando/Types.h"
#include "port/Rando/StaticData/StaticData.h"

#include "prop.h"

typedef struct {
    s16 flags;
    // u8 pad2[0x2];
    s32 actor_id;
    s32 count;
    s16 sfx_id;
    s16 sfx_volume;
    s16 sfx_sampleRate;
    // u8 pad12[0x2];
    f32 velocity_x;
    f32 randomVelocity_x;
    f32 velocity_y;
    f32 randomVelocity_y;
    f32 velocity_z;
    f32 randomVelocity_z;
    f32 bounce_factor;
    f32 yaw;
} BundleInfo;

actor_e GetRandomJunkActorId(RandoSaveCheck shuffledObject);
void UpdateSaveDataNoteScores();
void ApplyBundleActorPhysics(Actor* actor, int32_t bundle_id, BundleInfo* bundle_info, f32 bundleYaw);
void ApplyCustomActorPhysics(RandoCheckId randoCheckId, Actor* actor, bool isJinjoJiggy);

extern std::map<actor_e, std::pair<ActorInfo, int32_t>> actorInfoMap;

extern std::vector<actor_e> junkItemList;
void UpdateJunkList();

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