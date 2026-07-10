#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
// honeycomb.c: spawn the switch-revealed honeycomb (GV cactus / RBB boat house) if its just-applied
// map flag says a teammate pressed the switch — the actor itself never rides the wire.
void chHoneycomb_netRevealFromSwitch(void);
}

/**
 * SCOPED_FLAG
 *
 * Realtime sync of a transient level-/map-specific flag. Routed only to teammates in the same
 * level (level space) or map (map space); never queued. ctx = the sender's level/map id, so a
 * receiver that has since moved doesn't apply a stale flag.
 */

void Anchor::SendPacket_ScopedFlag(u8 space, s16 index, u8 value) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = SCOPED_FLAG;
    payload["space"] = space;
    payload["index"] = index;
    payload["value"] = value;

    if (space == ANCHOR_FLAGSPACE_LEVEL_SPECIFIC) {
        payload["ctx"] = (s32)map_getLevel(gsworld_getMap());
        SendToCurrentLevelPlayers(payload);
    } else {
        payload["ctx"] = (s32)gsworld_getMap();
        SendToCurrentMapPlayers(payload);
    }
}

void Anchor::HandlePacket_ScopedFlag(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    u8 space = payload.at("space").get<u8>();
    s16 index = payload.at("index").get<s16>();
    u8 value = payload.at("value").get<u8>();
    s32 ctx = payload.at("ctx").get<s32>();

    // triggerEvent = 0: applying a remote flag must not re-broadcast.
    if (space == ANCHOR_FLAGSPACE_LEVEL_SPECIFIC) {
        if ((s32)map_getLevel(gsworld_getMap()) == ctx) {
            levelSpecificFlags_setEx(index, value, 0);
        }
    } else if (space == ANCHOR_FLAGSPACE_MAP_SPECIFIC) {
        if ((s32)gsworld_getMap() == ctx) {
            mapSpecificFlags_setEx(index, value, 0);
            chHoneycomb_netRevealFromSwitch();
        }
    }
}
