#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/Authority.h"
#include "port/Network/Anchor/FightSync.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/ShipInit.hpp"

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

static void RegisterFightSync_Init() {
    const bool connected = Anchor::GetInstance()->isConnected;

    COND_VB_SHOULD(VB_FINAL_BOSS_BRAIN, EVENT_PRIORITY_NORMAL, connected, {
        if (FightSync_BossFollowerTick(va_arg(args, void*))) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_FINAL_BOSS_HIT, EVENT_PRIORITY_NORMAL, connected, {
        if (FightSync_ForwardBossHit(va_arg(args, s32))) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_JINJO_STATUE_EGG_HIT, EVENT_PRIORITY_NORMAL, connected, {
        const s32 statueId = va_arg(args, s32);
        const s32 padIndex = va_arg(args, s32);
        if (FightSync_ForwardEgg(statueId, padIndex)) {
            *should = false;
        }
    });

    COND_HOOK(OnFinalBossSpawned, EVENT_PRIORITY_NORMAL, true, [](IEvent*) { FightSync_OnBossSpawned(); });
    COND_HOOK(OnFinalBossDefeated, EVENT_PRIORITY_NORMAL, true, [](IEvent*) { FightSync_OnBossDefeated(); });
    COND_HOOK(OnJinjoStatueSlam, EVENT_PRIORITY_NORMAL, true,
              [](IEvent* event) { FightSync_OnJinjoSlam(((OnJinjoStatueSlam*)event)->statueId); });

    COND_HOOK(OnFinalBossSpell, EVENT_PRIORITY_NORMAL, connected,
              [](IEvent* event) { FightSync_OnSpellSpawned(((OnFinalBossSpell*)event)->kind); });
    COND_HOOK(OnFinalBossFlightPad, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        if (((OnFinalBossFlightPad*)event)->spawned) {
            FightSync_OnFlightPadSpawned();
        } else {
            FightSync_OnFlightPadDespawned();
        }
    });
    COND_HOOK(OnFinalBossBarrier, EVENT_PRIORITY_NORMAL, connected, [](IEvent*) { FightSync_OnBarrierSpawned(); });
    COND_HOOK(OnJinjoStatueSpawned, EVENT_PRIORITY_NORMAL, connected,
              [](IEvent* event) { FightSync_OnStatueSpawned(((OnJinjoStatueSpawned*)event)->statueId); });
    COND_HOOK(OnJinjoStatueEgg, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (OnJinjoStatueEgg*)event;
        FightSync_ReplicateEgg(ev->statueId, ev->padIndex);
    });
}

static RegisterShipInitFunc initFightSync(RegisterFightSync_Init, { ANCHOR_CONNECTED_PATH });
