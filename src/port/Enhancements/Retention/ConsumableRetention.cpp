// Consumable Collection Retention
#include <libultraship/bridge.h>
#include <libultraship/bridge/consolevariablebridge.h>
#include <spdlog/spdlog.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include <algorithm>

#include "enums.h"
#include "actor.h"
#include "prop.h"
#include "functions.h"

extern "C" {
extern f32 gBundle_yaw;
}

#define CVAR_CONSUMABLE_RETENTION CVAR_ENHANCEMENT("Gameplay.ConsumableRetention")
#define CVAR_VALUE CVarGetInteger(CVAR_CONSUMABLE_RETENTION, 0)

using retention::systemActive;

// Identity for hut drops. A sub-area restore copies the actor but assigns a new marker, so it lives in
// actor->local; magic tells ours from leftover data.
constexpr uint32_t kDropLocalMagic = 0x434F4E53u; // 'CONS'
struct DropLocal {
    uint32_t magic;
    ConsumablePickup pickup;
};

static ConsumableRetentionSaveData* store() {
    ShipSaveData* data = retention::activeShipData();
    return data != nullptr ? &data->consumableRetention : nullptr;
}

static ConsumablePickup pickupAt(int32_t x, int32_t y, int32_t z, int32_t drop = 0) {
    return { (u8)gsworld_getMap(), (u8)drop, (s16)x, (s16)y, (s16)z };
}

static bool isCollected(const ConsumablePickup& pickup) {
    ConsumableRetentionSaveData* s = store();
    return s != nullptr && std::any_of(s->collected, s->collected + s->count, [&](const ConsumablePickup& c) {
               return c.map == pickup.map && c.drop == pickup.drop && c.x == pickup.x && c.y == pickup.y &&
                      c.z == pickup.z;
           });
}

static void setCollected(const ConsumablePickup& pickup) {
    ConsumableRetentionSaveData* s = store();
    if (s == nullptr || isCollected(pickup)) {
        return;
    }
    if (s->count == CONSUMABLE_RETENTION_CAPACITY) {
        static bool warned = false;
        if (!warned) {
            warned = true;
            SPDLOG_WARN("[ConsumableRetention] All {} slots are in use; further pickups won't be kept",
                        CONSUMABLE_RETENTION_CAPACITY);
        }
        return;
    }
    s->collected[s->count++] = pickup;
}

static bool isConsumableSprite(s32 asset) {
    return asset == ASSET_6D7_SPRITE_BLUE_EGGS || asset == ASSET_580_SPRITE_RED_FEATHER ||
           asset == ASSET_6D1_SPRITE_GOLDFEATHER;
}

static bool isConsumableActor(s32 actorId) {
    return actorId == ACTOR_52_BLUE_EGG || actorId == ACTOR_129_RED_FEATHER || actorId == ACTOR_370_GOLD_FEATHER ||
           actorId == ACTOR_49_EXTRA_LIFE;
}

static void RegisterConsumableRetention_Init() {
    COND_VB_SHOULD(VB_OVERRIDE_PROP_SPAWN, EVENT_PRIORITY_NORMAL, CVAR_VALUE, {
        s16* position = va_arg(args, s16*);
        s32 spriteAsset = va_arg(args, s32);
        if (systemActive() && isConsumableSprite(spriteAsset) &&
            isCollected(pickupAt(position[0], position[1], position[2]))) {
            *should = true;
        }
    });

    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, CVAR_VALUE, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;
        if (!systemActive() || !isConsumableActor(ev->actorId) ||
            !isCollected(pickupAt(ev->posX, ev->posY, ev->posZ))) {
            return;
        }
        ev->result = nullptr;
        event->Cancelled = true;
    });

    COND_VB_SHOULD(VB_OVERRIDE_BUNDLE_SPAWN, EVENT_PRIORITY_NORMAL, true, {
        bundle_e bundleId = (bundle_e)va_arg(args, int);
        BundleInfo* bundleInfo = va_arg(args, BundleInfo*);
        s32 index = va_arg(args, s32);
        f32* position = va_arg(args, f32*);
        Actor** actorOut = va_arg(args, Actor**);

        if (!systemActive() || (bundleId != BUNDLE_1_MM_HUT_BLUE_EGG && bundleId != BUNDLE_6_MM_HUT_EXTRA_LIFE)) {
            return;
        }
        *should = true;

        const ConsumablePickup pickup = pickupAt(position[0], position[1], position[2], index + 1);
        if (CVAR_VALUE && isCollected(pickup)) {
            return;
        }
        Actor* actor = actor_spawnWithYaw_f32((actor_e)bundleInfo->actor_id, position, 0);
        if (actor == nullptr) {
            return;
        }
        ApplyBundleActorPhysics(actor, (int32_t)bundleId, bundleInfo, gBundle_yaw);
        DropLocal* local = reinterpret_cast<DropLocal*>(actor->local);
        local->magic = kDropLocalMagic;
        local->pickup = pickup;
        *actorOut = actor;
    });
}

static RegisterShipInitFunc initConsumableRetention(RegisterConsumableRetention_Init, { CVAR_CONSUMABLE_RETENTION });

static void RegisterConsumableRecording_Init() {
    REGISTER_LISTENER(OnConsumableCollected, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        Prop* prop = (Prop*)((OnConsumableCollected*)event)->prop;
        if (!systemActive() || prop == nullptr) {
            return;
        }
        if (prop->isActorProp) {
            Actor* actor = marker_getActor(prop->actorProp.marker);
            if (actor->is_bundle) {
                DropLocal* local = reinterpret_cast<DropLocal*>(actor->local);
                if (local->magic == kDropLocalMagic) {
                    local->magic = 0; // consumed
                    setCollected(local->pickup);
                }
                return;
            }
        }
        setCollected(pickupAt(prop->position_x, prop->position_y, prop->position_z));
    });
}

static RegisterShipInitFunc initConsumableRecording(RegisterConsumableRecording_Init);
