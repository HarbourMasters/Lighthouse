#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <map>
#include <vector>

#include "functions.h"
extern "C" {
#include "core2/fx/effect_jiggy_list.h"
extern Struct_core2_ABC00_0 jiggylist_list[];
extern s32 s_jiggyList_level_jiggy_count;
}

extern "C" {
void func_80332BB0(void);
void port_jiggySpawn_remove(int32_t jiggyId);
}

/**
 * JIGGY_SPAWN
 *
 * Sync of dynamically spawned jiggies (witch-switch puzzle, minigame reward, jinjo fifth, etc).
 * Team-wide record (map -> jiggyId -> position) re-applied every map visit.
 */

struct SpawnedJiggy {
    int16_t jiggyId;
    float x;
    float y;
    float z;
    bool inFlight = false;
};
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

// True once the jiggy list slot's marker is linked (covers bundle-pop, unlike jiggyscore_isSpawned).
bool Anchor_JiggyListHasObject(int16_t jiggyId) {
    return jiggylist_list[jiggyId - 1].unk10.marker != NULL;
}

// True when this call spawned it.
bool trySpawnRecordedJiggy(SpawnedJiggy& pj) {
    // Only a mismatched or broken peer sends an id outside the jiggy list.
    if (pj.jiggyId <= 0 || pj.jiggyId >= s_jiggyList_level_jiggy_count * 10) {
        return false;
    }
    if (jiggyscore_isCollected((enum jiggy_e)pj.jiggyId)) {
        return false;
    }
    if (Anchor_JiggyListHasObject(pj.jiggyId)) {
        pj.inFlight = false;
        return false;
    }
    if (pj.inFlight) {
        return false;
    }
    f32 pos[3] = { pj.x, pj.y, pj.z };
    // A recorded jiggy has no owner; Conga's or Grabba's callback from an earlier visit would write
    // through that visit's freed marker.
    jiggylist_list[pj.jiggyId - 1].unk10.unk14 = NULL;
    codeABC00_spawnJiggyAtLocation((enum jiggy_e)pj.jiggyId, pos);
    pj.inFlight = true;
    return true;
}

// jiggy_spawn's jingle, which the silent spawn above leaves out.
static void playSpawnJingle(int16_t jiggyId) {
    if (jiggyId != JIGGY_3E_GV_GRABBA && jiggyId != JIGGY_0B_TTC_JINJO) {
        midichannel_incOrDecCounter(false);
        func_8025A55C(0, 4000, 5);
        coMusicPlayer_playMusic(COMUSIC_3D_JIGGY_SPAWN, 0x7FFF);
        timedFunc_set_0(3.5f, func_80332BB0);
    }
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

// A timed BGS jiggy that ran out on the sender; receivers drop its record and their copy.
void Anchor::SendPacket_JiggyExpired(s16 jiggyId) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = JIGGY_SPAWN;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["jiggyId"] = jiggyId;
    payload["expired"] = true;

    SendJsonToRemote(payload);
}

// Our copy of a teammate's timed BGS jiggy goes too, unless that switch's timer is running here.
static void expireTimedJiggy(int16_t jiggyId) {
    s32 timerRunningFlag;

    switch (jiggyId) {
        case JIGGY_20_BGS_ELEVATED_WALKWAY:
            timerRunningFlag = BGS_SPECIFIC_FLAG_3_WALKWAY_JIGGY_TIMER_RUNNING;
            break;
        case JIGGY_25_BGS_MAZE:
            timerRunningFlag = BGS_SPECIFIC_FLAG_C_MAZE_JIGGY_TIMER_RUNNING;
            break;
        default:
            return;
    }
    if (gsworld_getMap() != MAP_D_BGS_BUBBLEGLOOP_SWAMP || mapSpecificFlags_get(timerRunningFlag) ||
        !Anchor_JiggyListHasObject(jiggyId)) {
        return;
    }
    if (Anchor_InEarshot(jiggylist_list[jiggyId - 1].unk10.position)) {
        func_8030E6D4(SFX_30_MAGIC_POOF);
    }
    marker_despawn(jiggylist_list[jiggyId - 1].unk10.marker);
}

void Anchor::HandlePacket_SpawnJiggy(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 jiggyId = payload.at("jiggyId").get<s16>();
    if (payload.value("expired", false)) {
        port_jiggySpawn_remove(jiggyId);
        expireTimedJiggy(jiggyId);
        return;
    }
    s32 map = payload.at("map").get<s32>();
    f32 x = payload.at("x").get<f32>();
    f32 y = payload.at("y").get<f32>();
    f32 z = payload.at("z").get<f32>();

    SpawnedJiggy& pj = recordJiggySpawn(map, jiggyId, x, y, z);

    // A jiggy that appears while we're here plays its jingle within earshot; one already waiting when we arrive
    // doesn't.
    f32 pos[3] = { x, y, z };
    if ((s32)gsworld_getMap() == map) {
        bool spawned = false;
        ApplyRemote([&] { spawned = trySpawnRecordedJiggy(pj); });
        if (spawned && Anchor_InEarshot(pos)) {
            playSpawnJingle(jiggyId);
        }
    }
}

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
        ApplyRemote([&] { trySpawnRecordedJiggy(pj); });
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
        // The collect that would clear a record can sit in a queue that was never replayed.
        if (jiggyscore_isCollected((enum jiggy_e)flat[i + 1])) {
            continue;
        }
        sSpawnedJiggies[flat[i]].push_back(
            { (int16_t)flat[i + 1], (float)flat[i + 2], (float)flat[i + 3], (float)flat[i + 4] });
    }
}

extern "C" void port_jiggySpawn_remove(int32_t jiggyId) {
    for (auto& [map, list] : sSpawnedJiggies) {
        std::erase_if(list, [jiggyId](const SpawnedJiggy& pj) { return pj.jiggyId == jiggyId; });
    }
}

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
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sSpawnedJiggies.clear(); });

    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        for (auto& [map, list] : sSpawnedJiggies) {
            for (auto& pj : list) {
                pj.inFlight = false;
            }
        }
    });
}

static RegisterShipInitFunc initSpawnJiggy(RegisterSpawnJiggy_Init, {});
