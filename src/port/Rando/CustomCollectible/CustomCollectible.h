#pragma once
#include "prop.h"
#include "port/Rando/Types.h"

typedef struct {
    uint16_t randoItemId;
} ActorLocal_CustomCollectible;

void CustomCollectible_Update(Actor* actor);

class CustomCollectible {
private:
    static Actor* AttachCustomVariables(int32_t randoItemId, Actor* customCollectible);
    static void OnCollect(struct actorMarker_s* self, struct actorMarker_s* other);
    static ActorInfo GetActorAndDrawInfo(RandoItemId randoItemId);

public:
    static Actor* Spawn(int32_t position[3], RandoItemId randoItemId);
    static void FacePlayer(Actor* actor);
};
