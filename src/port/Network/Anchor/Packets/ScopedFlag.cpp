#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/Rando/Rando.h"
#include "port/UI/Notification.h"

extern "C" {
#include "functions.h"
// honeycomb.c: spawn the switch-revealed honeycomb (GV cactus / RBB boat house) if its just-applied
// map flag says a teammate pressed the switch — the actor itself never rides the wire.
void chHoneycomb_netRevealFromSwitch(void);
}

// Display name for a world-open level flag set from the Lair jigsaw podiums (jigsawpicture.c
// afterPictureComplete), or nullptr for any other level-specific flag. Used to announce a
// teammate opening a world; the flags are level-scoped, so only Lair teammates receive them.
static const char* LevelOpenFlagName(s32 index) {
    switch (index) {
        case LEVEL_FLAG_1C_MM_OPEN:               return "Mumbo's Mountain";
        case LEVEL_FLAG_1D_TTC_OPEN:              return "Treasure Trove Cove";
        case LEVEL_FLAG_1E_CC_OPEN:               return "Clanker's Cavern";
        case LEVEL_FLAG_1F_BGS_OPEN:              return "Bubblegloop Swamp";
        case LEVEL_FLAG_20_FP_OPEN:               return "Freezeezy Peak";
        case LEVEL_FLAG_21_GV_OPEN:               return "Gobi's Valley";
        case LEVEL_FLAG_22_MMM_OPEN:              return "Mad Monster Mansion";
        case LEVEL_FLAG_23_RBB_OPEN:              return "Rusty Bucket Bay";
        case LEVEL_FLAG_24_CCW_OPEN:              return "Click Clock Wood";
        case LEVEL_FLAG_3F_LAIR_GRUNTY_DOOR_OPEN: return "the door to Gruntilda";
        default:                                  return nullptr;
    }
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
            bool wasSet = levelSpecificFlags_get(index) != 0;
            levelSpecificFlags_setEx(index, value, 0);
            // Vanilla only: announce a teammate opening a world at the Lair puzzle podiums. In a
            // randomizer, world access is shuffled and reported via SET_CHECK_STATUS instead.
            if (value && !wasSet && !IS_RANDO && ShouldShowNotifications()) {
                if (const char* opened = LevelOpenFlagName(index)) {
                    Notification::Emit({
                        .prefix = GetClientName(payload.value("clientId", 0u)),
                        .message = std::string("opened ") + opened,
                    });
                }
            }
        }
    } else if (space == ANCHOR_FLAGSPACE_MAP_SPECIFIC) {
        if ((s32)gsworld_getMap() == ctx) {
            mapSpecificFlags_setEx(index, value, 0);
            chHoneycomb_netRevealFromSwitch();
        }
    }
}
