#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/Rando/Rando.h"
#include "port/Rando/ItemQueue/ItemQueue.h"

/**
 * GIVE_ITEM
 *
 * Directed grant of a single randomizer item to this client.
 *
 * SET_CHECK_STATUS mirrors a check a teammate already obtained. GIVE_ITEM instead asserts "this
 * item is yours now", which is what an external item source (a multiworld or remote randomizer
 * client) needs in order to hand items to a running game.
 *
 * Payload:
 *   randoItemId  s32  a RandoItemId satisfying RI_UNKNOWN < randoItemId < RI_MAX
 *
 * Safety:
 *   - Only applied on a loaded rando save; a vanilla save has nowhere to put a RandoItemId.
 *   - The id is range checked before use, because ItemQueue and StaticData index Items[] with it
 *     directly, so a malformed or stale packet must never reach them.
 *   - Grants are a delta (counts increment, the next free jiggy/honeycomb slot is taken) and are
 *     deliberately not deduplicated here. Delivering the same packet twice grants the item twice,
 *     and a grant cannot be taken back, so at-most-once delivery is the sender's responsibility.
 */

void Anchor::SendPacket_GiveItem(s32 randoItemId) {
    if (!IS_RANDO || !IsSaveLoaded() || isProcessingIncomingPacket || !roomState.syncItemsAndFlags) {
        return;
    }

    if (randoItemId <= RI_UNKNOWN || randoItemId >= RI_MAX) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = GIVE_ITEM;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["randoItemId"] = randoItemId;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_GiveItem(nlohmann::json& payload) {
    if (!IS_RANDO || !IsSaveLoaded()) {
        return;
    }

    // The public room is full of strangers, and this packet writes straight to the save.
    if (IsGlobalRoom()) {
        return;
    }

    // Not gated on roomState.syncItemsAndFlags: that switch governs passive world mirroring
    // between co-op peers, while this is an explicit directed grant. Requiring it would force a
    // remote item source to also opt its player into full item and flag sync.

    s32 randoItemId = payload.value("randoItemId", (s32)RI_UNKNOWN);
    if (randoItemId <= RI_UNKNOWN || randoItemId >= RI_MAX) {
        return;
    }

    ItemQueue::GiveItem((RandoItemId)randoItemId);
    // Self-gates on the rando notification CVar, same as a local pickup.
    ItemQueue::SendNotification((RandoItemId)randoItemId);
}
