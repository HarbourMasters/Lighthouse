// Carried-Collectible Live-Despawn Sync (caterpillars, acorns, FP presents, TTC gold, MM oranges).
//
// Carried count is ITEM_22/23. Object identity is a hash of its fixed spawn position, keyed
// with (kind, mapId, hash).
//
#include <libultraship/bridge.h>
#include "port/ObjectExtension/ObjectExtension.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Network/Anchor/CarriedSync.h"
#include "port/Network/Anchor/Anchor.h"

#include <array>
#include <set>

#include "functions.h"
extern "C" {
#include "enums.h"
#include "actor.h"
s32 Anchor_IsWorldSyncActive(void);
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
        case ANCHOR_COLLECTIBLE_WORM:
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
            return ANCHOR_COLLECTIBLE_WORM;
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

} // namespace

extern "C" void port_carriedSync_applyRemoteCollect(int32_t kind, int32_t mapId, int32_t id, int32_t sameMap) {
    (void)sameMap;
    int32_t slot = slotForKind(kind);
    if (slot < 0 || id < 0) {
        return;
    }
    sCollected.insert({ slot, mapId, id });
}

// Snapshot/restore as flat [slot, mapId, hash] tuples.
std::vector<int32_t> port_carriedSync_snapshotCollected() {
    std::vector<int32_t> flat;
    flat.reserve(sCollected.size() * 3);
    for (const auto& e : sCollected) {
        flat.insert(flat.end(), e.begin(), e.end());
    }
    return flat;
}

void port_carriedSync_restoreCollected(const std::vector<int32_t>& flat) {
    sCollected.clear();
    for (size_t i = 0; i + 3 <= flat.size(); i += 3) {
        sCollected.insert({ flat[i], flat[i + 1], flat[i + 2] });
    }
}

static int32_t collectedCount(int32_t kind) {
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

void port_carriedSync_clearForLevel(int32_t levelId) {
    std::erase_if(sCollected, [levelId](const std::array<int32_t, 3>& e) {
        return (int32_t)map_getLevel((enum map_e)e[1]) == levelId;
    });
}

void RegisterCarriedSync_Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sCollected.clear(); });

    COND_HOOK(OnCarriedObjectSpawn, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnCarriedObjectSpawn*)event;
        if (slotForKind(kindForMarker(ev->marker->id)) >= 0) {
            ObjectExtension::GetInstance().Set<CarriedSpawnData>(
                ev->marker, CarriedSpawnData((int32_t)gsworld_getMap(), spawnHash(ev->x, ev->y, ev->z)));
        }
    });

    REGISTER_LISTENER(OnActorDestroy, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnActorDestroy* ev = (OnActorDestroy*)event;
        if (ev->actor != nullptr && ev->actor->marker != nullptr) {
            ObjectExtension::GetInstance().Remove<CarriedSpawnData>(ev->actor->marker);
        }
    });

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

    // A spend (feeding Eyrie/Nabnut) is -1 to the shared pool.
    COND_HOOK(OnCarriedObjectSpent, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        int32_t kind = kindForMarker(((OnCarriedObjectSpent*)event)->marker->id);
        if (slotForKind(kind) < 0 || inDemoPlayback() || !Anchor_IsWorldSyncActive()) {
            return;
        }
        CALL_EVENT(OnCollectibleCollected, kind, -1);
    });
}

static RegisterShipInitFunc initCarriedSync(RegisterCarriedSync_Init, {});

static void RegisterCarriedSyncGates_Init() {
    const bool connected = Anchor::GetInstance()->isConnected;

    COND_VB_SHOULD(VB_CARRIED_OBJECT_SPAWN, EVENT_PRIORITY_NORMAL, connected, {
        ActorMarker* marker = va_arg(args, ActorMarker*);
        const int32_t kind = kindForMarker(marker->id);
        const int32_t slot = slotForKind(kind);
        CarriedSpawnData* d = ObjectExtension::GetInstance().Get<CarriedSpawnData>(marker);
        if (slot >= 0 && d != nullptr && Anchor_IsWorldSyncActive() && !inDemoPlayback() &&
            kindAllowedFor(kind, d->mapId) && isCollected(slot, d->mapId, d->hash)) {
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_CARRIED_COLLECTED_COUNT, EVENT_PRIORITY_NORMAL, connected, {
        const s32 kind = va_arg(args, s32);
        s32* count = va_arg(args, s32*);
        *count = collectedCount(kind);
    });

    COND_VB_SHOULD(VB_CARRIED_OBJECT_REMOTE_DESPAWN, EVENT_PRIORITY_NORMAL, connected, {
        ActorMarker* marker = va_arg(args, ActorMarker*);
        const int32_t slot = slotForKind(kindForMarker(marker->id));
        CarriedSpawnData* d = ObjectExtension::GetInstance().Get<CarriedSpawnData>(marker);
        if (slot >= 0 && d != nullptr && Anchor_IsWorldSyncActive() && isCollected(slot, d->mapId, d->hash)) {
            *should = true;
        }
    });
}

static RegisterShipInitFunc initCarriedSyncGates(RegisterCarriedSyncGates_Init, { ANCHOR_CONNECTED_PATH });
