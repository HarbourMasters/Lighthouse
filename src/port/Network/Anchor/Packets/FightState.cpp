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
 * FIGHT_STATE
 */

void Anchor::SendPacket_FightState(u32 targetClientId) {
    if (!IsSaveLoaded() || gsworld_getMap() != MAP_90_GL_BATTLEMENTS) {
        return;
    }

    FightWorldSnapshot snap;
    if (!FightSync_GatherWorld(&snap)) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = FIGHT_STATE;
    payload["targetClientId"] = targetClientId;
    payload["pad"] = snap.pad;
    payload["bar"] = snap.barrier;
    payload["st"] = { snap.statue[0], snap.statue[1], snap.statue[2], snap.statue[3] };
    payload["jg"] = { snap.jinjoGone[0], snap.jinjoGone[1], snap.jinjoGone[2], snap.jinjoGone[3] };
    payload["jb"] = snap.jbase;
    payload["jp"] = { snap.jpads[0], snap.jpads[1], snap.jpads[2], snap.jpads[3] };

    SendJsonToRemote(payload);
}

extern "C" void FightSync_SendSnapshot(uint32_t clientId) {
    Anchor* anchor = Anchor::GetInstance();
    if (anchor == nullptr || !anchor->isConnected) {
        return;
    }
    anchor->SendPacket_FightState(clientId);
}

void Anchor::HandlePacket_FightState(nlohmann::json& payload) {
    if (gsworld_getMap() != MAP_90_GL_BATTLEMENTS) {
        return;
    }

    FightWorldSnapshot snap;
    std::vector<u8> st = payload["st"].get<std::vector<u8>>();
    std::vector<u8> jg = payload["jg"].get<std::vector<u8>>();
    std::vector<u8> jp = payload["jp"].get<std::vector<u8>>();
    if (st.size() < 4 || jg.size() < 4 || jp.size() < 4) {
        return;
    }

    snap.pad = payload.value("pad", (u8)0);
    snap.barrier = payload.value("bar", (u8)0);
    snap.jbase = payload.value("jb", (u8)0);
    for (s32 i = 0; i < 4; i++) {
        snap.statue[i] = st[i];
        snap.jinjoGone[i] = jg[i];
        snap.jpads[i] = jp[i];
    }

    FightSync_ApplyWorld(&snap);
}
