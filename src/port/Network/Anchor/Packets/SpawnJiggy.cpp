#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <map>
#include <vector>

extern "C" {
#include "functions.h"
}

/**
 * JIGGY_SPAWN
 *
 * Sync of dynamically spawned jiggies (witch-switch puzzle, minigame reward, jinjo fifth, Chimpy,
 * etc.). A jiggy spawned by jiggy_spawn fires OnJiggySpawned -> SendPacket_SpawnJiggy; the spawn is
 * recorded team-wide (map -> jiggyId -> position), broadcast, and rides the team-state snapshot.
 *
 * The record is the source of truth, NOT a one-shot buffer: a same-map teammate spawns it live, and
 * everyone re-applies the record every visit (FlushPendingJiggySpawns) so a spawned-but-uncollected
 * jiggy reappears on re-entry and for players who join later. Entries are pruned once the jiggy is
 * collected, and the whole record is dropped on save load (it's session state, never written to a
 * save). The remote position is identical to the local one because puzzle spawn points are
 * deterministic.
 */

namespace {
struct SpawnedJiggy {
    int16_t jiggyId;
    float x;
    float y;
    float z;
};
// The team's dynamically-spawned jiggies this session: map -> list. Session state, dropped on save
// load; rides the team-state snapshot so a joining client adopts it.
std::map<int32_t, std::vector<SpawnedJiggy>> sSpawnedJiggies;

// Record a spawn (dedupe by jiggyId within a map; update position).
void recordJiggySpawn(int32_t map, int16_t jiggyId, float x, float y, float z) {
    auto& list = sSpawnedJiggies[map];
    for (auto& pj : list) {
        if (pj.jiggyId == jiggyId) {
            pj.x = x;
            pj.y = y;
            pj.z = z;
            return;
        }
    }
    list.push_back({ jiggyId, x, y, z });
}
} // namespace

void Anchor::SendPacket_SpawnJiggy(s16 jiggyId, f32 x, f32 y, f32 z) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s32 map = (s32)gsworld_getMap();
    // Record our own spawn too, so it persists for us on re-entry and is included in the team-state
    // we hand to a joining teammate.
    recordJiggySpawn(map, jiggyId, x, y, z);

    nlohmann::json payload;
    payload["type"] = JIGGY_SPAWN;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["jiggyId"] = jiggyId;
    payload["x"] = x;
    payload["y"] = y;
    payload["z"] = z;
    payload["map"] = map;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_SpawnJiggy(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 jiggyId = payload.at("jiggyId").get<s16>();
    s32 map = payload.at("map").get<s32>();
    f32 x = payload.at("x").get<f32>();
    f32 y = payload.at("y").get<f32>();
    f32 z = payload.at("z").get<f32>();

    // Always record it (session-persistent), so it reappears on every visit and survives our own
    // re-entries — not just the first time we reach the map.
    recordJiggySpawn(map, jiggyId, x, y, z);

    // Already collected: nothing to spawn (FlushPendingJiggySpawns prunes it).
    if (jiggyscore_isCollected((enum jiggy_e)jiggyId)) {
        return;
    }

    // In the jiggy's map and not yet spawned: spawn it live. triggerEvent = 0: silent, no re-broadcast.
    if ((s32)gsworld_getMap() == map && !jiggyscore_isSpawned((enum jiggy_e)jiggyId)) {
        f32 pos[3] = { x, y, z };
        codeABC00_spawnJiggyAtLocationEx((enum jiggy_e)jiggyId, pos, 0);
    }
    // Otherwise it'll spawn from the record when we (re)enter that map.
}

// Re-apply the team's spawned jiggies for the map we're in: (re)spawn any that aren't currently in
// the world, and prune any that have been collected. Runs every frame while a save is loaded; the
// jiggyscore checks make it a cheap no-op once everything is settled.
void Anchor::FlushPendingJiggySpawns() {
    auto it = sSpawnedJiggies.find((s32)gsworld_getMap());
    if (it == sSpawnedJiggies.end()) {
        return;
    }
    auto& list = it->second;
    for (size_t i = 0; i < list.size();) {
        auto& pj = list[i];
        if (jiggyscore_isCollected((enum jiggy_e)pj.jiggyId)) {
            list.erase(list.begin() + i);
            continue;
        }
        if (!jiggyscore_isSpawned((enum jiggy_e)pj.jiggyId)) {
            f32 pos[3] = { pj.x, pj.y, pj.z };
            codeABC00_spawnJiggyAtLocationEx((enum jiggy_e)pj.jiggyId, pos, 0);
        }
        i++;
    }
}

// Authoritative team-state snapshot/restore (UpdateTeamState.cpp). Flat [map, jiggyId, x, y, z]; the
// positions round to int (jiggy spawn points are whole-unit world coords). A joining client adopts
// the team's spawned-jiggy record and re-applies it on its next map visit.
std::vector<int32_t> port_jiggySpawn_snapshot() {
    std::vector<int32_t> flat;
    for (const auto& [map, list] : sSpawnedJiggies) {
        for (const auto& pj : list) {
            flat.push_back(map);
            flat.push_back(pj.jiggyId);
            flat.push_back((int32_t)pj.x);
            flat.push_back((int32_t)pj.y);
            flat.push_back((int32_t)pj.z);
        }
    }
    return flat;
}

void port_jiggySpawn_restore(const std::vector<int32_t>& flat) {
    sSpawnedJiggies.clear();
    for (size_t i = 0; i + 5 <= flat.size(); i += 5) {
        sSpawnedJiggies[flat[i]].push_back(
            { (int16_t)flat[i + 1], (float)flat[i + 2], (float)flat[i + 3], (float)flat[i + 4] });
    }
}

// Whether a teammate (or we) has dynamically spawned this jiggy this session and it isn't collected
// yet — i.e. it will (re)appear from the record. Used by jinjo retention so it doesn't treat a jiggy
// a teammate spawned as "stranded" and clear the recorded jinjos before the jiggy re-spawns on entry.
extern "C" int32_t port_jiggySpawn_isRecorded(int32_t jiggyId) {
    if (jiggyscore_isCollected((enum jiggy_e)jiggyId)) {
        return 0;
    }
    for (const auto& [map, list] : sSpawnedJiggies) {
        for (const auto& pj : list) {
            if (pj.jiggyId == jiggyId) {
                return 1;
            }
        }
    }
    return 0;
}

void RegisterSpawnJiggy_Init() {
    // Session state — drop it on save load so a spawned-jiggy record can't leak across files.
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sSpawnedJiggies.clear(); });
}

static RegisterShipInitFunc initSpawnJiggy(RegisterSpawnJiggy_Init, {});
