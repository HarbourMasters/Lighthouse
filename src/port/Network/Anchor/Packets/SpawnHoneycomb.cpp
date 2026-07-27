#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"
extern "C" {
void chHoneycomb_netSpawnDropAt(s32 uid, s32 bundleId, f32 x, f32 y, f32 z);
}

void Anchor::SendPacket_SpawnHoneycomb(s16 honeycombId, s32 bundleId, f32 x, f32 y, f32 z) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = HONEYCOMB_SPAWN;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["honeycombId"] = honeycombId;
    payload["bundle"] = bundleId;
    payload["x"] = x;
    payload["y"] = y;
    payload["z"] = z;
    payload["ctx"] = (s32)gsworld_getMap();

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_SpawnHoneycomb(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 honeycombId = payload.at("honeycombId").get<s16>();
    if ((s32)gsworld_getMap() != payload.at("ctx").get<s32>()) {
        return;
    }
    if (honeycombscore_get((enum honeycomb_e)honeycombId)) {
        return;
    }
    // Already present locally, nothing to add.
    if (actorArray_findHoneycombMarkerById((enum honeycomb_e)honeycombId) != nullptr) {
        return;
    }

    chHoneycomb_netSpawnDropAt(honeycombId, payload.at("bundle").get<s32>(), payload.at("x").get<f32>(),
                               payload.at("y").get<f32>(), payload.at("z").get<f32>());
}
