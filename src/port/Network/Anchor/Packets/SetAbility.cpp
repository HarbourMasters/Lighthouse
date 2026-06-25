#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
}

/**
 * SET_ABILITY
 *
 * Realtime sync of a learned move (absolute value). Not queued — team state converges the
 * full ability bitfield on connect/save anyway, so offline teammates pick it up on reconnect.
 */

void Anchor::SendPacket_SetAbility(s16 move, u8 value) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = SET_ABILITY;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["quiet"] = true;
    payload["move"] = move;
    payload["value"] = value;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_SetAbility(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 move = payload.at("move").get<s16>();
    u8 value = payload.at("value").get<u8>();

    // triggerEvent = 0: applying the remote learn must not re-broadcast.
    ability_setLearnedEx(move, value, 0);
    // Mark the move used too (matching team state's used-bytes copy), so the receiver
    // doesn't get a first-use tutorial/ding for an ability they didn't earn locally.
    if (value) {
        ability_setHasUsed((enum ability_e)move);
    }
}
