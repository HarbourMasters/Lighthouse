#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/Authority.h"
#include "port/Network/Anchor/FightSync.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"
extern "C" {
#include "variables.h"
}

/**
 * FIGHT_UPDATE
 */

SeqGate gFightSeq;

void Anchor::SendPacket_FightUpdate(const f32 pos[3], f32 yaw, s32 state, s32 phase, s32 mirror, s32 vuln) {
    if (!IsSaveLoaded() || gsworld_getMap() != MAP_90_GL_BATTLEMENTS || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = FIGHT_UPDATE;
    payload["seq"] = SeqGate_Next(&gFightSeq);
    payload["pos"] = { pos[0], pos[1], pos[2] };
    payload["yaw"] = yaw;
    payload["st"] = state;
    payload["ph"] = phase;
    payload["mir"] = mirror;
    payload["vuln"] = vuln;
    payload["quiet"] = true;

    SendToCurrentMapPlayers(payload);
}

void Anchor::HandlePacket_FightUpdate(nlohmann::json& payload) {
    if (gsworld_getMap() != MAP_90_GL_BATTLEMENTS) {
        return;
    }
    if (!SeqGate_Accept(&gFightSeq, payload.value("seq", (u32)0))) {
        return;
    }

    std::vector<f32> pos = payload["pos"].get<std::vector<f32>>();
    if (pos.size() < 3) {
        return;
    }

    FightSync_ApplyUpdate(pos.data(), payload.value("yaw", 0.0f), payload.value("st", (s32)1),
                          payload.value("ph", (s32)0), payload.value("mir", (s32)0), payload.value("vuln", (s32)0));
}
