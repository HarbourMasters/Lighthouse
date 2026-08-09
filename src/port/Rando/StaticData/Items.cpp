#include "StaticData.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/ShipUtils.h"
#include "port/Rando/Rando.h"
// #include "port/Rando/Logic/Logic.h"

#include "enums.h"

namespace Rando {

namespace StaticData {

#define RI(id, article, name, type, actorId, worldId)      \
    {                                                      \
        id, {                                              \
            id, #id, article, name, type, actorId, worldId \
        }                                                  \
    }

// clang-format off
std::map<RandoItemId, RandoStaticItem> Items = {
    RI(RI_UNKNOWN,                              "",     "Unknown",                  RITYPE_UNKNOWN,             ACTOR_1_UNKNOWN,                    LEVEL_D_CUTSCENE),
    RI(RI_BLUE_EGG,                             "a",    "Blue Egg",                 RITYPE_BLUE_EGG,            ACTOR_52_BLUE_EGG,                  LEVEL_D_CUTSCENE),
    RI(RI_EMPTY_HONEYCOMB_BUBBLEGLOOP_SWAMP,    "a",    "Empty Honeycomb - BGS",    RITYPE_EMPTY_HONEYCOMB,     ACTOR_47_EMPTY_HONEYCOMB,           LEVEL_4_BUBBLEGLOOP_SWAMP),
    RI(RI_EMPTY_HONEYCOMB_CLANKERS_CAVERN,      "a",    "Empty Honeycomb - CC",     RITYPE_EMPTY_HONEYCOMB,     ACTOR_47_EMPTY_HONEYCOMB,           LEVEL_3_CLANKERS_CAVERN),
    RI(RI_EMPTY_HONEYCOMB_CLICK_CLOCK_WOOD,     "a",    "Empty Honeycomb - CCW",    RITYPE_EMPTY_HONEYCOMB,     ACTOR_47_EMPTY_HONEYCOMB,           LEVEL_8_CLICK_CLOCK_WOOD),
    RI(RI_EMPTY_HONEYCOMB_FREEZEEZY_PEAK,       "a",    "Empty Honeycomb - FP",     RITYPE_EMPTY_HONEYCOMB,     ACTOR_47_EMPTY_HONEYCOMB,           LEVEL_5_FREEZEEZY_PEAK),
    RI(RI_EMPTY_HONEYCOMB_GOBIS_VALLEY,         "a",    "Empty Honeycomb - GV",     RITYPE_EMPTY_HONEYCOMB,     ACTOR_47_EMPTY_HONEYCOMB,           LEVEL_7_GOBIS_VALLEY),
    RI(RI_EMPTY_HONEYCOMB_MAD_MONSTER_MANSION,  "a",    "Empty Honeycomb - MMM",    RITYPE_EMPTY_HONEYCOMB,     ACTOR_47_EMPTY_HONEYCOMB,           LEVEL_A_MAD_MONSTER_MANSION),
    RI(RI_EMPTY_HONEYCOMB_MUMBOS_MOUNTAIN,      "a",    "Empty Honeycomb - MM",     RITYPE_EMPTY_HONEYCOMB,     ACTOR_47_EMPTY_HONEYCOMB,           LEVEL_1_MUMBOS_MOUNTAIN),
    RI(RI_EMPTY_HONEYCOMB_RUSTY_BUCKET_BAY,     "a",    "Empty Honeycomb - RBB",    RITYPE_EMPTY_HONEYCOMB,     ACTOR_47_EMPTY_HONEYCOMB,           LEVEL_9_RUSTY_BUCKET_BAY),
    RI(RI_EMPTY_HONEYCOMB_SPIRAL_MOUNTAIN,      "a",    "Empty Honeycomb - SM",     RITYPE_EMPTY_HONEYCOMB,     ACTOR_47_EMPTY_HONEYCOMB,           LEVEL_B_SPIRAL_MOUNTAIN),
    RI(RI_EMPTY_HONEYCOMB_TREASURE_TROVE_COVE,  "a",    "Empty Honeycomb - TTC",    RITYPE_EMPTY_HONEYCOMB,     ACTOR_47_EMPTY_HONEYCOMB,           LEVEL_2_TREASURE_TROVE_COVE),
    RI(RI_EXTRA_LIFE,                           "an",   "Extra Life",               RITYPE_EXTRA_LIFE,          ACTOR_49_EXTRA_LIFE,                LEVEL_D_CUTSCENE),
    RI(RI_HONEYCOMB,                            "a",    "Honeycomb",                RITYPE_HONEYCOMB,           ACTOR_50_HONEYCOMB,                 LEVEL_D_CUTSCENE),
    RI(RI_JIGGY_BUBBLEGLOOP_SWAMP,              "a",    "Jiggy - BGS",              RITYPE_JIGGY,               ACTOR_46_JIGGY,                     LEVEL_4_BUBBLEGLOOP_SWAMP),
    RI(RI_JIGGY_CLANKERS_CAVERN,                "a",    "Jiggy - CC",               RITYPE_JIGGY,               ACTOR_46_JIGGY,                     LEVEL_3_CLANKERS_CAVERN),
    RI(RI_JIGGY_CLICK_CLOCK_WOOD,               "a",    "Jiggy - CCW",              RITYPE_JIGGY,               ACTOR_46_JIGGY,                     LEVEL_8_CLICK_CLOCK_WOOD),
    RI(RI_JIGGY_FREEZEEZY_PEAK,                 "a",    "Jiggy - FP",               RITYPE_JIGGY,               ACTOR_46_JIGGY,                     LEVEL_5_FREEZEEZY_PEAK),
    RI(RI_JIGGY_GOBIS_VALLEY,                   "a",    "Jiggy - GV",               RITYPE_JIGGY,               ACTOR_46_JIGGY,                     LEVEL_7_GOBIS_VALLEY),
    RI(RI_JIGGY_GRUNTILDAS_LAIR,                "a",    "Jiggy - GL",               RITYPE_JIGGY,               ACTOR_46_JIGGY,                     LEVEL_6_LAIR),
    RI(RI_JIGGY_MAD_MONSTER_MANSION,            "a",    "Jiggy - MMM",              RITYPE_JIGGY,               ACTOR_46_JIGGY,                     LEVEL_A_MAD_MONSTER_MANSION),
    RI(RI_JIGGY_MUMBOS_MOUNTAIN,                "a",    "Jiggy - MM",               RITYPE_JIGGY,               ACTOR_46_JIGGY,                     LEVEL_1_MUMBOS_MOUNTAIN),
    RI(RI_JIGGY_RUSTY_BUCKET_BAY,               "a",    "Jiggy - RBB",              RITYPE_JIGGY,               ACTOR_46_JIGGY,                     LEVEL_9_RUSTY_BUCKET_BAY),
    RI(RI_JIGGY_TREASURE_TROVE_COVE,            "a",    "Jiggy - TTC",              RITYPE_JIGGY,               ACTOR_46_JIGGY,                     LEVEL_2_TREASURE_TROVE_COVE),
    RI(RI_JINJO_BLUE_BUBBLEGLOOP_SWAMP,         "a",    "Blue Jinjo - BGS",         RITYPE_JINJO,               ACTOR_60_JINJO_BLUE,                LEVEL_4_BUBBLEGLOOP_SWAMP),
    RI(RI_JINJO_BLUE_CLANKERS_CAVERN,           "a",    "Blue Jinjo - CC",          RITYPE_JINJO,               ACTOR_60_JINJO_BLUE,                LEVEL_3_CLANKERS_CAVERN),
    RI(RI_JINJO_BLUE_CLICK_CLOCK_WOOD,          "a",    "Blue Jinjo - CCW",         RITYPE_JINJO,               ACTOR_60_JINJO_BLUE,                LEVEL_8_CLICK_CLOCK_WOOD),
    RI(RI_JINJO_BLUE_FREEZEEZY_PEAK,            "a",    "Blue Jinjo - FP",          RITYPE_JINJO,               ACTOR_60_JINJO_BLUE,                LEVEL_5_FREEZEEZY_PEAK),
    RI(RI_JINJO_BLUE_GOBIS_VALLEY,              "a",    "Blue Jinjo - GV",          RITYPE_JINJO,               ACTOR_60_JINJO_BLUE,                LEVEL_7_GOBIS_VALLEY),
    RI(RI_JINJO_BLUE_MAD_MONSTER_MANSION,       "a",    "Blue Jinjo - MMM",         RITYPE_JINJO,               ACTOR_60_JINJO_BLUE,                LEVEL_A_MAD_MONSTER_MANSION),
    RI(RI_JINJO_BLUE_MUMBOS_MOUNTAIN,           "a",    "Blue Jinjo - MM",          RITYPE_JINJO,               ACTOR_60_JINJO_BLUE,                LEVEL_1_MUMBOS_MOUNTAIN),
    RI(RI_JINJO_BLUE_RUSTY_BUCKET_BAY,          "a",    "Blue Jinjo - RBB",         RITYPE_JINJO,               ACTOR_60_JINJO_BLUE,                LEVEL_9_RUSTY_BUCKET_BAY),
    RI(RI_JINJO_BLUE_TREASURE_TROVE_COVE,       "a",    "Blue Jinjo - TTC",         RITYPE_JINJO,               ACTOR_60_JINJO_BLUE,                LEVEL_2_TREASURE_TROVE_COVE),
    RI(RI_JINJO_GREEN_BUBBLEGLOOP_SWAMP,        "a",    "Green Jinjo - BGS",        RITYPE_JINJO,               ACTOR_62_JINJO_GREEN,               LEVEL_4_BUBBLEGLOOP_SWAMP),
    RI(RI_JINJO_GREEN_CLANKERS_CAVERN,          "a",    "Green Jinjo - CC",         RITYPE_JINJO,               ACTOR_62_JINJO_GREEN,               LEVEL_3_CLANKERS_CAVERN),
    RI(RI_JINJO_GREEN_CLICK_CLOCK_WOOD,         "a",    "Green Jinjo - CCW",        RITYPE_JINJO,               ACTOR_62_JINJO_GREEN,               LEVEL_8_CLICK_CLOCK_WOOD),
    RI(RI_JINJO_GREEN_FREEZEEZY_PEAK,           "a",    "Green Jinjo - FP",         RITYPE_JINJO,               ACTOR_62_JINJO_GREEN,               LEVEL_5_FREEZEEZY_PEAK),
    RI(RI_JINJO_GREEN_GOBIS_VALLEY,             "a",    "Green Jinjo - GV",         RITYPE_JINJO,               ACTOR_62_JINJO_GREEN,               LEVEL_7_GOBIS_VALLEY),
    RI(RI_JINJO_GREEN_MAD_MONSTER_MANSION,      "a",    "Green Jinjo - MMM",        RITYPE_JINJO,               ACTOR_62_JINJO_GREEN,               LEVEL_A_MAD_MONSTER_MANSION),
    RI(RI_JINJO_GREEN_MUMBOS_MOUNTAIN,          "a",    "Green Jinjo - MM",         RITYPE_JINJO,               ACTOR_62_JINJO_GREEN,               LEVEL_1_MUMBOS_MOUNTAIN),
    RI(RI_JINJO_GREEN_RUSTY_BUCKET_BAY,         "a",    "Green Jinjo - RBB",        RITYPE_JINJO,               ACTOR_62_JINJO_GREEN,               LEVEL_9_RUSTY_BUCKET_BAY),
    RI(RI_JINJO_GREEN_TREASURE_TROVE_COVE,      "a",    "Green Jinjo - TTC",        RITYPE_JINJO,               ACTOR_62_JINJO_GREEN,               LEVEL_2_TREASURE_TROVE_COVE),
    RI(RI_JINJO_ORANGE_BUBBLEGLOOP_SWAMP,       "a",    "Orange Jinjo - BGS",       RITYPE_JINJO,               ACTOR_5F_JINJO_ORANGE,              LEVEL_4_BUBBLEGLOOP_SWAMP),
    RI(RI_JINJO_ORANGE_CLANKERS_CAVERN,         "a",    "Orange Jinjo - CC",        RITYPE_JINJO,               ACTOR_5F_JINJO_ORANGE,              LEVEL_3_CLANKERS_CAVERN),
    RI(RI_JINJO_ORANGE_CLICK_CLOCK_WOOD,        "a",    "Orange Jinjo - CCW",       RITYPE_JINJO,               ACTOR_5F_JINJO_ORANGE,              LEVEL_8_CLICK_CLOCK_WOOD),
    RI(RI_JINJO_ORANGE_FREEZEEZY_PEAK,          "a",    "Orange Jinjo - FP",        RITYPE_JINJO,               ACTOR_5F_JINJO_ORANGE,              LEVEL_5_FREEZEEZY_PEAK),
    RI(RI_JINJO_ORANGE_GOBIS_VALLEY,            "a",    "Orange Jinjo - GV",        RITYPE_JINJO,               ACTOR_5F_JINJO_ORANGE,              LEVEL_7_GOBIS_VALLEY),
    RI(RI_JINJO_ORANGE_MAD_MONSTER_MANSION,     "a",    "Orange Jinjo - MMM",       RITYPE_JINJO,               ACTOR_5F_JINJO_ORANGE,              LEVEL_A_MAD_MONSTER_MANSION),
    RI(RI_JINJO_ORANGE_MUMBOS_MOUNTAIN,         "a",    "Orange Jinjo - MM",        RITYPE_JINJO,               ACTOR_5F_JINJO_ORANGE,              LEVEL_1_MUMBOS_MOUNTAIN),
    RI(RI_JINJO_ORANGE_RUSTY_BUCKET_BAY,        "a",    "Orange Jinjo - RBB",       RITYPE_JINJO,               ACTOR_5F_JINJO_ORANGE,              LEVEL_9_RUSTY_BUCKET_BAY),
    RI(RI_JINJO_ORANGE_TREASURE_TROVE_COVE,     "a",    "Orange Jinjo - TTC",       RITYPE_JINJO,               ACTOR_5F_JINJO_ORANGE,              LEVEL_2_TREASURE_TROVE_COVE),
    RI(RI_JINJO_PINK_BUBBLEGLOOP_SWAMP,         "a",    "Pink Jinjo - BGS",         RITYPE_JINJO,               ACTOR_61_JINJO_PINK,                LEVEL_4_BUBBLEGLOOP_SWAMP),
    RI(RI_JINJO_PINK_CLANKERS_CAVERN,           "a",    "Pink Jinjo - CC",          RITYPE_JINJO,               ACTOR_61_JINJO_PINK,                LEVEL_3_CLANKERS_CAVERN),
    RI(RI_JINJO_PINK_CLICK_CLOCK_WOOD,          "a",    "Pink Jinjo - CCW",         RITYPE_JINJO,               ACTOR_61_JINJO_PINK,                LEVEL_8_CLICK_CLOCK_WOOD),
    RI(RI_JINJO_PINK_FREEZEEZY_PEAK,            "a",    "Pink Jinjo - FP",          RITYPE_JINJO,               ACTOR_61_JINJO_PINK,                LEVEL_5_FREEZEEZY_PEAK),
    RI(RI_JINJO_PINK_GOBIS_VALLEY,              "a",    "Pink Jinjo - GV",          RITYPE_JINJO,               ACTOR_61_JINJO_PINK,                LEVEL_7_GOBIS_VALLEY),
    RI(RI_JINJO_PINK_MAD_MONSTER_MANSION,       "a",    "Pink Jinjo - MMM",         RITYPE_JINJO,               ACTOR_61_JINJO_PINK,                LEVEL_A_MAD_MONSTER_MANSION),
    RI(RI_JINJO_PINK_MUMBOS_MOUNTAIN,           "a",    "Pink Jinjo - MM",          RITYPE_JINJO,               ACTOR_61_JINJO_PINK,                LEVEL_1_MUMBOS_MOUNTAIN),
    RI(RI_JINJO_PINK_RUSTY_BUCKET_BAY,          "a",    "Pink Jinjo - RBB",         RITYPE_JINJO,               ACTOR_61_JINJO_PINK,                LEVEL_9_RUSTY_BUCKET_BAY),
    RI(RI_JINJO_PINK_TREASURE_TROVE_COVE,       "a",    "Pink Jinjo - TTC",         RITYPE_JINJO,               ACTOR_61_JINJO_PINK,                LEVEL_2_TREASURE_TROVE_COVE),
    RI(RI_JINJO_YELLOW_BUBBLEGLOOP_SWAMP,       "a",    "Yellow Jinjo - BGS",       RITYPE_JINJO,               ACTOR_5E_JINJO_YELLOW,              LEVEL_4_BUBBLEGLOOP_SWAMP),
    RI(RI_JINJO_YELLOW_CLANKERS_CAVERN,         "a",    "Yellow Jinjo - CC",        RITYPE_JINJO,               ACTOR_5E_JINJO_YELLOW,              LEVEL_3_CLANKERS_CAVERN),
    RI(RI_JINJO_YELLOW_CLICK_CLOCK_WOOD,        "a",    "Yellow Jinjo - CCW",       RITYPE_JINJO,               ACTOR_5E_JINJO_YELLOW,              LEVEL_8_CLICK_CLOCK_WOOD),
    RI(RI_JINJO_YELLOW_FREEZEEZY_PEAK,          "a",    "Yellow Jinjo - FP",        RITYPE_JINJO,               ACTOR_5E_JINJO_YELLOW,              LEVEL_5_FREEZEEZY_PEAK),
    RI(RI_JINJO_YELLOW_GOBIS_VALLEY,            "a",    "Yellow Jinjo - GV",        RITYPE_JINJO,               ACTOR_5E_JINJO_YELLOW,              LEVEL_7_GOBIS_VALLEY),
    RI(RI_JINJO_YELLOW_MAD_MONSTER_MANSION,     "a",    "Yellow Jinjo - MMM",       RITYPE_JINJO,               ACTOR_5E_JINJO_YELLOW,              LEVEL_A_MAD_MONSTER_MANSION),
    RI(RI_JINJO_YELLOW_MUMBOS_MOUNTAIN,         "a",    "Yellow Jinjo - MM",        RITYPE_JINJO,               ACTOR_5E_JINJO_YELLOW,              LEVEL_1_MUMBOS_MOUNTAIN),
    RI(RI_JINJO_YELLOW_RUSTY_BUCKET_BAY,        "a",    "Yellow Jinjo - RBB",       RITYPE_JINJO,               ACTOR_5E_JINJO_YELLOW,              LEVEL_9_RUSTY_BUCKET_BAY),
    RI(RI_JINJO_YELLOW_TREASURE_TROVE_COVE,     "a",    "Yellow Jinjo - TTC",       RITYPE_JINJO,               ACTOR_5E_JINJO_YELLOW,              LEVEL_2_TREASURE_TROVE_COVE),
    RI(RI_MOLEHILL_BARGE,                       "",     "Beak Barge",               RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_B_SPIRAL_MOUNTAIN),
    RI(RI_MOLEHILL_BEAK_BOMB,                   "",     "Beak Bomb",                RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_5_FREEZEEZY_PEAK),
    RI(RI_MOLEHILL_BEAK_BUSTER,                 "",     "Beak Buster",              RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_1_MUMBOS_MOUNTAIN),
    RI(RI_MOLEHILL_CAMERA_CONTROL,              "",     "Camera Control",           RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_B_SPIRAL_MOUNTAIN),
    RI(RI_MOLEHILL_CLAW_SWIPE,                  "",     "Claw Swipe",               RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_B_SPIRAL_MOUNTAIN),
    RI(RI_MOLEHILL_CLIMB,                       "",     "Climb",                    RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_B_SPIRAL_MOUNTAIN),
    RI(RI_MOLEHILL_DIVE,                        "",     "Dive",                     RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_B_SPIRAL_MOUNTAIN),
    RI(RI_MOLEHILL_EGGS,                        "",     "Eggs",                     RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_1_MUMBOS_MOUNTAIN),
    RI(RI_MOLEHILL_FLAP_FLIP,                   "",     "Flap Flip",                RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_B_SPIRAL_MOUNTAIN),
    RI(RI_MOLEHILL_FLIGHT,                      "",     "Flight",                   RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_2_TREASURE_TROVE_COVE),
    RI(RI_MOLEHILL_SHOCK_JUMP,                  "",     "Shock Jump",               RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_2_TREASURE_TROVE_COVE),
    RI(RI_MOLEHILL_TALON_TROT,                  "",     "Talon Trot",               RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_1_MUMBOS_MOUNTAIN),
    RI(RI_MOLEHILL_TURBO_TALON,                 "",     "Turbo Talon",              RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_7_GOBIS_VALLEY),
    RI(RI_MOLEHILL_WADING_BOOTS,                "",     "Wading Boots",             RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_4_BUBBLEGLOOP_SWAMP),
    RI(RI_MOLEHILL_WONDERWING,                  "",     "Wonderwing",               RITYPE_MOLEHILL,            ACTOR_12C_MOLEHILL,                 LEVEL_3_CLANKERS_CAVERN),
    RI(RI_MUMBO_TOKEN,                          "a",    "Mumbo Token",              RITYPE_MUMBO_TOKEN,         ACTOR_2D_MUMBO_TOKEN,               LEVEL_D_CUTSCENE),
    RI(RI_MUSIC_NOTE_BUBBLEGLOOP_SWAMP,         "a",    "Note - BGS",               RITYPE_MUSIC_NOTE,          ACTOR_51_MUSIC_NOTE,                LEVEL_4_BUBBLEGLOOP_SWAMP),
    RI(RI_MUSIC_NOTE_CLANKERS_CAVERN,           "a",    "Note - CC",                RITYPE_MUSIC_NOTE,          ACTOR_51_MUSIC_NOTE,                LEVEL_3_CLANKERS_CAVERN),
    RI(RI_MUSIC_NOTE_CLICK_CLOCK_WOOD,          "a",    "Note - CCW",               RITYPE_MUSIC_NOTE,          ACTOR_51_MUSIC_NOTE,                LEVEL_8_CLICK_CLOCK_WOOD),
    RI(RI_MUSIC_NOTE_FREEZEEZY_PEAK,            "a",    "Note - FP",                RITYPE_MUSIC_NOTE,          ACTOR_51_MUSIC_NOTE,                LEVEL_5_FREEZEEZY_PEAK),
    RI(RI_MUSIC_NOTE_GOBIS_VALLEY,              "a",    "Note - GV",                RITYPE_MUSIC_NOTE,          ACTOR_51_MUSIC_NOTE,                LEVEL_7_GOBIS_VALLEY),
    RI(RI_MUSIC_NOTE_MAD_MONSTER_MANSION,       "a",    "Note - MMM",               RITYPE_MUSIC_NOTE,          ACTOR_51_MUSIC_NOTE,                LEVEL_A_MAD_MONSTER_MANSION),
    RI(RI_MUSIC_NOTE_MUMBOS_MOUNTAIN,           "a",    "Note - MM",                RITYPE_MUSIC_NOTE,          ACTOR_51_MUSIC_NOTE,                LEVEL_1_MUMBOS_MOUNTAIN),
    RI(RI_MUSIC_NOTE_RUSTY_BUCKET_BAY,          "a",    "Note - RBB",               RITYPE_MUSIC_NOTE,          ACTOR_51_MUSIC_NOTE,                LEVEL_9_RUSTY_BUCKET_BAY),
    RI(RI_MUSIC_NOTE_TREASURE_TROVE_COVE,       "a",    "Note - TTC",               RITYPE_MUSIC_NOTE,          ACTOR_51_MUSIC_NOTE,                LEVEL_2_TREASURE_TROVE_COVE),
    RI(RI_STOP_N_SWOP_EGG_BLUE,                 "a",    "Blue Egg",                 RITYPE_SNS_EGG,             ACTOR_25E_SNS_EGG,                  LEVEL_7_GOBIS_VALLEY),
    RI(RI_STOP_N_SWOP_EGG_CYAN,                 "a",    "Cyan Egg",                 RITYPE_SNS_EGG,             ACTOR_25E_SNS_EGG,                  LEVEL_A_MAD_MONSTER_MANSION),
    RI(RI_STOP_N_SWOP_EGG_GREEN,                "a",    "Green Egg",                RITYPE_SNS_EGG,             ACTOR_25E_SNS_EGG,                  LEVEL_A_MAD_MONSTER_MANSION),
    RI(RI_STOP_N_SWOP_EGG_PINK,                 "a",    "Pink Egg",                 RITYPE_SNS_EGG,             ACTOR_25E_SNS_EGG,                  LEVEL_2_TREASURE_TROVE_COVE),
    RI(RI_STOP_N_SWOP_EGG_RED,                  "a",    "Red Egg",                  RITYPE_SNS_EGG,             ACTOR_25E_SNS_EGG,                  LEVEL_9_RUSTY_BUCKET_BAY),
    RI(RI_STOP_N_SWOP_EGG_YELLOW,               "a",    "Yellow Egg",               RITYPE_SNS_EGG,             ACTOR_25E_SNS_EGG,                  LEVEL_8_CLICK_CLOCK_WOOD),
    RI(RI_STOP_N_SWOP_ICE_KEY,                  "an",   "Ice Key",                  RITYPE_SNS_KEY,             ACTOR_25D_ICE_KEY,                  LEVEL_5_FREEZEEZY_PEAK),
    RI(RI_AP_ITEM_PROGRESSION,                  "a",    "AP Progression Item",      RITYPE_AP_ITEM,             ACTOR_3CD_CUSTOM_COLLECTIBLE,       LEVEL_D_CUTSCENE),
};
// clang-format on

RandoItemId GetRandoItemByActorId(actor_e actorId) {
    for (auto& [randoItemId, randoStaticItem] : Items) {
        if (randoStaticItem.actorId == actorId) {
            return randoItemId;
        }
    }
    return RI_UNKNOWN;
}

actor_e GetActorIdByRandoItemId(RandoItemId randoItemId) {
    for (auto& [itemId, randoStaticItem] : Items) {
        if (itemId == randoItemId) {
            return (actor_e)randoStaticItem.actorId;
        }
    }
    return ACTOR_1_UNKNOWN;
}

} // namespace StaticData
} // namespace Rando