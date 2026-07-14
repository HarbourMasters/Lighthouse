#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/Patches/Patches.h"

extern "C" {
#include "functions.h"
}

/**
 * CARRY_THROW
 *
 * A player threw their carried collectible (gold bullion / FP present / MM orange) at its
 * delivery target. Teammates in the same map replay the identical ballistic arc on their
 * display copy of the carried object (level_collectible.c remote states) — visual/audio only;
 * the delivery flags, count spends, and quest progress all ride their own sync paths.
 */

void Anchor::SendPacket_CarryThrow(s32 markerId, f32 start[3], f32 target[3]) {
    if (!IsSaveLoaded()) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = CARRY_THROW;
    payload["marker"] = markerId;
    payload["start"] = { start[0], start[1], start[2] };
    payload["target"] = { target[0], target[1], target[2] };

    SendToCurrentMapPlayers(payload);
}

void Anchor::HandlePacket_CarryThrow(nlohmann::json& payload) {
    if (!IsSaveLoaded()) {
        return;
    }

    uint32_t clientId = payload.value("clientId", 0u);
    s32 markerId = payload.value("marker", 0);
    std::vector<f32> start = payload["start"].get<std::vector<f32>>();
    std::vector<f32> target = payload["target"].get<std::vector<f32>>();
    if (start.size() < 3 || target.size() < 3) {
        return;
    }

    port_remoteCarry_throw(clientId, markerId, start.data(), target.data());
}

// C->C++ bridge (level_collectible.c): broadcast our own carried-object throw.
extern "C" void port_anchor_onCarryThrow(s32 markerId, f32 start[3], f32 target[3]) {
    Anchor* anchor = Anchor::GetInstance();
    if (anchor == nullptr || !anchor->isConnected) {
        return;
    }
    anchor->SendPacket_CarryThrow(markerId, start, target);
}

// C++->C bridge (level_collectible.c): position + yaw of a client's dummy player, or 0 when
// that client (or their dummy) isn't present in our map — the display copy despawns on 0.
extern "C" s32 port_anchor_getDummyTransform(u32 clientId, f32 pos[3], f32* yaw) {
    Anchor* anchor = Anchor::GetInstance();
    if (anchor == nullptr || !anchor->isConnected) {
        return 0;
    }
    auto it = anchor->clients.find(clientId);
    if (it == anchor->clients.end() || !it->second.online || it->second.dummy == nullptr ||
        (s32)it->second.map != (s32)gsworld_getMap()) {
        return 0;
    }
    it->second.dummy->dummy_getPosition(pos);
    *yaw = it->second.dummy->dummy_getYaw();
    return 1;
}
