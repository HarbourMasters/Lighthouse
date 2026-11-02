#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"
extern "C" {
s32 item_adjustByDiff(enum item_e item, s32 diff, s32 no_hud);
}

/**
 * ITEM_COUNT
 *
 * Realtime sync of a change to a spendable item count. Jiggies and Mumbo tokens are queued;
 * consumables aren't.
 */

void Anchor::SendPacket_SetItemCount(s16 item, s32 delta) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = ITEM_COUNT;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["quiet"] = true;
    payload["item"] = item;
    payload["delta"] = delta;
    if (item == ITEM_26_JIGGY_TOTAL || item == ITEM_1C_MUMBO_TOKEN) {
        payload["addToQueue"] = true;
    }

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_SetItemCount(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 item = payload.at("item").get<s16>();
    s32 delta = payload.at("delta").get<s32>();

    if (item == ITEM_26_JIGGY_TOTAL) {
        s32 noHud = (delta > 0 || func_802FADD4(ITEM_2B_UNKNOWN)) ? 1 : 0;
        ApplyRemote([&] { item_adjustByDiff((enum item_e)item, delta, noHud); });
        return;
    }

    ApplyRemote([&] { item_adjustByDiff((enum item_e)item, delta, 0); });
}
