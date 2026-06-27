// CCW Carried-Collectible Live-Despawn Sync (worms for Eyrie, acorns for Nabnut)
//
// Both are a shared pool: the carried count (ITEM_22_CATERPILLAR / ITEM_23_ACORNS) is delta-synced
// through the COLLECT_ITEM packet (+1 on collect, -1 on spend; see CollectItem.cpp), so one a
// player collects can be spent by anyone. This module gives each world object a stable identity so
// a pickup despawns it on every client (no double-collect).
//
// Identity is the object's FIXED SPAWN POSITION, hashed to an int. The spawn position comes from
// the map's prop data, so it's identical on every client and independent of spawn/update order or
// player path — unlike a spawn-order counter (the objects init lazily as players approach them) or
// the live position (the worms crawl away from where they spawned). The hash is attached to the
// marker via ObjectExtension when the actor registers at init, and masked non-negative so it never
// collides with the -1 spend sentinel.
//
// The picked-up set is keyed by (kind, mapId, hash) and persists for the session (cleared on save
// load), so a teammate's pickup also suppresses that object when anyone next enters that map —
// fixing cross-map re-collection. Tradeoff: a collected worm/acorn no longer respawns per visit.
//
#include <libultraship/bridge.h>
#include "port/ObjectExtension/ObjectExtension.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"

#include <array>
#include <set>

extern "C" {
#include "enums.h"
#include "actor.h"
#include "functions.h"
}

namespace {

// Per-object identity attached to its marker at registration. A constructor lets us build with
// parentheses, since brace-init commas break the event macros.
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
        default:
            return -1;
    }
}

// Stable, client-independent identity from the fixed spawn position. Masked non-negative so it is
// never confused with the -1 spend sentinel carried by the same packet.
int32_t spawnHash(int32_t x, int32_t y, int32_t z) {
    uint32_t h = (uint32_t)x * 73856093u ^ (uint32_t)y * 19349663u ^ (uint32_t)z * 83492791u;
    return (int32_t)(h & 0x7FFFFFFFu);
}

bool isCollected(int32_t slot, int32_t mapId, int32_t hash) {
    return sCollected.count({ slot, mapId, hash }) != 0;
}

} // namespace

extern "C" void port_carriedSync_beginMapLoad(int32_t mapId) {
    // Identity is spawn-position based and the picked-up set is session-persistent, so there is
    // nothing per-map to reset here. Kept as the single map-load entry point in case that changes.
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
    if (isCollected(slot, mapId, hash)) {
        *suppress = 1;
    }
}

extern "C" void port_carriedSync_onLocalCollect(int32_t kind, void* marker) {
    int32_t slot = slotForKind(kind);
    if (slot < 0) {
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
    if (slotForKind(kind) < 0) {
        return;
    }
    // A spend (feeding Eyrie/Nabnut) is a -1 to the shared pool, no object identity.
    CALL_EVENT(OnCollectibleCollected, kind, -1);
}

extern "C" void port_carriedSync_applyRemoteCollect(int32_t kind, int32_t mapId, int32_t id, int32_t sameMap) {
    (void)sameMap;
    int32_t slot = slotForKind(kind);
    if (slot < 0 || id < 0) {
        return;
    }
    // Record regardless of our current map so a later visit suppresses it. If we're in this map now,
    // the matching object despawns itself via consumeRemoteDespawn on its next update.
    sCollected.insert({ slot, mapId, id });
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
    // Drop the picked-up set when a save loads, so it never leaks across files/playthroughs.
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sCollected.clear(); });

    // Detach per-marker spawn data when an object is destroyed, so a reused marker starts clean.
    REGISTER_LISTENER(OnActorDestroy, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnActorDestroy* ev = (OnActorDestroy*)event;
        if (ev->actor != nullptr && ev->actor->marker != nullptr) {
            ObjectExtension::GetInstance().Remove<CarriedSpawnData>(ev->actor->marker);
        }
    });
}

static RegisterShipInitFunc initWormSync(RegisterWormSync_Init, {});
