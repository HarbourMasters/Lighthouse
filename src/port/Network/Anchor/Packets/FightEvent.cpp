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
 * FIGHT_EVENT
 */

void Anchor::SendPacket_FightEvent(s32 ev, s32 a, s32 b, const f32 v0[3], const f32 v1[3], const f32 v2[3]) {
    if (!IsSaveLoaded() || gsworld_getMap() != MAP_90_GL_BATTLEMENTS || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = FIGHT_EVENT;
    payload["ev"] = ev;
    payload["a"] = a;
    payload["b"] = b;
    if (v0 != nullptr && v1 != nullptr && v2 != nullptr) {
        payload["p"] = { v0[0], v0[1], v0[2] };
        payload["v"] = { v1[0], v1[1], v1[2] };
        payload["w"] = { v2[0], v2[1], v2[2] };
    }

    SendToCurrentMapPlayers(payload);
}

extern "C" void FightSync_SendEvent(int32_t ev, int32_t a, int32_t b, const float v0[3], const float v1[3],
                                    const float v2[3]) {
    Anchor* anchor = Anchor::GetInstance();
    if (anchor == nullptr || !anchor->isConnected) {
        return;
    }
    anchor->SendPacket_FightEvent(ev, a, b, v0, v1, v2);
}

void Anchor::HandlePacket_FightEvent(nlohmann::json& payload) {
    if (gsworld_getMap() != MAP_90_GL_BATTLEMENTS) {
        return;
    }

    s32 ev = payload.value("ev", (s32)-1);
    s32 a = payload.value("a", (s32)0);
    s32 b = payload.value("b", (s32)0);
    f32 p[3] = { 0.0f, 0.0f, 0.0f };
    f32 v[3] = { 0.0f, 0.0f, 0.0f };
    f32 w[3] = { 0.0f, 0.0f, 0.0f };
    bool hasVectors = payload.contains("p");

    if (hasVectors) {
        std::vector<f32> pv = payload["p"].get<std::vector<f32>>();
        std::vector<f32> vv = payload["v"].get<std::vector<f32>>();
        std::vector<f32> wv = payload["w"].get<std::vector<f32>>();
        if (pv.size() < 3 || vv.size() < 3 || wv.size() < 3) {
            return;
        }
        for (s32 i = 0; i < 3; i++) {
            p[i] = pv[i];
            v[i] = vv[i];
            w[i] = wv[i];
        }
    }

    FightSync_ApplyEvent(ev, a, b, hasVectors ? p : nullptr, hasVectors ? v : nullptr, hasVectors ? w : nullptr);
}
