#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/Authority.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

/**
 * AUTHORITY_STATE
 *
 * Claim or release of authority over a network activity (see Authority.h). Broadcast to
 * the team; receivers resolve conflicting claims deterministically (lowest
 * clientId wins). The owner is the sender. With "accept", it's a receiver's reply to the
 * claim it let win, sent to the claimant alone.
 */

void Anchor::SendPacket_AuthorityState(u8 activity, bool claimed, uint32_t seq) {
    nlohmann::json payload;
    payload["type"] = AUTHORITY_STATE;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["activity"] = activity;
    payload["claimed"] = claimed;
    payload["seq"] = seq;

    SendJsonToRemote(payload);
}

void Anchor::SendPacket_AuthorityAccept(u8 activity, uint32_t claimant, uint32_t seq) {
    nlohmann::json payload;
    payload["type"] = AUTHORITY_STATE;
    payload["targetClientId"] = claimant;
    payload["activity"] = activity;
    payload["accept"] = true;
    payload["seq"] = seq;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_AuthorityState(nlohmann::json& payload) {
    uint32_t clientId = payload.at("clientId").get<uint32_t>();
    s32 activity = payload.value("activity", (s32)NET_ACTIVITY_NONE);
    if (activity <= NET_ACTIVITY_NONE || activity >= NET_ACTIVITY_COUNT) {
        return;
    }

    const uint32_t seq = payload.value("seq", 0u);
    if (payload.value("accept", false)) {
        Authority_OnAccept((NetworkActivityId)activity, clientId, seq);
        return;
    }
    Authority_ApplyRemote((NetworkActivityId)activity, clientId, payload.value("claimed", false), seq);
}
