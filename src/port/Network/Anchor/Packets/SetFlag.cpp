#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
}

#include "port/Patches/Patches.h"
#include "port/Rando/Rando.h"

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

    SPDLOG_INFO("[Anchor][flagdiag] received SetFlag space={} flag={:#x}", flagSpace, flag);

    if (flagSpace == ANCHOR_FLAGSPACE_RANDO_INF) {
        // Non-derived rando save flag (currently only the MM bridge-repair dialog flag). Set the
        // save array directly rather than via SetRandoInfFlag so applying it doesn't re-fire the
        // event and echo back onto the wire. Check-derived RANDO_INF flags never come through
        // here — they ride SET_CHECK_STATUS and are recomputed by ModifyRandoInfFlagState.
        if (flag > RANDO_INF_UNKNOWN && flag < RANDO_INF_MAX) {
            RANDO_SAVE_FLAGS[flag].flagState = 1;
        }
    } else if (flagSpace == ANCHOR_FLAGSPACE_VOLATILE) {
        volatileFlag_setEx((enum volatile_flags_e)flag, 1, 0);
    } else {
        fileProgressFlag_setEx((enum file_progress_e)flag, 1, 0);
        // If a teammate opened a note door or broke a lair object (cobweb, brickwall, ice
        // ball, grate, etc.), replay that effect live if the matching actor is spawned in our
        // map. Both match on the unique flag, so an object elsewhere is never affected.
        port_notedoor_remoteOpen(flag);
        port_breakable_remoteBreak(flag);
    }
}
