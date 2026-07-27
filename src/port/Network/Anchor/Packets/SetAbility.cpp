#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"
extern "C" {
void chSmBottles_netApplyTutorialComplete(void);
}

/**
 * SET_ABILITY
 *
 * Realtime sync of a learned move. Not queued.
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

    ability_setLearnedEx(move, value, 0);
    if (value) {
        ability_setHasUsed((enum ability_e)move);
    }

    // A remote unlock just completed the SM tutorial set while we're standing in Spiral
    // Mountain with a pre-completion world. Apply the completion state live.
    if (value && (s32)gsworld_getMap() == MAP_1_SM_SPIRAL_MOUNTAIN &&
        !mapSpecificFlags_get(SM_SPECIFIC_FLAG_3_ALL_SM_ABILITIES_LEARNED) &&
        chmole_learnedAllSpiralMountainAbilities()) {
        chSmBottles_netApplyTutorialComplete();
    }
}
