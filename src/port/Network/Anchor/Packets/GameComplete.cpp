#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/UI/Notification.h"

const std::string gameCompleteMessages[] = {
    "killed Ganon",           "saved Zelda",         "proved their Courage",
    "collected the Triforce", "is the Hero of Time", "proved Mido wrong",
};

/**
 * GAME_COMPLETE
 *
 * unimplemented
 */

void Anchor::SendPacket_GameComplete() {
    if (!IsSaveLoaded()) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = GAME_COMPLETE;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_GameComplete(nlohmann::json& payload) {
    uint32_t clientId = payload.at("clientId").get<uint32_t>();
    if (!clients.contains(clientId)) {
        return;
    }

    AnchorClient& anchorClient = clients[clientId];
    anchorClient.isGameComplete = true;

    // Notification::Emit({
    //     .prefix = IsGlobalRoom() ? "Someone" : anchorClient.name,
    //     .message = ShipUtils::RandomElement(gameCompleteMessages),
    // });
}
