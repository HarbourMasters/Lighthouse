#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/Enhancements/Retention/Retention.h"

extern "C" {
#include "functions.h"
}

/**
 * COLLECT_ITEM
 *
 * Realtime collectible pickup (jiggy/honeycomb/Mumbo token). Sets the collected bit and, for
 * teammates in the same map, despawns the actor live. Counts ride the ITEM_COUNT packet
 * (jiggy total / token) or are recomputed from the score (honeycomb), so this never touches
 * counts directly.
 */

void Anchor::SendPacket_CollectItem(u8 kind, s16 id) {
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
    s16 id = payload.at("id").get<s16>();
    s32 map = payload.at("map").get<s32>();
    bool sameMap = (s32)gsworld_getMap() == map;

    switch (kind) {
        case ANCHOR_COLLECTIBLE_JIGGY:
            if (!jiggyscore_isCollected((enum jiggy_e)id)) {
                jiggyscore_setCollected(id, true);
                func_8034798C(); // recompute the current-level jiggy HUD count
            }
            if (sameMap) {
                ActorMarker* m = func_8032B16C((enum jiggy_e)id);
                if (m != nullptr) {
                    marker_despawn(m);
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
    }
}
