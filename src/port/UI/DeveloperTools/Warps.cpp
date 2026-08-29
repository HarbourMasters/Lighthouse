#include "Warps.h"
#include "MapNames.h"

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/UI/LighthouseGui.hpp"
#include "port/UI/UIWidgets.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <imgui.h>
#include <libultraship/libultraship.h>
#include <spdlog/fmt/fmt.h>

#include "enums.h"
#include "functions.h"

namespace {

// level is stored rather than derived because map_getLevel dereferences its lookup
// without a null check, and unused maps aren't in that table.
struct WarpEntry {
    int32_t map;
    int32_t exit;
    int32_t level;
    const char* name;
};

// An entrance prop found in the loaded map.
struct WarpEntranceInfo {
    int32_t id;
    int32_t pos[3];
    uint32_t yaw;
};

// clang-format off
// Generated from the map_warp_* enums in enums.h. Entrance indices are written as
// their enum constants; the enum tags were matched to maps against the warp
// destinations in cutscene_skip.c, since the two naming schemes don't always agree.
// Indices with no constant (demo slots, Furnace Fun) stay as literals.
constexpr WarpEntry warpsInfo[] = {
    // LEVEL_1_MUMBOS_MOUNTAIN
    { MAP_2_MM_MUMBOS_MOUNTAIN, WARP_MM_1_MUMBOS_HUT, LEVEL_1_MUMBOS_MOUNTAIN, "Outside Mumbo's Skull" },
    { MAP_2_MM_MUMBOS_MOUNTAIN, WARP_MM_2_TOWER_BOTTOM, LEVEL_1_MUMBOS_MOUNTAIN, "Outside Ticker's Bottom" },
    { MAP_2_MM_MUMBOS_MOUNTAIN, WARP_MM_3_TOWER_TOP, LEVEL_1_MUMBOS_MOUNTAIN, "Outside Ticker's Top" },
    { MAP_2_MM_MUMBOS_MOUNTAIN, WARP_MM_4_WITCH_SWITCH, LEVEL_1_MUMBOS_MOUNTAIN, "Witch Switch Return" },
    { MAP_2_MM_MUMBOS_MOUNTAIN, WARP_MM_5_WORLD_ENTRACE, LEVEL_1_MUMBOS_MOUNTAIN, "Warp Pad" },
    { MAP_2_MM_MUMBOS_MOUNTAIN, 91, LEVEL_1_MUMBOS_MOUNTAIN, "Demo" },
    { MAP_C_MM_TICKERS_TOWER, WARP_MM_TICKERS_TOWER_1_TOP, LEVEL_1_MUMBOS_MOUNTAIN, "Ticker's Tower Top" },
    { MAP_C_MM_TICKERS_TOWER, WARP_MM_TICKERS_TOWER_2_BOTTOM, LEVEL_1_MUMBOS_MOUNTAIN, "Ticker's Tower Bottom" },
    { MAP_E_MM_MUMBOS_SKULL, WARP_MM_MUMBOS_HUT_1_ENTRANCE, LEVEL_1_MUMBOS_MOUNTAIN, "Mumbo's Skull" },
    // LEVEL_2_TREASURE_TROVE_COVE
    { MAP_5_TTC_BLUBBERS_SHIP, WARP_TCC_BLUBBERS_SHIP_5_TOP_HATCH, LEVEL_2_TREASURE_TROVE_COVE, "Crate" },
    { MAP_5_TTC_BLUBBERS_SHIP, WARP_TCC_BLUBBERS_SHIP_6_SIDE_HATCH, LEVEL_2_TREASURE_TROVE_COVE, "Underwater" },
    { MAP_6_TTC_NIPPERS_SHELL, WARP_TCC_NIPPERS_SHELL_1_ENTRANCE, LEVEL_2_TREASURE_TROVE_COVE, "Entrance" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_1_LEAKY, LEVEL_2_TREASURE_TROVE_COVE, "Leaky Cutscene Return" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_3_SANDCASTLE, LEVEL_2_TREASURE_TROVE_COVE, "Outside Sandcastle" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_4_WORLD_ENTRANCE, LEVEL_2_TREASURE_TROVE_COVE, "Warp Pad" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_6_SHIP_TOP_HATCH, LEVEL_2_TREASURE_TROVE_COVE, "Ship Deck" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_7_SHIP_SIDE_HATCH, LEVEL_2_TREASURE_TROVE_COVE, "Ship Pool" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_8_LIGHTHOUSE_TOP, LEVEL_2_TREASURE_TROVE_COVE, "Lighthouse Top" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_A_NIPPER, LEVEL_2_TREASURE_TROVE_COVE, "Nipper" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_C_LIGHTHOUSE_BOTTOM, LEVEL_2_TREASURE_TROVE_COVE, "Lighthouse Bottom" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_E_ISLAND_TOP_STAIRS, LEVEL_2_TREASURE_TROVE_COVE, "Heights" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_F_CLIFFSIDE_STAIRS, LEVEL_2_TREASURE_TROVE_COVE, "Cave" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_14_WITCH_SWITCH, LEVEL_2_TREASURE_TROVE_COVE, "Witch Switch Return" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, 91, LEVEL_2_TREASURE_TROVE_COVE, "Demo" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, 95, LEVEL_2_TREASURE_TROVE_COVE, "Sharkfood SNS Cutscene" },
    { MAP_7_TTC_TREASURE_TROVE_COVE, 128, LEVEL_2_TREASURE_TROVE_COVE, "Outside Sharkfood" },
    { MAP_8F_TTC_SHARKFOOD_ISLAND, WARP_TCC_SHARKFOOD_ISLAND_1_ENTRANCE, LEVEL_2_TREASURE_TROVE_COVE, "Entrance" },
    { MAP_A_TTC_SANDCASTLE, WARP_TCC_SANDCASTLE_1_ENTRANCE, LEVEL_2_TREASURE_TROVE_COVE, "Entrance" },
    { MAP_A_TTC_SANDCASTLE, 2, LEVEL_2_TREASURE_TROVE_COVE, "FF Dark Snippet Fight" },
    // LEVEL_3_CLANKERS_CAVERN
    { MAP_21_CC_WITCH_SWITCH_ROOM, WARP_CC_WITCH_SWITCH_1_TOP_ENTRANCE, LEVEL_3_CLANKERS_CAVERN, "From Blowhole" },
    { MAP_21_CC_WITCH_SWITCH_ROOM, WARP_CC_WITCH_SWITCH_14_WITCH_SWITCH, LEVEL_3_CLANKERS_CAVERN, "Witch Switch Return" },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_1_STOMACH_ROOFTOP, LEVEL_3_CLANKERS_CAVERN, "Stomach Ceiling" },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_2_MOUTH_ROOFTOP, LEVEL_3_CLANKERS_CAVERN, "Mouth Ceiling" },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_3_GOLD_FEATHER_ENTRANCE, LEVEL_3_CLANKERS_CAVERN, "Top" },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_4_LEFT_GILL, LEVEL_3_CLANKERS_CAVERN, "Lower Gill" },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_5_RIGHT_GILL, LEVEL_3_CLANKERS_CAVERN, "Upper Gill" },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_6_LEFT_TOOTH, LEVEL_3_CLANKERS_CAVERN, "Jiggy Tooth" },
    { MAP_22_CC_INSIDE_CLANKER, WARP_CC_INSIDE_CLANKER_7_RIGHT_TOOTH, LEVEL_3_CLANKERS_CAVERN, "Token Tooth" },
    { MAP_22_CC_INSIDE_CLANKER, 91, LEVEL_3_CLANKERS_CAVERN, "Mouth Ceiling Demo" },
    { MAP_23_CC_GOLDFEATHER_ROOM, WARP_CC_GOLD_FEATHER_1_ENTRANCE, LEVEL_3_CLANKERS_CAVERN, "Wonder Wing Room" },
    { MAP_B_CC_CLANKERS_CAVERN, WARP_CC_1_RIGHT_TOOTH, LEVEL_3_CLANKERS_CAVERN, "Left Tooth" },
    { MAP_B_CC_CLANKERS_CAVERN, WARP_CC_2_LEFT_TOOTH, LEVEL_3_CLANKERS_CAVERN, "Right Tooth" },
    { MAP_B_CC_CLANKERS_CAVERN, WARP_CC_3_RIGHT_GILLS, LEVEL_3_CLANKERS_CAVERN, "Left Gills" },
    { MAP_B_CC_CLANKERS_CAVERN, WARP_CC_4_LEFT_GILLS, LEVEL_3_CLANKERS_CAVERN, "Right Gills" },
    { MAP_B_CC_CLANKERS_CAVERN, WARP_CC_5_WORLD_ENTRANCE, LEVEL_3_CLANKERS_CAVERN, "CC Warp Pad" },
    { MAP_B_CC_CLANKERS_CAVERN, 92, LEVEL_3_CLANKERS_CAVERN, "FF Mutie Fight" },
    // LEVEL_4_BUBBLEGLOOP_SWAMP
    { MAP_10_BGS_MR_VILE, WARP_BGS_MR_VILE_2_CENTER, LEVEL_4_BUBBLEGLOOP_SWAMP, "Minigame Finish" },
    { MAP_10_BGS_MR_VILE, WARP_BGS_MR_VILE_3_LEFT_ENTRANCE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Right Nostril Tunnel" },
    { MAP_10_BGS_MR_VILE, WARP_BGS_MR_VILE_4_RIGHT_ENTRANCE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Left Nostril Tunnel" },
    { MAP_11_BGS_TIPTUP, WARP_BGS_TIPTUP_1_ENTRANCE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Inside Tanktup" },
    { MAP_11_BGS_TIPTUP, WARP_BGS_TIPTUP_2_PODIUM, LEVEL_4_BUBBLEGLOOP_SWAMP, "Minigame Win Return" },
    { MAP_11_BGS_TIPTUP, 92, LEVEL_4_BUBBLEGLOOP_SWAMP, "FF Tiptup Minigame" },
    { MAP_47_BGS_MUMBOS_SKULL, WARP_BGS_MUMBOS_HUT_1_ENTRANCE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Mumbo's Skull" },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_1_CROC_FRONT, LEVEL_4_BUBBLEGLOOP_SWAMP, "Croc Front" },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_2_WORLD_ENTRANCE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Warp Pad" },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_3_TURTLE, LEVEL_4_BUBBLEGLOOP_SWAMP, "Tanktup Mouth" },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_4_CROC_LEFT, LEVEL_4_BUBBLEGLOOP_SWAMP, "Right Nostril" },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_5_CROC_RIGHT, LEVEL_4_BUBBLEGLOOP_SWAMP, "Left Nostril" },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_6_MUMBOS_HUT, LEVEL_4_BUBBLEGLOOP_SWAMP, "Outside Mumbo's Skull" },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, WARP_BGS_14_WITCH_SWITCH, LEVEL_4_BUBBLEGLOOP_SWAMP, "Switch Return" },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, 91, LEVEL_4_BUBBLEGLOOP_SWAMP, "Demo?" },
    // LEVEL_5_FREEZEEZY_PEAK
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_1_WORLD_ENTRANCE, LEVEL_5_FREEZEEZY_PEAK, "Warp Pad" },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_6_WOZZA_CAVE, LEVEL_5_FREEZEEZY_PEAK, "Outside Wozza's Cave" },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_7_MUMBOS_HUT, LEVEL_5_FREEZEEZY_PEAK, "Outside Mumbo's Skull" },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_8_IGLOO, LEVEL_5_FREEZEEZY_PEAK, "Outside Igloo" },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_9_TREE_BOTTOM, LEVEL_5_FREEZEEZY_PEAK, "Christmas Tree" },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_D_TREE_STAR, LEVEL_5_FREEZEEZY_PEAK, "Lit Tree CS Return" },
    { MAP_27_FP_FREEZEEZY_PEAK, WARP_FP_15_WITCH_SWITCH, LEVEL_5_FREEZEEZY_PEAK, "Switch Return" },
    { MAP_41_FP_BOGGYS_IGLOO, WARP_FP_BOGGYS_IGLOO_1_ENTRANCE, LEVEL_5_FREEZEEZY_PEAK, "Entrance" },
    { MAP_48_FP_MUMBOS_SKULL, WARP_FP_MUMBOS_HUT_1_ENTRANCE, LEVEL_5_FREEZEEZY_PEAK, "Entrance" },
    { MAP_53_FP_CHRISTMAS_TREE, WARP_FP_CHRISTMAS_TREE_1_ENTRANCE, LEVEL_5_FREEZEEZY_PEAK, "Inside" },
    { MAP_7F_FP_WOZZAS_CAVE, WARP_FP_WOZZAS_CAVE_1_ENTRANCE, LEVEL_5_FREEZEEZY_PEAK, "Entrance" },
    // LEVEL_6_LAIR
    { MAP_69_GL_MM_LOBBY, WARP_GL_MM_LOBBY_1_NOTE_DOOR, LEVEL_6_LAIR, "50 Note Door" },
    { MAP_69_GL_MM_LOBBY, WARP_GL_MM_LOBBY_2_MM_ENTRANCE, LEVEL_6_LAIR, "Level Entrance" },
    { MAP_69_GL_MM_LOBBY, WARP_GL_MM_LOBBY_A_PUZZLE, LEVEL_6_LAIR, "Puzzle" },
    { MAP_69_GL_MM_LOBBY, WARP_GL_MM_LOBBY_12_ENTRANCE, LEVEL_6_LAIR, "Lair Entrance" },
    { MAP_6A_GL_TTC_AND_CC_PUZZLE, WARP_GL_TTC_PUZZLE_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Lower" },
    { MAP_6A_GL_TTC_AND_CC_PUZZLE, WARP_GL_TTC_PUZZLE_2_HIGHER_EXIT, LEVEL_6_LAIR, "Upper" },
    { MAP_6A_GL_TTC_AND_CC_PUZZLE, WARP_GL_TTC_PUZZLE_8_CAULDRON, LEVEL_6_LAIR, "Purple Cauldron" },
    { MAP_6A_GL_TTC_AND_CC_PUZZLE, WARP_GL_TTC_PUZZLE_A_CC_PUZZLE, LEVEL_6_LAIR, "CC Puzzle" },
    { MAP_6A_GL_TTC_AND_CC_PUZZLE, WARP_GL_TTC_PUZZLE_B_TTC_PUZZLE, LEVEL_6_LAIR, "TTC Puzzle" },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Middle" },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_2_PIPE_ROOM, LEVEL_6_LAIR, "Big Pipe" },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_3_TO_TTC, LEVEL_6_LAIR, "Red Eyes" },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_4_TO_CC, LEVEL_6_LAIR, "Small Pipe" },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_5_HIGHER_EXIT, LEVEL_6_LAIR, "Top" },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_A_WATERFALL, LEVEL_6_LAIR, "Waterfall" },
    { MAP_6B_GL_180_NOTE_DOOR, WARP_GL_CCW_PUZZLE_B_PUZZLE, LEVEL_6_LAIR, "Puzzle" },
    { MAP_6C_GL_RED_CAULDRON_ROOM, WARP_GL_PIPE_ROOM_1_ENTRANCE, LEVEL_6_LAIR, "Pipe" },
    { MAP_6C_GL_RED_CAULDRON_ROOM, WARP_GL_PIPE_ROOM_8_CAULDRON, LEVEL_6_LAIR, "Cauldron" },
    { MAP_6D_GL_TTC_LOBBY, WARP_GL_TTC_LOBBY_1_ENTRANCE, LEVEL_6_LAIR, "Stairs" },
    { MAP_6D_GL_TTC_LOBBY, WARP_GL_TTC_LOBBY_4_TTC_ENTRANCE, LEVEL_6_LAIR, "Level Exit" },
    { MAP_6E_GL_GV_LOBBY, WARP_GL_GV_LOBBY_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Stairs Down" },
    { MAP_6E_GL_GV_LOBBY, WARP_GL_GV_LOBBY_2_HIGHER_EXIT, LEVEL_6_LAIR, "Higher Exit" },
    { MAP_6E_GL_GV_LOBBY, WARP_GL_GV_LOBBY_3_GV_ENTRANCE, LEVEL_6_LAIR, "Level Entrance" },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Lower" },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_2_HIGHEST_EXIT, LEVEL_6_LAIR, "Upper" },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_5_MOUTH_EXIT, LEVEL_6_LAIR, "Witch" },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_6_FP_ENTRANCE, LEVEL_6_LAIR, "Level Entrance" },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_8_LOWER_CAULDRON, LEVEL_6_LAIR, "Purple Cauldron" },
    { MAP_6F_GL_FP_LOBBY, WARP_GL_FP_LOBBY_9_HIGHER_CAULDRON, LEVEL_6_LAIR, "Green Cauldron" },
    { MAP_70_GL_CC_LOBBY, WARP_GL_CC_LOBBY_1_ENTRANCE, LEVEL_6_LAIR, "Pipe" },
    { MAP_70_GL_CC_LOBBY, WARP_GL_CC_LOBBY_2_CC_ENTRANCE, LEVEL_6_LAIR, "Level Entrance" },
    { MAP_70_GL_CC_LOBBY, WARP_GL_CC_LOBBY_B_PUZZLE, LEVEL_6_LAIR, "BGS Puzzle" },
    { MAP_71_GL_STATUE_ROOM, WARP_GL_STATUE_ROOM_1_HIGHER_EXIT, LEVEL_6_LAIR, "Stairs Up" },
    { MAP_71_GL_STATUE_ROOM, WARP_GL_STATUE_ROOM_2_LOWER_ENTRANCE, LEVEL_6_LAIR, "Down" },
    { MAP_71_GL_STATUE_ROOM, WARP_GL_STATUE_ROOM_3_TO_BGS, LEVEL_6_LAIR, "Wood Tunnel" },
    { MAP_71_GL_STATUE_ROOM, WARP_GL_STATUE_ROOM_4_CEILING_EXIT, LEVEL_6_LAIR, "Ceiling" },
    { MAP_72_GL_BGS_LOBBY, WARP_GL_BGS_LOBBY_1_ENTRANCE, LEVEL_6_LAIR, "Entrance" },
    { MAP_72_GL_BGS_LOBBY, WARP_GL_BGS_LOBBY_2_BGS_ENTRANCE, LEVEL_6_LAIR, "Level Entrance" },
    { MAP_72_GL_BGS_LOBBY, WARP_GL_BGS_LOBBY_A_PUZZLE, LEVEL_6_LAIR, "FP Puzzle" },
    { MAP_74_GL_GV_PUZZLE, WARP_GL_GV_PUZZLE_1_TO_MMM, LEVEL_6_LAIR, "Red Eyes" },
    { MAP_74_GL_GV_PUZZLE, WARP_GL_GV_PUZZLE_2_ENTRANCE, LEVEL_6_LAIR, "Lower" },
    { MAP_74_GL_GV_PUZZLE, WARP_GL_GV_PUZZLE_A_PUZZLE, LEVEL_6_LAIR, "Puzzle" },
    { MAP_75_GL_MMM_LOBBY, WARP_GL_MMM_LOBBY_1_ENTRANCE, LEVEL_6_LAIR, "Front" },
    { MAP_75_GL_MMM_LOBBY, WARP_GL_MMM_LOBBY_2_MM_ENTRANCE, LEVEL_6_LAIR, "Level Entrance" },
    { MAP_75_GL_MMM_LOBBY, WARP_GL_MMM_LOBBY_3_COFFIN, LEVEL_6_LAIR, "Crypt Door" },
    { MAP_76_GL_640_NOTE_DOOR, WARP_GL_WATER_SWITCH_ROOM_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Mid" },
    { MAP_76_GL_640_NOTE_DOOR, WARP_GL_WATER_SWITCH_ROOM_2_MUMBO_TOKEN_EXIT, LEVEL_6_LAIR, "Top Red" },
    { MAP_76_GL_640_NOTE_DOOR, WARP_GL_WATER_SWITCH_ROOM_3_HIGHER_EXIT, LEVEL_6_LAIR, "Top Green" },
    { MAP_76_GL_640_NOTE_DOOR, WARP_GL_WATER_SWITCH_ROOM_4_UNDERWATER_EXIT, LEVEL_6_LAIR, "Red Eyes" },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_1_LOWER_ENTRANCE, LEVEL_6_LAIR, "Mid" },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_2_RBB_ENTRANCE, LEVEL_6_LAIR, "Level Entrance" },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_3_UNDERWATER_EXIT, LEVEL_6_LAIR, "Underwater Exit" },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_4_HIGHER_EXIT, LEVEL_6_LAIR, "Bricks" },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_8_CAULDRON, LEVEL_6_LAIR, "Green Cauldron" },
    { MAP_77_GL_RBB_LOBBY, WARP_GL_RBB_LOBBY_A_SWITCH, LEVEL_6_LAIR, "Switch CS Return" },
    { MAP_78_GL_RBB_AND_MMM_PUZZLE, WARP_GL_MMM_PUZZLE_1_UNDERWATER_ENTRANCE, LEVEL_6_LAIR, "MMM Entrance" },
    { MAP_78_GL_RBB_AND_MMM_PUZZLE, WARP_GL_MMM_PUZZLE_2_HIGHER_ENTRANCE, LEVEL_6_LAIR, "RBB Entrance" },
    { MAP_78_GL_RBB_AND_MMM_PUZZLE, WARP_GL_MMM_PUZZLE_5_RBB_PUZZLE, LEVEL_6_LAIR, "RBB Puzzle" },
    { MAP_78_GL_RBB_AND_MMM_PUZZLE, WARP_GL_MMM_PUZZLE_6_MMM_PUZZLE, LEVEL_6_LAIR, "MMM Puzzle" },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_1_WHIPCRACK_ENTRANCE, LEVEL_6_LAIR, "Whipcracks" },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_2_HIGHER_ENTRANCE, LEVEL_6_LAIR, "Red Dirt" },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_3_FURNACE_EXIT, LEVEL_6_LAIR, "Green" },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_6_CCW_ENTRANCE, LEVEL_6_LAIR, "Level Entrance" },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_8_CAULDRON, LEVEL_6_LAIR, "Red Cauldron" },
    { MAP_79_GL_CCW_LOBBY, WARP_GL_CCW_LOBBY_12_SWITCH, LEVEL_6_LAIR, "Switch CS Return" },
    { MAP_7A_GL_CRYPT, WARP_GL_COFFIN_ROOM_1_ENTRANCE, LEVEL_6_LAIR, "Entrance" },
    { MAP_7A_GL_CRYPT, WARP_GL_COFFIN_ROOM_A_SWITCH, LEVEL_6_LAIR, "Switch" },
    { MAP_80_GL_FF_ENTRANCE, WARP_GL_FURNACE_FUN_PATH_1_ENTRANCE, LEVEL_6_LAIR, "Lower" },
    { MAP_80_GL_FF_ENTRANCE, WARP_GL_FURNACE_FUN_PATH_2_EXIT, LEVEL_6_LAIR, "Pad" },
    { MAP_8E_GL_FURNACE_FUN, WARP_GL_FURNACE_FUN_1_AFTER_PAD, LEVEL_6_LAIR, "After Pad" },
    { MAP_8E_GL_FURNACE_FUN, WARP_GL_FURNACE_FUN_2_ENTRANCE_PAD, LEVEL_6_LAIR, "Pad" },
    { MAP_8E_GL_FURNACE_FUN, WARP_GL_FURNACE_FUN_4_AFTER_BOARD, LEVEL_6_LAIR, "After Board" },
    { MAP_8E_GL_FURNACE_FUN, WARP_GL_FURNACE_FUN_5_TO_TOWER, LEVEL_6_LAIR, "Back Exit" },
    { MAP_8E_GL_FURNACE_FUN, 5, LEVEL_6_LAIR, "GL Furnace Fun - 5" },
    { MAP_8E_GL_FURNACE_FUN, WARP_GL_FURNACE_FUN_8_CAULDRON, LEVEL_6_LAIR, "FF Yellow Cauldron" },
    { MAP_93_GL_DINGPOT, WARP_GL_TOWER_2_DINGPOT, LEVEL_6_LAIR, "Dingpot" },
    { MAP_93_GL_DINGPOT, WARP_GL_TOWER_5_ENTRANCE, LEVEL_6_LAIR, "Lower" },
    { MAP_93_GL_DINGPOT, WARP_GL_TOWER_8_CAULDRON, LEVEL_6_LAIR, "Yellow Cauldron" },
    { MAP_93_GL_DINGPOT, WARP_GL_TOWER_A_GRUNTY_PUZZLE, LEVEL_6_LAIR, "Puzzle" },
    // LEVEL_7_GOBIS_VALLEY
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_2_JINXY_SPHINX, LEVEL_7_GOBIS_VALLEY, "Jinxy Door" },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_3_MEMORY_PYRAMID, LEVEL_7_GOBIS_VALLEY, "Sun Door" },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_4_MAZE_PYRAMID_FRONT, LEVEL_7_GOBIS_VALLEY, "Central Pyramid Entrance" },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_5_WATER_PYRAMID, LEVEL_7_GOBIS_VALLEY, "Lower Star Door" },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_6_RUBEE_PYRAMID, LEVEL_7_GOBIS_VALLEY, "Kazooie Door" },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_7_MAZE_PYRAMID_BACK, LEVEL_7_GOBIS_VALLEY, "Central Pyramid Exit" },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_8_WORLD_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Warp Pad" },
    { MAP_12_GV_GOBIS_VALLEY, WARP_GV_A_EGG_ROOM, LEVEL_7_GOBIS_VALLEY, "Outside SNS Tomb" },
    { MAP_13_GV_MEMORY_GAME, WARP_GV_MEMORY_PYRAMID_1_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Entrance" },
    { MAP_13_GV_MEMORY_GAME, WARP_GV_MEMORY_PYRAMID_2_CENTER, LEVEL_7_GOBIS_VALLEY, "Center" },
    { MAP_14_GV_SANDYBUTTS_MAZE, WARP_GV_MAZE_PYRAMID_1_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Entrance" },
    { MAP_15_GV_WATER_PYRAMID, WARP_GV_WATER_PYRAMID_1_RETURN_TO_JIGGY, LEVEL_7_GOBIS_VALLEY, "Return To Jiggy" },
    { MAP_15_GV_WATER_PYRAMID, WARP_GV_WATER_PYRAMID_2_CEILING, LEVEL_7_GOBIS_VALLEY, "Ceiling" },
    { MAP_15_GV_WATER_PYRAMID, WARP_GV_WATER_PYRAMID_6_LOWER_EXIT, LEVEL_7_GOBIS_VALLEY, "Lower Exit" },
    { MAP_16_GV_RUBEES_CHAMBER, WARP_GV_RUBEES_CHAMBER_7_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Entrance" },
    { MAP_1A_GV_INSIDE_JINXY, WARP_GV_INSIDE_JINXY_2_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Entrance" },
    { MAP_92_GV_SNS_CHAMBER, WARP_GV_EGG_ROOM_5_ENTRANCE, LEVEL_7_GOBIS_VALLEY, "Entrance" },
    // LEVEL_8_CLICK_CLOCK_WOOD
    { MAP_40_CCW_HUB, WARP_CCW_1_WINTER, LEVEL_8_CLICK_CLOCK_WOOD, "Winter" },
    { MAP_40_CCW_HUB, WARP_CCW_2_SPRING, LEVEL_8_CLICK_CLOCK_WOOD, "Spring" },
    { MAP_40_CCW_HUB, WARP_CCW_3_SUMMER, LEVEL_8_CLICK_CLOCK_WOOD, "Summer" },
    { MAP_40_CCW_HUB, WARP_CCW_4_AUTUMN, LEVEL_8_CLICK_CLOCK_WOOD, "Autumn" },
    { MAP_40_CCW_HUB, WARP_CCW_5_SPRING_SWITCH, LEVEL_8_CLICK_CLOCK_WOOD, "Spring Switch" },
    { MAP_40_CCW_HUB, WARP_CCW_7_WORLD_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Warp Pad" },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_4_NABNUT_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Nabnut's Window" },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_5_SUMMER_SWITCH, LEVEL_8_CLICK_CLOCK_WOOD, "Summer Switch" },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_6_ZUBBA_HIVE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Zubba Hive" },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_7_NABNUT_HOUSE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's House" },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_8_TOP_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Top Door" },
    { MAP_43_CCW_SPRING, WARP_CCW_SPRING_9_MUMBOS_HUT, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Mumbo's Hut" },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_4_NABNUT_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's Window" },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_5_AUTUMN_SWITCH, LEVEL_8_CLICK_CLOCK_WOOD, "Autumn Switch" },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_6_ZUBBA_HIVE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Zubba Hive" },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_7_NABNUT_HOUSE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's House" },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_8_TOP_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Top Door" },
    { MAP_44_CCW_SUMMER, WARP_CCW_SUMMER_9_MUMBOS_HUT, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Mumbo's Hut" },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_4_NABNUT_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's Window" },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_5_ZUBBA_HIVE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Zubba Hive" },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_6_NABNUT_UPPER_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's Upper Window" },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_7_NABNUT_HOUSE, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's House" },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_8_TOP_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Top Door" },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_9_MUMBOS_HUT, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Mumbo's Hut" },
    { MAP_45_CCW_AUTUMN, WARP_CCW_AUTUMN_F_WINTER_SWITCH, LEVEL_8_CLICK_CLOCK_WOOD, "Winter Switch" },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_4_NABNUT_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's Window" },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_5_HIGHEST_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Highest Window" },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_6_NABNUT_UPPER_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Nabnut's Upper Window" },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_8_TOP_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Top Door" },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_9_MUMBOS_HUT, LEVEL_8_CLICK_CLOCK_WOOD, "Outside Mumbo's Hut" },
    { MAP_46_CCW_WINTER, WARP_CCW_WINTER_15_WITCH_SWITCH, LEVEL_8_CLICK_CLOCK_WOOD, "Witch Switch" },
    { MAP_46_CCW_WINTER, 91, LEVEL_8_CLICK_CLOCK_WOOD, "Demo" },
    { MAP_4A_CCW_SPRING_MUMBOS_SKULL, WARP_CCW_MUMBOS_HUT_SPRING_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_4B_CCW_SUMMER_MUMBOS_SKULL, WARP_CCW_MUMBOS_HUT_SUMMER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_4C_CCW_AUTUMN_MUMBOS_SKULL, WARP_CCW_MUMBOS_HUT_AUTUMN_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_4D_CCW_WINTER_MUMBOS_SKULL, WARP_CCW_MUMBOS_HUT_WINTER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_5A_CCW_SUMMER_ZUBBA_HIVE, WARP_CCW_BEEHIVE_SUMMER_2_TOP, LEVEL_8_CLICK_CLOCK_WOOD, "Top" },
    { MAP_5A_CCW_SUMMER_ZUBBA_HIVE, WARP_CCW_BEEHIVE_SUMMER_3_REDO, LEVEL_8_CLICK_CLOCK_WOOD, "Redo" },
    { MAP_5B_CCW_SPRING_ZUBBA_HIVE, WARP_CCW_BEEHIVE_SPRING_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_5C_CCW_AUTUMN_ZUBBA_HIVE, WARP_CCW_BEEHIVE_AUTUMN_2_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_5E_CCW_SPRING_NABNUTS_HOUSE, WARP_CCW_NABNUT_SPRING_1_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Door" },
    { MAP_5E_CCW_SPRING_NABNUTS_HOUSE, WARP_CCW_NABNUT_SPRING_2_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Window" },
    { MAP_5F_CCW_SUMMER_NABNUTS_HOUSE, WARP_CCW_NABNUT_SUMMER_1_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Door" },
    { MAP_5F_CCW_SUMMER_NABNUTS_HOUSE, WARP_CCW_NABNUT_SUMMER_2_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Window" },
    { MAP_60_CCW_AUTUMN_NABNUTS_HOUSE, WARP_CCW_NABNUT_AUTUMN_1_DOOR, LEVEL_8_CLICK_CLOCK_WOOD, "Door" },
    { MAP_60_CCW_AUTUMN_NABNUTS_HOUSE, WARP_CCW_NABNUT_AUTUMN_2_WINDOW, LEVEL_8_CLICK_CLOCK_WOOD, "Window" },
    { MAP_61_CCW_WINTER_NABNUTS_HOUSE, 2, LEVEL_8_CLICK_CLOCK_WOOD, "Window" },
    { MAP_62_CCW_WINTER_HONEYCOMB_ROOM, WARP_CCW_ATTIC_WINTER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_63_CCW_AUTUMN_NABNUTS_WATER_SUPPLY, WARP_CCW_WATER_ROOM_AUTUMN_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_64_CCW_WINTER_NABNUTS_WATER_SUPPLY, WARP_CCW_WATER_ROOM_WINTER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_65_CCW_SPRING_WHIPCRACK_ROOM, WARP_CCW_WHIPCRACK_SPRING_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_66_CCW_SUMMER_WHIPCRACK_ROOM, WARP_CCW_WHIPCRACK_SUMMER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_67_CCW_AUTUMN_WHIPCRACK_ROOM, WARP_CCW_WHIPCRACK_AUTUMN_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    { MAP_68_CCW_WINTER_WHIPCRACK_ROOM, WARP_CCW_WHIPCRACK_WINTER_1_ENTRANCE, LEVEL_8_CLICK_CLOCK_WOOD, "Entrance" },
    // LEVEL_9_RUSTY_BUCKET_BAY
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_1_CAPTAIN_ROOM_WINDOW, LEVEL_9_RUSTY_BUCKET_BAY, "Captain Room Window" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_2_CREW_ROOM_WINDOW, LEVEL_9_RUSTY_BUCKET_BAY, "Crew Room Window" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_3_ENGINE_SWITCH_PIPE, LEVEL_9_RUSTY_BUCKET_BAY, "Engine Switch Pipe" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_4_KITCHEN_PIPE, LEVEL_9_RUSTY_BUCKET_BAY, "Kitchen Pipe" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_5_NAV_ROOM_PIPE, LEVEL_9_RUSTY_BUCKET_BAY, "Nav Room Pipe" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_6_STORAGE_PIPE, LEVEL_9_RUSTY_BUCKET_BAY, "Storage Pipe" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_7_ENGINE_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Engine Entrance" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_8_BOAT_ROOM, LEVEL_9_RUSTY_BUCKET_BAY, "Boat Room" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_9_FIRST_CONTAINER, LEVEL_9_RUSTY_BUCKET_BAY, "First Container" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_A_SECOND_CONTAINER, LEVEL_9_RUSTY_BUCKET_BAY, "Second Container" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_B_THIRD_CONTAINER, LEVEL_9_RUSTY_BUCKET_BAY, "Third Container" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_C_AFTER_BOSS, LEVEL_9_RUSTY_BUCKET_BAY, "After Boss" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_D_FISH_WAREHOUSE, LEVEL_9_RUSTY_BUCKET_BAY, "Fish Warehouse" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_E_WITCH_SWITCH, LEVEL_9_RUSTY_BUCKET_BAY, "Witch Switch" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_10_WORLD_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "World Entrance" },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, WARP_RBB_13_ANCHOR_ROOM, LEVEL_9_RUSTY_BUCKET_BAY, "Anchor Room" },
    { MAP_34_RBB_ENGINE_ROOM, WARP_RBB_ENGINE_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_34_RBB_ENGINE_ROOM, WARP_RBB_ENGINE_2_LEFT_SWITCH, LEVEL_9_RUSTY_BUCKET_BAY, "Left Switch" },
    { MAP_34_RBB_ENGINE_ROOM, WARP_RBB_ENGINE_3_RIGHT_SWITCH, LEVEL_9_RUSTY_BUCKET_BAY, "Right Switch" },
    { MAP_34_RBB_ENGINE_ROOM, WARP_RBB_ENGINE_4_PIPE_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Pipe Entrance" },
    { MAP_35_RBB_WAREHOUSE, WARP_RBB_WAREHOUSE_1_DOOR, LEVEL_9_RUSTY_BUCKET_BAY, "Door" },
    { MAP_35_RBB_WAREHOUSE, WARP_RBB_WAREHOUSE_2_WINDOW, LEVEL_9_RUSTY_BUCKET_BAY, "Window" },
    { MAP_36_RBB_BOATHOUSE, WARP_RBB_BOAT_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_37_RBB_CONTAINER_1, WARP_RBB_CONTAINER_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_38_RBB_CONTAINER_3, WARP_RBB_CONTAINER_3_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_39_RBB_CREW_CABIN, WARP_RBB_CREW_CABIN_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_3A_RBB_BOSS_BOOM_BOX, WARP_RBB_BOSS_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_3A_RBB_BOSS_BOOM_BOX, WARP_RBB_BOSS_2_REDO, LEVEL_9_RUSTY_BUCKET_BAY, "Redo" },
    { MAP_3B_RBB_STORAGE_ROOM, WARP_RBB_STORAGE_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_3C_RBB_KITCHEN, WARP_RBB_KITCHEN_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_3D_RBB_NAVIGATION_ROOM, WARP_RBB_NAVIGATION_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_3E_RBB_CONTAINER_2, WARP_RBB_CONTAINER_2_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_3F_RBB_CAPTAINS_CABIN, WARP_RBB_CAPTAIN_CABIN_1_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_8B_RBB_ANCHOR_ROOM, WARP_RBB_ANCHOR_2_ENTRANCE, LEVEL_9_RUSTY_BUCKET_BAY, "Entrance" },
    { MAP_8B_RBB_ANCHOR_ROOM, WARP_RBB_ANCHOR_4_SWITCH, LEVEL_9_RUSTY_BUCKET_BAY, "Switch" },
    // LEVEL_A_MAD_MONSTER_MANSION
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_2_MASNION_DOOR, LEVEL_A_MAD_MONSTER_MANSION, "Mansion Door" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_3_WELL_TOP, LEVEL_A_MAD_MONSTER_MANSION, "Well Top" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_4_SHED, LEVEL_A_MAD_MONSTER_MANSION, "Shed" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_5_CHURCH_DOOR, LEVEL_A_MAD_MONSTER_MANSION, "Church Door" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_6_CHURCH_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "Church Window" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_7_CHIMNEY, LEVEL_A_MAD_MONSTER_MANSION, "Chimney" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_8_DRAINPIPE_BOTTOM, LEVEL_A_MAD_MONSTER_MANSION, "Outside Drainpipe" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_9_CELLAR, LEVEL_A_MAD_MONSTER_MANSION, "Outside Cellar" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_A_F1_RED_FEATHER_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F1 Red Feather Window" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_B_F1_EGG_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F1 Egg Window" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_C_F2_BATHROOM_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F2 Bathroom Window" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_D_F2_GOLD_FEATHER_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F2 Gold Feather Window" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_E_F3_BEDROOM_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F3 Bedroom Window" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_F_F3_NOTE_WINDOW, LEVEL_A_MAD_MONSTER_MANSION, "F3 Note Window" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_10_CHURCH_TOP, LEVEL_A_MAD_MONSTER_MANSION, "Church Top" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_11_CHURCH_CLOCK, LEVEL_A_MAD_MONSTER_MANSION, "Church Clock" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_12_MUMBOS_HUT, LEVEL_A_MAD_MONSTER_MANSION, "Outside Mumbo's Hut" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_13_WELL_BOTTOM, LEVEL_A_MAD_MONSTER_MANSION, "Well Bottom" },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, WARP_MMM_14_WORLD_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "World Entrance" },
    { MAP_1C_MMM_CHURCH, WARP_MMM_CHURCH_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    { MAP_1C_MMM_CHURCH, WARP_MMM_CHURCH_14_WITCH_SWITCH, LEVEL_A_MAD_MONSTER_MANSION, "Witch Switch" },
    { MAP_1D_MMM_CELLAR, WARP_MMM_CELLAR_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    { MAP_24_MMM_TUMBLARS_SHED, WARP_MMM_SHED_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    { MAP_25_MMM_WELL, WARP_MMM_WELL_1_TOP_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Top" },
    { MAP_25_MMM_WELL, WARP_MMM_WELL_4_BOTTOM_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Bottom" },
    { MAP_26_MMM_NAPPERS_ROOM, WARP_MMM_DINING_1_FRONT_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Front" },
    { MAP_26_MMM_NAPPERS_ROOM, WARP_MMM_DINING_2_CHIMNEY, LEVEL_A_MAD_MONSTER_MANSION, "Chimney" },
    { MAP_28_MMM_EGG_ROOM, WARP_MMM_EGG_ROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    { MAP_29_MMM_NOTE_ROOM, WARP_MMM_NOTE_ROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    { MAP_2A_MMM_FEATHER_ROOM, WARP_MMM_RED_FEATHER_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    { MAP_2B_MMM_SECRET_CHURCH_ROOM, WARP_MMM_SECRET_CHURCH_ROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    { MAP_2C_MMM_BATHROOM, WARP_MMM_BATHROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    { MAP_2C_MMM_BATHROOM, WARP_MMM_BATHROOM_4_LOGGO, LEVEL_A_MAD_MONSTER_MANSION, "Loggo" },
    { MAP_2D_MMM_BEDROOM, WARP_MMM_BEDROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    { MAP_2E_MMM_HONEYCOMB_ROOM, WARP_MMM_HONEYCOMB_ROOM_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    { MAP_2F_MMM_WATERDRAIN_BARREL, WARP_MMM_DRAINPIPE_1_TOP_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Top" },
    { MAP_2F_MMM_WATERDRAIN_BARREL, WARP_MMM_DRAINPIPE_2_BOTTOM_EXIT, LEVEL_A_MAD_MONSTER_MANSION, "Bottom" },
    { MAP_30_MMM_MUMBOS_SKULL, WARP_MMM_MUMBOS_HUT_1_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    { MAP_8D_MMM_INSIDE_LOGGO, WARP_MMM_SEPTIC_TANK_4_ENTRANCE, LEVEL_A_MAD_MONSTER_MANSION, "Entrance" },
    // LEVEL_B_SPIRAL_MOUNTAIN
    { MAP_1_SM_SPIRAL_MOUNTAIN, WARP_SM_SPIRAL_MOUNTAIN_1_BANJOS_HOUSE, LEVEL_B_SPIRAL_MOUNTAIN, "Outside Banjo's House" },
    { MAP_1_SM_SPIRAL_MOUNTAIN, WARP_SM_SPIRAL_MOUNTAIN_13_LAIR_ENTRANCE, LEVEL_B_SPIRAL_MOUNTAIN, "Lair Entrance" },
    { MAP_8C_SM_BANJOS_HOUSE, WARP_SM_BANJOS_HOUSE_1_ENTRANCE, LEVEL_B_SPIRAL_MOUNTAIN, "Banjo's House" },
    { MAP_8C_SM_BANJOS_HOUSE, WARP_SM_BANJOS_HOUSE_2_BOTTLES, LEVEL_B_SPIRAL_MOUNTAIN, "SM Banjo's House - Bottles" },
    // LEVEL_C_BOSS
    { MAP_90_GL_BATTLEMENTS, WARP_GL_BOSS_1_ENTRANCE, LEVEL_C_BOSS, "GL Battlements - Entrance" },
};
// clang-format on

struct LevelNameEntry {
    const char* displayName;
};

constexpr LevelNameEntry levelNames[] = {
    { "Unknown" },          { "Mumbo's Mountain" },    { "Treasure Trove Cove" }, { "Clanker's Cavern" },
    { "Bubblegloop Swamp" }, { "Freezeezy Peak" },     { "Gruntilda's Lair" },    { "Gobi's Valley" },
    { "Click Clock Wood" }, { "Rusty Bucket Bay" },    { "Mad Monster Mansion" }, { "Spiral Mountain" },
    { "Boss Arena" },       { "Cutscenes" },
};
constexpr int32_t levelCount = (int32_t)(sizeof(levelNames) / sizeof(levelNames[0]));

// func_803084F0 only maps these index ranges to marker actors; everything else
// returns actor 0 and can never resolve to a prop. Indices >= 0x80 (and 0x63/0x65)
// take the scripted-spawn path instead and are deliberately left out.
constexpr struct {
    int32_t first;
    int32_t last;
} probeRanges[] = {
    { 0x01, 0x19 },
    { 0x5B, 0x62 },
};

const char* quickWarpNames[] = {
    "Mumbo's Mountain", "Treasure Trove Cove", "Clanker's Cavern", "Bubblegloop Swamp",   "Freezeezy Peak",
    "Gobi's Valley",    "Click Clock Wood",    "Rusty Bucket Bay", "Mad Monster Mansion", "Spiral Mountain",
    "Cutscene",         "Gruntilda's Lair",    "Boss Arena",
};

constexpr int32_t quickWarpMaps[] = {
    MAP_2_MM_MUMBOS_MOUNTAIN,    MAP_7_TTC_TREASURE_TROVE_COVE, MAP_B_CC_CLANKERS_CAVERN,
    MAP_D_BGS_BUBBLEGLOOP_SWAMP, MAP_27_FP_FREEZEEZY_PEAK,      MAP_12_GV_GOBIS_VALLEY,
    MAP_40_CCW_HUB,              MAP_31_RBB_RUSTY_BUCKET_BAY,   MAP_1B_MMM_MAD_MONSTER_MANSION,
    MAP_1_SM_SPIRAL_MOUNTAIN,    MAP_7B_CS_INTRO_GL_DINGPOT_1,  MAP_69_GL_MM_LOBBY,
    MAP_90_GL_BATTLEMENTS,
};

int32_t quickWarpIndex = 0;
int32_t currentExitId = 0;
std::vector<WarpEntranceInfo> mapEntrances;
// Props are built after OnMapLoad fires, so the scan waits for the next frame.
bool entranceScanPending = false;
// warpsInfo never changes, so the level grouping is built once at startup.
std::vector<std::vector<const WarpEntry*>> warpsByLevel;

void ScanMapEntrances() {
    mapEntrances.clear();
    for (const auto& range : probeRanges) {
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
            mapEntrances.push_back(info);
        }
    }
}

const char* MapDisplayName(int32_t map) {
    return (map >= 0 && map < MAP_NUM_MAPS) ? mapNames[map].displayName : "Unknown";
}

const char* LevelDisplayName(int32_t level) {
    return levelNames[(level > 0 && level < levelCount) ? level : 0].displayName;
}

const char* FindWarpName(int32_t map, int32_t exit) {
    for (const auto& entry : warpsInfo) {
        if (entry.map == map && entry.exit == exit) {
            return entry.name;
        }
    }
    return nullptr;
}

bool ExitIsProbeable(int32_t exit) {
    for (const auto& range : probeRanges) {
        if (exit >= range.first && exit <= range.last) {
            return true;
        }
    }
    return false;
}

bool EntranceExists(int32_t exit) {
    return std::any_of(mapEntrances.begin(), mapEntrances.end(),
                       [exit](const WarpEntranceInfo& info) { return info.id == exit; });
}

void BuildGrouping() {
    warpsByLevel.assign(levelCount, {});
    for (const auto& entry : warpsInfo) {
        warpsByLevel[(entry.level > 0 && entry.level < levelCount) ? entry.level : 0].push_back(&entry);
    }
}

void DrawMapEntranceSelector(int32_t currentMap) {
    ImGui::SeparatorText("Current Map");
    ImGui::Text("%s (0x%02X)", MapDisplayName(currentMap), currentMap);

    if (mapEntrances.empty()) {
        ImGui::TextDisabled("No entrance props detected in this map.");
        return;
    }

    static int32_t entranceIndex = 0;
    entranceIndex = std::clamp(entranceIndex, 0, (int32_t)mapEntrances.size() - 1);
    auto describe = [currentMap](const WarpEntranceInfo& info) {
        const char* known = FindWarpName(currentMap, info.id);
        return fmt::format("Entrance {} - {}  ({}, {}, {})", info.id, known ? known : "unnamed", info.pos[0],
                           info.pos[1], info.pos[2]);
    };
    if (ImGui::BeginCombo("##currentMapEntrance", describe(mapEntrances[entranceIndex]).c_str())) {
        for (int32_t i = 0; i < (int32_t)mapEntrances.size(); i++) {
            // Unnamed indices dim, so the known ones stand out at a glance.
            bool known = FindWarpName(currentMap, mapEntrances[i].id) != nullptr;
            if (!known) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            }
            if (ImGui::Selectable(describe(mapEntrances[i]).c_str(), entranceIndex == i)) {
                entranceIndex = i;
            }
            if (!known) {
                ImGui::PopStyleColor();
            }
        }
        ImGui::EndCombo();
    }
    if (UIWidgets::Button("Warp to Entrance", { .color = THEME_COLOR })) {
        func_8031D04C((map_e)currentMap, mapEntrances[entranceIndex].id);
    }
}

void DrawDestinationList(int32_t currentMap) {
    ImGui::SeparatorText("Destinations");
    if (ImGui::BeginChild("WarpDestinations")) {
        // Starts at 0 so an entry whose level never resolved still has a group to live in.
        for (int32_t level = 0; level < levelCount; level++) {
            const std::vector<const WarpEntry*>& entries = warpsByLevel[level];
            if (entries.empty()) {
                continue;
            }
            ImGui::PushID(level);
            if (ImGui::CollapsingHeader(fmt::format("{} ({})", LevelDisplayName(level), entries.size()).c_str())) {
                if (ImGui::BeginTable("LevelWarps", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Destination", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Map / Entrance");
                    for (int32_t i = 0; i < (int32_t)entries.size(); i++) {
                        const WarpEntry& entry = *entries[i];
                        ImGui::PushID(i);
                        ImGui::TableNextColumn();
                        if (UIWidgets::Button(entry.name, { .color = THEME_COLOR })) {
                            func_8031D04C((map_e)entry.map, entry.exit);
                        }
                        ImGui::TableNextColumn();
                        ImGui::Text("0x%02X / %d", entry.map, entry.exit);
                        // Only the loaded map has been scanned, so this can only speak for it.
                        if (entry.map == currentMap && ExitIsProbeable(entry.exit) && !EntranceExists(entry.exit)) {
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "(!)");
                            UIWidgets::Tooltip("No entrance prop with this index in the loaded map.");
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
    }
}

void DrawPlayerTools() {
    int32_t playerPosition[3];
    player_getPosition_s32(playerPosition);

    ImGui::SeparatorText("Player Position");
    if (ImGui::BeginTable("CurrentPlayerPosition", 2)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("PlayerPos", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        for (int i = 0; i < 3; i++) {
            ImGui::TableNextColumn();
            ImGui::Text("Pos %c: ", "XYZ"[i]);
            ImGui::TableNextColumn();
            ImGui::Text("%d", playerPosition[i]);
        }
        ImGui::EndTable();
    }

    ImGui::SeparatorText("Warp Player");
    static std::string warpPos[3] = { "0", "0", "0" };
    if (ImGui::BeginTable("WarpPlayer", 2)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("PlayerPos", ImGuiTableColumnFlags_WidthFixed, 250.0f);
        for (int i = 0; i < 3; i++) {
            ImGui::PushID(i);
            ImGui::TableNextColumn();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
            ImGui::Text("Pos %c: ", "XYZ"[i]);
            ImGui::TableNextColumn();
            UIWidgets::InputString("##WarpPos", &warpPos[i],
                                   UIWidgets::InputOptions()
                                       .Size(ImGui::GetContentRegionAvail() -
                                             ImVec2((ImGui::GetFontSize() * 5 + ImGui::GetStyle().ItemSpacing.x), 0))
                                       .Color(THEME_COLOR));
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (UIWidgets::Button("Warp Player", UIWidgets::ButtonOptions().Color(THEME_COLOR).Size(UIWidgets::Sizes::Inline))) {
        try {
            f32 target[3] = { std::stof(warpPos[0]), std::stof(warpPos[1]), std::stof(warpPos[2]) };
            playerPosition_set(target);
        } catch (const std::exception&) {
            // A half-typed or empty coordinate just means there is nothing to warp to yet.
        }
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Copy Current Player Coords",
                          UIWidgets::ButtonOptions().Color(THEME_COLOR).Size(UIWidgets::Sizes::Inline))) {
        for (int i = 0; i < 3; i++) {
            warpPos[i] = std::to_string(playerPosition[i]);
        }
    }
}

} // namespace

void DrawWarpsTab() {
    int32_t currentMap = (int32_t)gsworld_getMap();

    if (ImGui::BeginTable("WarpsLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("Warps", ImGuiTableColumnFlags_WidthStretch, 2.0f);
        ImGui::TableSetupColumn("Player", ImGuiTableColumnFlags_WidthStretch, 1.0f);

        ImGui::TableNextColumn();
        DrawMapEntranceSelector(currentMap);
        DrawDestinationList(currentMap);

        ImGui::TableNextColumn();
        DrawPlayerTools();

        ImGui::EndTable();
    }
}

void Warps_Register() {
    BuildGrouping();

    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent*) { entranceScanPending = true; });

    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, [](IEvent*) {
        if (!entranceScanPending) {
            return;
        }
        entranceScanPending = false;
        ScanMapEntrances();
    });
}

static RegisterShipInitFunc warpsInitFunc(Warps_Register);
