#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "functions.h"
#include "port/Network/Anchor/Puzzles/Puzzles.h"

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

static const char* WitchSwitchLevelName(s16 flag) {
    static const enum level_e kWitchSwitchLevels[] = {
        LEVEL_1_MUMBOS_MOUNTAIN,  LEVEL_A_MAD_MONSTER_MANSION, LEVEL_2_TREASURE_TROVE_COVE,
        LEVEL_9_RUSTY_BUCKET_BAY, LEVEL_8_CLICK_CLOCK_WOOD,    LEVEL_5_FREEZEEZY_PEAK,
        LEVEL_3_CLANKERS_CAVERN,  LEVEL_4_BUBBLEGLOOP_SWAMP,   LEVEL_7_GOBIS_VALLEY,
    };
    // The flag each switch's cutscene sets as it ends. A pressed switch sets its volatile flag again after a load
    // (func_802D4AC0).
    static const enum file_progress_e kWitchSwitchDone[] = {
        FILEPROG_18_MM_WITCH_SWITCH_JIGGY_PRESSED,  FILEPROG_19_MMM_WITCH_SWITCH_JIGGY_PRESSED,
        FILEPROG_1A_TTC_WITCH_SWITCH_JIGGY_PRESSED, FILEPROG_1C_RBB_WITCH_SWITCH_JIGGY_PRESSED,
        FILEPROG_46_CCW_WITCH_SWITCH_JIGGY_PRESSED, FILEPROG_47_FP_WITCH_SWITCH_JIGGY_PRESSED,
        FILEPROG_9A_CC_WITCH_SWITCH_PRESSED,        FILEPROG_9F_BGS_WITCH_SWITCH_JIGGY_PRESSED,
        FILEPROG_A0_GV_WITCH_SWITCH_JIGGY_PRESSED,
    };
    if (flag >= VOLATILE_FLAG_B6_WITCH_SWITCH_PRESSED_MM && flag <= VOLATILE_FLAG_BE_WITCH_SWITCH_PRESSED_GV) {
        const s32 i = flag - VOLATILE_FLAG_B6_WITCH_SWITCH_PRESSED_MM;
        return fileProgressFlag_get(kWitchSwitchDone[i]) ? nullptr : port_levelName(kWitchSwitchLevels[i]);
    }
    return nullptr;
}

static std::string ProgressFlagNews(s16 flag) {
    if (flag >= FILEPROG_3A_NOTE_DOOR_50_OPEN && flag <= FILEPROG_45_NOTE_DOOR_882_OPEN) {
        return "opened the " + std::to_string(NoteDoorNumbers_GetCost(flag - FILEPROG_3A_NOTE_DOOR_50_OPEN)) +
               " note door";
    }
    switch (flag) {
        case FILEPROG_22_WATER_SWITCH_1_PRESSED:
            return std::string("hit water switch 1 in ") + port_levelName(LEVEL_6_LAIR);
        case FILEPROG_24_WATER_SWITCH_2_PRESSED:
            return std::string("hit water switch 2 in ") + port_levelName(LEVEL_6_LAIR);
        case FILEPROG_26_WATER_SWITCH_3_PRESSED:
            return std::string("hit water switch 3 in ") + port_levelName(LEVEL_6_LAIR);
        case FILEPROG_B9_DOUBLE_HEALTH:
            return "unlocked double health";
        default:
            return "";
    }
}

// count > 1 sends a multi-bit field: values holds its bits, lowest first.
void Anchor::SendPacket_SetFlag(u8 flagSpace, s16 flag, u32 values, s32 count) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = SET_FLAG;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["flagSpace"] = flagSpace;
    payload["flag"] = flag;
    if (count > 1) {
        payload["values"] = values;
        payload["count"] = count;
    }

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_SetFlag(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    u8 flagSpace = payload.at("flagSpace").get<u8>();
    s16 flag = payload.at("flag").get<s16>();

    if (payload.contains("count")) {
        const u32 values = payload.at("values").get<u32>();
        const s32 count = payload.at("count").get<s32>();
        ApplyRemote([&] {
            for (s32 i = 0; i < count; i++) {
                const s32 bit = (values >> i) & 1;
                if (flagSpace == ANCHOR_FLAGSPACE_VOLATILE) {
                    volatileFlag_set((enum volatile_flags_e)(flag + i), bit);
                } else {
                    fileProgressFlag_set((enum file_progress_e)(flag + i), bit);
                }
            }
        });
        return;
    }

    if (flagSpace == ANCHOR_FLAGSPACE_RANDO_INF) {
        // Non-derived rando flag; set directly.
        if (IS_RANDO && flag > RANDO_INF_UNKNOWN && flag < RANDO_INF_MAX) {
            RANDO_SAVE_FLAGS[flag].flagState = 1;
        }
    } else if (flagSpace == ANCHOR_FLAGSPACE_VOLATILE) {
        bool wasSet = volatileFlag_get((enum volatile_flags_e)flag) != 0;
        ApplyRemote([&] { volatileFlag_set((enum volatile_flags_e)flag, 1); });
        if (!wasSet && ShouldShowNotifications()) {
            if (const char* where = WitchSwitchLevelName(flag)) {
                Notification::Emit({
                    .prefix = GetClientName(payload.value("clientId", 0u)),
                    .message = std::string("hit the witch switch in ") + where,
                });
            }
        }
    } else {
        bool wasSet = fileProgressFlag_get((enum file_progress_e)flag) != 0;
        ApplyRemote([&] { fileProgressFlag_set((enum file_progress_e)flag, 1); });
        // A duplicate (two players breaking the same cobweb) would replay the cue over the one already playing.
        if (!wasSet) {
            AnchorPuzzles_ReplayLairFlag(flag);
        }
        // Vanilla only; rando reports world access via SET_CHECK_STATUS instead.
        if (!wasSet && !IS_RANDO && ShouldShowNotifications()) {
            if (const char* opened = LevelOpenSeenFlagName(flag)) {
                Notification::Emit({
                    .prefix = GetClientName(payload.value("clientId", 0u)),
                    .message = std::string("opened ") + opened,
                });
            }
        }
        if (!wasSet && ShouldShowNotifications()) {
            const std::string news = ProgressFlagNews(flag);
            if (!news.empty()) {
                Notification::Emit({
                    .prefix = GetClientName(payload.value("clientId", 0u)),
                    .message = news,
                });
            }
        }
    }
}
