#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/UI/cvar_prefixes.h"
#include "port/Rando/Rando.h"
#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Rando/StaticData/StaticData.h"

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
    // Shuffled-check obtainment only exists in a randomizer file. Never send it otherwise.
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

void Anchor::HandlePacket_SetCheckStatus(nlohmann::json& payload) {
    // Only a randomizer file has shuffled checks / a populated shuffledPool. Applying this in a
    // vanilla file would walk stale rando state (RANDO_SAVE_CHECKS, CheckObtainedEX,
    // RefreshReachableRegions) and interfere with normal collectible handling — ignore it.
    if (!IS_RANDO || !IsSaveLoaded() || !roomState.syncItemsAndFlags) {
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

    // The BGS timed-switch checks: each same-map client runs its own countdown (the timer flags are
    // sync-excluded), so when a teammate grabs the shuffled item, stop our hourglass like the
    // vanilla collect path does for the timed jiggies.
    if ((s32)gsworld_getMap() == map && (rc == RC_BGS_JIGGY_ELEVATED_WALKWAY || rc == RC_BGS_JIGGY_MAZE)) {
        func_802D6924();
    }

    // Apply the obtained state through the same funnel local collects use. isInit = true so it
    // suppresses the "you collected" notification and, crucially, doesn't re-fire
    // OnRandoCheckObtained (no echo back onto the wire).
    CustomObject::CheckObtainedEX(rc, true);

    // Re-add a notification for the remote collect (CheckObtainedEX suppressed the local one),
    // attributed to the teammate who obtained it. Gated by both the rando collection-notification
    // setting and Anchor's own notification toggle.
    if (CVarGetInteger(CVAR_RANDOMIZER_SETTING("RandoNotifications"), 0) && ShouldShowNotifications()) {
        Rando::StaticData::SendRemoteCheckNotification(rc, GetClientName(payload.value("clientId", 0u)));
    }
}
