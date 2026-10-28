#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/ShipInit.hpp"

#include "port/Patches/Patches.h"

#include "functions.h"

/**
 * CARRY_THROW
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

    remoteCarry_throw(clientId, markerId, start.data(), target.data());
}

static void RegisterCarryThrowSync_Init() {
    COND_VB_SHOULD(VB_REMOTE_CARRY_POSE, EVENT_PRIORITY_NORMAL, Anchor::GetInstance()->isConnected, {
        const u32 clientId = va_arg(args, u32);
        f32* pos = va_arg(args, f32*);
        f32* yaw = va_arg(args, f32*);
        Anchor* anchor = Anchor::GetInstance();
        auto it = anchor->clients.find(clientId);
        if (it != anchor->clients.end() && it->second.online && it->second.dummy != nullptr &&
            (s32)it->second.map == (s32)gsworld_getMap()) {
            it->second.dummy->dummy_getPosition(pos);
            *yaw = it->second.dummy->dummy_getYaw();
            *should = true;
        }
    });
}

static RegisterShipInitFunc initCarryThrowSync(RegisterCarryThrowSync_Init, { ANCHOR_CONNECTED_PATH });
