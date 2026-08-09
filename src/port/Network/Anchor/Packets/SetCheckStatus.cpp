#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/UI/cvar_prefixes.h"
#include "port/Rando/Rando.h"
#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Rando/StaticData/StaticData.h"

#include "functions.h"
extern "C" {
void marker_despawn(ActorMarker* marker);
}

Actor* FindActorByRandoCheckId(RandoCheckId randoCheckId);

/**
 * SET_CHECK_STATUS
 *
 * Fired when a shuffled rando check is first obtained; teammates mark it obtained and
 * despawn their copy if in the same map.
 */

void Anchor::SendPacket_SetCheckStatus(s32 rc, s32 map) {
    if (!IS_RANDO || !IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = SET_CHECK_STATUS;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["rc"] = rc;
    payload["map"] = map;

    SendJsonToRemote(payload);
}

// Adopt a check a teammate obtained: despawn our copy, mark obtained silently (no item grant,
// notification, or re-broadcast).
void Anchor::AdoptRemoteCheck(s32 rcRaw) {
    RandoCheckId rc = (RandoCheckId)rcRaw;
    if (rc <= RC_UNKNOWN || rc >= RC_MAX || RANDO_SAVE_CHECKS[rc].eligible) {
        return;
    }
    if (CustomObject::CheckSpawnedIdList(rc)) {
        Actor* actor = FindActorByRandoCheckId(rc);
        if (actor != NULL && actor->marker != NULL) {
            marker_despawn(actor->marker);
        }
    }
    CustomObject::CheckObtainedEX(rc, true);
}

void Anchor::HandlePacket_SetCheckStatus(nlohmann::json& payload) {
    if (!IS_RANDO || !IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    RandoCheckId rc = (RandoCheckId)payload.at("rc").get<s32>();
    s32 map = payload.at("map").get<s32>();

    if (rc <= RC_UNKNOWN || rc >= RC_MAX) {
        return;
    }
    if (RANDO_SAVE_CHECKS[rc].eligible) {
        return;
    }

    // BGS timed-switch checks cleanup
    if ((s32)gsworld_getMap() == map && (rc == RC_BGS_JIGGY_ELEVATED_WALKWAY || rc == RC_BGS_JIGGY_MAZE)) {
        func_802D6924();
    }

    AdoptRemoteCheck(rc);

    if (CVarGetInteger(CVAR_RANDOMIZER_SETTING("RandoNotifications"), 0) && ShouldShowNotifications()) {
        Rando::StaticData::SendRemoteCheckNotification(rc, GetClientName(payload.value("clientId", 0u)));
    }
}
