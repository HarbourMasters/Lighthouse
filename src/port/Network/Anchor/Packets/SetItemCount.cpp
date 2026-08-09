#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"
extern "C" {
s32 item_adjustByDiff(enum item_e item, s32 diff, s32 no_hud, s32 triggerEvent);
}

/**
 * ITEM_COUNT
 *
 * Realtime sync of a spendable item count (absolute value). Not queued.
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

    // Jiggy-total increase is a teammate's collect: apply silently (paired COLLECT_ITEM pops the
    // HUD counter). Decreases (pedestal spends) keep the vanilla total pop.
    if (item == ITEM_26_JIGGY_TOTAL && count > item_getCount(ITEM_26_JIGGY_TOTAL)) {
        item_adjustByDiff((enum item_e)item, count - item_getCount(ITEM_26_JIGGY_TOTAL), 1, 0);
        return;
    }

    item_setEx(item, count, 0);
}
