#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/Authority.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

/**
 * AUTHORITY_STATE
 *
 * Claim or release of authority over a network activity (see Authority.h). Broadcast to
 * the whole room; receivers resolve conflicting claims deterministically (lowest
 * clientId wins), so no acknowledgement is needed. The owner is the sender.
 */

void Anchor::SendPacket_AuthorityState(u8 activity, bool claimed) {
    nlohmann::json payload;
    payload["type"] = AUTHORITY_STATE;
    payload["activity"] = activity;
    payload["claimed"] = claimed;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_AuthorityState(nlohmann::json& payload) {
    uint32_t clientId = payload.at("clientId").get<uint32_t>();
    s32 activity = payload.value("activity", (s32)NET_ACTIVITY_NONE);
    if (activity <= NET_ACTIVITY_NONE || activity >= NET_ACTIVITY_COUNT) {
        return;
    }

    Authority_ApplyRemote((NetworkActivityId)activity, clientId, payload.value("claimed", false));
}
