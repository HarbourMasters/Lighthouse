#pragma once

// Generated from the map_e enum in include/enums.h. Keeps the warp catalog able to
// print real enum names in its exported rows instead of bare map ids.

#include "enums.h"

struct MapNameEntry {
    const char* enumName;
    const char* displayName;
};

inline constexpr MapNameEntry mapNames[MAP_NUM_MAPS] = {
    { "MAP_0_UNKNOWN", "Unknown" }, // 0x00
    { "MAP_1_SM_SPIRAL_MOUNTAIN", "SM Spiral Mountain" }, // 0x01
    { "MAP_2_MM_MUMBOS_MOUNTAIN", "MM Mumbo's Mountain" }, // 0x02
    { "MAP_3_UNUSED", "Unused" }, // 0x03
    { "MAP_4_UNUSED", "Unused" }, // 0x04
    { "MAP_5_TTC_BLUBBERS_SHIP", "TTC Blubber's Ship" }, // 0x05
    { "MAP_6_TTC_NIPPERS_SHELL", "TTC Nipper's Shell" }, // 0x06
    { "MAP_7_TTC_TREASURE_TROVE_COVE", "TTC Treasure Trove Cove" }, // 0x07
    { "MAP_8_UNUSED", "Unused" }, // 0x08
    { "MAP_9_UNUSED", "Unused" }, // 0x09
    { "MAP_A_TTC_SANDCASTLE", "TTC Sandcastle" }, // 0x0A
    { "MAP_B_CC_CLANKERS_CAVERN", "CC Clanker's Cavern" }, // 0x0B
    { "MAP_C_MM_TICKERS_TOWER", "MM Ticker's Tower" }, // 0x0C
    { "MAP_D_BGS_BUBBLEGLOOP_SWAMP", "BGS Bubblegloop Swamp" }, // 0x0D
    { "MAP_E_MM_MUMBOS_SKULL", "MM Mumbo's Skull" }, // 0x0E
    { "MAP_F_UNUSED", "Unused" }, // 0x0F
    { "MAP_10_BGS_MR_VILE", "BGS Mr Vile" }, // 0x10
    { "MAP_11_BGS_TIPTUP", "BGS Tiptup" }, // 0x11
    { "MAP_12_GV_GOBIS_VALLEY", "GV Gobi's Valley" }, // 0x12
    { "MAP_13_GV_MEMORY_GAME", "GV Memory Game" }, // 0x13
    { "MAP_14_GV_SANDYBUTTS_MAZE", "GV Sandybutt's Maze" }, // 0x14
    { "MAP_15_GV_WATER_PYRAMID", "GV Water Pyramid" }, // 0x15
    { "MAP_16_GV_RUBEES_CHAMBER", "GV Rubee's Chamber" }, // 0x16
    { "MAP_17_UNUSED", "Unused" }, // 0x17
    { "MAP_18_UNUSED", "Unused" }, // 0x18
    { "MAP_19_UNUSED", "Unused" }, // 0x19
    { "MAP_1A_GV_INSIDE_JINXY", "GV Inside Jinxy" }, // 0x1A
    { "MAP_1B_MMM_MAD_MONSTER_MANSION", "MMM Mad Monster Mansion" }, // 0x1B
    { "MAP_1C_MMM_CHURCH", "MMM Church" }, // 0x1C
    { "MAP_1D_MMM_CELLAR", "MMM Cellar" }, // 0x1D
    { "MAP_1E_CS_START_NINTENDO", "CS Start Nintendo" }, // 0x1E
    { "MAP_1F_CS_START_RAREWARE", "CS Start Rareware" }, // 0x1F
    { "MAP_20_CS_END_NOT_100", "CS End Not 100" }, // 0x20
    { "MAP_21_CC_WITCH_SWITCH_ROOM", "CC Witch Switch Room" }, // 0x21
    { "MAP_22_CC_INSIDE_CLANKER", "CC Inside Clanker" }, // 0x22
    { "MAP_23_CC_GOLDFEATHER_ROOM", "CC Goldfeather Room" }, // 0x23
    { "MAP_24_MMM_TUMBLARS_SHED", "MMM Tumblar's Shed" }, // 0x24
    { "MAP_25_MMM_WELL", "MMM Well" }, // 0x25
    { "MAP_26_MMM_NAPPERS_ROOM", "MMM Napper's Room" }, // 0x26
    { "MAP_27_FP_FREEZEEZY_PEAK", "FP Freezeezy Peak" }, // 0x27
    { "MAP_28_MMM_EGG_ROOM", "MMM Egg Room" }, // 0x28
    { "MAP_29_MMM_NOTE_ROOM", "MMM Note Room" }, // 0x29
    { "MAP_2A_MMM_FEATHER_ROOM", "MMM Feather Room" }, // 0x2A
    { "MAP_2B_MMM_SECRET_CHURCH_ROOM", "MMM Secret Church Room" }, // 0x2B
    { "MAP_2C_MMM_BATHROOM", "MMM Bathroom" }, // 0x2C
    { "MAP_2D_MMM_BEDROOM", "MMM Bedroom" }, // 0x2D
    { "MAP_2E_MMM_HONEYCOMB_ROOM", "MMM Honeycomb Room" }, // 0x2E
    { "MAP_2F_MMM_WATERDRAIN_BARREL", "MMM Waterdrain Barrel" }, // 0x2F
    { "MAP_30_MMM_MUMBOS_SKULL", "MMM Mumbo's Skull" }, // 0x30
    { "MAP_31_RBB_RUSTY_BUCKET_BAY", "RBB Rusty Bucket Bay" }, // 0x31
    { "MAP_32_UNUSED", "Unused" }, // 0x32
    { "MAP_33_UNUSED", "Unused" }, // 0x33
    { "MAP_34_RBB_ENGINE_ROOM", "RBB Engine Room" }, // 0x34
    { "MAP_35_RBB_WAREHOUSE", "RBB Warehouse" }, // 0x35
    { "MAP_36_RBB_BOATHOUSE", "RBB Boathouse" }, // 0x36
    { "MAP_37_RBB_CONTAINER_1", "RBB Container 1" }, // 0x37
    { "MAP_38_RBB_CONTAINER_3", "RBB Container 3" }, // 0x38
    { "MAP_39_RBB_CREW_CABIN", "RBB Crew Cabin" }, // 0x39
    { "MAP_3A_RBB_BOSS_BOOM_BOX", "RBB Boss Boom Box" }, // 0x3A
    { "MAP_3B_RBB_STORAGE_ROOM", "RBB Storage Room" }, // 0x3B
    { "MAP_3C_RBB_KITCHEN", "RBB Kitchen" }, // 0x3C
    { "MAP_3D_RBB_NAVIGATION_ROOM", "RBB Navigation Room" }, // 0x3D
    { "MAP_3E_RBB_CONTAINER_2", "RBB Container 2" }, // 0x3E
    { "MAP_3F_RBB_CAPTAINS_CABIN", "RBB Captain's Cabin" }, // 0x3F
    { "MAP_40_CCW_HUB", "CCW Hub" }, // 0x40
    { "MAP_41_FP_BOGGYS_IGLOO", "FP Boggy's Igloo" }, // 0x41
    { "MAP_42_UNUSED", "Unused" }, // 0x42
    { "MAP_43_CCW_SPRING", "CCW Spring" }, // 0x43
    { "MAP_44_CCW_SUMMER", "CCW Summer" }, // 0x44
    { "MAP_45_CCW_AUTUMN", "CCW Autumn" }, // 0x45
    { "MAP_46_CCW_WINTER", "CCW Winter" }, // 0x46
    { "MAP_47_BGS_MUMBOS_SKULL", "BGS Mumbo's Skull" }, // 0x47
    { "MAP_48_FP_MUMBOS_SKULL", "FP Mumbo's Skull" }, // 0x48
    { "MAP_49_UNUSED", "Unused" }, // 0x49
    { "MAP_4A_CCW_SPRING_MUMBOS_SKULL", "CCW Spring Mumbo's Skull" }, // 0x4A
    { "MAP_4B_CCW_SUMMER_MUMBOS_SKULL", "CCW Summer Mumbo's Skull" }, // 0x4B
    { "MAP_4C_CCW_AUTUMN_MUMBOS_SKULL", "CCW Autumn Mumbo's Skull" }, // 0x4C
    { "MAP_4D_CCW_WINTER_MUMBOS_SKULL", "CCW Winter Mumbo's Skull" }, // 0x4D
    { "MAP_4E_UNUSED", "Unused" }, // 0x4E
    { "MAP_4F_UNUSED", "Unused" }, // 0x4F
    { "MAP_50_UNUSED", "Unused" }, // 0x50
    { "MAP_51_UNUSED", "Unused" }, // 0x51
    { "MAP_52_UNUSED", "Unused" }, // 0x52
    { "MAP_53_FP_CHRISTMAS_TREE", "FP Christmas Tree" }, // 0x53
    { "MAP_54_UNUSED", "Unused" }, // 0x54
    { "MAP_55_UNUSED", "Unused" }, // 0x55
    { "MAP_56_UNUSED", "Unused" }, // 0x56
    { "MAP_57_UNUSED", "Unused" }, // 0x57
    { "MAP_58_UNUSED", "Unused" }, // 0x58
    { "MAP_59_UNUSED", "Unused" }, // 0x59
    { "MAP_5A_CCW_SUMMER_ZUBBA_HIVE", "CCW Summer Zubba Hive" }, // 0x5A
    { "MAP_5B_CCW_SPRING_ZUBBA_HIVE", "CCW Spring Zubba Hive" }, // 0x5B
    { "MAP_5C_CCW_AUTUMN_ZUBBA_HIVE", "CCW Autumn Zubba Hive" }, // 0x5C
    { "MAP_5D_UNUSED", "Unused" }, // 0x5D
    { "MAP_5E_CCW_SPRING_NABNUTS_HOUSE", "CCW Spring Nabnut's House" }, // 0x5E
    { "MAP_5F_CCW_SUMMER_NABNUTS_HOUSE", "CCW Summer Nabnut's House" }, // 0x5F
    { "MAP_60_CCW_AUTUMN_NABNUTS_HOUSE", "CCW Autumn Nabnut's House" }, // 0x60
    { "MAP_61_CCW_WINTER_NABNUTS_HOUSE", "CCW Winter Nabnut's House" }, // 0x61
    { "MAP_62_CCW_WINTER_HONEYCOMB_ROOM", "CCW Winter Honeycomb Room" }, // 0x62
    { "MAP_63_CCW_AUTUMN_NABNUTS_WATER_SUPPLY", "CCW Autumn Nabnut's Water Supply" }, // 0x63
    { "MAP_64_CCW_WINTER_NABNUTS_WATER_SUPPLY", "CCW Winter Nabnut's Water Supply" }, // 0x64
    { "MAP_65_CCW_SPRING_WHIPCRACK_ROOM", "CCW Spring Whipcrack Room" }, // 0x65
    { "MAP_66_CCW_SUMMER_WHIPCRACK_ROOM", "CCW Summer Whipcrack Room" }, // 0x66
    { "MAP_67_CCW_AUTUMN_WHIPCRACK_ROOM", "CCW Autumn Whipcrack Room" }, // 0x67
    { "MAP_68_CCW_WINTER_WHIPCRACK_ROOM", "CCW Winter Whipcrack Room" }, // 0x68
    { "MAP_69_GL_MM_LOBBY", "GL MM Lobby" }, // 0x69
    { "MAP_6A_GL_TTC_AND_CC_PUZZLE", "GL TTC and CC Puzzle" }, // 0x6A
    { "MAP_6B_GL_180_NOTE_DOOR", "GL 180 Note Door" }, // 0x6B
    { "MAP_6C_GL_RED_CAULDRON_ROOM", "GL Red Cauldron Room" }, // 0x6C
    { "MAP_6D_GL_TTC_LOBBY", "GL TTC Lobby" }, // 0x6D
    { "MAP_6E_GL_GV_LOBBY", "GL GV Lobby" }, // 0x6E
    { "MAP_6F_GL_FP_LOBBY", "GL FP Lobby" }, // 0x6F
    { "MAP_70_GL_CC_LOBBY", "GL CC Lobby" }, // 0x70
    { "MAP_71_GL_STATUE_ROOM", "GL Statue Room" }, // 0x71
    { "MAP_72_GL_BGS_LOBBY", "GL BGS Lobby" }, // 0x72
    { "MAP_73_UNUSED", "Unused" }, // 0x73
    { "MAP_74_GL_GV_PUZZLE", "GL GV Puzzle" }, // 0x74
    { "MAP_75_GL_MMM_LOBBY", "GL MMM Lobby" }, // 0x75
    { "MAP_76_GL_640_NOTE_DOOR", "GL 640 Note Door" }, // 0x76
    { "MAP_77_GL_RBB_LOBBY", "GL RBB Lobby" }, // 0x77
    { "MAP_78_GL_RBB_AND_MMM_PUZZLE", "GL RBB and MMM Puzzle" }, // 0x78
    { "MAP_79_GL_CCW_LOBBY", "GL CCW Lobby" }, // 0x79
    { "MAP_7A_GL_CRYPT", "GL Crypt" }, // 0x7A
    { "MAP_7B_CS_INTRO_GL_DINGPOT_1", "CS Intro GL Dingpot 1" }, // 0x7B
    { "MAP_7C_CS_INTRO_BANJOS_HOUSE_1", "CS Intro Banjo's House 1" }, // 0x7C
    { "MAP_7D_CS_SPIRAL_MOUNTAIN_1", "CS Spiral Mountain 1" }, // 0x7D
    { "MAP_7E_CS_SPIRAL_MOUNTAIN_2", "CS Spiral Mountain 2" }, // 0x7E
    { "MAP_7F_FP_WOZZAS_CAVE", "FP Wozza's Cave" }, // 0x7F
    { "MAP_80_GL_FF_ENTRANCE", "GL FF Entrance" }, // 0x80
    { "MAP_81_CS_INTRO_GL_DINGPOT_2", "CS Intro GL Dingpot 2" }, // 0x81
    { "MAP_82_CS_ENTERING_GL_MACHINE_ROOM", "CS Entering GL Machine Room" }, // 0x82
    { "MAP_83_CS_GAME_OVER_MACHINE_ROOM", "CS Game Over Machine Room" }, // 0x83
    { "MAP_84_CS_UNUSED_MACHINE_ROOM", "CS Unused Machine Room" }, // 0x84
    { "MAP_85_CS_SPIRAL_MOUNTAIN_3", "CS Spiral Mountain 3" }, // 0x85
    { "MAP_86_CS_SPIRAL_MOUNTAIN_4", "CS Spiral Mountain 4" }, // 0x86
    { "MAP_87_CS_SPIRAL_MOUNTAIN_5", "CS Spiral Mountain 5" }, // 0x87
    { "MAP_88_CS_SPIRAL_MOUNTAIN_6", "CS Spiral Mountain 6" }, // 0x88
    { "MAP_89_CS_INTRO_BANJOS_HOUSE_2", "CS Intro Banjo's House 2" }, // 0x89
    { "MAP_8A_CS_INTRO_BANJOS_HOUSE_3", "CS Intro Banjo's House 3" }, // 0x8A
    { "MAP_8B_RBB_ANCHOR_ROOM", "RBB Anchor Room" }, // 0x8B
    { "MAP_8C_SM_BANJOS_HOUSE", "SM Banjo's House" }, // 0x8C
    { "MAP_8D_MMM_INSIDE_LOGGO", "MMM Inside Loggo" }, // 0x8D
    { "MAP_8E_GL_FURNACE_FUN", "GL Furnace Fun" }, // 0x8E
    { "MAP_8F_TTC_SHARKFOOD_ISLAND", "TTC Sharkfood Island" }, // 0x8F
    { "MAP_90_GL_BATTLEMENTS", "GL Battlements" }, // 0x90
    { "MAP_91_FILE_SELECT", "File Select" }, // 0x91
    { "MAP_92_GV_SNS_CHAMBER", "GV SNS Chamber" }, // 0x92
    { "MAP_93_GL_DINGPOT", "GL Dingpot" }, // 0x93
    { "MAP_94_CS_INTRO_SPIRAL_7", "CS Intro Spiral 7" }, // 0x94
    { "MAP_95_CS_END_ALL_100", "CS End All 100" }, // 0x95
    { "MAP_96_CS_END_BEACH_1", "CS End Beach 1" }, // 0x96
    { "MAP_97_CS_END_BEACH_2", "CS End Beach 2" }, // 0x97
    { "MAP_98_CS_END_SPIRAL_MOUNTAIN_1", "CS End Spiral Mountain 1" }, // 0x98
    { "MAP_99_CS_END_SPIRAL_MOUNTAIN_2", "CS End Spiral Mountain 2" }, // 0x99
};
