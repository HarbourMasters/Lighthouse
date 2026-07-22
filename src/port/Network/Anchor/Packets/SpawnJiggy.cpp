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
 * Sync of dynamically spawned jiggies (witch-switch puzzle, minigame reward, jinjo fifth, etc).
 * The team-wide record (map -> jiggyId -> position) is the source of truth, re-applied every map
 * visit, so an uncollected spawn reappears on re-entry or for a late joiner.
 */

struct SpawnedJiggy {
    int16_t jiggyId;
    float x;
    float y;
    float z;
    // True while waiting on the spawn queue to materialize the actor; prevents restacking a
    // duplicate spawn each frame in the meantime. Reset on map load.
    bool inFlight = false;
};
// The team's dynamically-spawned jiggies this session: map -> list. Session state only.
std::map<int32_t, std::vector<SpawnedJiggy>> sSpawnedJiggies;

SpawnedJiggy& recordJiggySpawn(int32_t map, int16_t jiggyId, float x, float y, float z) {
    auto& list = sSpawnedJiggies[map];
    for (auto& pj : list) {
        if (pj.jiggyId == jiggyId) {
            pj.x = x;
            pj.y = y;
            pj.z = z;
            return pj;
        }
    }
    list.push_back({ jiggyId, x, y, z });
    return list.back();
}

// Spawn a recorded jiggy if it isn't already present, collected, or mid-spawn.
void trySpawnRecordedJiggy(SpawnedJiggy& pj) {
    if (jiggyscore_isCollected((enum jiggy_e)pj.jiggyId)) {
        return;
    }
    if (jiggylist_hasSpawnedObject((enum jiggy_e)pj.jiggyId)) {
        pj.inFlight = false;
        return;
    }
    if (pj.inFlight) {
        return;
    }
    f32 pos[3] = { pj.x, pj.y, pj.z };
    codeABC00_spawnJiggyAtLocationEx((enum jiggy_e)pj.jiggyId, pos, 0); // silent, no re-broadcast
    pj.inFlight = true;
}

void Anchor::SendPacket_SpawnJiggy(s16 jiggyId, f32 x, f32 y, f32 z) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s32 map = (s32)gsworld_getMap();
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

    SpawnedJiggy& pj = recordJiggySpawn(map, jiggyId, x, y, z);

    // If we're in the jiggy's map, spawn it live now; otherwise it spawns from the record on entry.
    if ((s32)gsworld_getMap() == map) {
        trySpawnRecordedJiggy(pj);
    }
}

// Re-apply the team's spawned jiggies for the current map: (re)spawn missing ones, prune collected
// ones. Runs every frame while a save is loaded.
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
        trySpawnRecordedJiggy(pj);
        i++;
    }
}

// Team-state snapshot/restore (UpdateTeamState.cpp). Flat [map, jiggyId, x, y, z] tuples.
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

// Drop a spawned-jiggy record (all maps) for a timed jiggy whose hourglass ran out, so the
// per-frame flush doesn't immediately re-spawn what the timer just despawned.
extern "C" void port_jiggySpawn_remove(int32_t jiggyId) {
    for (auto& [map, list] : sSpawnedJiggies) {
        std::erase_if(list, [jiggyId](const SpawnedJiggy& pj) { return pj.jiggyId == jiggyId; });
    }
}

// Whether this jiggy has been dynamically spawned this session and isn't collected yet. Used by
// jinjo retention so it doesn't treat a teammate's spawn as "stranded".
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
    // Drop the record on save load so it can't leak across files.
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sSpawnedJiggies.clear(); });

    // A map load frees the previous map's actors, so clear inFlight to let the flush re-issue.
    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        for (auto& [map, list] : sSpawnedJiggies) {
            for (auto& pj : list) {
                pj.inFlight = false;
            }
        }
    });
}

static RegisterShipInitFunc initSpawnJiggy(RegisterSpawnJiggy_Init, {});
