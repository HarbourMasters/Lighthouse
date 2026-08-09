// CCW Carried-Collectible Live-Despawn Sync (worms for Eyrie, acorns for Nabnut).
//
// Carried count is ITEM_22/23. Object identity is a hash of its fixed spawn position, keyed
// with (kind, mapId, hash).
//
#include <libultraship/bridge.h>
#include "port/ObjectExtension/ObjectExtension.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"

#include <array>
#include <set>

#include "functions.h"
extern "C" {
#include "enums.h"
#include "actor.h"
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

extern "C" void port_carriedSync_beginMapLoad(int32_t mapId) {
    (void)mapId;
}

extern "C" void port_carriedSync_register(int32_t kind, void* marker, int32_t x, int32_t y, int32_t z,
                                          int32_t* suppress) {
    *suppress = 0;
    int32_t slot = slotForKind(kind);
    if (slot < 0 || marker == nullptr) {
        return;
    }
    int32_t mapId = (int32_t)gsworld_getMap();
    int32_t hash = spawnHash(x, y, z);
    ObjectExtension::GetInstance().Set<CarriedSpawnData>(marker, CarriedSpawnData(mapId, hash));
    if (!inDemoPlayback() && kindAllowedFor(kind, mapId) && isCollected(slot, mapId, hash)) {
        *suppress = 1;
    }
}

extern "C" void port_carriedSync_onLocalCollect(int32_t kind, void* marker) {
    int32_t slot = slotForKind(kind);
    if (slot < 0 || inDemoPlayback()) {
        return;
    }
    CarriedSpawnData* d = ObjectExtension::GetInstance().Get<CarriedSpawnData>(marker);
    if (d == nullptr) {
        return;
    }
    sCollected.insert({ slot, d->mapId, d->hash });
    CALL_EVENT(OnCollectibleCollected, kind, d->hash);
}

extern "C" void port_carriedSync_onLocalSpend(int32_t kind) {
    if (slotForKind(kind) < 0 || inDemoPlayback()) {
        return;
    }
    // A spend (feeding Eyrie/Nabnut) is -1 to the shared pool.
    CALL_EVENT(OnCollectibleCollected, kind, -1);
}

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

extern "C" int32_t port_carriedSync_collectedCount(int32_t kind) {
    int32_t slot = slotForKind(kind);
    if (slot < 0) {
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

extern "C" int32_t port_carriedSync_consumeRemoteDespawn(int32_t kind, void* marker) {
    int32_t slot = slotForKind(kind);
    if (slot < 0) {
        return 0;
    }
    CarriedSpawnData* d = ObjectExtension::GetInstance().Get<CarriedSpawnData>(marker);
    if (d == nullptr) {
        return 0;
    }
    return isCollected(slot, d->mapId, d->hash) ? 1 : 0;
}

void RegisterWormSync_Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sCollected.clear(); });

    REGISTER_LISTENER(OnActorDestroy, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnActorDestroy* ev = (OnActorDestroy*)event;
        if (ev->actor != nullptr && ev->actor->marker != nullptr) {
            ObjectExtension::GetInstance().Remove<CarriedSpawnData>(ev->actor->marker);
        }
    });
}

static RegisterShipInitFunc initWormSync(RegisterWormSync_Init, {});
