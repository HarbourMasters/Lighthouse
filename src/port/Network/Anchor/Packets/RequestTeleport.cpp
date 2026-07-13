#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

/**
 * REQUEST_TELEPORT
 *
 * Asks another client for their live location. They respond with a TELEPORT_TO packet
 * carrying their current map, position, and yaw, which we use to warp directly to them.
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

// Reusable function to check if teleporting to a client is allowed
bool Anchor::CanTeleportTo(uint32_t clientId) {
    // Teleporting is disabled
    if (roomState.teleportMode == 0) {
        return false;
    }

    // You're not loaded into a save
    if (!IsSaveLoaded()) {
        return false;
    }

    // The client doesn't exist
    if (clients.find(clientId) == clients.end()) {
        return false;
    }

    AnchorClient& client = clients[clientId];

    // The client is yourself
    if (client.self) {
        return false;
    }

    // The client isn't online or loaded into a save
    if (!client.online || !client.isSaveLoaded) {
        return false;
    }

    // Teleporting to team only, but the client is not on your team
    std::string ownTeamId = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    if (roomState.teleportMode == 1 && client.teamId != ownTeamId) {
        return false;
    }

    return true;
}
