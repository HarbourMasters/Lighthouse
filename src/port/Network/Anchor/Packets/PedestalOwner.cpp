#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/JigsawPedestal.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

/**
 * PEDESTAL_OWNER
 *
 * Claim/release of a jigsaw podium lock; conflicts resolve by lowest clientId.
 */

void Anchor::SendPacket_PedestalOwner(s32 id, bool claimed) {
    nlohmann::json payload;
    payload["type"] = PEDESTAL_OWNER;
    payload["id"] = id;
    payload["claimed"] = claimed;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_PedestalOwner(nlohmann::json& payload) {
    uint32_t clientId = payload.at("clientId").get<uint32_t>();
    s32 id = payload.at("id").get<s32>();
    bool claimed = payload.value("claimed", false);

    JigsawPedestal_ApplyRemote(id, clientId, claimed);
}
