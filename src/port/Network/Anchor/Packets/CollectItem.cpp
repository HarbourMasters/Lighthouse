#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/Enhancements/Retention/Retention.h"
#include "port/Rando/Rando.h"
#include "port/UI/Notification.h"

#include "functions.h"
#include "port/ShipUtils.h"

static const char* const kJiggyLevelNames[10] = {
    "Mumbo's Mountain", "Treasure Trove Cove", "Clanker's Cavern", "Bubblegloop Swamp", "Freezeezy Peak",
    "Gruntilda's Lair", "Gobi's Valley",       "Click Clock Wood", "Rusty Bucket Bay",  "Mad Monster Mansion",
};

/**
 * COLLECT_ITEM
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
            if (!port_jiggyscore_isCollectedRaw((enum jiggy_e)id)) {
                jiggyscore_setCollected(id, true);
                func_8034798C(); // recompute the current-level jiggy HUD count
                if ((s32)map_getLevel((enum map_e)map) == (s32)level_get()) {
                    code_73640_printItemCount(ITEM_E_JIGGY);
                } else {
                    code_73640_printItemCount(ITEM_26_JIGGY_TOTAL);
                }
                if (!IS_RANDO && ShouldShowNotifications()) {
                    size_t levelIdx = (size_t)(id - 1) / 10;
                    const char* where = levelIdx < (sizeof(kJiggyLevelNames) / sizeof(kJiggyLevelNames[0]))
                                            ? kJiggyLevelNames[levelIdx]
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
                if (id == JIGGY_20_BGS_ELEVATED_WALKWAY || id == JIGGY_25_BGS_MAZE) {
                    func_802D6924();
                }
            }
            break;
        case ANCHOR_COLLECTIBLE_HONEYCOMB:
            if (!port_honeycombscore_getRaw((enum honeycomb_e)id)) {
                honeycombscore_set((enum honeycomb_e)id, 1);
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
            enum item_e item;
            switch (kind) {
                case ANCHOR_COLLECTIBLE_WORM:
                    item = ITEM_22_CATERPILLAR;
                    break;
                case ANCHOR_COLLECTIBLE_ACORN:
                    item = ITEM_23_ACORNS;
                    break;
                case ANCHOR_COLLECTIBLE_PRESENT_BLUE:
                    item = ITEM_20_BLUE_PRESENT;
                    break;
                case ANCHOR_COLLECTIBLE_PRESENT_GREEN:
                    item = ITEM_1F_GREEN_PRESENT;
                    break;
                case ANCHOR_COLLECTIBLE_PRESENT_RED:
                    item = ITEM_21_RED_PRESENT;
                    break;
                case ANCHOR_COLLECTIBLE_ORANGE:
                    item = ITEM_19_ORANGE;
                    break;
                default:
                    item = ITEM_18_GOLD_BULLIONS;
                    break;
            }
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
