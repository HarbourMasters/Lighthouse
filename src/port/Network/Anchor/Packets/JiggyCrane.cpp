#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"
#include "port/Network/Anchor/Puzzles/Puzzles.h"

/**
 * JIGGY_CRANE
 */

void Anchor::SendPacket_JiggyCrane(s32 stage, s32 ticks) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = JIGGY_CRANE;
    payload["stage"] = stage;
    payload["ticks"] = ticks;

    SendToCurrentMapPlayers(payload);
}

void Anchor::HandlePacket_JiggyCrane(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    AnchorPuzzles_ReplayJiggyCrane(payload.at("stage").get<s32>(), payload.at("ticks").get<s32>());
}
