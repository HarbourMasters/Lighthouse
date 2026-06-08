#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "variables.h"
#include "functions.h"
}

/**
 * MAP_LOAD
 *
 * Sent when the local player fully loads into a new map.
 * Receivers update the sender's map state and add/remove their dummy accordingly.
 */

void Anchor::SendPacket_MapLoad(GameMap map, s32 exit) {
    nlohmann::json payload;
    payload["type"] = MAP_LOAD;
    payload["map"] = map;
    payload["exit"] = exit;
    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_MapLoad(nlohmann::json& payload) {
    uint32_t clientId = payload.at("clientId").get<uint32_t>();
    if (!clients.contains(clientId))
        return;

    clients[clientId].map = payload.at("map").get<GameMap>();
    clients[clientId].exit = payload.at("exit").get<s32>();
    clients[clientId].isSaveLoaded = clients[clientId].map != MAP_1E_CS_START_NINTENDO &&
                                     clients[clientId].map != MAP_1F_CS_START_RAREWARE &&
                                     clients[clientId].map != MAP_91_FILE_SELECT;
    EvaluateDummyForClient(clientId);
}
