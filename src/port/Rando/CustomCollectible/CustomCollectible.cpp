#include "CustomCollectible.h"
#include "CustomCollectibleDrawCustom.h"
#include <unordered_map>
#include "port/Rando/Rando.h"
#include <print>

extern "C" {
#include "core1/sns.h"
Actor* actor_new(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags);
Actor* actor_draw(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx);
Actor* fxTouchSparkle_draw(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx);
Actor* marker_getActor(ActorMarker* thisx);
void marker_despawn(ActorMarker* marker);

Actor* func_802D94B4(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx);
void func_8028E964(f32 pos[3]);
int func_80257F18(f32 src[3], f32 target[3], f32* yaw);
bool func_8028AED4(f32 arg0[3], f32 arg1);
void coMusicPlayer_playMusic(enum comusic_e track_id, s32 volume);
void __chJinjo_802CDC9C(Actor* actor, s16 arg1);
void fxSparkle_honeycomb(s16 position[3]);
}

ActorAnimationInfo moleAnimations[] = {
    { 0, 0.0f },
    { ASSET_13A_ANIM_BOTTLES_ENTER, 2000000000.0f },
    { ASSET_13A_ANIM_BOTTLES_ENTER, 4.5f },
    { ASSET_13B_ANIM_BOTTLES_IDLE, 7.0f },
    { ASSET_139_ANIM_BOTTLES_EXIT, 1.7f },
    { ASSET_13A_ANIM_BOTTLES_ENTER, 2000000000.0f },
};

ActorInfo customActorInfo = { MARKER_300_CUSTOM_COLLECTIBLE,
                              ACTOR_3CD_CUSTOM_COLLECTIBLE,
                              ASSET_387_MODEL_BOTTLES,
                              3,
                              NULL,
                              CustomCollectible_Update,
                              NULL,
                              NULL,
                              0,
                              0,
                              0.9f,
                              0 };

void CustomCollectible_Update(Actor* actor) {
    ActorLocal_CustomCollectible* customLocal = (ActorLocal_CustomCollectible*)&actor->local;

    if (!actor->initialized) {
        actor->initialized = true;
        actor->marker->collidable = true;
    }

    RandoItemType itemType = Rando::StaticData::Items[(RandoItemId)customLocal->randoItemId].randoItemType;
    if (itemType == RITYPE_MOLEHILL) {
        CustomCollectible::FacePlayer(actor);
    } else {
        actor->yaw += 5.0f;
    }
}

void CustomCollectible::FacePlayer(Actor* actor) {
    // Copied from source code inside jinjo.c
    f32 sp7C[3];
    f32 sp6C;
    f32* sp30 = actor->position;
    func_8028E964(sp7C);
    func_80257F18(sp30, sp7C, &sp6C);
    s16 sp64 = (actor->yaw * 182.04444);
    s16 sp66 = (s32)(sp6C * 182.04444);
    sp66 = sp64 - sp66;
    s32 sp60 = func_8028AED4(sp30, 55.0f);
    __chJinjo_802CDC9C(actor, sp66);
}

Actor* CustomCollectible::AttachCustomVariables(int32_t randoItemId, Actor* customCollectible) {
    ActorLocal_CustomCollectible* customLocal = (ActorLocal_CustomCollectible*)&customCollectible->local;
    customLocal->randoItemId = randoItemId;

    return customCollectible;
}

Actor* CustomCollectible::Spawn(int32_t position[3], RandoItemId randoItemId) {
    int32_t spawnPosition[3] = { position[0], position[1], position[2] };

    int32_t flags = ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_21;

    // Custom models exported for OoT are centered instead of offset to the marker, so they appear underground otherwise
    actor_e actorId = (actor_e)Rando::StaticData::Items[randoItemId].actorId;
    RandoItemType itemType = Rando::StaticData::Items[randoItemId].randoItemType;
    if (itemType == RITYPE_AP_ITEM) {
        spawnPosition[1] += 50;
    }

    // actor_new stores the ActorInfo* permanently, so it must outlive Spawn()
    static std::unordered_map<RandoItemId, ActorInfo> sActorInfoCache;
    ActorInfo& collectibleInfo = sActorInfoCache[randoItemId];
    collectibleInfo = CustomCollectible::GetActorAndDrawInfo(randoItemId);

    // Spawn actor
    Actor* customCollectible = actor_new(spawnPosition, 0, &collectibleInfo, flags);
    customCollectible->marker->collisionFunc = CustomCollectible::OnCollect;
    customCollectible = CustomCollectible::AttachCustomVariables(randoItemId, customCollectible);

    return customCollectible;
}

ActorInfo CustomCollectible::GetActorAndDrawInfo(RandoItemId randoItemId) {
    ActorInfo collectibleInfo = customActorInfo;

    actor_e actorId = (actor_e)Rando::StaticData::Items[randoItemId].actorId;
    RandoItemType itemType = Rando::StaticData::Items[randoItemId].randoItemType;

    switch (itemType) {
        case RITYPE_MOLEHILL:
            collectibleInfo.draw_func = func_802D94B4;
            collectibleInfo.modelId = ASSET_387_MODEL_BOTTLES;
            collectibleInfo.animations = moleAnimations;
            break;
        case RITYPE_AP_ITEM:
            collectibleInfo.draw_func = CustomCollectible_DrawCustomModel;
            break;
        default:
            break;
    }

    return collectibleInfo;
}

void CustomCollectible::OnCollect(struct actorMarker_s* self, struct actorMarker_s* other) {
    fxSparkle_honeycomb(&self->propPtr->x);
    coMusicPlayer_playMusic(COMUSIC_9_NOTE_COLLECTED, 16000);
    marker_despawn(self);
}
