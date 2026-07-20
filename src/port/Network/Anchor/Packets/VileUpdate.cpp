#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/VileSync.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"
#include "variables.h"

/**
 * VILE_UPDATE
 *
 * Mr. Vile transform + anim mode, streamed by the minigame authority to other clients
 * in the chamber at the player-update cadence. Remote clients skip Mr. Vile's AI and
 * apply this instead.
 *
 * animMode is the chMrVile_setAction mode (101 idle / 102 walk / 103 munch / 104 burp).
 *
 * Note: sent every frame while the minigame runs, so keep this payload lean.
 */

void Anchor::SendPacket_VileUpdate(const f32 position[3], f32 pitch, f32 yaw, f32 roll, u8 animMode) {
    if (!IsSaveLoaded() || gsworld_getMap() != MAP_10_BGS_MR_VILE || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = VILE_UPDATE;
    payload["seq"] = VileSync_NextOutgoingSeq();
    payload["pos"] = { position[0], position[1], position[2] };
    payload["rot"] = { pitch, yaw, roll };
    payload["mode"] = animMode;
    payload["quiet"] = true;

    SendToCurrentMapPlayers(payload);
}

void Anchor::HandlePacket_VileUpdate(nlohmann::json& payload) {
    if (gsworld_getMap() != MAP_10_BGS_MR_VILE) {
        return;
    }
    if (!VileSync_AcceptIncomingSeq(payload.value("seq", (u32)0))) {
        return;
    }

    std::vector<f32> pos = payload["pos"].get<std::vector<f32>>();
    std::vector<f32> rot = payload["rot"].get<std::vector<f32>>();
    if (pos.size() < 3 || rot.size() < 3) {
        return;
    }

    VileSync_ApplyVileUpdate(pos.data(), rot[0], rot[1], rot[2], payload.value("mode", (u8)101));
}
