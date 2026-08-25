#include "WarpCatalog.h"
#include "MapNames.h"

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/UI/LighthouseGui.hpp"
#include "port/UI/UIWidgets.hpp"

#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <imgui.h>
#include <libultraship/libultraship.h>
#include <nlohmann/json.hpp>
#include <spdlog/fmt/fmt.h>

#include "enums.h"
#include "functions.h"

namespace {

// One catalogued destination. level is stored rather than derived so a hand-edited
// file can never feed a bogus map to map_getLevel, which dereferences a null lookup.
struct WarpEntry {
    int32_t map = 0;
    int32_t exit = 0;
    int32_t level = 0;
    std::string name;
    bool builtIn = false;
};

// A map load as it happened, kept so an entrance can be named after arriving.
struct Arrival {
    int32_t fromMap = 0;
    int32_t map = 0;
    int32_t exit = 0;
};

constexpr size_t kArrivalHistory = 16;
int32_t mapId = 0;
int32_t exitId = 0;

const char* mapNames[] = {
    "Mumbo's Mountain", "Treasure Trove Cove", "Clanker's Cavern", "Bubblegloop Swamp",   "Freezeezy Peak",
    "Gobi's Valley",    "Click Clock Wood",    "Rusty Bucket Bay", "Mad Monster Mansion", "Spiral Mountain",
    "Cutscene",         "Gruntilda's Lair",    "Boss Arena",
};

std::vector<int32_t> mapIdList = {
    MAP_2_MM_MUMBOS_MOUNTAIN,
    MAP_7_TTC_TREASURE_TROVE_COVE,
    MAP_B_CC_CLANKERS_CAVERN,
    MAP_D_BGS_BUBBLEGLOOP_SWAMP,
    MAP_27_FP_FREEZEEZY_PEAK,
    MAP_12_GV_GOBIS_VALLEY,
    MAP_40_CCW_HUB,
    MAP_31_RBB_RUSTY_BUCKET_BAY,
    MAP_1B_MMM_MAD_MONSTER_MANSION,
    MAP_1_SM_SPIRAL_MOUNTAIN,
    MAP_7B_CS_INTRO_GL_DINGPOT_1,
    MAP_69_GL_MM_LOBBY,
    MAP_90_GL_BATTLEMENTS,
};

// clang-format off
// Generated from the map_warp_* enums in enums.h. Entrance indices are written as
// their enum constants; the enum tags were matched to maps against the warp
// destinations in cutscene_skip.c, since the two naming schemes don't always agree.
// Indices with no constant (demo slots, Furnace Fun) stay as literals.
const WarpEntry kBuiltInWarps[] = {
    // LEVEL_1_MUMBOS_MOUNTAIN
    { MAP_2_MM_MUMBOS_MOUNTAIN, WARP_MM_1_MUMBOS_HUT, LEVEL_1_MUMBOS_MOUNTAIN, "Outside Mumbo's Skull", true },
    { MAP_2_MM_MUMBOS_MOUNTAIN, WARP_MM_2_TOWER_BOTTOM, LEVEL_1_MUMBOS_MOUNTAIN, "Outside Ticker's Bottom", true },
    { MAP_2_MM_MUMBOS_MOUNTAIN, WARP_MM_3_TOWER_TOP, LEVEL_1_MUMBOS_MOUNTAIN, "Outside Ticker's Top", true },
    { MAP_2_MM_MUMBOS_MOUNTAIN, WARP_MM_4_WITCH_SWITCH, LEVEL_1_MUMBOS_MOUNTAIN, "Witch Switch Return", true },
    { MAP_2_MM_MUMBOS_MOUNTAIN, WARP_MM_5_WORLD_ENTRACE, LEVEL_1_MUMBOS_MOUNTAIN, "Warp Pad", true },
    { MAP_2_MM_MUMBOS_MOUNTAIN, 91, LEVEL_1_MUMBOS_MOUNTAIN, "Demo", true },
    { MAP_C_MM_TICKERS_TOWER, WARP_MM_TICKERS_TOWER_1_TOP, LEVEL_1_MUMBOS_MOUNTAIN, "Ticker's Tower Top", true },
    { MAP_C_MM_TICKERS_TOWER, WARP_MM_TICKERS_TOWER_2_BOTTOM, LEVEL_1_MUMBOS_MOUNTAIN, "Ticker's Tower Bottom", true },
    { MAP_E_MM_MUMBOS_SKULL, WARP_MM_MUMBOS_HUT_1_ENTRANCE, LEVEL_1_MUMBOS_MOUNTAIN, "Mumbo's Skull", true },
    // LEVEL_2_TREASURE_TROVE_COVE
    { MAP_5_TTC_BLUBBERS_SHIP, WARP_TCC_BLUBBERS_SHIP_5_TOP_HATCH, LEVEL_2_TREASURE_TROVE_COVE, "Crate", true },
    { MAP_5_TTC_BLUBBERS_SHIP, WARP_TCC_BLUBBERS_SHIP_6_SIDE_HATCH, LEVEL_2_TREASURE_TROVE_COVE, "Underwater", true },
    { MAP_6_TTC_NIPPERS_SHELL, WARP_TCC_NIPPERS_SHELL_1_ENTRANCE, LEVEL_2_TREASURE_TROVE_COVE, "Entrance", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_1_LEAKY, LEVEL_2_TREASURE_TROVE_COVE, "Leaky Cutscene Return", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_3_SANDCASTLE, LEVEL_2_TREASURE_TROVE_COVE, "Outside Sandcastle", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_4_WORLD_ENTRANCE, LEVEL_2_TREASURE_TROVE_COVE, "Warp Pad", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_6_SHIP_TOP_HATCH, LEVEL_2_TREASURE_TROVE_COVE, "Ship Deck", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_7_SHIP_SIDE_HATCH, LEVEL_2_TREASURE_TROVE_COVE, "Ship Pool", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_8_LIGHTHOUSE_TOP, LEVEL_2_TREASURE_TROVE_COVE, "Lighthouse Top", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_A_NIPPER, LEVEL_2_TREASURE_TROVE_COVE, "Nipper", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_C_LIGHTHOUSE_BOTTOM, LEVEL_2_TREASURE_TROVE_COVE, "Lighthouse Bottom", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_E_ISLAND_TOP_STAIRS, LEVEL_2_TREASURE_TROVE_COVE, "Heights", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_F_CLIFFSIDE_STAIRS, LEVEL_2_TREASURE_TROVE_COVE, "Cave", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_14_WITCH_SWITCH, LEVEL_2_TREASURE_TROVE_COVE, "Witch Switch Return", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, 91, LEVEL_2_TREASURE_TROVE_COVE, "Demo", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, 95, LEVEL_2_TREASURE_TROVE_COVE, "Sharkfood SNS Cutscene", true },
    { MAP_7_TTC_TREASURE_TROVE_COVE, 128, LEVEL_2_TREASURE_TROVE_COVE, "Outside Sharkfood", true },
    { MAP_8F_TTC_SHARKFOOD_ISLAND, WARP_TCC_SHARKFOOD_ISLAND_1_ENTRANCE, LEVEL_2_TREASURE_TROVE_COVE, "Entrance", true },
    { MAP_A_TTC_SANDCASTLE, WARP_TCC_SANDCASTLE_1_ENTRANCE, LEVEL_2_TREASURE_TROVE_COVE, "Entrance", true },
    { MAP_A_TTC_SANDCASTLE, 2, LEVEL_2_TREASURE_TROVE_COVE, "FF Dark Snippet Fight", true },
    // LEVEL_3_CLANKERS_CAVERN
    { MAP_21_CC_WITCH_SWITCH_ROOM, WARP_CC_WITCH_SWITCH_1_TOP_ENTRANCE, LEVEL_3_CLANKERS_CAVERN, "From Blowhole", true },
    { MAP_21_CC_WITCH_SWITCH_ROOM, WARP_CC_WITCH_SWITCH_14_WITCH_SWITCH, LEVEL_3_CLANKERS_CAVERN, "Witch Switch Return", true },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_1_STOMACH_ROOFTOP, LEVEL_3_CLANKERS_CAVERN, "Stomach Ceiling", true },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_2_MOUTH_ROOFTOP, LEVEL_3_CLANKERS_CAVERN, "Mouth Ceiling", true },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_3_GOLD_FEATHER_ENTRANCE, LEVEL_3_CLANKERS_CAVERN, "Top", true },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_4_LEFT_GILL, LEVEL_3_CLANKERS_CAVERN, "Lower Gill", true },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_5_RIGHT_GILL, LEVEL_3_CLANKERS_CAVERN, "Upper Gill", true },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_6_LEFT_TOOTH, LEVEL_3_CLANKERS_CAVERN, "Jiggy Tooth", true },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_7_RIGHT_TOOTH, LEVEL_3_CLANKERS_CAVERN, "Token Tooth", true },
    { MAP_22_CC_INSIDE_CLANKER, 91, LEVEL_3_CLANKERS_CAVERN, "Mouth Ceiling Demo", true },
    { MAP_23_CC_GOLDFEATHER_ROOM, WARP_CC_GOLD_FEATHER_1_ENTRANCE, LEVEL_3_CLANKERS_CAVERN, "Wonder Wing Room", true },
    { MAP_B_CC_CLANKERS_CAVERN, WARP_CC_1_RIGHT_TOOTH, LEVEL_3_CLANKERS_CAVERN, "Left Tooth", true },
    { MAP_B_CC_CLANKERS_CAVERN, WARP_CC_2_LEFT_TOOTH, LEVEL_3_CLANKERS_CAVERN, "Right Tooth", true },
    { MAP_B_CC_CLANKERS_CAVERN, WARP_CC_3_RIGHT_GILLS, LEVEL_3_CLANKERS_CAVERN, "Left Gills", true },
    { MAP_B_CC_CLANKERS_CAVERN, WARP_CC_4_LEFT_GILLS, LEVEL_3_CLANKERS_CAVERN, "Right Gills", true },
    { MAP_B_CC_CLANKERS_CAVERN, WARP_CC_5_WORLD_ENTRANCE, LEVEL_3_CLANKERS_CAVERN, "CC Warp Pad", true },
    { MAP_B_CC_CLANKERS_CAVERN, 92, LEVEL_3_CLANKERS_CAVERN, "FF Mutie Fight", true },
    // LEVEL_4_BUBBLEGLOOP_SWAMP
    { MAP_10_BGS_MR_VILE, WARP_BGS_MR_VILE_2_CENTER, LEVEL_4_BUBBLEGLOOP_SWAMP, "Minigame Finish", true },
    { MAP_10_BGS_MR_VILE, WARP_BGS_MR_VILE_3_LEFT_ENTRANCE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Right Nostril Tunnel", true },
    { MAP_10_BGS_MR_VILE, WARP_BGS_MR_VILE_4_RIGHT_ENTRANCE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Left Nostril Tunnel", true },
    { MAP_11_BGS_TIPTUP, WARP_BGS_TIPTUP_1_ENTRANCE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Inside Tanktup", true },
    { MAP_11_BGS_TIPTUP, WARP_BGS_TIPTUP_2_PODIUM, LEVEL_4_BUBBLEGLOOP_SWAMP, "Minigame Win Return", true },
    { MAP_11_BGS_TIPTUP, 92, LEVEL_4_BUBBLEGLOOP_SWAMP, "FF Tiptup Minigame", true },
    { MAP_47_BGS_MUMBOS_SKULL, WARP_BGS_MUMBOS_HUT_1_ENTRANCE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Mumbo's Skull", true },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_1_CROC_FRONT, LEVEL_4_BUBBLEGLOOP_SWAMP, "Croc Front", true },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_2_WORLD_ENTRANCE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Warp Pad", true },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_3_TURTLE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Tanktup Mouth", true },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_4_CROC_LEFT, LEVEL_4_BUBBLEGLOOP_SWAMP, "Right Nostril", true },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_5_CROC_RIGHT, LEVEL_4_BUBBLEGLOOP_SWAMP, "Left Nostril", true },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_6_MUMBOS_HUT, LEVEL_4_BUBBLEGLOOP_SWAMP, "Outside Mumbo's Skull", true },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_14_WITCH_SWITCH, LEVEL_4_BUBBLEGLOOP_SWAMP, "Switch Return", true },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, 91, LEVEL_4_BUBBLEGLOOP_SWAMP, "Demo?", true },
    // LEVEL_5_FREEZEEZY_PEAK
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_1_WORLD_ENTRANCE, LEVEL_5_FREEZEEZY_PEAK, "Warp Pad", true },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_6_WOZZA_CAVE, LEVEL_5_FREEZEEZY_PEAK, "Outside Wozza's Cave", true },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_7_MUMBOS_HUT, LEVEL_5_FREEZEEZY_PEAK, "Outside Mumbo's Skull", true },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_8_IGLOO, LEVEL_5_FREEZEEZY_PEAK, "Outside Igloo", true },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_9_TREE_BOTTOM, LEVEL_5_FREEZEEZY_PEAK, "Christmas Tree", true },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_D_TREE_STAR, LEVEL_5_FREEZEEZY_PEAK, "Lit Tree CS Return", true },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_15_WITCH_SWITCH, LEVEL_5_FREEZEEZY_PEAK, "Switch Return", true },
    { MAP_41_FP_BOGGYS_IGLOO, WARP_FP_BOGGYS_IGLOO_1_ENTRANCE, LEVEL_5_FREEZEEZY_PEAK, "Entrance", true },
    { MAP_48_FP_MUMBOS_SKULL, WARP_FP_MUMBOS_HUT_1_ENTRANCE, LEVEL_5_FREEZEEZY_PEAK, "Entrance", true },
    { MAP_53_FP_CHRISTMAS_TREE, WARP_FP_CHRISTMAS_TREE_1_ENTRANCE, LEVEL_5_FREEZEEZY_PEAK, "Inside", true },
    { MAP_7F_FP_WOZZAS_CAVE, WARP_FP_WOZZAS_CAVE_1_ENTRANCE, LEVEL_5_FREEZEEZY_PEAK, "Entrance", true },
    // LEVEL_6_LAIR
    { MAP_69_GL_MM_LOBBY, WARP_GL_MM_LOBBY_1_NOTE_DOOR, LEVEL_6_LAIR, "50 Note Door", true },
    { MAP_69_GL_MM_LOBBY, WARP_GL_MM_LOBBY_2_MM_ENTRANCE, LEVEL_6_LAIR, "Level Entrance", true },
    { MAP_69_GL_MM_LOBBY, WARP_GL_MM_LOBBY_A_PUZZLE, LEVEL_6_LAIR, "Puzzle", true },
    { MAP_69_GL_MM_LOBBY, WARP_GL_MM_LOBBY_12_ENTRANCE, LEVEL_6_LAIR, "Lair Entrance", true },
    { MAP_6A_GL_TTC_AND_CC_PUZZLE, WARP_GL_TTC_PUZZLE_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Lower", true },
    { MAP_6A_GL_TTC_AND_CC_PUZZLE, WARP_GL_TTC_PUZZLE_2_HIGHER_EXIT, LEVEL_6_LAIR, "Upper", true },
    { MAP_6A_GL_TTC_AND_CC_PUZZLE, WARP_GL_TTC_PUZZLE_8_CAULDRON, LEVEL_6_LAIR, "Purple Cauldron", true },
    { MAP_6A_GL_TTC_AND_CC_PUZZLE, WARP_GL_TTC_PUZZLE_A_CC_PUZZLE, LEVEL_6_LAIR, "CC Puzzle", true },
    { MAP_6A_GL_TTC_AND_CC_PUZZLE, WARP_GL_TTC_PUZZLE_B_TTC_PUZZLE, LEVEL_6_LAIR, "TTC Puzzle", true },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Middle", true },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_2_PIPE_ROOM, LEVEL_6_LAIR, "Big Pipe", true },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_3_TO_TTC, LEVEL_6_LAIR, "Red Eyes", true },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_4_TO_CC, LEVEL_6_LAIR, "Small Pipe", true },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_5_HIGHER_EXIT, LEVEL_6_LAIR, "Top", true },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_A_WATERFALL, LEVEL_6_LAIR, "Waterfall", true },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_B_PUZZLE, LEVEL_6_LAIR, "Puzzle", true },
    { MAP_6C_GL_RED_CAULDRON_ROOM, WARP_GL_PIPE_ROOM_1_ENTRANCE, LEVEL_6_LAIR, "Pipe", true },
    { MAP_6C_GL_RED_CAULDRON_ROOM, WARP_GL_PIPE_ROOM_8_CAULDRON, LEVEL_6_LAIR, "Cauldron", true },
    { MAP_6D_GL_TTC_LOBBY, WARP_GL_TTC_LOBBY_1_ENTRANCE, LEVEL_6_LAIR, "Stairs", true },
    { MAP_6D_GL_TTC_LOBBY, WARP_GL_TTC_LOBBY_4_TTC_ENTRANCE, LEVEL_6_LAIR, "Level Exit", true },
    { MAP_6E_GL_GV_LOBBY, WARP_GL_GV_LOBBY_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Stairs Down", true },
    { MAP_6E_GL_GV_LOBBY, WARP_GL_GV_LOBBY_2_HIGHER_EXIT, LEVEL_6_LAIR, "Higher Exit", true },
    { MAP_6E_GL_GV_LOBBY, WARP_GL_GV_LOBBY_3_GV_ENTRANCE, LEVEL_6_LAIR, "Level Entrance", true },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Lower", true },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_2_HIGHEST_EXIT, LEVEL_6_LAIR, "Upper", true },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_5_MOUTH_EXIT, LEVEL_6_LAIR, "Witch", true },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_6_FP_ENTRANCE, LEVEL_6_LAIR, "Level Entrance", true },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_8_LOWER_CAULDRON, LEVEL_6_LAIR, "Purple Cauldron", true },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_9_HIGHER_CAULDRON, LEVEL_6_LAIR, "Green Cauldron", true },
    { MAP_70_GL_CC_LOBBY, WARP_GL_CC_LOBBY_1_ENTRANCE, LEVEL_6_LAIR, "Pipe", true },
    { MAP_70_GL_CC_LOBBY, WARP_GL_CC_LOBBY_2_CC_ENTRANCE, LEVEL_6_LAIR, "Level Entrance", true },
    { MAP_70_GL_CC_LOBBY, WARP_GL_CC_LOBBY_B_PUZZLE, LEVEL_6_LAIR, "BGS Puzzle", true },
    { MAP_71_GL_STATUE_ROOM, WARP_GL_STATUE_ROOM_1_HIGHER_EXIT, LEVEL_6_LAIR, "Stairs Up", true },
    { MAP_71_GL_STATUE_ROOM, WARP_GL_STATUE_ROOM_2_LOWER_ENTRANCE, LEVEL_6_LAIR, "Down", true },
    { MAP_71_GL_STATUE_ROOM, WARP_GL_STATUE_ROOM_3_TO_BGS, LEVEL_6_LAIR, "Wood Tunnel", true },
    { MAP_71_GL_STATUE_ROOM, WARP_GL_STATUE_ROOM_4_CEILING_EXIT, LEVEL_6_LAIR, "Ceiling", true },
    { MAP_72_GL_BGS_LOBBY, WARP_GL_BGS_LOBBY_1_ENTRANCE, LEVEL_6_LAIR, "Entrance", true },
    { MAP_72_GL_BGS_LOBBY, WARP_GL_BGS_LOBBY_2_BGS_ENTRANCE, LEVEL_6_LAIR, "Level Entrance", true },
    { MAP_72_GL_BGS_LOBBY, WARP_GL_BGS_LOBBY_A_PUZZLE, LEVEL_6_LAIR, "FP Puzzle", true },
    { MAP_74_GL_GV_PUZZLE, WARP_GL_GV_PUZZLE_1_TO_MMM, LEVEL_6_LAIR, "Red Eyes", true },
    { MAP_74_GL_GV_PUZZLE, WARP_GL_GV_PUZZLE_2_ENTRANCE, LEVEL_6_LAIR, "Lower", true },
    { MAP_74_GL_GV_PUZZLE, WARP_GL_GV_PUZZLE_A_PUZZLE, LEVEL_6_LAIR, "Puzzle", true },
    { MAP_75_GL_MMM_LOBBY, WARP_GL_MMM_LOBBY_1_ENTRANCE, LEVEL_6_LAIR, "Front", true },
    { MAP_75_GL_MMM_LOBBY, WARP_GL_MMM_LOBBY_2_MM_ENTRANCE, LEVEL_6_LAIR, "Level Entrance", true },
    { MAP_75_GL_MMM_LOBBY, WARP_GL_MMM_LOBBY_3_COFFIN, LEVEL_6_LAIR, "Crypt Door", true },
    { MAP_76_GL_640_NOTE_DOOR, WARP_GL_WATER_SWITCH_ROOM_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Mid", true },
    { MAP_76_GL_640_NOTE_DOOR, WARP_GL_WATER_SWITCH_ROOM_2_MUMBO_TOKEN_EXIT, LEVEL_6_LAIR, "Top Red", true },
    { MAP_76_GL_640_NOTE_DOOR, WARP_GL_WATER_SWITCH_ROOM_3_HIGHER_EXIT, LEVEL_6_LAIR, "Top Green", true },
    { MAP_76_GL_640_NOTE_DOOR, WARP_GL_WATER_SWITCH_ROOM_4_UNDERWATER_EXIT, LEVEL_6_LAIR, "Red Eyes", true },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Mid", true },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_2_RBB_ENTRANCE, LEVEL_6_LAIR, "Level Entrance", true },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_3_UNDERWATER_EXIT, LEVEL_6_LAIR, "Underwater Exit", true },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_4_HIGHER_EXIT, LEVEL_6_LAIR, "Bricks", true },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_8_CAULDRON, LEVEL_6_LAIR, "Green Cauldron", true },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_A_SWITCH, LEVEL_6_LAIR, "Switch CS Return", true },
    { MAP_78_GL_RBB_AND_MMM_PUZZLE, WARP_GL_MMM_PUZZLE_1_UNDERWATER_ENTRANCE, LEVEL_6_LAIR, "MMM Entrance", true },
    { MAP_78_GL_RBB_AND_MMM_PUZZLE, WARP_GL_MMM_PUZZLE_2_HIGHER_ENTRANCE, LEVEL_6_LAIR, "RBB Entrance", true },
    { MAP_78_GL_RBB_AND_MMM_PUZZLE, WARP_GL_MMM_PUZZLE_5_RBB_PUZZLE, LEVEL_6_LAIR, "RBB Puzzle", true },
    { MAP_78_GL_RBB_AND_MMM_PUZZLE, WARP_GL_MMM_PUZZLE_6_MMM_PUZZLE, LEVEL_6_LAIR, "MMM Puzzle", true },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_1_WHIPCRACK_ENTRANCE, LEVEL_6_LAIR, "Whipcracks", true },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_2_HIGHER_ENTRANCE, LEVEL_6_LAIR, "Red Dirt", true },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_3_FURNACE_EXIT, LEVEL_6_LAIR, "Green", true },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_6_CCW_ENTRANCE, LEVEL_6_LAIR, "Level Entrance", true },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_8_CAULDRON, LEVEL_6_LAIR, "Red Cauldron", true },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_12_SWITCH, LEVEL_6_LAIR, "Switch CS Return", true },
    { MAP_7A_GL_CRYPT, WARP_GL_COFFIN_ROOM_1_ENTRANCE, LEVEL_6_LAIR, "Entrance", true },
    { MAP_7A_GL_CRYPT, WARP_GL_COFFIN_ROOM_A_SWITCH, LEVEL_6_LAIR, "Switch", true },
    { MAP_80_GL_FF_ENTRANCE, WARP_GL_FURNACE_FUN_PATH_1_ENTRANCE, LEVEL_6_LAIR, "Lower", true },
    { MAP_80_GL_FF_ENTRANCE, WARP_GL_FURNACE_FUN_PATH_2_EXIT, LEVEL_6_LAIR, "Pad", true },
    { MAP_8E_GL_FURNACE_FUN, WARP_GL_FURNACE_FUN_1_AFTER_PAD, LEVEL_6_LAIR, "After Pad", true },
    { MAP_8E_GL_FURNACE_FUN, WARP_GL_FURNACE_FUN_2_ENTRANCE_PAD, LEVEL_6_LAIR, "Pad", true },
    { MAP_8E_GL_FURNACE_FUN, WARP_GL_FURNACE_FUN_4_AFTER_BOARD, LEVEL_6_LAIR, "After Board", true },
    { MAP_8E_GL_FURNACE_FUN, WARP_GL_FURNACE_FUN_5_TO_TOWER, LEVEL_6_LAIR, "Back Exit", true },
    { MAP_8E_GL_FURNACE_FUN, 5, LEVEL_6_LAIR, "GL Furnace Fun - 5", true },
    { MAP_8E_GL_FURNACE_FUN, WARP_GL_FURNACE_FUN_8_CAULDRON, LEVEL_6_LAIR, "FF Yellow Cauldron", true },
    { MAP_93_GL_DINGPOT, WARP_GL_TOWER_2_DINGPOT, LEVEL_6_LAIR, "Dingpot", true },
    { MAP_93_GL_DINGPOT, WARP_GL_TOWER_5_ENTRANCE, LEVEL_6_LAIR, "Lower", true },
    { MAP_93_GL_DINGPOT, WARP_GL_TOWER_8_CAULDRON, LEVEL_6_LAIR, "Yellow Cauldron", true },
    { MAP_93_GL_DINGPOT, WARP_GL_TOWER_A_GRUNTY_PUZZLE, LEVEL_6_LAIR, "Puzzle", true },
    // LEVEL_7_GOBIS_VALLEY
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_2_JINXY_SPHINX, LEVEL_7_GOBIS_VALLEY, "Jinxy Door", true },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_3_MEMORY_PYRAMID, LEVEL_7_GOBIS_VALLEY, "Sun Door", true },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_4_MAZE_PYRAMID_FRONT, LEVEL_7_GOBIS_VALLEY, "Central Pyramid Entrance", true },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_5_WATER_PYRAMID, LEVEL_7_GOBIS_VALLEY, "Lower Star Door", true },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_6_RUBEE_PYRAMID, LEVEL_7_GOBIS_VALLEY, "Kazooie Door", true },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_7_MAZE_PYRAMID_BACK, LEVEL_7_GOBIS_VALLEY, "Central Pyramid Exit", true },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_8_WORLD_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Warp Pad", true },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_A_EGG_ROOM, LEVEL_7_GOBIS_VALLEY, "Outside SNS Tomb", true },
    { MAP_13_GV_MEMORY_GAME, WARP_GV_MEMORY_PYRAMID_1_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Entrance", true },
    { MAP_13_GV_MEMORY_GAME, WARP_GV_MEMORY_PYRAMID_2_CENTER, LEVEL_7_GOBIS_VALLEY, "Center", true },
    { MAP_14_GV_SANDYBUTTS_MAZE, WARP_GV_MAZE_PYRAMID_1_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Entrance", true },
    { MAP_15_GV_WATER_PYRAMID, WARP_GV_WATER_PYRAMID_1_RETURN_TO_JIGGY, LEVEL_7_GOBIS_VALLEY, "Return To Jiggy", true },
    { MAP_15_GV_WATER_PYRAMID, WARP_GV_WATER_PYRAMID_2_CEILING, LEVEL_7_GOBIS_VALLEY, "Ceiling", true },
    { MAP_15_GV_WATER_PYRAMID, WARP_GV_WATER_PYRAMID_6_LOWER_EXIT, LEVEL_7_GOBIS_VALLEY, "Lower Exit", true },
    { MAP_16_GV_RUBEES_CHAMBER, WARP_GV_RUBEES_CHAMBER_7_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Entrance", true },
    { MAP_1A_GV_INSIDE_JINXY, WARP_GV_INSIDE_JINXY_2_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Entrance", true },
    { MAP_92_GV_SNS_CHAMBER, WARP_GV_EGG_ROOM_5_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Entrance", true },
    // LEVEL_8_CLICK_CLOCK_WOOD
    { MAP_40_CCW_HUB, WARP_CCW_1_WINTER, LEVEL_8_CLICK_CLOCK_WOOD, "Winter", true },
    { MAP_40_CCW_HUB, WARP_CCW_2_SPRING, LEVEL_8_CLICK_CLOCK_WOOD, "Spring", true },
    { MAP_40_CCW_HUB, WARP_CCW_3_SUMMER, LEVEL_8_CLICK_CLOCK_WOOD, "Summer", true },
    { MAP_40_CCW_HUB, WARP_CCW_4_AUTUMN, LEVEL_8_CLICK_CLOCK_WOOD, "Autumn", true },
    { MAP_40_CCW_HUB, WARP_CCW_5_SPRING_SWITCH, LEVEL_8_CLICK_CLOCK_WOOD, "Spring Switch", true },
    { MAP_40_CCW_HUB, WARP_CCW_7_WORLD_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Warp Pad", true },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_4_NABNUT_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Nabnut's Window", true },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_5_SUMMER_SWITCH, LEVEL_8_CLICK_CLOCK_WOOD, "Summer Switch", true },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_6_ZUBBA_HIVE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Zubba Hive", true },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_7_NABNUT_HOUSE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's House", true },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_8_TOP_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Top Door", true },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_9_MUMBOS_HUT, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Mumbo's Hut", true },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_4_NABNUT_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's Window", true },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_5_AUTUMN_SWITCH, LEVEL_8_CLICK_CLOCK_WOOD, "Autumn Switch", true },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_6_ZUBBA_HIVE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Zubba Hive", true },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_7_NABNUT_HOUSE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's House", true },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_8_TOP_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Top Door", true },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_9_MUMBOS_HUT, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Mumbo's Hut", true },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_4_NABNUT_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's Window", true },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_5_ZUBBA_HIVE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Zubba Hive", true },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_6_NABNUT_UPPER_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's Upper Window", true },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_7_NABNUT_HOUSE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's House", true },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_8_TOP_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Top Door", true },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_9_MUMBOS_HUT, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Mumbo's Hut", true },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_F_WINTER_SWITCH, LEVEL_8_CLICK_CLOCK_WOOD, "Winter Switch", true },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_4_NABNUT_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's Window", true },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_5_HIGHEST_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Highest Window", true },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_6_NABNUT_UPPER_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's Upper Window", true },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_8_TOP_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Top Door", true },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_9_MUMBOS_HUT, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Mumbo's Hut", true },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_15_WITCH_SWITCH, LEVEL_8_CLICK_CLOCK_WOOD, "Witch Switch", true },
    { MAP_46_CCW_WINTER, 91, LEVEL_8_CLICK_CLOCK_WOOD, "Demo", true },
    { MAP_4A_CCW_SPRING_MUMBOS_SKULL, WARP_CCW_MUMBOS_HUT_SPRING_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_4B_CCW_SUMMER_MUMBOS_SKULL, WARP_CCW_MUMBOS_HUT_SUMMER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_4C_CCW_AUTUMN_MUMBOS_SKULL, WARP_CCW_MUMBOS_HUT_AUTUMN_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_4D_CCW_WINTER_MUMBOS_SKULL, WARP_CCW_MUMBOS_HUT_WINTER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_5A_CCW_SUMMER_ZUBBA_HIVE, WARP_CCW_BEEHIVE_SUMMER_2_TOP, LEVEL_8_CLICK_CLOCK_WOOD, "Top", true },
    { MAP_5A_CCW_SUMMER_ZUBBA_HIVE, WARP_CCW_BEEHIVE_SUMMER_3_REDO, LEVEL_8_CLICK_CLOCK_WOOD, "Redo", true },
    { MAP_5B_CCW_SPRING_ZUBBA_HIVE, WARP_CCW_BEEHIVE_SPRING_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_5C_CCW_AUTUMN_ZUBBA_HIVE, WARP_CCW_BEEHIVE_AUTUMN_2_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_5E_CCW_SPRING_NABNUTS_HOUSE, WARP_CCW_NABNUT_SPRING_1_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Door", true },
    { MAP_5E_CCW_SPRING_NABNUTS_HOUSE, WARP_CCW_NABNUT_SPRING_2_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Window", true },
    { MAP_5F_CCW_SUMMER_NABNUTS_HOUSE, WARP_CCW_NABNUT_SUMMER_1_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Door", true },
    { MAP_5F_CCW_SUMMER_NABNUTS_HOUSE, WARP_CCW_NABNUT_SUMMER_2_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Window", true },
    { MAP_60_CCW_AUTUMN_NABNUTS_HOUSE, WARP_CCW_NABNUT_AUTUMN_1_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Door", true },
    { MAP_60_CCW_AUTUMN_NABNUTS_HOUSE, WARP_CCW_NABNUT_AUTUMN_2_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Window", true },
    { MAP_61_CCW_WINTER_NABNUTS_HOUSE, 2, LEVEL_8_CLICK_CLOCK_WOOD, "Window", true },
    { MAP_62_CCW_WINTER_HONEYCOMB_ROOM, WARP_CCW_ATTIC_WINTER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_63_CCW_AUTUMN_NABNUTS_WATER_SUPPLY, WARP_CCW_WATER_ROOM_AUTUMN_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_64_CCW_WINTER_NABNUTS_WATER_SUPPLY, WARP_CCW_WATER_ROOM_WINTER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_65_CCW_SPRING_WHIPCRACK_ROOM, WARP_CCW_WHIPCRACK_SPRING_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_66_CCW_SUMMER_WHIPCRACK_ROOM, WARP_CCW_WHIPCRACK_SUMMER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_67_CCW_AUTUMN_WHIPCRACK_ROOM, WARP_CCW_WHIPCRACK_AUTUMN_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    { MAP_68_CCW_WINTER_WHIPCRACK_ROOM, WARP_CCW_WHIPCRACK_WINTER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance", true },
    // LEVEL_9_RUSTY_BUCKET_BAY
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_1_CAPTAIN_ROOM_WINDOW, LEVEL_9_RUSTY_BUCKET_BAY, "Captain Room Window", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_2_CREW_ROOM_WINDOW, LEVEL_9_RUSTY_BUCKET_BAY, "Crew Room Window", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_3_ENGINE_SWITCH_PIPE, LEVEL_9_RUSTY_BUCKET_BAY, "Engine Switch Pipe", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_4_KITCHEN_PIPE, LEVEL_9_RUSTY_BUCKET_BAY, "Kitchen Pipe", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_5_NAV_ROOM_PIPE, LEVEL_9_RUSTY_BUCKET_BAY, "Nav Room Pipe", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_6_STORAGE_PIPE, LEVEL_9_RUSTY_BUCKET_BAY, "Storage Pipe", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_7_ENGINE_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Engine Entrance", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_8_BOAT_ROOM, LEVEL_9_RUSTY_BUCKET_BAY, "Boat Room", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_9_FIRST_CONTAINER, LEVEL_9_RUSTY_BUCKET_BAY, "First Container", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_A_SECOND_CONTAINER, LEVEL_9_RUSTY_BUCKET_BAY, "Second Container", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_B_THIRD_CONTAINER, LEVEL_9_RUSTY_BUCKET_BAY, "Third Container", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_C_AFTER_BOSS, LEVEL_9_RUSTY_BUCKET_BAY, "After Boss", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_D_FISH_WAREHOUSE, LEVEL_9_RUSTY_BUCKET_BAY, "Fish Warehouse", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_E_WITCH_SWITCH, LEVEL_9_RUSTY_BUCKET_BAY, "Witch Switch", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_10_WORLD_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "World Entrance", true },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_13_ANCHOR_ROOM, LEVEL_9_RUSTY_BUCKET_BAY, "Anchor Room", true },
    { MAP_34_RBB_ENGINE_ROOM, WARP_RBB_ENGINE_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_34_RBB_ENGINE_ROOM, WARP_RBB_ENGINE_2_LEFT_SWITCH, LEVEL_9_RUSTY_BUCKET_BAY, "Left Switch", true },
    { MAP_34_RBB_ENGINE_ROOM, WARP_RBB_ENGINE_3_RIGHT_SWITCH, LEVEL_9_RUSTY_BUCKET_BAY, "Right Switch", true },
    { MAP_34_RBB_ENGINE_ROOM, WARP_RBB_ENGINE_4_PIPE_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Pipe Entrance", true },
    { MAP_35_RBB_WAREHOUSE, WARP_RBB_WAREHOUSE_1_DOOR, LEVEL_9_RUSTY_BUCKET_BAY, "Door", true },
    { MAP_35_RBB_WAREHOUSE, WARP_RBB_WAREHOUSE_2_WINDOW, LEVEL_9_RUSTY_BUCKET_BAY, "Window", true },
    { MAP_36_RBB_BOATHOUSE, WARP_RBB_BOAT_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_37_RBB_CONTAINER_1, WARP_RBB_CONTAINER_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_38_RBB_CONTAINER_3, WARP_RBB_CONTAINER_3_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_39_RBB_CREW_CABIN, WARP_RBB_CREW_CABIN_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_3A_RBB_BOSS_BOOM_BOX, WARP_RBB_BOSS_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_3A_RBB_BOSS_BOOM_BOX, WARP_RBB_BOSS_2_REDO, LEVEL_9_RUSTY_BUCKET_BAY, "Redo", true },
    { MAP_3B_RBB_STORAGE_ROOM, WARP_RBB_STORAGE_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_3C_RBB_KITCHEN, WARP_RBB_KITCHEN_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_3D_RBB_NAVIGATION_ROOM, WARP_RBB_NAVIGATION_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_3E_RBB_CONTAINER_2, WARP_RBB_CONTAINER_2_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_3F_RBB_CAPTAINS_CABIN, WARP_RBB_CAPTAIN_CABIN_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_8B_RBB_ANCHOR_ROOM, WARP_RBB_ANCHOR_2_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance", true },
    { MAP_8B_RBB_ANCHOR_ROOM, WARP_RBB_ANCHOR_4_SWITCH, LEVEL_9_RUSTY_BUCKET_BAY, "Switch", true },
    // LEVEL_A_MAD_MONSTER_MANSION
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_2_MASNION_DOOR, LEVEL_A_MAD_MONSTER_MANSION, "Mansion Door", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_3_WELL_TOP, LEVEL_A_MAD_MONSTER_MANSION, "Well Top", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_4_SHED, LEVEL_A_MAD_MONSTER_MANSION, "Shed", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_5_CHURCH_DOOR, LEVEL_A_MAD_MONSTER_MANSION, "Church Door", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_6_CHURCH_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "Church Window", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_7_CHIMNEY, LEVEL_A_MAD_MONSTER_MANSION, "Chimney", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_8_DRAINPIPE_BOTTOM, LEVEL_A_MAD_MONSTER_MANSION, "Outside Drainpipe", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_9_CELLAR, LEVEL_A_MAD_MONSTER_MANSION, "Outside Cellar", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_A_F1_RED_FEATHER_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F1 Red Feather Window", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_B_F1_EGG_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F1 Egg Window", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_C_F2_BATHROOM_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F2 Bathroom Window", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_D_F2_GOLD_FEATHER_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F2 Gold Feather Window", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_E_F3_BEDROOM_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F3 Bedroom Window", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_F_F3_NOTE_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F3 Note Window", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_10_CHURCH_TOP, LEVEL_A_MAD_MONSTER_MANSION, "Church Top", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_11_CHURCH_CLOCK, LEVEL_A_MAD_MONSTER_MANSION, "Church Clock", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_12_MUMBOS_HUT, LEVEL_A_MAD_MONSTER_MANSION, "Outside Mumbo's Hut", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_13_WELL_BOTTOM, LEVEL_A_MAD_MONSTER_MANSION, "Well Bottom", true },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_14_WORLD_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "World Entrance", true },
    { MAP_1C_MMM_CHURCH, WARP_MMM_CHURCH_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    { MAP_1C_MMM_CHURCH, WARP_MMM_CHURCH_14_WITCH_SWITCH, LEVEL_A_MAD_MONSTER_MANSION, "Witch Switch", true },
    { MAP_1D_MMM_CELLAR, WARP_MMM_CELLAR_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    { MAP_24_MMM_TUMBLARS_SHED, WARP_MMM_SHED_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    { MAP_25_MMM_WELL, WARP_MMM_WELL_1_TOP_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Top", true },
    { MAP_25_MMM_WELL, WARP_MMM_WELL_4_BOTTOM_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Bottom", true },
    { MAP_26_MMM_NAPPERS_ROOM, WARP_MMM_DINING_1_FRONT_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Front", true },
    { MAP_26_MMM_NAPPERS_ROOM, WARP_MMM_DINING_2_CHIMNEY, LEVEL_A_MAD_MONSTER_MANSION, "Chimney", true },
    { MAP_28_MMM_EGG_ROOM, WARP_MMM_EGG_ROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    { MAP_29_MMM_NOTE_ROOM, WARP_MMM_NOTE_ROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    { MAP_2A_MMM_FEATHER_ROOM, WARP_MMM_RED_FEATHER_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    { MAP_2B_MMM_SECRET_CHURCH_ROOM, WARP_MMM_SECRET_CHURCH_ROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    { MAP_2C_MMM_BATHROOM, WARP_MMM_BATHROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    { MAP_2C_MMM_BATHROOM, WARP_MMM_BATHROOM_4_LOGGO, LEVEL_A_MAD_MONSTER_MANSION, "Loggo", true },
    { MAP_2D_MMM_BEDROOM, WARP_MMM_BEDROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    { MAP_2E_MMM_HONEYCOMB_ROOM, WARP_MMM_HONEYCOMB_ROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    { MAP_2F_MMM_WATERDRAIN_BARREL, WARP_MMM_DRAINPIPE_1_TOP_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Top", true },
    { MAP_2F_MMM_WATERDRAIN_BARREL, WARP_MMM_DRAINPIPE_2_BOTTOM_EXIT, LEVEL_A_MAD_MONSTER_MANSION, "Bottom", true },
    { MAP_30_MMM_MUMBOS_SKULL, WARP_MMM_MUMBOS_HUT_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    { MAP_8D_MMM_INSIDE_LOGGO, WARP_MMM_SEPTIC_TANK_4_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance", true },
    // LEVEL_B_SPIRAL_MOUNTAIN
    { MAP_1_SM_SPIRAL_MOUNTAIN, WARP_SM_SPIRAL_MOUNTAIN_1_BANJOS_HOUSE, LEVEL_B_SPIRAL_MOUNTAIN, "Outside Banjo's House", true },
    { MAP_1_SM_SPIRAL_MOUNTAIN, WARP_SM_SPIRAL_MOUNTAIN_13_LAIR_ENTRANCE, LEVEL_B_SPIRAL_MOUNTAIN, "Lair Entrance", true },
    { MAP_8C_SM_BANJOS_HOUSE, WARP_SM_BANJOS_HOUSE_1_ENTRANCE, LEVEL_B_SPIRAL_MOUNTAIN, "Banjo's House", true },
    { MAP_8C_SM_BANJOS_HOUSE, WARP_SM_BANJOS_HOUSE_2_BOTTLES, LEVEL_B_SPIRAL_MOUNTAIN, "SM Banjo's House - Bottles", true },
    // LEVEL_C_BOSS
    { MAP_90_GL_BATTLEMENTS, WARP_GL_BOSS_1_ENTRANCE, LEVEL_C_BOSS, "GL Battlements - Entrance", true },
};
// clang-format on};
// clang-format on};
// clang-format on

struct LevelNameEntry {
    const char* enumName;
    const char* displayName;
};

constexpr LevelNameEntry kLevelNames[] = {
    { "0", "Unknown" },
    { "LEVEL_1_MUMBOS_MOUNTAIN", "Mumbo's Mountain" },
    { "LEVEL_2_TREASURE_TROVE_COVE", "Treasure Trove Cove" },
    { "LEVEL_3_CLANKERS_CAVERN", "Clanker's Cavern" },
    { "LEVEL_4_BUBBLEGLOOP_SWAMP", "Bubblegloop Swamp" },
    { "LEVEL_5_FREEZEEZY_PEAK", "Freezeezy Peak" },
    { "LEVEL_6_LAIR", "Gruntilda's Lair" },
    { "LEVEL_7_GOBIS_VALLEY", "Gobi's Valley" },
    { "LEVEL_8_CLICK_CLOCK_WOOD", "Click Clock Wood" },
    { "LEVEL_9_RUSTY_BUCKET_BAY", "Rusty Bucket Bay" },
    { "LEVEL_A_MAD_MONSTER_MANSION", "Mad Monster Mansion" },
    { "LEVEL_B_SPIRAL_MOUNTAIN", "Spiral Mountain" },
    { "LEVEL_C_BOSS", "Boss Arena" },
    { "LEVEL_D_CUTSCENE", "Cutscenes" },
};
constexpr int32_t kLevelCount = (int32_t)(sizeof(kLevelNames) / sizeof(kLevelNames[0]));

// func_803084F0 only maps these index ranges to marker actors; everything else
// returns actor 0 and can never resolve to a prop. Indices >= 0x80 (and 0x63/0x65)
// take the scripted-spawn path instead and are deliberately left out.
constexpr struct {
    int32_t first;
    int32_t last;
} kProbeRanges[] = {
    { 0x01, 0x19 },
    { 0x5B, 0x62 },
};

std::vector<WarpEntry> sUserWarps;
std::deque<Arrival> sArrivals;
int32_t sSelectedArrival = 0;
std::string sPendingName;
std::string sCatalogPath;
std::vector<WarpEntranceInfo> sMapEntrances;
// Props are built after OnMapLoad fires, so the scan waits for the next frame.
bool sEntranceScanPending = false;
// Level grouping over both tables. Cached because regrouping a few hundred entries
// every frame would be pure waste; the pointers into sUserWarps mean any mutation
// there has to set the dirty flag.
std::vector<std::vector<const WarpEntry*>> sByLevel;
bool sGroupingDirty = true;

void ScanMapEntrances() {
    sMapEntrances.clear();
    for (const auto& range : kProbeRanges) {
        for (int32_t id = range.first; id <= range.last; id++) {
            enum actor_e marker = func_803084F0(id);
            if (marker == 0) {
                continue;
            }
            s32 position[3];
            if (!nodeprop_findPositionFromActorId(marker, position)) {
                continue;
            }
            WarpEntranceInfo info;
            info.id = id;
            info.pos[0] = position[0];
            info.pos[1] = position[1];
            info.pos[2] = position[2];
            u32 yaw = 0;
            func_80305344((s32)marker, &yaw);
            info.yaw = yaw;
            sMapEntrances.push_back(info);
        }
    }
}

const char* MapEnumName(int32_t map) {
    return (map >= 0 && map < MAP_NUM_MAPS) ? kMapNames[map].enumName : "MAP_0_UNKNOWN";
}

const char* MapDisplayName(int32_t map) {
    return (map >= 0 && map < MAP_NUM_MAPS) ? kMapNames[map].displayName : "Unknown";
}

const LevelNameEntry& LevelNames(int32_t level) {
    return (level > 0 && level < kLevelCount) ? kLevelNames[level] : kLevelNames[0];
}

// Only maps the game actually loaded reach here, so the section lookup behind
// map_getLevel is guaranteed populated.
int32_t LevelOfLoadedMap(int32_t map) {
    int32_t level = (int32_t)map_getLevel((enum map_e)map);
    return (level > 0 && level < kLevelCount) ? level : 0;
}

std::string SourceRow(const WarpEntry& entry) {
    return fmt::format("    {{ {}, {}, {}, \"{}\", true }},", MapEnumName(entry.map), entry.exit,
                       LevelNames(entry.level).enumName, entry.name);
}

void SaveCatalog() {
    nlohmann::json root;
    root["fileType"] = "warpCatalog";
    root["entries"] = nlohmann::json::array();
    for (const auto& entry : sUserWarps) {
        root["entries"].push_back({
            { "map", entry.map },
            { "exit", entry.exit },
            { "level", entry.level },
            { "name", entry.name },
        });
    }

    std::ofstream file(sCatalogPath);
    if (!file.is_open()) {
        SPDLOG_ERROR("[WarpCatalog] Could not open {} for writing", sCatalogPath);
        return;
    }
    file << root.dump(4);
}

void LoadCatalog() {
    sUserWarps.clear();
    if (!std::filesystem::exists(sCatalogPath)) {
        return;
    }
    try {
        std::ifstream file(sCatalogPath);
        nlohmann::json root = nlohmann::json::parse(file);
        for (const auto& item : root.value("entries", nlohmann::json::array())) {
            WarpEntry entry;
            entry.map = item.value("map", 0);
            entry.exit = item.value("exit", 0);
            entry.level = item.value("level", 0);
            entry.name = item.value("name", std::string());
            if (entry.map > 0 && entry.map < MAP_NUM_MAPS && !entry.name.empty()) {
                sUserWarps.push_back(std::move(entry));
            }
        }
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[WarpCatalog] Failed to read {}: {}", sCatalogPath, e.what());
    }
    sGroupingDirty = true;
}

// Built-ins first, then user entries, both ordered by map then entrance.
void RebuildGrouping() {
    sByLevel.assign(kLevelCount, {});
    auto bucket = [](int32_t level) { return (level > 0 && level < kLevelCount) ? level : 0; };
    for (const auto& entry : kBuiltInWarps) {
        sByLevel[bucket(entry.level)].push_back(&entry);
    }
    std::vector<size_t> builtInCount(kLevelCount);
    for (int32_t level = 0; level < kLevelCount; level++) {
        builtInCount[level] = sByLevel[level].size();
    }
    for (const auto& entry : sUserWarps) {
        // A recorded entry that has since been promoted into kBuiltInWarps would
        // otherwise show up twice.
        bool promoted = std::any_of(std::begin(kBuiltInWarps), std::end(kBuiltInWarps),
                                    [&entry](const WarpEntry& builtIn) {
                                        return builtIn.map == entry.map && builtIn.exit == entry.exit;
                                    });
        if (!promoted) {
            sByLevel[bucket(entry.level)].push_back(&entry);
        }
    }
    auto byMap = [](const WarpEntry* a, const WarpEntry* b) {
        return a->map != b->map ? a->map < b->map : a->exit < b->exit;
    };
    for (int32_t level = 0; level < kLevelCount; level++) {
        auto userStart = sByLevel[level].begin() + (std::ptrdiff_t)builtInCount[level];
        std::sort(sByLevel[level].begin(), userStart, byMap);
        std::sort(userStart, sByLevel[level].end(), byMap);
    }
    sGroupingDirty = false;
}

} // namespace

const char* WarpCatalog_MapDisplayName(int map) {
    return MapDisplayName(map);
}

const std::vector<WarpEntranceInfo>& WarpCatalog_GetMapEntrances() {
    return sMapEntrances;
}

bool WarpCatalog_ExitIsProbeable(int exitId) {
    for (const auto& range : kProbeRanges) {
        if (exitId >= range.first && exitId <= range.last) {
            return true;
        }
    }
    return false;
}

bool WarpCatalog_EntranceExists(int exitId) {
    return std::any_of(sMapEntrances.begin(), sMapEntrances.end(),
                       [exitId](const WarpEntranceInfo& info) { return info.id == exitId; });
}

const char* WarpCatalog_FindName(int map, int exit) {
    for (const auto& entry : kBuiltInWarps) {
        if (entry.map == map && entry.exit == exit) {
            return entry.name.c_str();
        }
    }
    for (const auto& entry : sUserWarps) {
        if (entry.map == map && entry.exit == exit) {
            return entry.name.c_str();
        }
    }
    return nullptr;
}

void DrawEntranceRecorder() {
    ImGui::TextWrapped("Every map load is logged below. Pick one, describe it, and add it to the catalog.");

    int32_t currentMap = (int32_t)gsworld_getMap();
    ImGui::Text("Current map: %s (0x%02X)", MapDisplayName(currentMap), currentMap);

    if (sMapEntrances.empty()) {
        ImGui::TextDisabled("Entrance props present: none");
    } else {
        std::string ids;
        for (const auto& info : sMapEntrances) {
            ids += fmt::format("{}{}", ids.empty() ? "" : ", ", info.id);
        }
        ImGui::TextWrapped("Entrance props present: %s", ids.c_str());
    }
    UIWidgets::Tooltip("Entrance indices with a marker prop in this map. Scripted spawn ids (0x63, 0x65, "
                       "0x80+) resolve without props and cannot be detected.");

    if (sArrivals.empty()) {
        ImGui::TextDisabled("No map loads recorded yet.");
        return;
    }

    sSelectedArrival = std::min(sSelectedArrival, (int32_t)sArrivals.size() - 1);

    // Five rows plus the frozen header; the rest scrolls.
    const ImVec2 listSize(0.0f, ImGui::GetFrameHeightWithSpacing() * 5.0f + ImGui::GetTextLineHeightWithSpacing());
    if (ImGui::BeginTable("WarpArrivals", 5,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_BordersOuter,
                          listSize)) {
        ImGui::TableSetupColumn("");
        ImGui::TableSetupColumn("Arrived At");
        ImGui::TableSetupColumn("Entrance");
        ImGui::TableSetupColumn("Known As");
        ImGui::TableSetupColumn("Came From");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        for (int32_t i = 0; i < (int32_t)sArrivals.size(); i++) {
            const Arrival& arrival = sArrivals[i];
            ImGui::PushID(i);
            ImGui::TableNextColumn();
            if (ImGui::RadioButton("##pick", sSelectedArrival == i)) {
                sSelectedArrival = i;
            }
            ImGui::TableNextColumn();
            ImGui::Text("%s (0x%02X)", MapDisplayName(arrival.map), arrival.map);
            ImGui::TableNextColumn();
            ImGui::Text("%d", arrival.exit);
            ImGui::TableNextColumn();
            if (const char* known = WarpCatalog_FindName(arrival.map, arrival.exit)) {
                ImGui::Text("%s", known);
            } else {
                ImGui::TextDisabled("unnamed");
            }
            ImGui::TableNextColumn();
            ImGui::Text("%s (0x%02X)", MapDisplayName(arrival.fromMap), arrival.fromMap);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    const Arrival& picked = sArrivals[sSelectedArrival];
    UIWidgets::InputString("Description", &sPendingName,
                           UIWidgets::InputOptions()
                               .Color(THEME_COLOR)
                               .Size({ 300, 0 })
                               .PlaceholderText("e.g. Inside Mumbo's Skull")
                               .LabelPosition(UIWidgets::LabelPositions::Near));

    if (ImGui::BeginTable("WarpRecorderActions", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        ImGui::BeginDisabled(sPendingName.empty());
        if (UIWidgets::Button("Add to Catalog", UIWidgets::ButtonOptions().Color(THEME_COLOR))) {
            WarpEntry entry;
            entry.map = picked.map;
            entry.exit = picked.exit;
            entry.level = LevelOfLoadedMap(picked.map);
            entry.name = sPendingName;
            sUserWarps.push_back(entry);
            sGroupingDirty = true;
            SaveCatalog();
            sPendingName.clear();
        }
        ImGui::EndDisabled();
        ImGui::TableNextColumn();
        if (UIWidgets::Button("Warp Here Again", UIWidgets::ButtonOptions().Color(THEME_COLOR))) {
            func_8031D04C((enum map_e)picked.map, picked.exit);
        }
        ImGui::EndTable();
    }
}

void DrawWarpCatalog() {
    // Must happen before anything reads sByLevel: adding an entry earlier this frame
    // reallocates sUserWarps and leaves the cached pointers dangling.
    if (sGroupingDirty) {
        RebuildGrouping();
    }

    int32_t currentMap = (int32_t)gsworld_getMap();
    int32_t removeIndex = -1;
    // Starts at 0 so an entry whose level never resolved still has a group to live in.
    for (int32_t level = 0; level < kLevelCount; level++) {
        const std::vector<const WarpEntry*>& entries = sByLevel[level];
        if (entries.empty()) {
            continue;
        }
        ImGui::PushID(level);
        if (ImGui::CollapsingHeader(
                fmt::format("{} ({})", LevelNames(level).displayName, entries.size()).c_str())) {
            if (ImGui::BeginTable("LevelWarps", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Destination", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Map / Entrance");
                ImGui::TableSetupColumn("");
                for (int32_t i = 0; i < (int32_t)entries.size(); i++) {
                    const WarpEntry& entry = *entries[i];
                    ImGui::PushID(i);
                    ImGui::TableNextColumn();
                    if (UIWidgets::Button(entry.name.c_str(), UIWidgets::ButtonOptions().Color(THEME_COLOR))) {
                        func_8031D04C((enum map_e)entry.map, entry.exit);
                    }
                    ImGui::TableNextColumn();
                    ImGui::Text("0x%02X / %d", entry.map, entry.exit);
                    // Only the loaded map has been scanned, so this can only speak for it.
                    if (entry.map == currentMap && WarpCatalog_ExitIsProbeable(entry.exit) &&
                        !WarpCatalog_EntranceExists(entry.exit)) {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "(!)");
                        UIWidgets::Tooltip("No entrance prop with this index in the loaded map.");
                    }
                    ImGui::TableNextColumn();
                    if (!entry.builtIn) {
                        if (UIWidgets::Button("Remove", UIWidgets::ButtonOptions()
                                                            .Size(UIWidgets::Sizes::Inline)
                                                            .Color(UIWidgets::Colors::Red))) {
                            auto match = std::find_if(sUserWarps.begin(), sUserWarps.end(), [&](const WarpEntry& u) {
                                return u.map == entry.map && u.exit == entry.exit && u.name == entry.name;
                            });
                            if (match != sUserWarps.end()) {
                                removeIndex = (int32_t)(match - sUserWarps.begin());
                            }
                        }
                    } else {
                        ImGui::TextDisabled("built in");
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        ImGui::PopID();
    }

    if (removeIndex >= 0) {
        sUserWarps.erase(sUserWarps.begin() + removeIndex);
        sGroupingDirty = true;
        SaveCatalog();
    }

    ImGui::BeginDisabled(sUserWarps.empty());
    if (UIWidgets::Button("Copy Recorded Entries as C++",
                          UIWidgets::ButtonOptions()
                              .Color(THEME_COLOR)
                              .Tooltip("Copies kBuiltInWarps rows for every recorded entry, ready to paste "
                                       "into WarpCatalog.cpp."))) {
        std::string rows;
        std::vector<WarpEntry> sorted = sUserWarps;
        std::sort(sorted.begin(), sorted.end(), [](const WarpEntry& a, const WarpEntry& b) {
            return a.level != b.level ? a.level < b.level : (a.map != b.map ? a.map < b.map : a.exit < b.exit);
        });
        for (const auto& entry : sorted) {
            rows += SourceRow(entry) + "\n";
        }
        ImGui::SetClipboardText(rows.c_str());
    }
    ImGui::EndDisabled();
}

void DrawWarpTab() {
    ImGui::SeparatorText("Custom Warp Selector");
    if (ImGui::BeginChild("WarpChild")) {
        ImGui::Text("Map Select ");
        ImGui::SameLine();
        UIWidgets::Combobox("##mapSelect", &mapId, mapNames,
            { .labelPosition = UIWidgets::LabelPositions::None, .color = THEME_COLOR });
        UIWidgets::SliderInt("Exit ID", &exitId,
            {
                .format = "Exit: %i",
                .min = 0,
                .max = 20,
                .clamp = true,
                .labelPosition = UIWidgets::LabelPositions::None,
                .color = THEME_COLOR,
            });
        if (UIWidgets::Button(mapNames[mapId], { .color = THEME_COLOR })) {
            func_8031D04C((map_e)mapIdList[mapId], exitId);
        }

        ImGui::SeparatorText("Current Map");
        int32_t currentMap = (int32_t)gsworld_getMap();
        ImGui::Text("%s (0x%02X)", WarpCatalog_MapDisplayName(currentMap), currentMap);

        const std::vector<WarpEntranceInfo>& entrances = WarpCatalog_GetMapEntrances();
        if (entrances.empty()) {
            ImGui::TextDisabled("No entrance props detected in this map.");
        } else {
            static int32_t entranceIndex = 0;
            entranceIndex = std::clamp(entranceIndex, 0, (int32_t)entrances.size() - 1);
            auto describe = [currentMap](const WarpEntranceInfo& info) {
                const char* known = WarpCatalog_FindName(currentMap, info.id);
                return fmt::format("Entrance {} - {}  ({}, {}, {})", info.id, known ? known : "unnamed", info.pos[0],
                    info.pos[1], info.pos[2]);
                };
            if (ImGui::BeginCombo("##currentMapEntrance", describe(entrances[entranceIndex]).c_str())) {
                for (int32_t i = 0; i < (int32_t)entrances.size(); i++) {
                    // Unnamed indices dim, so the catalogued ones stand out at a glance.
                    bool known = WarpCatalog_FindName(currentMap, entrances[i].id) != nullptr;
                    if (!known) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                    }
                    if (ImGui::Selectable(describe(entrances[i]).c_str(), entranceIndex == i)) {
                        entranceIndex = i;
                    }
                    if (!known) {
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::EndCombo();
            }
            if (UIWidgets::Button("Warp to Entrance", { .color = THEME_COLOR })) {
                func_8031D04C((map_e)currentMap, entrances[entranceIndex].id);
            }
        }

        // Keeps a path to ids the scan can't see: scripted spawns and 0x80+.
        std::string reloadLabel = fmt::format("Reload with Exit {}", exitId);
        if (UIWidgets::Button(reloadLabel.c_str(), { .color = THEME_COLOR })) {
            func_8031D04C((map_e)currentMap, exitId);
        }

        ImGui::EndChild();
    }
}

void WarpCatalog_Register() {
    sCatalogPath = Ship::Context::GetRawInstance()->GetPathRelativeToAppDirectory("warp_catalog.json");
    LoadCatalog();

    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (OnMapLoad*)event;
        // Braces would split this across the macro's argument list; the preprocessor
        // only balances parentheses.
        Arrival arrival;
        arrival.fromMap = (int32_t)ev->prevMap;
        arrival.map = (int32_t)ev->nextMap;
        arrival.exit = ev->exit;
        sEntranceScanPending = true;
        // A repeated load of the same entrance adds nothing to map out.
        if (!sArrivals.empty() && sArrivals.front().map == arrival.map && sArrivals.front().exit == arrival.exit &&
            sArrivals.front().fromMap == arrival.fromMap) {
            return;
        }
        sArrivals.push_front(arrival);
        if (sArrivals.size() > kArrivalHistory) {
            sArrivals.pop_back();
        }
        sSelectedArrival = 0;
    });

    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, [](IEvent*) {
        if (!sEntranceScanPending) {
            return;
        }
        sEntranceScanPending = false;
        ScanMapEntrances();
    });
}

static RegisterShipInitFunc warpCatalogInitFunc(WarpCatalog_Register);
