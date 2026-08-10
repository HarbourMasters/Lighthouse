#pragma once
#include "port/Rando/ItemQueue/ItemQueue.h"
#include "prop.h"

typedef struct {
    RandoCheckId randoCheckId;
    RandoItemId randoItemId;
    actor_e actorId;
    RandoItemType itemType;
} ActorLocal_CustomCollectible;

enum CustomCollectibleDrawTypes {
    CCT_GENERIC_MODEL,
    CCT_GENERIC_SPRITE,
    CCT_JINJO,
    CCT_MOLEHILL,
    CCT_VANILLA_SNS_EGG,
    CCT_CUSTOM_MODEL,
};

typedef struct {
    asset_e drawModel;
    CustomCollectibleDrawTypes drawType;
} CustomCollectibleDrawInfo;

typedef struct {
    s32 position[3];
    RandoCheckId randoCheckId;
} QueuedProp;

void CustomCollectible_Update(Actor* actor);

class CustomCollectible {
private:
    static Actor* AttachCustomVariables(RandoCheckId randoItemId, Actor* customCollectible);
    static void OnCollect(struct actorMarker_s* self, struct actorMarker_s* other);
    static ActorInfo GetActorAndDrawInfo(RandoItemId randoItemId);
    static uint32_t GetSNSEggColor(RandoItemId randoItemId);

public:
    static Actor* Spawn(int32_t position[3], RandoCheckId randoCheckId);
    static f32 GetScale(RandoItemType itemType);
    static void FacePlayer(Actor* actor);
    static Actor* GetActorByRC(RandoCheckId randoCheckId);
    static void QueueProp(int32_t position[3], RandoCheckId randoCheckId);
    static void ProcessPropQueue();
};
