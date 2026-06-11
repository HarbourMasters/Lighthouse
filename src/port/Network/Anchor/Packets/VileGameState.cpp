#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/VileSync.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
#include "variables.h"
}

/**
 * VILE_GAME_STATE
 *
 * Periodic full snapshot of the Mr. Vile minigame, broadcast by the authority to other
 * clients in the chamber every second or two while the game is running. Application is
 * idempotent: it corrects any drift from lost packets and brings late joiners fully up
 * to date (a late joiner is just a client whose drift is 100%).
 *
 * Events (VILE_HOLE_STATE / VILE_UPDATE) make the game feel synced; this packet makes
 * it provably synced.
 */

void Anchor::SendPacket_VileGameState() {
    if (!IsSaveLoaded() || gsworld_getMap() != MAP_10_BGS_MR_VILE || GetCurrentMapPlayers() == 0) {
        return;
    }

    VileGameSnapshot snapshot;
    if (!VileSync_GatherGameSnapshot(&snapshot)) {
        return;
    }

    std::vector<u8> holeStates(VILE_HOLE_COUNT);
    std::vector<u8> holeTypes(VILE_HOLE_COUNT);
    for (s32 i = 0; i < VILE_HOLE_COUNT; i++) {
        holeStates[i] = snapshot.holes[i].state;
        holeTypes[i] = snapshot.holes[i].type;
    }

    nlohmann::json payload;
    payload["type"] = VILE_GAME_STATE;
    payload["seq"] = VileSync_NextOutgoingSeq();
    payload["gameState"] = snapshot.gameState;
    payload["round"] = snapshot.round;
    payload["maxRound"] = snapshot.maxRound;
    payload["currentType"] = snapshot.currentType;
    payload["typeTimer"] = snapshot.typeChangeTimer;
    payload["playerScore"] = snapshot.playerScore;
    payload["vileScore"] = snapshot.vileScore;
    payload["hourglass"] = snapshot.hourglassRemaining;
    payload["holeStates"] = holeStates;
    payload["holeTypes"] = holeTypes;
    payload["quiet"] = true;

    SendToCurrentMapPlayers(payload);
}

void Anchor::HandlePacket_VileGameState(nlohmann::json& payload) {
    if (gsworld_getMap() != MAP_10_BGS_MR_VILE) {
        return;
    }
    if (!VileSync_AcceptIncomingSeq(payload.value("seq", (u32)0))) {
        return;
    }

    std::vector<u8> holeStates = payload["holeStates"].get<std::vector<u8>>();
    std::vector<u8> holeTypes = payload["holeTypes"].get<std::vector<u8>>();
    if (holeStates.size() != VILE_HOLE_COUNT || holeTypes.size() != VILE_HOLE_COUNT) {
        return;
    }

    VileGameSnapshot snapshot = {};
    snapshot.gameState = payload.value("gameState", (u8)0);
    snapshot.round = payload.value("round", (u8)0);
    snapshot.maxRound = payload.value("maxRound", (u8)0);
    snapshot.currentType = payload.value("currentType", (u8)0);
    snapshot.typeChangeTimer = payload.value("typeTimer", 0.0f);
    snapshot.playerScore = payload.value("playerScore", (u8)0);
    snapshot.vileScore = payload.value("vileScore", (u8)0);
    snapshot.hourglassRemaining = payload.value("hourglass", (s32)0);
    for (s32 i = 0; i < VILE_HOLE_COUNT; i++) {
        snapshot.holes[i].state = holeStates[i];
        snapshot.holes[i].type = holeTypes[i];
    }

    VileSync_ApplyGameSnapshot(&snapshot);
}
