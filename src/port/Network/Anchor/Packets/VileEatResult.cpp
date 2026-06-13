#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/VileSync.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
#include "variables.h"
}

/**
 * VILE_EAT_RESULT
 *
 * Sent by the minigame authority to confirm a successful VILE_EAT_REQUEST. The
 * requester suppressed its local consume (so its croc never played the chomp), so on
 * receipt it replays the eat feedback: the chomp animation/SFX, plus the wrong-type
 * reaction when the eaten piece didn't match the required type. Score and piece removal
 * are authority-driven and arrive via the snapshot / eaten VILE_HOLE_STATE separately.
 *
 * Broadcast to the chamber with the eater's id; only the eater acts on it. No sequence
 * number: this is one-shot cosmetic feedback, not part of the ordered state stream.
 */

void Anchor::SendPacket_VileEatResult(u32 eaterClientId, u8 pieceType, u8 correctType) {
    if (!IsSaveLoaded() || gsworld_getMap() != MAP_10_BGS_MR_VILE || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = VILE_EAT_RESULT;
    payload["eater"] = eaterClientId;
    payload["pieceType"] = pieceType;
    payload["correctType"] = correctType;

    SendToCurrentMapPlayers(payload);
}

void Anchor::HandlePacket_VileEatResult(nlohmann::json& payload) {
    if (gsworld_getMap() != MAP_10_BGS_MR_VILE) {
        return;
    }
    if (payload.value("eater", (uint32_t)0) != ownClientId) {
        return;
    }

    VileSync_PlayLocalEatFeedback(payload.value("pieceType", (s32)0), payload.value("correctType", (s32)0));
}
