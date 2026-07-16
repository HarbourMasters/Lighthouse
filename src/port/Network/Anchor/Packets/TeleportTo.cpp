#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"

// Exit id the game reserves for "spawn at an explicit position" (see func_8028E4B0).
#define EXIT_WARP_DESTINATION 0x63

/**
 * TELEPORT_TO
 *
 * See REQUEST_TELEPORT; carries the sender's live map/position/yaw.
 */

void Anchor::SendPacket_TeleportTo(uint32_t clientId) {
    if (!IsSaveLoaded()) {
        return;
    }

    f32 pos[3];
    player_getPosition(pos);

    nlohmann::json payload;
    payload["type"] = TELEPORT_TO;
    payload["targetClientId"] = clientId;
    payload["map"] = gsworld_getMap();
    payload["exit"] = gsworld_getExit();
    payload["pos"] = pos;
    payload["yaw"] = player_getYaw();

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_TeleportTo(nlohmann::json& payload) {
    if (!IsSaveLoaded()) {
        return;
    }

    GameMap map = payload.at("map").get<GameMap>();
    std::vector<f32> pos = payload.at("pos").get<std::vector<f32>>();
    f32 yaw = payload.at("yaw").get<f32>();

    if (map == (GameMap)gsworld_getMap()) {
        yaw_set(yaw);
        yaw_setIdeal(yaw);
        yaw_applyIdeal();
        func_8028F85C(pos.data());
        return;
    }

    player_setWarpDestination(pos.data(), yaw, payload.value("exit", 0));
    func_8031D04C(map, EXIT_WARP_DESTINATION);
}
