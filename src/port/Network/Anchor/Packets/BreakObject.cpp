#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <array>
#include <set>

extern "C" {
#include "functions.h"
}

#include "port/Patches/Patches.h"

/**
 * BREAK_OBJECT
 *
 * Live + temporary-persistence sync of a non-persistent breakable (glass windows, etc.) that
 * breaks + despawns with no synced flag of its own. These objects are static, so their spawn
 * position is identical on every client; we key them by (map, marker, position).
 *
 * The break is recorded in an in-memory set (never written to the save) and broadcast team-wide,
 * so every client remembers it for the session. A teammate in the same map replays the real break
 * live; everyone else applies it at spawn (the object despawns on (re)load if it was broken). The
 * set is cleared on save load so it never leaks across files.
 */

std::set<std::array<int32_t, 5>> sBrokenObjects;

extern "C" int32_t port_breakable_isBroken(int32_t map, int32_t markerId, int32_t x, int32_t y, int32_t z) {
    return sBrokenObjects.count({ map, markerId, x, y, z }) != 0 ? 1 : 0;
}

// Snapshot/restore for the authoritative team-state sync (UpdateTeamState.cpp). Flat [map, marker,
// x, y, z] tuples. Restore overwrites — a client adopting team state takes that session's set.
std::vector<int32_t> port_breakable_snapshotBroken() {
    std::vector<int32_t> flat;
    flat.reserve(sBrokenObjects.size() * 5);
    for (const auto& e : sBrokenObjects) {
        flat.insert(flat.end(), e.begin(), e.end());
    }
    return flat;
}

void port_breakable_restoreBroken(const std::vector<int32_t>& flat) {
    sBrokenObjects.clear();
    for (size_t i = 0; i + 5 <= flat.size(); i += 5) {
        sBrokenObjects.insert({ flat[i], flat[i + 1], flat[i + 2], flat[i + 3], flat[i + 4] });
    }
}

void port_breakable_clearForLevel(int32_t levelId) {
    std::erase_if(sBrokenObjects,
                  [levelId](const std::array<int32_t, 5>& e) { return (int32_t)map_getLevel((enum map_e)e[0]) == levelId; });
}

void Anchor::SendPacket_BreakObject(s16 markerId, s32 x, s32 y, s32 z, s32 map, bool replay) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = BREAK_OBJECT;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["marker"] = markerId;
    payload["x"] = x;
    payload["y"] = y;
    payload["z"] = z;
    payload["map"] = map;
    // replay=false: don't invoke remote break handler
    payload["replay"] = replay;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_BreakObject(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 markerId = payload.at("marker").get<s16>();
    s32 x = payload.at("x").get<s32>();
    s32 y = payload.at("y").get<s32>();
    s32 z = payload.at("z").get<s32>();
    s32 map = payload.at("map").get<s32>();
    bool replay = payload.contains("replay") ? payload.at("replay").get<bool>() : true;

    // Replay break for non-self-polling breakables
    sBrokenObjects.insert({ map, markerId, x, y, z });
    if (replay && (s32)gsworld_getMap() == map) {
        port_breakable_remoteBreakAt(markerId, x, y, z);
    }
}

extern "C" void port_breakable_broadcastBreak(int32_t markerId, int32_t x, int32_t y, int32_t z) {
    s32 map = (s32)gsworld_getMap();
    // Check if object is already broken
    if (sBrokenObjects.count({ map, markerId, x, y, z }) != 0) {
        return;
    }
    sBrokenObjects.insert({ map, markerId, x, y, z });
    Anchor::GetInstance()->SendPacket_BreakObject((s16)markerId, x, y, z, map);
}

extern "C" void port_breakable_recordBreak(int32_t markerId, int32_t x, int32_t y, int32_t z) {
    s32 map = (s32)gsworld_getMap();
    if (sBrokenObjects.count({ map, markerId, x, y, z }) != 0) {
        return;
    }
    sBrokenObjects.insert({ map, markerId, x, y, z });
    Anchor::GetInstance()->SendPacket_BreakObject((s16)markerId, x, y, z, map, false);
}

// game.c: silent despawn sweep of live actors recorded broken (no die effects).
extern "C" void port_breakable_despawnBrokenRestores(s32 map);

void RegisterBreakObject_Init() {
    // Clear on file start
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sBrokenObjects.clear(); });

    // Re-break on save state restore
    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnMapLoad* ev = (OnMapLoad*)event;
        port_breakable_despawnBrokenRestores((s32)ev->nextMap);
    });
}

static RegisterShipInitFunc initBreakObject(RegisterBreakObject_Init, {});
