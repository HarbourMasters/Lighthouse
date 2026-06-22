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

    // Only spawn the actor where the jiggy lives, and not if it's already present/collected.
    if ((s32)gsworld_getMap() != payload.at("map").get<s32>() || jiggyscore_isSpawned((enum jiggy_e)jiggyId)) {
        return;
    }

    f32 pos[3] = { payload.at("x").get<f32>(), payload.at("y").get<f32>(), payload.at("z").get<f32>() };
    // triggerEvent = 0: silent spawn, no re-broadcast.
    codeABC00_spawnJiggyAtLocationEx((enum jiggy_e)jiggyId, pos, 0);
}
