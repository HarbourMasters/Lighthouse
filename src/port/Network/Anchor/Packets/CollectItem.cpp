#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/Enhancements/Retention/Retention.h"
#include "port/Rando/Rando.h"
#include "port/UI/Notification.h"

extern "C" {
#include "functions.h"
}

// World a jiggy belongs to, indexed by its 0-based level index ((jiggyId - 1) / 10). Jiggy ids
// are grouped 10 per level (JIGGY_ID_MULTIPLIER); ids 1..100 map to indices 0..9. Kept local so
// the notification doesn't depend on the C++-linkage worldNameList global from another TU.
static const char* const kJiggyLevelNames[10] = {
    "Mumbo's Mountain", "Treasure Trove Cove", "Clanker's Cavern",   "Bubblegloop Swamp",
    "Freezeezy Peak",   "Gruntilda's Lair",    "Gobi's Valley",      "Click Clock Wood",
    "Rusty Bucket Bay", "Mad Monster Mansion",
};

/**
 * COLLECT_ITEM
 *
 * Realtime collectible pickup (jiggy/honeycomb/Mumbo token). Sets the collected bit and, for
 * teammates in the same map, despawns the actor live. Counts ride the ITEM_COUNT packet
 * (jiggy total / token) or are recomputed from the score (honeycomb), so this never touches
 * counts directly.
 */

void Anchor::SendPacket_CollectItem(u8 kind, s32 id) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = COLLECT_ITEM;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["kind"] = kind;
    payload["id"] = id;
    payload["map"] = (s32)gsworld_getMap();

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_CollectItem(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    u8 kind = payload.at("kind").get<u8>();
    s32 id = payload.at("id").get<s32>();
    s32 map = payload.at("map").get<s32>();
    bool sameMap = (s32)gsworld_getMap() == map;

    switch (kind) {
        case ANCHOR_COLLECTIBLE_JIGGY:
            if (!jiggyscore_isCollected((enum jiggy_e)id)) {
                jiggyscore_setCollected(id, true);
                func_8034798C(); // recompute the current-level jiggy HUD count
                // Mirror the local pickup's HUD: pop the level tally when the jiggy was
                // collected in the level we're standing in, the file total otherwise. The
                // total itself arrives via ITEM_COUNT, which applies increases silently.
                if ((s32)map_getLevel((enum map_e)map) == (s32)level_get()) {
                    code_73640_printItemCount(ITEM_E_JIGGY);
                } else {
                    code_73640_printItemCount(ITEM_26_JIGGY_TOTAL);
                }
                // Vanilla only: announce the teammate's jiggy. In a randomizer, jiggies are
                // shuffled checks handled by SET_CHECK_STATUS, which posts its own notification.
                // Jiggy ids are grouped 10 per level (JIGGY_ID_MULTIPLIER), so the 0-based level
                // index is (id-1)/10 and the jiggy's number within that level is ((id-1)%10)+1.
                if (!IS_RANDO && ShouldShowNotifications()) {
                    size_t levelIdx = (size_t)(id - 1) / 10;
                    const char* where =
                        levelIdx < (sizeof(kJiggyLevelNames) / sizeof(kJiggyLevelNames[0])) ? kJiggyLevelNames[levelIdx]
                                                                                             : "an unknown level";
                    Notification::Emit({
                        .prefix = GetClientName(payload.value("clientId", 0u)),
                        .message = "collected Jiggy #" + std::to_string(((id - 1) % 10) + 1) + " in " + where,
                    });
                }
            }
            if (sameMap) {
                ActorMarker* m = func_8032B16C((enum jiggy_e)id);
                if (m != nullptr) {
                    marker_despawn(m);
                }
                // Timed jiggies: the local collector also stops the switch countdown (ba_marker.c
                // calls func_802D6924 for these ids). Mirror it so our own running hourglass — each
                // BGS client runs its own timer — doesn't keep counting after the team has the jiggy.
                if (id == JIGGY_20_BGS_ELEVATED_WALKWAY || id == JIGGY_25_BGS_MAZE) {
                    func_802D6924();
                }
            }
            break;
        case ANCHOR_COLLECTIBLE_HONEYCOMB:
            if (!honeycombscore_get((enum honeycomb_e)id)) {
                honeycombscore_set((enum honeycomb_e)id, 1);
                // Mirror the local pickup (ba_marker.c) so the empty-honeycomb carrier HUD
                // animates and every 6th honeycomb plays the health-upgrade sequence. The carrier
                // drives ITEM_15_HEALTH_TOTAL / ITEM_14_HEALTH itself, so we don't recompute here.
                // NOTE: the >= 6 case calls gcpausemenu_80314AC8(0), which locks input until the
                // carrier releases it ~1.5s later (handled the same way for the local collector).
                item_inc(ITEM_13_EMPTY_HONEYCOMB);
                if (!(item_getCount(ITEM_13_EMPTY_HONEYCOMB) < 6)) {
                    gcpausemenu_80314AC8(0);
                }
            }
            if (sameMap) {
                ActorMarker* m = actorArray_findHoneycombMarkerById((enum honeycomb_e)id);
                if (m != nullptr) {
                    marker_despawn(m);
                }
            }
            break;
        case ANCHOR_COLLECTIBLE_MUMBO:
            if (!mumboscore_get((enum mumbotoken_e)id)) {
                mumboscore_set((enum mumbotoken_e)id, true);
            }
            if (sameMap) {
                ActorMarker* m = actorArray_findMumboTokenMarkerById((enum mumbotoken_e)id);
                if (m != nullptr) {
                    marker_despawn(m);
                }
            }
            break;
        case ANCHOR_COLLECTIBLE_NOTE:
            port_noteRetention_applyRemoteCollect(map, id, sameMap ? 1 : 0);
            break;
        case ANCHOR_COLLECTIBLE_JINJO:
            port_jinjoRetention_applyRemoteCollect(map, id, sameMap ? 1 : 0);
            break;
        case ANCHOR_COLLECTIBLE_WORM:
        case ANCHOR_COLLECTIBLE_ACORN:
        case ANCHOR_COLLECTIBLE_PRESENT_BLUE:
        case ANCHOR_COLLECTIBLE_PRESENT_GREEN:
        case ANCHOR_COLLECTIBLE_PRESENT_RED:
        case ANCHOR_COLLECTIBLE_GOLD:
        case ANCHOR_COLLECTIBLE_ORANGE: {
            // Shared-pool carried collectible (CCW worms/acorns, FP presents, TTC gold). The count
            // is delta-synced through this packet so a concurrent collect and spend compose instead
            // of clobbering: id >= 0 is a pickup (+1, plus a spawn-position hash whose world object
            // we despawn); id < 0 is a spend (-1). The delta applies regardless of map (the pool is
            // the shared inventory count); only the despawn is gated on the collector's map. The
            // item isn't in Anchor_ShouldSyncItemCount, so the +/-1 won't echo. Count clamps at 0.
            enum item_e item;
            switch (kind) {
                case ANCHOR_COLLECTIBLE_WORM:          item = ITEM_22_CATERPILLAR;  break;
                case ANCHOR_COLLECTIBLE_ACORN:         item = ITEM_23_ACORNS;       break;
                case ANCHOR_COLLECTIBLE_PRESENT_BLUE:  item = ITEM_20_BLUE_PRESENT;  break;
                case ANCHOR_COLLECTIBLE_PRESENT_GREEN: item = ITEM_1F_GREEN_PRESENT; break;
                case ANCHOR_COLLECTIBLE_PRESENT_RED:   item = ITEM_21_RED_PRESENT;   break;
                case ANCHOR_COLLECTIBLE_ORANGE:        item = ITEM_19_ORANGE;        break;
                default:                               item = ITEM_18_GOLD_BULLIONS; break;
            }
            // [port] The orange is MM-local (carried to Chimpy in that map). Sync its shared count
            // everywhere as usual, but don't pop the HUD for a teammate who isn't in that map
            // (sameMap) — only the in-map players should see Chimpy's orange count change.
            bool noHud = (kind == ANCHOR_COLLECTIBLE_ORANGE) && !sameMap;
            s32 delta = (id < 0) ? -1 : 1;
            if (noHud) {
                item_adjustByDiffWithoutHud(item, delta);
            } else {
                item_adjustByDiffWithHud(item, delta);
            }
            if (id >= 0) {
                port_carriedSync_applyRemoteCollect(kind, map, id, sameMap ? 1 : 0);
            }
            break;
        }
    }
}
