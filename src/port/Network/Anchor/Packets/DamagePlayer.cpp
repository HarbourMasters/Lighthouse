#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "macros.h"
#include "functions.h"

/**
 * DAMAGE_PLAYER
 */

void Anchor::SendPacket_DamagePlayer(u32 clientId, u8 damageEffect, u8 damage) {
    if (!IsSaveLoaded()) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = DAMAGE_PLAYER;
    payload["targetClientId"] = clientId;
    payload["damageEffect"] = damageEffect;
    payload["damage"] = damage;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_DamagePlayer(nlohmann::json& payload) {
    return; // unimplemented
    uint32_t clientId = payload.at("clientId").get<uint32_t>();

    AnchorClient& anchorClient = clients[clientId];
}
