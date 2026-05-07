#ifndef CUSTOM_OBJECT_H
#define CUSTOM_OBJECT_H

#include <stdint.h>
#include "port/Rando/Types.h"
#include "port/Rando/StaticData/StaticData.h"

#include "prop.h"

typedef struct {
    RandoCheckId randoCheckId;
    int32_t location[3];
} CustomActor;

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

CustomActor CreateCustomActor(RandoCheckId randoCheckId, int32_t position[3]);
actor_e GetActorIdByShuffledObjectState(Rando::StaticData::RandoShuffledPool shuffledObject);
void ApplyBundleActorPhysics(Actor* actor, int32_t bundle_id, BundleInfo* bundle_info, f32 bundleYaw);
void ApplyCustomActorPhysics(RandoCheckId randoCheckId, Actor* actor, bool isJinjoJiggy);

void UpdateJunkList();

class CustomObject {
public:
    static bool CheckSpawnQueue(RandoCheckId randoCheckId);
    static Actor* SetCustomActorParameters(Actor* actor, RandoCheckId randoCheckId);
    static Actor* SpawnCustomActor(actor_e actorId, int32_t position[3]);
    static Actor* GetCustomActor(RandoCheckId randoCheckId);
    static void SpawnJinjoJiggy(int16_t levelId, int16_t position[3]);
    static void AddToCustomActorMap(RandoCheckId randoCheckId, Actor* actor);
    static void AddToSpawnQueue(RandoCheckId randoCheckId, int32_t position[3]);
    static void InitializeSpawnQueue();
    static void ResolveCustomActorCollision(RandoCheckId randoCheckId, Actor* customActor);
    static void CheckObtained(RandoCheckId randoCheckId);
    static void ObjectCollected(Prop* prop);
};

#endif // CUSTOM_OBJECT_H