// Carried-Collectible Live-Despawn Sync (caterpillars, acorns, gold, presents and the orange).
//
// Object identity is a hash of its fixed spawn position, keyed with (slot, mapId, hash).
//
#include <libultraship/bridge.h>
#include "port/ObjectExtension/ObjectExtension.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"
#include "port/Network/Anchor/Anchor.h"

#include <array>
#include <set>

#include "functions.h"
extern "C" {
#include "enums.h"
#include "actor.h"
ActorMarker* bacarry_getMarkerWithExtraSteps(void);
}

namespace {

// Per-object identity. Parenthesized ctor avoids brace-init commas breaking event macros.
struct CarriedSpawnData {
    int32_t mapId;
    int32_t hash;
    CarriedSpawnData(int32_t m = 0, int32_t h = 0) : mapId(m), hash(h) {
    }
};
ObjectExtension::Register<CarriedSpawnData> CarriedSpawnDataRegister;

// (slot, mapId, hash) of every carried collectible the team has picked up this session.
std::set<std::array<int32_t, 3>> sCollected;

int32_t slotForKind(int32_t kind) {
    switch (kind) {
        case ANCHOR_COLLECTIBLE_CATERPILLAR:
            return 0;
        case ANCHOR_COLLECTIBLE_ACORN:
            return 1;
        case ANCHOR_COLLECTIBLE_PRESENT_BLUE:
            return 2;
        case ANCHOR_COLLECTIBLE_PRESENT_GREEN:
            return 3;
        case ANCHOR_COLLECTIBLE_PRESENT_RED:
            return 4;
        case ANCHOR_COLLECTIBLE_GOLD:
            return 5;
        case ANCHOR_COLLECTIBLE_ORANGE:
            return 6;
        default:
            return -1;
    }
}

int32_t kindForMarker(int32_t markerId) {
    switch (markerId) {
        case MARKER_1B5_CATERPILLAR:
            return ANCHOR_COLLECTIBLE_CATERPILLAR;
        case MARKER_1BC_CARRIED_ACORN:
            return ANCHOR_COLLECTIBLE_ACORN;
        case MARKER_1FD_BLUE_PRESENT_COLLECTIBLE:
            return ANCHOR_COLLECTIBLE_PRESENT_BLUE;
        case MARKER_1FE_GREEN_PRESENT_COLLECTIBLE:
            return ANCHOR_COLLECTIBLE_PRESENT_GREEN;
        case MARKER_1FF_RED_PRESENT_COLLECTIBLE:
            return ANCHOR_COLLECTIBLE_PRESENT_RED;
        case MARKER_37_GOLD_BULLION:
            return ANCHOR_COLLECTIBLE_GOLD;
        case MARKER_36_ORANGE_COLLECTIBLE:
            return ANCHOR_COLLECTIBLE_ORANGE;
        default:
            return -1;
    }
}

// Identity from spawn position; masked non-negative to avoid the -1 spend sentinel.
int32_t spawnHash(int32_t x, int32_t y, int32_t z) {
    uint32_t h = (uint32_t)x * 73856093u ^ (uint32_t)y * 19349663u ^ (uint32_t)z * 83492791u;
    return (int32_t)(h & 0x7FFFFFFFu);
}

bool isCollected(int32_t slot, int32_t mapId, int32_t hash) {
    return sCollected.count({ slot, mapId, hash }) != 0;
}

// Gold and the presents feed a credit-back reconcile that rebuilds carried counts as
// collected-minus-delivered. Respawn suppression and that credit must fire together
// or a collected item is lost across a reload, so both are scoped to the consumer's
// level; outside it, neither runs and the collectible simply respawns, as on hardware.
bool kindAllowedFor(int32_t kind, int32_t mapId) {
    switch (kind) {
        case ANCHOR_COLLECTIBLE_GOLD:
            return (int32_t)map_getLevel((enum map_e)mapId) == LEVEL_2_TREASURE_TROVE_COVE;
        case ANCHOR_COLLECTIBLE_PRESENT_BLUE:
        case ANCHOR_COLLECTIBLE_PRESENT_GREEN:
        case ANCHOR_COLLECTIBLE_PRESENT_RED:
            return (int32_t)map_getLevel((enum map_e)mapId) == LEVEL_5_FREEZEEZY_PEAK;
        default:
            return true;
    }
}

bool inDemoPlayback() {
    return func_802E4A08() != 0;
}

// True if a teammate already collected it.
bool registerSpawn(int32_t kind, ActorMarker* marker, int32_t x, int32_t y, int32_t z) {
    int32_t slot = slotForKind(kind);
    if (slot < 0 || marker == nullptr) {
        return false;
    }
    int32_t mapId = (int32_t)gsworld_getMap();
    int32_t hash = spawnHash(x, y, z);
    ObjectExtension::GetInstance().Set<CarriedSpawnData>(marker, CarriedSpawnData(mapId, hash));
    return Anchor_IsWorldSyncActive() && !inDemoPlayback() && kindAllowedFor(kind, mapId) &&
           isCollected(slot, mapId, hash);
}

bool collectedRemotely(int32_t kind, ActorMarker* marker) {
    int32_t slot = slotForKind(kind);
    if (slot < 0 || !Anchor_IsWorldSyncActive()) {
        return false;
    }
    CarriedSpawnData* d = ObjectExtension::GetInstance().Get<CarriedSpawnData>(marker);
    if (d == nullptr) {
        return false;
    }
    return isCollected(slot, d->mapId, d->hash);
}

// A world object setting up this update, not one pulled out of the backpack.
bool setsUp(Actor* actor) {
    switch (actor->modelCacheIndex) {
        case ACTOR_2A9_ACORN: // its first update only marks it initialized
            return actor->volatile_initialized && actor->state == 0 &&
                   bacarry_getMarkerWithExtraSteps() != actor->marker;
        case ACTOR_2A2_CATERPILLAR:
            return !actor->volatile_initialized && bacarry_getMarkerWithExtraSteps() != actor->marker;
        default:
            return !actor->volatile_initialized && !actor->unk138_22;
    }
}

bool liesOut(Actor* actor) {
    switch (actor->modelCacheIndex) {
        case ACTOR_2A9_ACORN:
        case ACTOR_2A2_CATERPILLAR:
            return actor->state == 1; // STATE_1_PULL_OUT
        default:
            return !actor->unk138_22;
    }
}

void CarryableUpdate(IEvent* event) {
    auto* ev = (ShouldActorUpdate*)event;
    Actor* actor = ev->actor;
    int32_t kind = kindForMarker(actor->marker->id);
    if ((setsUp(actor) && registerSpawn(kind, actor->marker, (int32_t)actor->position[0], (int32_t)actor->position[1],
                                        (int32_t)actor->position[2])) ||
        (liesOut(actor) && collectedRemotely(kind, actor->marker))) {
        marker_despawn(actor->marker);
        *ev->should = false;
    }
}

} // namespace

extern "C" void CarrySync_ApplyRemoteCollect(int32_t kind, int32_t mapId, int32_t id, int32_t sameMap) {
    (void)sameMap;
    int32_t slot = slotForKind(kind);
    if (slot < 0 || id < 0) {
        return;
    }
    sCollected.insert({ slot, mapId, id });
}

// Snapshot/restore as flat [slot, mapId, hash] tuples.
std::vector<int32_t> CarrySync_SnapshotCollected() {
    std::vector<int32_t> flat;
    flat.reserve(sCollected.size() * 3);
    for (const auto& e : sCollected) {
        flat.insert(flat.end(), e.begin(), e.end());
    }
    return flat;
}

void CarrySync_RestoreCollected(const std::vector<int32_t>& flat) {
    sCollected.clear();
    for (size_t i = 0; i + 3 <= flat.size(); i += 3) {
        sCollected.insert({ flat[i], flat[i + 1], flat[i + 2] });
    }
}

extern "C" int32_t CarrySync_GetCollectedCount(int32_t kind) {
    int32_t slot = slotForKind(kind);
    if (slot < 0 || !Anchor_IsWorldSyncActive()) {
        return 0;
    }
    int32_t count = 0;
    for (const auto& e : sCollected) {
        if (e[0] == slot && kindAllowedFor(kind, e[1])) {
            count++;
        }
    }
    return count;
}

void CarrySync_ClearForLevel(int32_t levelId) {
    std::erase_if(sCollected, [levelId](const std::array<int32_t, 3>& e) {
        return (int32_t)map_getLevel((enum map_e)e[1]) == levelId;
    });
}

void RegisterCarrySync_Init() {
    // Always on: a pickup or throw counts only while world sync is live.
    COND_HOOK(OnSaveLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) { sCollected.clear(); });

    COND_HOOK(OnCarriedObjectCollected, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        ActorMarker* marker = ((OnCarriedObjectCollected*)event)->marker;
        int32_t kind = kindForMarker(marker->id);
        int32_t slot = slotForKind(kind);
        if (slot < 0 || inDemoPlayback() || !Anchor_IsWorldSyncActive()) {
            return;
        }
        CarriedSpawnData* d = ObjectExtension::GetInstance().Get<CarriedSpawnData>(marker);
        if (d == nullptr) {
            return;
        }
        sCollected.insert({ slot, d->mapId, d->hash });
        CALL_EVENT(OnCollectibleCollected, kind, d->hash);
    });

    COND_HOOK(OnCarryThrow, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        int32_t kind = kindForMarker(((OnCarryThrow*)event)->markerId);
        if (slotForKind(kind) < 0 || inDemoPlayback() || !Anchor_IsWorldSyncActive()) {
            return;
        }
        CALL_EVENT(OnCollectibleCollected, kind, -1);
    });

    // Always on: an object takes its identity as it sets up, so one set up before connecting still records its pickup.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_29_ORANGE_COLLECTIBLE, EVENT_PRIORITY_NORMAL, true, CarryableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_2A_GOLD_BULLION, EVENT_PRIORITY_NORMAL, true, CarryableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1ED_BLUE_PRESENT_COLLECTIBLE, EVENT_PRIORITY_NORMAL, true, CarryableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1EF_GREEN_PRESENT_COLLECTIBLE, EVENT_PRIORITY_NORMAL, true, CarryableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1F1_RED_PRESENT_COLLECTIBLE, EVENT_PRIORITY_NORMAL, true, CarryableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_2A2_CATERPILLAR, EVENT_PRIORITY_NORMAL, true, CarryableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_2A9_ACORN, EVENT_PRIORITY_NORMAL, true, CarryableUpdate);

    COND_HOOK(OnActorDestroy, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        OnActorDestroy* ev = (OnActorDestroy*)event;
        if (ev->actor != nullptr && ev->actor->marker != nullptr) {
            ObjectExtension::GetInstance().Remove<CarriedSpawnData>(ev->actor->marker);
        }
    });
}

static RegisterShipInitFunc initCarrySync(RegisterCarrySync_Init, {});
