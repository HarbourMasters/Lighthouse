#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
}

#include "port/Patches/Patches.h"

/**
 * JIGGY_CRANE
 *
 * Live-only sync of the RBB jiggy-cage crane (a transient timed minigame). The triggering player
 * broadcasts the lower (stage 2) and, when their hourglass runs out, the raise (stage 4); same-map
 * teammates replay the crane movement silently (no camera / hourglass — see crane_jiggycage.c).
 * Nothing is persisted: the crane resets on its own, and the jiggy reward already syncs via
 * jiggyscore.
 */

void Anchor::SendPacket_JiggyCrane(s32 stage) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = JIGGY_CRANE;
    payload["stage"] = stage;

    SendToCurrentMapPlayers(payload);
}

void Anchor::HandlePacket_JiggyCrane(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    port_jiggyCrane_remoteApply(payload.at("stage").get<s32>());
}

extern "C" void port_jiggyCrane_broadcast(int32_t stage) {
    Anchor::GetInstance()->SendPacket_JiggyCrane(stage);
}
