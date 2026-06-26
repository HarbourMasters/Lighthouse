#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
}

/**
 * JIGGY_SPAWN
 *
 * Realtime jiggy spawn (witch-switch puzzle, minigame reward, etc.). A same-map teammate
 * spawns the jiggy live at the same position so it doesn't only appear on re-entry.
 */

void Anchor::SendPacket_SpawnJiggy(s16 jiggyId, f32 x, f32 y, f32 z) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = JIGGY_SPAWN;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["jiggyId"] = jiggyId;
    payload["x"] = x;
    payload["y"] = y;
    payload["z"] = z;
    payload["map"] = (s32)gsworld_getMap();

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_SpawnJiggy(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 jiggyId = payload.at("jiggyId").get<s16>();
    s32 map = payload.at("map").get<s32>();

    // Already have it (spawned or collected): nothing to do, and don't bother buffering.
    if (jiggyscore_isSpawned((enum jiggy_e)jiggyId)) {
        return;
    }

    f32 x = payload.at("x").get<f32>();
    f32 y = payload.at("y").get<f32>();
    f32 z = payload.at("z").get<f32>();

    if ((s32)gsworld_getMap() == map) {
        // In the jiggy's map: spawn it live. triggerEvent = 0: silent, no re-broadcast.
        f32 pos[3] = { x, y, z };
        codeABC00_spawnJiggyAtLocationEx((enum jiggy_e)jiggyId, pos, 0);
        return;
    }

    // We're elsewhere (online in another map, or this is a queued replay from before we joined).
    // Buffer it so it spawns when we enter that map, instead of dropping it. Dedupe by id.
    auto& pending = pendingJiggySpawns[map];
    for (auto& pj : pending) {
        if (pj.jiggyId == jiggyId) {
            pj.x = x;
            pj.y = y;
            pj.z = z;
            return;
        }
    }
    pending.push_back({ jiggyId, x, y, z });
}

// Spawn any buffered jiggies for the map we're now in (called each frame while a save is
// loaded; cheap no-op when nothing is pending for the current map).
void Anchor::FlushPendingJiggySpawns() {
    auto it = pendingJiggySpawns.find((s32)gsworld_getMap());
    if (it == pendingJiggySpawns.end()) {
        return;
    }
    for (auto& pj : it->second) {
        if (!jiggyscore_isSpawned((enum jiggy_e)pj.jiggyId) && !jiggyscore_isCollected((enum jiggy_e)pj.jiggyId)) {
            f32 pos[3] = { pj.x, pj.y, pj.z };
            codeABC00_spawnJiggyAtLocationEx((enum jiggy_e)pj.jiggyId, pos, 0);
        }
    }
    pendingJiggySpawns.erase(it);
}
