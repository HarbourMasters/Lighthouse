#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"
#include "port/ShipUtils.h"

#include <cstring>

// Spawned through the same per-map bundle as the local drop so it gets the pickup drop physics.
static void spawnDrop(uintptr_t uid, uintptr_t bundle, uintptr_t x, uintptr_t y, uintptr_t z) {
    const u32 bits[3] = { (u32)x, (u32)y, (u32)z };
    f32 pos[3];
    memcpy(pos, bits, sizeof(pos));
    func_802CA1CC((enum honeycomb_e)uid);
    bundle_spawn_f32((enum bundle_e)bundle, pos);
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
    if (Ship_GetHoneycombScoreRaw((enum honeycomb_e)honeycombId)) {
        return;
    }
    // Already present locally, nothing to add.
    if (Anchor_FindHoneycombMarker((enum honeycomb_e)honeycombId) != nullptr) {
        return;
    }

    const f32 pos[3] = { payload.at("x").get<f32>(), payload.at("y").get<f32>(), payload.at("z").get<f32>() };
    u32 bits[3];
    memcpy(bits, pos, sizeof(bits));
    __spawnQueue_add_5(spawnDrop, (uintptr_t)honeycombId, (uintptr_t)payload.at("bundle").get<s32>(), bits[0], bits[1],
                       bits[2]);
}
