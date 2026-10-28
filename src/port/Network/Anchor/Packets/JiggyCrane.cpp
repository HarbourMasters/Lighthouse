#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/ShipInit.hpp"

#include "functions.h"

#include "port/Patches/Patches.h"

/**
 * JIGGY_CRANE
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

static void RegisterJiggyCrane_Init() {
    COND_HOOK(OnJiggyCageStateChange, EVENT_PRIORITY_NORMAL, Anchor::GetInstance()->isConnected, [](IEvent* event) {
        Anchor::GetInstance()->SendPacket_JiggyCrane(((OnJiggyCageStateChange*)event)->state);
    });
}

static RegisterShipInitFunc initJiggyCrane(RegisterJiggyCrane_Init, { ANCHOR_CONNECTED_PATH });
