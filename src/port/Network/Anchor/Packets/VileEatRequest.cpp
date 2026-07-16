#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/VileSync.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"
#include "variables.h"

/**
 * VILE_EAT_REQUEST
 *
 * Sent by a non-authority client when its local player chomps a piece. The minigame
 * authority validates the request against the live piece list and, on success, consumes
 * the piece and broadcasts the result as a VILE_HOLE_STATE (eaten) packet.
 */

void Anchor::SendPacket_VileEatRequest(u8 holeId) {
    if (!IsSaveLoaded() || gsworld_getMap() != MAP_10_BGS_MR_VILE || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = VILE_EAT_REQUEST;
    payload["holeId"] = holeId;

    SendToCurrentMapPlayers(payload);
}

void Anchor::HandlePacket_VileEatRequest(nlohmann::json& payload) {
    if (gsworld_getMap() != MAP_10_BGS_MR_VILE) {
        return;
    }

    s32 holeId = payload.value("holeId", (s32)VILE_HOLE_NONE);
    if (holeId <= VILE_HOLE_NONE || holeId >= VILE_HOLE_COUNT) {
        return;
    }

    uint32_t eaterClientId = payload.at("clientId").get<uint32_t>();
    s32 pieceType = 0;
    s32 correctType = 0;
    if (VileSync_HandleEatRequest(holeId, eaterClientId, &pieceType, &correctType)) {
        SendPacket_VileEatResult(eaterClientId, (u8)pieceType, (u8)correctType);
    }
}
