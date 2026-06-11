#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/VileSync.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
#include "variables.h"
}

/**
 * VILE_HOLE_STATE
 *
 * Sent by the minigame authority when a yumblie/grumblie hole changes state
 * (appear, hide, eaten). Only sent to clients in Mr. Vile's chamber.
 *
 * For the eaten state, eater identifies who consumed the piece
 * (VILE_EATER_MR_VILE = Mr. Vile, otherwise a clientId).
 */

void Anchor::SendPacket_VileHoleState(u8 holeId, u8 holeState, u8 pieceType, u32 eaterClientId) {
    if (!IsSaveLoaded() || gsworld_getMap() != MAP_10_BGS_MR_VILE || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = VILE_HOLE_STATE;
    payload["seq"] = VileSync_NextOutgoingSeq();
    payload["holeId"] = holeId;
    payload["holeState"] = holeState;
    payload["pieceType"] = pieceType;
    payload["eater"] = eaterClientId;

    SendToCurrentMapPlayers(payload);
}

void Anchor::HandlePacket_VileHoleState(nlohmann::json& payload) {
    if (gsworld_getMap() != MAP_10_BGS_MR_VILE) {
        return;
    }
    if (!VileSync_AcceptIncomingSeq(payload.value("seq", (u32)0))) {
        return;
    }

    s32 holeId = payload.value("holeId", (s32)VILE_HOLE_NONE);
    if (holeId <= VILE_HOLE_NONE || holeId >= VILE_HOLE_COUNT) {
        return;
    }

    VileSync_ApplyHoleState(holeId, payload.value("holeState", (s32)0), payload.value("pieceType", (s32)0),
                            payload.value("eater", VILE_EATER_MR_VILE));
}
