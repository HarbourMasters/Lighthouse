#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/UI/cvar_prefixes.h"
#include "port/Rando/Rando.h"
#include "port/Rando/ItemQueue/ItemQueue.h"

/**
 * GIVE_ITEM
 *
 * Hands a single item to this client. SET_CHECK_STATUS mirrors a check a teammate already
 * obtained; GIVE_ITEM instead says "this item is yours now", which is what an external item source
 * needs when it decides an item belongs to this player.
 *
 * Payload (unchanged from the packet this file inherited, so existing senders keep working):
 *   modId      u16  which item table getItemId belongs to
 *                     0  vanilla item table, not implemented here, ignored
 *                     1  randomizer item table, getItemId is a RandoItemId
 *   getItemId  s16  the item id, read according to modId
 *
 * Only the randomizer table is implemented. A packet with any other modId is dropped rather than
 * guessed at.
 *
 * Safety:
 *   - Guarded like every other world packet: rando save loaded and room item sync on. That also
 *     covers the public global room, where HandlePacket_UpdateRoomState forces syncItemsAndFlags
 *     to 0 and OnIncomingJson then drops GIVE_ITEM before it reaches this handler.
 *   - getItemId is range checked before use, because ItemQueue and StaticData index Items[] with
 *     it directly, so a malformed or stale packet must never reach them.
 *   - A grant is a delta: counts increment, and the next free jiggy or honeycomb slot is taken.
 *     Nothing here deduplicates, so the same packet delivered twice grants the item twice, and a
 *     grant cannot be taken back. At-most-once delivery is the sender's responsibility.
 */

// Which item table modId selects. The values match the sender this packet came from.
static constexpr u16 GIVE_ITEM_MOD_VANILLA = 0;
static constexpr u16 GIVE_ITEM_MOD_RANDOMIZER = 1;

void Anchor::SendPacket_GiveItem(u16 modId, s16 getItemId) {
    if (!IsSaveLoaded() || isProcessingIncomingPacket || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = GIVE_ITEM;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["modId"] = modId;
    payload["getItemId"] = getItemId;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_GiveItem(nlohmann::json& payload) {
    if (!IS_RANDO || !IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    u16 modId = payload.value("modId", (u16)GIVE_ITEM_MOD_VANILLA);
    if (modId != GIVE_ITEM_MOD_RANDOMIZER) {
        return;
    }

    // Read into s32 so a value outside the RandoItemId range is rejected below rather than
    // wrapping into a valid id.
    s32 getItemId = payload.value("getItemId", (s32)RI_UNKNOWN);
    if (getItemId <= RI_UNKNOWN || getItemId >= RI_MAX) {
        return;
    }

    ItemQueue::GiveItem((RandoItemId)getItemId);
    // Self-gates on the rando notification CVar, same as a local pickup.
    ItemQueue::SendNotification((RandoItemId)getItemId);
}
