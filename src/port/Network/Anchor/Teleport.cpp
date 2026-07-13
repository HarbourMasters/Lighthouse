#include "port/Network/Anchor/Anchor.h"
#include <libultraship/libultraship.h>

extern "C" {
void func_8031D04C(enum map_e arg0, s32 exit_id);
}

/**
 * Teleport to player
 *
 * Every client's current map and entry exit already ride MAP_LOAD / ALL_CLIENT_STATE, so no
 * packet round trip is needed: dispatch a warp to the target's map and entry exit through the
 * game's warp system, the same way the dev tools warp selector does.
 */

void Anchor::TeleportToClient(uint32_t clientId) {
    if (!CanTeleportTo(clientId)) {
        return;
    }

    AnchorClient& client = clients[clientId];
    func_8031D04C(client.map, client.exit);
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
