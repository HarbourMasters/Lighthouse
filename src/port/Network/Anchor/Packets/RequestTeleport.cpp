#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

/**
 * REQUEST_TELEPORT
 *
 * Asks a client for their location; they reply with TELEPORT_TO (map, position, yaw).
 */

void Anchor::SendPacket_RequestTeleport(uint32_t clientId) {
    if (!CanTeleportTo(clientId)) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = REQUEST_TELEPORT;
    payload["targetClientId"] = clientId;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_RequestTeleport(nlohmann::json& payload) {
    if (!IsSaveLoaded()) {
        return;
    }

    uint32_t clientId = payload.at("clientId").get<uint32_t>();
    SendPacket_TeleportTo(clientId);
}

bool Anchor::CanTeleportTo(uint32_t clientId) {
    if (roomState.teleportMode == 0) {
        return false;
    }

    if (!IsSaveLoaded()) {
        return false;
    }

    if (clients.find(clientId) == clients.end()) {
        return false;
    }

    AnchorClient& client = clients[clientId];

    if (client.self) {
        return false;
    }

    if (!client.online || !client.isSaveLoaded) {
        return false;
    }

    std::string ownTeamId = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    if (roomState.teleportMode == 1 && client.teamId != ownTeamId) {
        return false;
    }

    return true;
}
