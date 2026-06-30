#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <array>
#include <map>
#include <vector>

extern "C" {
#include "functions.h"
}

/**
 * HUT_SMASH
 *
 * Live + temporary-persistence sync of the MM and BGS huts (which break with no flag of their own).
 * A hut is keyed by its (map, spawn position); the value is the bundle index it dropped. BGS picks
 * its bundle from the hut's position (deterministic), but MM picks from a global smash-order counter
 * that isn't shared, so the index must travel with the break or the same hut gives different loot on
 * each client.
 *
 * The record is broadcast team-wide and rides the team-state snapshot. A teammate already in the map
 * sees the hut break + its full drop live; on (re)entry the hut is restored broken and re-drops only
 * its non-tracked loot (pads/eggs/extra-lives/enemies) — notes, jinjos and jiggies are left to their
 * own retention/spawn sync. Cleared on save load (session state, never saved).
 */

namespace {
// (map, x, y, z) -> bundle index dropped.
std::map<std::array<int32_t, 4>, int32_t> sHuts;
} // namespace

extern "C" int32_t port_hutSmash_get(int32_t x, int32_t y, int32_t z) {
    auto it = sHuts.find({ (int32_t)gsworld_getMap(), x, y, z });
    return it != sHuts.end() ? it->second : -1;
}

extern "C" void port_hutSmash_record(int32_t x, int32_t y, int32_t z, int32_t loot) {
    int32_t map = (int32_t)gsworld_getMap();
    std::array<int32_t, 4> key = { map, x, y, z };
    // Idempotent: first smasher wins; replays/reloads re-running this won't re-broadcast.
    if (sHuts.find(key) != sHuts.end()) {
        return;
    }
    sHuts[key] = loot;
    Anchor::GetInstance()->SendPacket_HutSmash(x, y, z, loot, map);
}

void Anchor::SendPacket_HutSmash(s32 x, s32 y, s32 z, s32 loot, s32 map) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = HUT_SMASH;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["x"] = x;
    payload["y"] = y;
    payload["z"] = z;
    payload["loot"] = loot;
    payload["map"] = map;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_HutSmash(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s32 x = payload.at("x").get<s32>();
    s32 y = payload.at("y").get<s32>();
    s32 z = payload.at("z").get<s32>();
    s32 loot = payload.at("loot").get<s32>();
    s32 map = payload.at("map").get<s32>();

    // Record only; the hut actor notices it on its next update — breaking + dropping live if we're
    // already in the map, or restoring broken at spawn on a later visit.
    sHuts.emplace(std::array<int32_t, 4>{ map, x, y, z }, loot);
}

// Authoritative team-state snapshot/restore (UpdateTeamState.cpp). Flat [map, x, y, z, loot].
std::vector<int32_t> port_hutSmash_snapshot() {
    std::vector<int32_t> flat;
    flat.reserve(sHuts.size() * 5);
    for (const auto& [key, loot] : sHuts) {
        flat.push_back(key[0]);
        flat.push_back(key[1]);
        flat.push_back(key[2]);
        flat.push_back(key[3]);
        flat.push_back(loot);
    }
    return flat;
}

void port_hutSmash_restore(const std::vector<int32_t>& flat) {
    sHuts.clear();
    for (size_t i = 0; i + 5 <= flat.size(); i += 5) {
        sHuts[{ flat[i], flat[i + 1], flat[i + 2], flat[i + 3] }] = flat[i + 4];
    }
}

void RegisterHutSmash_Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sHuts.clear(); });
}

static RegisterShipInitFunc initHutSmash(RegisterHutSmash_Init, {});
