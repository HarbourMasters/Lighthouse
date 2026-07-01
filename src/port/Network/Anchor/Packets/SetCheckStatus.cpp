#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/UI/cvar_prefixes.h"
#include "port/Rando/Rando.h"
#include "port/Rando/CustomObject/CustomObject.h"

extern "C" {
#include "functions.h"
void marker_despawn(ActorMarker* marker);
}

// Defined (C++ linkage) in ObjectBehavior.cpp. Declared here rather than in ObjectBehavior.h
// because that header is included where the Actor type isn't yet visible.
Actor* FindActorByRandoCheckId(RandoCheckId randoCheckId);

/**
 * SET_CHECK_STATUS
 *
 * Fired when a shuffled rando check is first obtained. The collector broadcasts its
 * RandoCheckId; teammates mark the check obtained (which derives its rando_inf flags via
 * ModifyRandoInfFlagState and refreshes reachable regions) and, if standing in the same
 * map, live-despawn their spawned copy of the shuffled object.
 */

void Anchor::SendPacket_SetCheckStatus(s32 rc, s32 map) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
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

void Anchor::HandlePacket_SetCheckStatus(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    RandoCheckId rc = (RandoCheckId)payload.at("rc").get<s32>();
    s32 map = payload.at("map").get<s32>();

    if (rc <= RC_UNKNOWN || rc >= RC_MAX) {
        return;
    }
    if (RANDO_SAVE_CHECKS[rc].obtained) {
        return; // already have it — nothing to apply or despawn
    }

    // Live-despawn our copy if we're in the collector's map and the object is currently
    // spawned. Guard on CheckSpawnedIdList: FindActorByRandoCheckId would otherwise spawn a
    // fresh actor just to hand it back.
    if ((s32)gsworld_getMap() == map && CustomObject::CheckSpawnedIdList(rc)) {
        Actor* actor = FindActorByRandoCheckId(rc);
        if (actor != NULL && actor->marker != NULL) {
            marker_despawn(actor->marker);
        }
    }

    // Apply the obtained state through the same funnel local collects use. isInit = true so it
    // suppresses the "you collected" notification and, crucially, doesn't re-fire
    // OnRandoCheckObtained (no echo back onto the wire).
    CustomObject::CheckObtainedEX(rc, true);
}
