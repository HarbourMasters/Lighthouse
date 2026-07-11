#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
// gamestate.c: the core adjuster behind item_setEx, with explicit no_hud/triggerEvent control.
s32 item_adjustByDiff(enum item_e item, s32 diff, s32 no_hud, s32 triggerEvent);
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

    // A jiggy-total increase is a teammate's collect: apply it silently — the paired
    // COLLECT_ITEM packet pops the right counter on the HUD (the level tally when the collect
    // happened in our level, the file total otherwise). Decreases (pedestal spends) keep the
    // vanilla total pop, since no collect packet accompanies them.
    if (item == ITEM_26_JIGGY_TOTAL && count > item_getCount(ITEM_26_JIGGY_TOTAL)) {
        // no_hud = 1; triggerEvent = 0 so the remote apply isn't re-broadcast.
        item_adjustByDiff((enum item_e)item, count - item_getCount(ITEM_26_JIGGY_TOTAL), 1, 0);
        return;
    }

    // triggerEvent = 0: applying the remote count must not re-broadcast.
    item_setEx(item, count, 0);
}
