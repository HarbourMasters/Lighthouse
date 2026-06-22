#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
}

/**
 * ITEM_COUNT
 *
 * Realtime sync of a spendable item count (absolute value). Not queued — high-frequency,
 * and team state converges counts on connect/save anyway.
 */

void Anchor::SendPacket_SetItemCount(s16 item, s32 count) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = ITEM_COUNT;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["quiet"] = true;
    payload["item"] = item;
    payload["count"] = count;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_SetItemCount(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 item = payload.at("item").get<s16>();
    s32 count = payload.at("count").get<s32>();

    // triggerEvent = 0: applying the remote count must not re-broadcast.
    item_setEx(item, count, 0);
}
