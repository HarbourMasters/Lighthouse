#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
}

#include "port/Patches/Patches.h"

/**
 * SET_FLAG
 *
 * Fired when a flag bit is set (raised) in either flag space.
 */

void Anchor::SendPacket_SetFlag(u8 flagSpace, s16 flag) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = SET_FLAG;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["flagSpace"] = flagSpace;
    payload["flag"] = flag;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_SetFlag(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    u8 flagSpace = payload.at("flagSpace").get<u8>();
    s16 flag = payload.at("flag").get<s16>();

    if (flagSpace == ANCHOR_FLAGSPACE_VOLATILE) {
        volatileFlag_setEx((enum volatile_flags_e)flag, 1, 0);
    } else {
        fileProgressFlag_setEx((enum file_progress_e)flag, 1, 0);
        // If a teammate opened a note door, animate + despawn that same door live if it's
        // spawned in our map (matched by the unique open flag, so a different door elsewhere
        // is never affected).
        port_notedoor_remoteOpen(flag);
    }
}
