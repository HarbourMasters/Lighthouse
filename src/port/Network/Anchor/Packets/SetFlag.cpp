#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"

#include "port/Patches/Patches.h"
#include "port/Rando/Rando.h"
#include "port/GameStatus.h"
#include "port/UI/Notification.h"

/**
 * SET_FLAG
 *
 * Fired when a flag bit is set (raised) in either flag space.
 */

static const char* LevelOpenSeenFlagName(s16 flag) {
    static const enum level_e kSeenFlagLevels[] = {
        LEVEL_1_MUMBOS_MOUNTAIN,     LEVEL_2_TREASURE_TROVE_COVE, LEVEL_3_CLANKERS_CAVERN,
        LEVEL_4_BUBBLEGLOOP_SWAMP,   LEVEL_5_FREEZEEZY_PEAK,      LEVEL_7_GOBIS_VALLEY,
        LEVEL_A_MAD_MONSTER_MANSION, LEVEL_9_RUSTY_BUCKET_BAY,    LEVEL_8_CLICK_CLOCK_WOOD,
    };
    if (flag >= 0x28 && flag <= 0x30) {
        return port_levelName(kSeenFlagLevels[flag - 0x28]);
    }
    if (flag == FILEPROG_E2_DOOR_OF_GRUNTY_OPEN) {
        return "the door to Gruntilda";
    }
    return nullptr;
}

void Anchor::SendPacket_SetFlag(u8 flagSpace, s16 flag) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = SET_FLAG;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["flagSpace"] = flagSpace;
    payload["flag"] = flag;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_SetFlag(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    u8 flagSpace = payload.at("flagSpace").get<u8>();
    s16 flag = payload.at("flag").get<s16>();

    if (flagSpace == ANCHOR_FLAGSPACE_RANDO_INF) {
        // Non-derived rando flag; set directly.
        if (IS_RANDO && flag > RANDO_INF_UNKNOWN && flag < RANDO_INF_MAX) {
            RANDO_SAVE_FLAGS[flag].flagState = 1;
        }
    } else if (flagSpace == ANCHOR_FLAGSPACE_VOLATILE) {
        volatileFlag_setEx((enum volatile_flags_e)flag, 1, 0);
    } else {
        bool wasSet = fileProgressFlag_get((enum file_progress_e)flag) != 0;
        fileProgressFlag_setEx((enum file_progress_e)flag, 1, 0);
        port_progressFlag_remoteCue(flag);
        // Vanilla only; rando reports world access via SET_CHECK_STATUS instead.
        if (!wasSet && !IS_RANDO && ShouldShowNotifications()) {
            if (const char* opened = LevelOpenSeenFlagName(flag)) {
                Notification::Emit({
                    .prefix = GetClientName(payload.value("clientId", 0u)),
                    .message = std::string("opened ") + opened,
                });
            }
        }
    }
}
