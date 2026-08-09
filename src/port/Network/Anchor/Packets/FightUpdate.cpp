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

static uint32_t sOutgoingSeq = 0;
static uint32_t sLastAcceptedSeq = 0;

extern "C" uint32_t FightSyncSeq_Next(void) {
    return ++sOutgoingSeq;
}

extern "C" bool FightSyncSeq_Accept(uint32_t seq) {
    if (seq <= sLastAcceptedSeq) {
        return false;
    }
    sLastAcceptedSeq = seq;
    return true;
}

extern "C" void FightSyncSeq_Reset(void) {
    sOutgoingSeq = 0;
    sLastAcceptedSeq = 0;
}

void Anchor::SendPacket_FightUpdate(const f32 pos[3], f32 yaw, s32 state, s32 phase, s32 mirror, s32 vuln) {
    if (!IsSaveLoaded() || gsworld_getMap() != MAP_90_GL_BATTLEMENTS || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = FIGHT_UPDATE;
    payload["seq"] = FightSyncSeq_Next();
    payload["pos"] = { pos[0], pos[1], pos[2] };
    payload["yaw"] = yaw;
    payload["st"] = state;
    payload["ph"] = phase;
    payload["mir"] = mirror;
    payload["vuln"] = vuln;
    payload["quiet"] = true;

    SendToCurrentMapPlayers(payload);
}

extern "C" void FightSync_SendUpdate(const float pos[3], float yaw, int32_t state, int32_t phase, int32_t mirror,
                                     int32_t vuln) {
    Anchor::GetInstance()->SendPacket_FightUpdate(pos, yaw, state, phase, mirror, vuln);
}

void Anchor::HandlePacket_FightUpdate(nlohmann::json& payload) {
    if (gsworld_getMap() != MAP_90_GL_BATTLEMENTS) {
        return;
    }
    if (!FightSyncSeq_Accept(payload.value("seq", (u32)0))) {
        return;
    }

    std::vector<f32> pos = payload["pos"].get<std::vector<f32>>();
    if (pos.size() < 3) {
        return;
    }

    FightSync_ApplyUpdate(pos.data(), payload.value("yaw", 0.0f), payload.value("st", (s32)1),
                          payload.value("ph", (s32)0), payload.value("mir", (s32)0), payload.value("vuln", (s32)0));
}
