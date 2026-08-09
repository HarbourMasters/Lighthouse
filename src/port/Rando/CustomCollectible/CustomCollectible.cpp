#include "CustomCollectible.h"
#include "CustomCollectibleDrawCustom.h"
#include <unordered_map>

#include <libultraship/bridge.h>
#include "port/ShipInit.hpp"
#include <queue>

extern "C" {
#include "functions.h"
#include "actor.h"
void __chJinjo_802CDC9C(Actor* thisx, s16 arg1);
Actor* func_802D94B4(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx);
Actor* chSnsEgg_draw(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx);
}

static std::queue<QueuedProp> propQueue;

std::unordered_map<actor_e, CustomCollectibleDrawInfo> customCollectibleDrawInfo = {
    { ACTOR_52_BLUE_EGG, { ASSET_36D_SPRITE_BLUE_EGG, CCT_GENERIC_SPRITE } },
    { ACTOR_47_EMPTY_HONEYCOMB, { ASSET_361_MODEL_EMPTY_HONEYCOMB, CCT_GENERIC_MODEL } },
    { ACTOR_49_EXTRA_LIFE, { ASSET_36E_MODEL_EXTRA_LIFE, CCT_GENERIC_SPRITE } },
    { ACTOR_50_HONEYCOMB, { ASSET_363_MODEL_HONEYCOMB, CCT_GENERIC_MODEL } },
    { ACTOR_46_JIGGY, { ASSET_35F_MODEL_JIGGY, CCT_GENERIC_MODEL } },
    { ACTOR_60_JINJO_BLUE, { ASSET_3C0_MODEL_JINJO_BLUE, CCT_JINJO } },
    { ACTOR_62_JINJO_GREEN, { ASSET_3C2_MODEL_JINJO_GREEN, CCT_JINJO } },
    { ACTOR_5F_JINJO_ORANGE, { ASSET_3BC_MODEL_JINJO_ORANGE, CCT_JINJO } },
    { ACTOR_61_JINJO_PINK, { ASSET_3C1_MODEL_JINJO_PINK, CCT_JINJO } },
    { ACTOR_5E_JINJO_YELLOW, { ASSET_3BB_MODEL_JINJO_YELLOW, CCT_JINJO } },
    { ACTOR_12C_MOLEHILL, { ASSET_387_MODEL_BOTTLES, CCT_MOLEHILL } },
    { ACTOR_2D_MUMBO_TOKEN, { ASSET_41A_SPRITE_MUMBO_TOKEN, CCT_GENERIC_SPRITE } },
    { ACTOR_51_MUSIC_NOTE, { ASSET_6D6_SPRITE_MUSIC_NOTE, CCT_GENERIC_SPRITE } },
    { ACTOR_25E_SNS_EGG, { ASSET_50D_MODEL_SNS_EGG, CCT_VANILLA_SNS_EGG } },
    { ACTOR_25D_ICE_KEY, { ASSET_50C_MODEL_ICE_KEY, CCT_GENERIC_MODEL } },
    { ACTOR_3CD_CUSTOM_COLLECTIBLE, { ASSET_35F_MODEL_JIGGY, CCT_CUSTOM_MODEL } },
};

ActorAnimationInfo moleAnimation[] = {
    { ASSET_13B_ANIM_BOTTLES_IDLE, 7.0f },
};

ActorAnimationInfo chJinjoAnimation[] = {
    { ASSET_2F_ANIM_JINJO_HELP, 1.5f },
};

ActorInfo customActorInfo = { MARKER_300_CUSTOM_COLLECTIBLE,
                              ACTOR_3CD_CUSTOM_COLLECTIBLE,
                              ASSET_0_NONE,
                              0,
                              NULL,
                              CustomCollectible_Update,
                              NULL,
                              NULL,
                              0,
                              0,
                              0.8f,
                              0 };

void CustomCollectible_Update(Actor* actor) {
    ActorLocal_CustomCollectible* customLocal = (ActorLocal_CustomCollectible*)&actor->local;

    if (!actor->initialized) {
        actor->initialized = true;
        actor->marker->collidable = true;

        actor->scale = CustomCollectible::GetScale(customLocal->itemType);
    }

    if (customLocal->itemType == RITYPE_MOLEHILL || customLocal->itemType == RITYPE_JINJO) {
        CustomCollectible::FacePlayer(actor);
    } else {
        actor->yaw += 5.0f;
    }

    // Sparkles
    if (customCollectibleDrawInfo[customLocal->actorId].drawType != CCT_GENERIC_SPRITE) {
        for (int i = 0; i < 4; i++) {
            if (randf() < 0.03) {
                commonParticle_add(actor->marker, i + 5, func_80329904);
                commonParticle_new((common_particle_e)8, actor->marker->unk14_21);
            }
        }
    }

    // Allows shadows
    if (customLocal->itemType == RITYPE_AP_ITEM) {
        actor->unk124_6 = 1;
        actor->marker->unk14_21 = 1;
    }
}

f32 CustomCollectible::GetScale(RandoItemType itemType) {
    switch (itemType) {
        case RITYPE_MUSIC_NOTE:
        case RITYPE_SNS_EGG:
            return 0.42857143f;
        case RITYPE_EMPTY_HONEYCOMB:
        case RITYPE_MOLEHILL:
            return 0.8f;
        default:
            return 1.0f;
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

Actor* CustomCollectible::AttachCustomVariables(RandoCheckId randoCheckId, Actor* customCollectible) {
    ActorLocal_CustomCollectible* customLocal = (ActorLocal_CustomCollectible*)&customCollectible->local;

    RandoItemId randoItemId = RANDO_SAVE_CHECKS[randoCheckId].randoItemId;

    customLocal->randoCheckId = randoCheckId;
    customLocal->randoItemId = randoItemId;
    customLocal->actorId = (actor_e)Rando::StaticData::Items[randoItemId].actorId;
    customLocal->itemType = Rando::StaticData::Items[randoItemId].randoItemType;

    return customCollectible;
}

Actor* CustomCollectible::Spawn(int32_t position[3], RandoCheckId randoCheckId) {
    int32_t spawnPosition[3] = { position[0], position[1], position[2] };

    int32_t flags = ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_21;

    RandoItemId randoItemId = RANDO_SAVE_CHECKS[randoCheckId].randoItemId;
    RandoItemType itemType = Rando::StaticData::Items[randoItemId].randoItemType;

    // actor_new stores the ActorInfo* permanently, so it must outlive Spawn()
    static std::unordered_map<RandoItemId, ActorInfo> sActorInfoCache;
    ActorInfo& collectibleInfo = sActorInfoCache[randoItemId];
    collectibleInfo = CustomCollectible::GetActorAndDrawInfo(randoItemId);

    // Spawn actor
    Actor* customCollectible = actor_new(spawnPosition, 0, &collectibleInfo, flags);
    if (itemType == RITYPE_SNS_EGG) {
        customCollectible->actorTypeSpecificField = CustomCollectible::GetSNSEggColor(randoItemId);
    }
    customCollectible->marker->collisionFunc = CustomCollectible::OnCollect;
    customCollectible = CustomCollectible::AttachCustomVariables(randoCheckId, customCollectible);

    return customCollectible;
}

uint32_t CustomCollectible::GetSNSEggColor(RandoItemId randoItemId) {
    switch (randoItemId) {
        case RI_STOP_N_SWOP_EGG_BLUE:
            return (uint32_t)SNS_ITEM_EGG_BLUE;
        case RI_STOP_N_SWOP_EGG_CYAN:
            return (uint32_t)SNS_ITEM_EGG_CYAN;
        case RI_STOP_N_SWOP_EGG_GREEN:
            return (uint32_t)SNS_ITEM_EGG_GREEN;
        case RI_STOP_N_SWOP_EGG_PINK:
            return (uint32_t)SNS_ITEM_EGG_PINK;
        case RI_STOP_N_SWOP_EGG_RED:
            return (uint32_t)SNS_ITEM_EGG_RED;
        case RI_STOP_N_SWOP_EGG_YELLOW:
            return (uint32_t)SNS_ITEM_EGG_YELLOW;
        default:
            return (uint32_t)SNS_ITEM_EGG_CYAN;
    }
}

ActorInfo CustomCollectible::GetActorAndDrawInfo(RandoItemId randoItemId) {
    ActorInfo collectibleInfo = customActorInfo;

    actor_e actorId = (actor_e)Rando::StaticData::Items[randoItemId].actorId;
    auto drawInfo = customCollectibleDrawInfo[actorId];

    collectibleInfo.modelId = drawInfo.drawModel;

    switch (drawInfo.drawType) {
        case CCT_GENERIC_MODEL:
            collectibleInfo.draw_func = actor_draw;
            break;
        case CCT_GENERIC_SPRITE:
            collectibleInfo.draw_func = fxTouchSparkle_draw;
            collectibleInfo.shadow_scale = 0.0f;
            break;
        case CCT_JINJO:
            collectibleInfo.draw_func = actor_draw;
            collectibleInfo.animations = chJinjoAnimation;
            break;
        case CCT_MOLEHILL:
            collectibleInfo.draw_func = func_802D94B4;
            collectibleInfo.animations = moleAnimation;
            break;
        case CCT_VANILLA_SNS_EGG:
            collectibleInfo.draw_func = chSnsEgg_draw;
            break;
        case CCT_CUSTOM_MODEL:
            collectibleInfo.draw_func = CustomCollectible_DrawCustomModel;
            break;
        default:
            break;
    }
    return collectibleInfo;
}

void CustomCollectible::OnCollect(struct actorMarker_s* self, struct actorMarker_s* other) {
    Actor* actor = marker_getActor(self);
    ActorLocal_CustomCollectible* customLocal = (ActorLocal_CustomCollectible*)&actor->local;
    fxSparkle_honeycomb(&self->propPtr->x);
    ItemQueue::AddCheck(customLocal->randoCheckId);
    marker_despawn(self);
}

// When regular props are spawned in BK, the game isn't fully set up yet to spawn
// actors. So we delay prop spawns until the game's ready to spawn actors.
void CustomCollectible::QueueProp(int32_t position[3], RandoCheckId randoCheckId) {
    QueuedProp queuedProp;
    queuedProp.position[0] = position[0];
    queuedProp.position[1] = position[1];
    queuedProp.position[2] = position[2];
    queuedProp.randoCheckId = randoCheckId;
    propQueue.push(queuedProp);
}

void CustomCollectible::ProcessPropQueue() {
    while (propQueue.size() > 0) {
        QueuedProp prop = propQueue.front();
        Actor* actor = CustomCollectible::Spawn(prop.position, prop.randoCheckId);
        propQueue.pop();
    }
}

void RegisterCustomCollectible() {
    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, IS_RANDO,
              [](IEvent* event) { CustomCollectible::ProcessPropQueue(); });
}

static RegisterShipInitFunc initFunc(RegisterCustomCollectible, { "IS_RANDO" });
