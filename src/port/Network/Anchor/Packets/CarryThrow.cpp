#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "port/Patches/Patches.h"

#include "functions.h"
extern "C" {
void __chLevelCollectible_presentReturnEmitSparkles(f32 position[3], enum asset_e sprite_id);
}

/**
 * CARRY_THROW
 */

// Teammates' carried objects, shown here as display copies on their dummies.
constexpr s32 kRemoteCarryMax = 16;
static struct {
    u32 clientId;        // 0 = free slot
    ActorMarker* marker; // NULL while the spawn is still queued
    s32 markerId;
    u8 throwPending;
    f32 throwStart[3];
    f32 throwTarget[3];
    f32 carryOff[3]; // held pose: object position relative to the owner
    f32 carryYawOff; // held pose: object yaw relative to the owner's yaw
} sRemoteCarry[kRemoteCarryMax];

static s32 getDummyTransform(u32 clientId, f32 pos[3], f32* yaw) {
    Anchor* anchor = Anchor::GetInstance();
    if (anchor == nullptr || !anchor->isConnected) {
        return 0;
    }
    auto it = anchor->clients.find(clientId);
    if (it == anchor->clients.end() || !it->second.online || it->second.dummy == nullptr ||
        (s32)it->second.map != (s32)gsworld_getMap()) {
        return 0;
    }
    it->second.dummy->dummy_getPosition(pos);
    *yaw = it->second.dummy->dummy_getYaw();
    return 1;
}

static s32 actorIdForMarker(s32 markerId) {
    switch (markerId) {
        case MARKER_36_ORANGE_COLLECTIBLE:
            return ACTOR_29_ORANGE_COLLECTIBLE;
        case MARKER_37_GOLD_BULLION:
            return ACTOR_2A_GOLD_BULLION;
        case MARKER_1FD_BLUE_PRESENT_COLLECTIBLE:
            return ACTOR_1ED_BLUE_PRESENT_COLLECTIBLE;
        case MARKER_1FE_GREEN_PRESENT_COLLECTIBLE:
            return ACTOR_1EF_GREEN_PRESENT_COLLECTIBLE;
        case MARKER_1FF_RED_PRESENT_COLLECTIBLE:
            return ACTOR_1F1_RED_PRESENT_COLLECTIBLE;
        case MARKER_1B5_CATERPILLAR:
            return ACTOR_2A2_CATERPILLAR;
        case MARKER_1BC_CARRIED_ACORN:
            return ACTOR_2A9_ACORN;
        default:
            return 0;
    }
}

static s32 findByClient(u32 clientId) {
    for (s32 i = 0; i < kRemoteCarryMax; i++) {
        if (sRemoteCarry[i].clientId == clientId) {
            return i;
        }
    }
    return -1;
}

static s32 findByMarker(ActorMarker* marker) {
    for (s32 i = 0; i < kRemoteCarryMax; i++) {
        if (sRemoteCarry[i].clientId != 0 && sRemoteCarry[i].marker == marker) {
            return i;
        }
    }
    return -1;
}

static void clearSlot(s32 slot) {
    sRemoteCarry[slot].clientId = 0;
    sRemoteCarry[slot].marker = NULL;
    sRemoteCarry[slot].markerId = 0;
    sRemoteCarry[slot].throwPending = 0;
    sRemoteCarry[slot].carryOff[0] = sRemoteCarry[slot].carryOff[1] = sRemoteCarry[slot].carryOff[2] = 0.0f;
    sRemoteCarry[slot].carryYawOff = 0.0f;
}

static void setPose(s32 slot, f32 offset[3], f32 yawOffset) {
    sRemoteCarry[slot].carryOff[0] = offset[0];
    sRemoteCarry[slot].carryOff[1] = offset[1];
    sRemoteCarry[slot].carryOff[2] = offset[2];
    sRemoteCarry[slot].carryYawOff = yawOffset;
}

// Mirror of func_802D7DE8's arc math without its side effects. State 7 = remote flight.
static void launch(Actor* actor, f32 start[3], f32 target[3]) {
    s32 tgt[3];
    f32 var_f12;
    f32 var_f14;
    f32 var_f18;

    actor->position[0] = start[0];
    actor->position[1] = start[1];
    actor->position[2] = start[2];
    ml_vec3f_to_vec3w(tgt, target);
    actor->unk1C[0] = target[0];
    actor->unk1C[1] = target[1];
    actor->unk1C[2] = target[2];
    actor->velocity[0] = (f32)tgt[0] - actor->position[0];
    actor->velocity[1] = 28.0f;
    actor->velocity[2] = (f32)tgt[2] - actor->position[2];
    var_f12 = actor->position[1];
    var_f14 = 28.0f;
    var_f18 = 0.0f;
    while (!(var_f12 < tgt[1]) || !(var_f14 < 0.0f)) {
        var_f18 += 1.0f;
        var_f12 += (var_f14 -= 5.0);
    }
    actor->velocity[0] /= var_f18;
    actor->velocity[2] /= var_f18;
    actor->unk38_31 = (u32)var_f18;
    subaddie_set_state(actor, 7);
}

static void spawnMethod(uintptr_t markerId, uintptr_t clientId) {
    f32 pos[3];
    f32 yaw = 0.0f;
    Actor* actor;
    s32 slot = findByClient((u32)clientId);
    s32 actorId = actorIdForMarker((s32)markerId);

    if (slot < 0 || sRemoteCarry[slot].markerId != (s32)markerId || actorId == 0) {
        return; // stale queue entry
    }
    if (!getDummyTransform((u32)clientId, pos, &yaw)) {
        clearSlot(slot);
        return;
    }
    actor = actor_spawnWithYaw_f32((enum actor_e)actorId, pos, (s32)yaw);
    // Display copy: skip init, no collision with the local player.
    actor->volatile_initialized = true;
    actor->marker->propPtr->unk8_3 = false;
    if (sRemoteCarry[slot].throwPending) {
        launch(actor, sRemoteCarry[slot].throwStart, sRemoteCarry[slot].throwTarget);
        clearSlot(slot);
    } else {
        sRemoteCarry[slot].marker = actor->marker;
        subaddie_set_state(actor, 6);
    }
}

extern "C" void port_remoteCarry_setCarried(u32 clientId, s32 markerId, f32 offset[3], f32 yawOffset) {
    s32 slot;
    if (clientId == 0) {
        return;
    }
    if (markerId != 0 && actorIdForMarker(markerId) == 0) {
        markerId = 0; // not a synced carryable
    }
    slot = findByClient(clientId);
    if (markerId == 0) {
        if (slot >= 0 && !sRemoteCarry[slot].throwPending) {
            if (sRemoteCarry[slot].marker != NULL) {
                marker_despawn(sRemoteCarry[slot].marker);
            }
            clearSlot(slot);
        }
        return;
    }
    if (slot >= 0) {
        if (sRemoteCarry[slot].markerId == markerId) {
            setPose(slot, offset, yawOffset);
            return;
        }
        if (sRemoteCarry[slot].marker != NULL) {
            marker_despawn(sRemoteCarry[slot].marker);
        }
        clearSlot(slot);
    }
    slot = findByClient(0);
    if (slot < 0) {
        return; // registry full
    }
    sRemoteCarry[slot].clientId = clientId;
    sRemoteCarry[slot].markerId = markerId;
    sRemoteCarry[slot].marker = NULL;
    sRemoteCarry[slot].throwPending = 0;
    setPose(slot, offset, yawOffset);
    __spawnQueue_add_2(spawnMethod, markerId, clientId);
}

static void remoteThrow(u32 clientId, s32 markerId, f32 start[3], f32 target[3]) {
    s32 slot;
    if (clientId == 0 || actorIdForMarker(markerId) == 0) {
        return;
    }
    slot = findByClient(clientId);
    if (slot >= 0 && sRemoteCarry[slot].marker != NULL) {
        Actor* actor = marker_getActor(sRemoteCarry[slot].marker);
        launch(actor, start, target);
        clearSlot(slot);
        return;
    }
    if (slot < 0) {
        slot = findByClient(0);
        if (slot < 0) {
            return;
        }
        sRemoteCarry[slot].clientId = clientId;
        __spawnQueue_add_2(spawnMethod, markerId, clientId);
    }
    sRemoteCarry[slot].markerId = markerId;
    sRemoteCarry[slot].throwPending = 1;
    sRemoteCarry[slot].throwStart[0] = start[0];
    sRemoteCarry[slot].throwStart[1] = start[1];
    sRemoteCarry[slot].throwStart[2] = start[2];
    sRemoteCarry[slot].throwTarget[0] = target[0];
    sRemoteCarry[slot].throwTarget[1] = target[1];
    sRemoteCarry[slot].throwTarget[2] = target[2];
}

static bool displayUpdate(Actor* actor) {
    s32 slot = findByMarker(actor->marker);

    if (actor->state == 6 || slot >= 0) { // carried: glued to the owner's dummy
        f32 pos[3];
        f32 yaw = 0.0f;
        if (slot < 0 || !getDummyTransform(sRemoteCarry[slot].clientId, pos, &yaw)) {
            if (slot >= 0) {
                clearSlot(slot);
            }
            marker_despawn(actor->marker);
            return true;
        }
        actor->position[0] = pos[0] + sRemoteCarry[slot].carryOff[0];
        actor->position[1] = pos[1] + sRemoteCarry[slot].carryOff[1];
        actor->position[2] = pos[2] + sRemoteCarry[slot].carryOff[2];
        actor->yaw = mlNormalizeAngle(yaw + sRemoteCarry[slot].carryYawOff);
        return true;
    }

    if (actor->state == 7) { // thrown: replay the arc, land with feedback only
        f32 landY;
        actor->position[0] += actor->velocity[0];
        actor->position[1] += (actor->velocity[1] -= 5.0);
        actor->position[2] += actor->velocity[2];
        landY = (--actor->unk38_31 < 4) ? actor->unk1C[1] : actor->position[1];
        if (actor->position[1] < landY) {
            if (actor->modelCacheIndex == ACTOR_2A2_CATERPILLAR || actor->modelCacheIndex == ACTOR_2A9_ACORN) {
                if (Anchor_InEarshot(actor->position)) {
                    coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 28000);
                }
                marker_despawn(actor->marker);
                return true;
            }
            if (actor->modelCacheIndex == ACTOR_2A_GOLD_BULLION && Anchor_InEarshot(actor->position)) {
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 32000);
            }
            if (actor->marker->id != MARKER_36_ORANGE_COLLECTIBLE) {
                sfx_playFadeShorthandDefault(SFX_21_EGG_BOUNCE_1, 0.76f, 25000, actor->position, 1000, 2000);
            } else {
                sfx_playFadeShorthandDefault(SFX_B3_ORANGE_TALKING, 1.0f, 25000, actor->position, 1000, 2000);
            }
            // State 8: landed display copy.
            actor->position[1] = landY;
            subaddie_set_state(actor, 8);
            return true;
        }
        switch (actor->marker->id) {
            case MARKER_1FD_BLUE_PRESENT_COLLECTIBLE:
                __chLevelCollectible_presentReturnEmitSparkles(actor->position, ASSET_711_SPRITE_SPARKLE_DARK_BLUE);
                break;
            case MARKER_1FE_GREEN_PRESENT_COLLECTIBLE:
                __chLevelCollectible_presentReturnEmitSparkles(actor->position, ASSET_712_SPRITE_SPARKLE_GREEN);
                break;
            case MARKER_1FF_RED_PRESENT_COLLECTIBLE:
                __chLevelCollectible_presentReturnEmitSparkles(actor->position, ASSET_715_SPRITE_SPARKLE_RED);
                break;
        }
        return true;
    }

    // State 8 (landed display copy) falls through to the vanilla lifecycle.

    return false;
}

static void RemoteCarryUpdate(IEvent* event) {
    auto* ev = (ShouldActorUpdate*)event;
    if (displayUpdate(ev->actor)) {
        *ev->should = false;
    }
}

void Anchor::SendPacket_CarryThrow(s32 markerId, f32 start[3], f32 target[3]) {
    if (!IsSaveLoaded()) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = CARRY_THROW;
    payload["marker"] = markerId;
    payload["start"] = { start[0], start[1], start[2] };
    payload["target"] = { target[0], target[1], target[2] };

    SendToCurrentMapPlayers(payload);
}

void Anchor::HandlePacket_CarryThrow(nlohmann::json& payload) {
    if (!IsSaveLoaded()) {
        return;
    }

    uint32_t clientId = payload.value("clientId", 0u);
    s32 markerId = payload.value("marker", 0);
    std::vector<f32> start = payload["start"].get<std::vector<f32>>();
    std::vector<f32> target = payload["target"].get<std::vector<f32>>();
    if (start.size() < 3 || target.size() < 3) {
        return;
    }

    remoteThrow(clientId, markerId, start.data(), target.data());
}

void RegisterCarryThrow_Init() {
    // Always on: a throw sends only while connected, and the display copies clear with the actor array either way.
    COND_HOOK(OnCarryThrow, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnCarryThrow*)event;
        Anchor* anchor = Anchor::GetInstance();
        if (anchor == nullptr || !anchor->isConnected) {
            return;
        }
        anchor->SendPacket_CarryThrow(ev->markerId, ev->start, ev->target);
    });

    COND_HOOK(OnActorArrayFree, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        for (s32 i = 0; i < kRemoteCarryMax; i++) {
            clearSlot(i);
        }
    });

    // Always on: a display copy a disconnect leaves behind still despawns.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_29_ORANGE_COLLECTIBLE, EVENT_PRIORITY_NORMAL, true, RemoteCarryUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_2A_GOLD_BULLION, EVENT_PRIORITY_NORMAL, true, RemoteCarryUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1ED_BLUE_PRESENT_COLLECTIBLE, EVENT_PRIORITY_NORMAL, true, RemoteCarryUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1EF_GREEN_PRESENT_COLLECTIBLE, EVENT_PRIORITY_NORMAL, true,
                 RemoteCarryUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1F1_RED_PRESENT_COLLECTIBLE, EVENT_PRIORITY_NORMAL, true, RemoteCarryUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_2A2_CATERPILLAR, EVENT_PRIORITY_NORMAL, true, RemoteCarryUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_2A9_ACORN, EVENT_PRIORITY_NORMAL, true, RemoteCarryUpdate);
}

static RegisterShipInitFunc initCarryThrow(RegisterCarryThrow_Init, {});
