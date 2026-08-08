#include "GameplayTools.h"
#include "port/Rando/Rando.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "port/UI/UIWidgets.hpp"
#include "port/UI/Notification.h"
#include "port/ShipUtils.h"

#include <string>
#include <imgui.h>
#include <libultraship/libultraship.h>
#include "port/UI/LighthouseGui.hpp"
#include "port/UI/cvar_prefixes.h"

#include "enums.h"
#include "prop.h"
#include "actor.h"
#include "include/core1/sns.h"

extern "C" {
typedef struct {
    enum honeycomb_e uid;
    s32 unk4;
} ActorLocal_EmptyHoneycomb;

typedef struct chjiggy_s {
    u32 unk0;
    u32 index;
} ActorLocal_Jiggy;

typedef struct {
    enum mumbotoken_e uid;
} ActorLocal_MumboToken;
}

typedef struct {
    int32_t flagId;
    const char* flagName;
    level_e levelId;
} GameplayToolsMapData;

int32_t playerPosition[3];
int32_t spawnOffset[3];
int32_t spawnPosition[3];
int32_t mapId = 0;
int32_t exitId = 0;

actor_e selectedJinjo = ACTOR_60_JINJO_BLUE;
int32_t selectedSnsItem = SNS_ITEM_EGG_YELLOW;
int32_t selectedJiggy = JIGGY_01_MM_JINJO;
int32_t selectedHoneycomb = HONEYCOMB_1_MM_HILL;
int32_t selectedToken = MUMBOTOKEN_01_MM_STUMP_NEAR_CONGA;

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

std::map<actor_e, std::tuple<const char*, UIWidgets::Colors, bool>> jinjoDataMap = {
    { ACTOR_5E_JINJO_YELLOW, { "Yellow Jinjo", UIWidgets::Colors::Yellow, false } },
    { ACTOR_5F_JINJO_ORANGE, { "Orange Jinjo", UIWidgets::Colors::Orange, false } },
    { ACTOR_60_JINJO_BLUE, { "Blue Jinjo", UIWidgets::Colors::SkyBlue, false } },
    { ACTOR_61_JINJO_PINK, { "Pink Jinjo", UIWidgets::Colors::Pink, false } },
    { ACTOR_62_JINJO_GREEN, { "Green Jinjo", UIWidgets::Colors::Green, false } },
};

std::map<StopNSwop_Item, std::tuple<const char*, UIWidgets::Colors, bool>> snsDataMap = {
    { SNS_ITEM_EGG_YELLOW, { "SNS Yellow Egg", UIWidgets::Colors::Yellow, false } },
    { SNS_ITEM_EGG_RED, { "SNS Red Egg", UIWidgets::Colors::Red, false } },
    { SNS_ITEM_EGG_GREEN, { "SNS Green Egg", UIWidgets::Colors::Green, false } },
    { SNS_ITEM_EGG_BLUE, { "SNS Blue Egg", UIWidgets::Colors::Blue, false } },
    { SNS_ITEM_EGG_PINK, { "SNS Pink Egg", UIWidgets::Colors::Pink, false } },
    { SNS_ITEM_EGG_CYAN, { "SNS Cyan Egg", UIWidgets::Colors::Cyan, false } },
};

std::map<map_e, std::pair<const char*, int32_t>> commonWarpMap = {
    { MAP_1_SM_SPIRAL_MOUNTAIN, { "Outside Banjo's House", 0 } },
    { MAP_2_MM_MUMBOS_MOUNTAIN, { "Mumbo's Mountain Warp Pad", 5 } },
    { MAP_7_TTC_TREASURE_TROVE_COVE, { "Treasure Trove Cove Warp Pad", 4 } },
    { MAP_B_CC_CLANKERS_CAVERN, { "Clanker's Cavern Warp Pad", 5 } },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, { "Bubblegloop Swamp Warp Pad", 2 } },
    { MAP_27_FP_FREEZEEZY_PEAK, { "Freezeezy Peak Warp Pad", 1 } },
    { MAP_12_GV_GOBIS_VALLEY, { "Gobi's Valley Warp Pad", 8 } },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, { "Mad Monster Mansion Warp Pad", 20 } },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, { "Rusty Bucket Bay Warp Pad", 16 } },
    { MAP_40_CCW_HUB, { "Click Clock Wood Warp Pad", 7 } },
    { MAP_43_CCW_SPRING, { "Click Clock Wood - Spring", 1 } },
    { MAP_44_CCW_SUMMER, { "Click Clock Wood - Summer", 1 } },
    { MAP_45_CCW_AUTUMN, { "Click Clock Wood - Autumn", 1 } },
    { MAP_46_CCW_WINTER, { "Click Clock Wood - Winter", 1 } },
    { MAP_8E_GL_FURNACE_FUN, { "Grunty's Furnace Fun", WARP_GL_FURNACE_FUN_2_ENTRANCE_PAD } },
};

// clang-format off
std::vector<GameplayToolsMapData> mapSpecificFlagList = {
    { MM_SPECIFIC_FLAG_0_CHIMPY_STUMP_RAISED,					"MM_SPECIFIC_FLAG_0_CHIMPY_STUMP_RAISED", 					LEVEL_1_MUMBOS_MOUNTAIN },
    { MM_SPECIFIC_FLAG_1_ORANGE_HAS_BEEN_COLLECTED,				"MM_SPECIFIC_FLAG_1_ORANGE_HAS_BEEN_COLLECTED", 			LEVEL_1_MUMBOS_MOUNTAIN },
    { MM_SPECIFIC_FLAG_2_ORANGE_HAS_BEEN_RETURNED,				"MM_SPECIFIC_FLAG_2_ORANGE_HAS_BEEN_RETURNED", 				LEVEL_1_MUMBOS_MOUNTAIN },
    { MM_SPECIFIC_FLAG_3_CHIMPY_HAS_LEFT,						"MM_SPECIFIC_FLAG_3_CHIMPY_HAS_LEFT", 						LEVEL_1_MUMBOS_MOUNTAIN },
    { MM_SPECIFIC_FLAG_4_SHAKE,									"MM_SPECIFIC_FLAG_4_SHAKE", 								LEVEL_1_MUMBOS_MOUNTAIN },
    { MM_SPECIFIC_FLAG_CONGA_WARNED_BLOCKS,						"MM_SPECIFIC_FLAG_CONGA_WARNED_BLOCKS", 					LEVEL_1_MUMBOS_MOUNTAIN },
    { MM_SPECIFIC_FLAG_8_HIT_WITH_ORANGE,						"MM_SPECIFIC_FLAG_8_HIT_WITH_ORANGE", 						LEVEL_1_MUMBOS_MOUNTAIN },
    { MM_SPECIFIC_FLAG_9_JUJU_HAS_HALF_TURNED,					"MM_SPECIFIC_FLAG_9_JUJU_HAS_HALF_TURNED", 					LEVEL_1_MUMBOS_MOUNTAIN },
    { MM_SPECIFIC_FLAG_A_UNKNOWN,								"MM_SPECIFIC_FLAG_A_UNKNOWN", 								LEVEL_1_MUMBOS_MOUNTAIN },
    { TTC_SPECIFIC_FLAG_0_BLUBBER_UNKNOWN,						"TTC_SPECIFIC_FLAG_0_BLUBBER_UNKNOWN", 						LEVEL_2_TREASURE_TROVE_COVE },
    { TTC_SPECIFIC_FLAG_1_UNKNOWN,								"TTC_SPECIFIC_FLAG_1_UNKNOWN", 								LEVEL_2_TREASURE_TROVE_COVE },
    { TTC_SPECIFIC_FLAG_2_BLUBBER_JIGGY_SPAWNED_TEXT_SHOWN,		"TTC_SPECIFIC_FLAG_2_BLUBBER_JIGGY_SPAWNED_TEXT_SHOWN", 	LEVEL_2_TREASURE_TROVE_COVE },
    { TTC_SPECIFIC_FLAG_3_BLUBBER_SHOW_JIGGY_SPAWNED_TEXT_FLAG,	"TTC_SPECIFIC_FLAG_3_BLUBBER_SHOW_JIGGY_SPAWNED_TEXT_FLAG", LEVEL_2_TREASURE_TROVE_COVE },
    { TTC_SPECIFIC_FLAG_5_CLAM_FIRST_MEET_TEXT_SHOWN,			"TTC_SPECIFIC_FLAG_5_CLAM_FIRST_MEET_TEXT_SHOWN", 			LEVEL_2_TREASURE_TROVE_COVE },
    { TTC_SPECIFIC_FLAG_7_NIPPER_FIRST_MEET_TEXT_SHOWN,			"TTC_SPECIFIC_FLAG_7_NIPPER_FIRST_MEET_TEXT_SHOWN", 		LEVEL_2_TREASURE_TROVE_COVE },
    { BGS_SPECIFIC_FLAG_1,					 					"BGS_SPECIFIC_FLAG_1", 										LEVEL_4_BUBBLEGLOOP_SWAMP },
    { BGS_SPECIFIC_FLAG_2_WALKWAY_JIGGY_RESET,					"BGS_SPECIFIC_FLAG_2_WALKWAY_JIGGY_RESET", 					LEVEL_4_BUBBLEGLOOP_SWAMP },
    { BGS_SPECIFIC_FLAG_3_WALKWAY_JIGGY_TIMER_RUNNING,			"BGS_SPECIFIC_FLAG_3_WALKWAY_JIGGY_TIMER_RUNNING", 			LEVEL_4_BUBBLEGLOOP_SWAMP },
    { BGS_SPECIFIC_FLAG_4_WALKWAY_JIGGY,					 		"BGS_SPECIFIC_FLAG_4_WALKWAY_JIGGY", 							LEVEL_4_BUBBLEGLOOP_SWAMP },
    { BGS_SPECIFIC_FLAG_5_WALKWAY_JIGGY_SWITCH_PRESSED,			"BGS_SPECIFIC_FLAG_5_WALKWAY_JIGGY_SWITCH_PRESSED", 			LEVEL_4_BUBBLEGLOOP_SWAMP },
    { BGS_SPECIFIC_FLAG_7,					 					"BGS_SPECIFIC_FLAG_7", 										LEVEL_4_BUBBLEGLOOP_SWAMP },
    { BGS_SPECIFIC_FLAG_8,					 					"BGS_SPECIFIC_FLAG_8", 										LEVEL_4_BUBBLEGLOOP_SWAMP },
    { BGS_SPECIFIC_FLAG_9_MAZE_JIGGY_SWITCH_PRESSED,				"BGS_SPECIFIC_FLAG_9_MAZE_JIGGY_SWITCH_PRESSED", 				LEVEL_4_BUBBLEGLOOP_SWAMP },
    { BGS_SPECIFIC_FLAG_A,					 					"BGS_SPECIFIC_FLAG_A", 										LEVEL_4_BUBBLEGLOOP_SWAMP },
    { BGS_SPECIFIC_FLAG_B_MAZE_JIGGY_RESET,					 	"BGS_SPECIFIC_FLAG_B_MAZE_JIGGY_RESET", 						LEVEL_4_BUBBLEGLOOP_SWAMP },
    { BGS_SPECIFIC_FLAG_C_MAZE_JIGGY_TIMER_RUNNING,				"BGS_SPECIFIC_FLAG_C_MAZE_JIGGY_TIMER_RUNNING", 				LEVEL_4_BUBBLEGLOOP_SWAMP },
    { BGS_SPECIFIC_FLAG_D_MAZE_JIGGY,					 			"BGS_SPECIFIC_FLAG_D_MAZE_JIGGY", 							LEVEL_4_BUBBLEGLOOP_SWAMP },
    { FP_SPECIFIC_FLAG_0_XMAS_TREE_LIGHTS_ON,				 	"FP_SPECIFIC_FLAG_0_XMAS_TREE_LIGHTS_ON", 					LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_1_UNKNOWN,				 				"FP_SPECIFIC_FLAG_1_UNKNOWN", 								LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_2_XMAS_TREE_SWITCH,				 		"FP_SPECIFIC_FLAG_2_XMAS_TREE_SWITCH", 						LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_3_XMAS_TREE_STAR_COMPLETE,				"FP_SPECIFIC_FLAG_3_XMAS_TREE_STAR_COMPLETE", 				LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_4_UNKNOWN,				 				"FP_SPECIFIC_FLAG_4_UNKNOWN", 								LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_5_WALRUS_RACE_FAILED,				 	"FP_SPECIFIC_FLAG_5_WALRUS_RACE_FAILED", 					LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_6_BANJO_RACE_FAILED,				 		"FP_SPECIFIC_FLAG_6_BANJO_RACE_FAILED", 					LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_7_WOZZA_FIRST_CONTACT,				 	"FP_SPECIFIC_FLAG_7_WOZZA_FIRST_CONTACT", 					LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_8_WOZZA_MET_WALRUS,				 		"FP_SPECIFIC_FLAG_8_WOZZA_MET_WALRUS", 						LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_9_UNKNOWN,				 				"FP_SPECIFIC_FLAG_9_UNKNOWN", 								LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_A_FIRST_TWINKLY_EATEN,				 	"FP_SPECIFIC_FLAG_A_FIRST_TWINKLY_EATEN", 					LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_B_UNKNOWN,				 				"FP_SPECIFIC_FLAG_B_UNKNOWN", 								LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_C_UNKNOWN,				 				"FP_SPECIFIC_FLAG_C_UNKNOWN", 								LEVEL_5_FREEZEEZY_PEAK },
    { FP_SPECIFIC_FLAG_D_UNKNOWN,				 				"FP_SPECIFIC_FLAG_D_UNKNOWN", 								LEVEL_5_FREEZEEZY_PEAK },
    { LAIR_SPECIFIC_FLAG_7_UKNOWN, 								"LAIR_SPECIFIC_FLAG_7_UKNOWN", 								LEVEL_6_LAIR },
    { LAIR_SPECIFIC_FLAG_9_UKNOWN, 								"LAIR_SPECIFIC_FLAG_9_UKNOWN", 								LEVEL_6_LAIR },
    { MMM_SPECIFIC_FLAG_0_UNKNOWN, 								"MMM_SPECIFIC_FLAG_0_UNKNOWN", 								LEVEL_A_MAD_MONSTER_MANSION },
    { MMM_SPECIFIC_FLAG_TUMBLAR_BROKEN, 						"MMM_SPECIFIC_FLAG_TUMBLAR_BROKEN", 						LEVEL_A_MAD_MONSTER_MANSION },
    { MMM_SPECIFIC_FLAG_2_UNKNOWN, 								"MMM_SPECIFIC_FLAG_2_UNKNOWN", 								LEVEL_A_MAD_MONSTER_MANSION },
    { MMM_SPECIFIC_FLAG_3_UNKNOWN, 								"MMM_SPECIFIC_FLAG_3_UNKNOWN", 								LEVEL_A_MAD_MONSTER_MANSION },
    { MMM_SPECIFIC_FLAG_4_UNKNOWN, 								"MMM_SPECIFIC_FLAG_4_UNKNOWN", 								LEVEL_A_MAD_MONSTER_MANSION },
    { MMM_SPECIFIC_FLAG_5_UNKNOWN, 								"MMM_SPECIFIC_FLAG_5_UNKNOWN", 								LEVEL_A_MAD_MONSTER_MANSION },
    { MMM_SPECIFIC_FLAG_CONGA_WARNED_BLOCKS, 					"MMM_SPECIFIC_FLAG_CONGA_WARNED_BLOCKS", 					LEVEL_A_MAD_MONSTER_MANSION },
    { MMM_SPECIFIC_FLAG_7_UNKNOWN, 								"MMM_SPECIFIC_FLAG_7_UNKNOWN", 								LEVEL_A_MAD_MONSTER_MANSION },
    { SM_SPECIFIC_FLAG_1_TALKED_TO_BOTTLES, 					"SM_SPECIFIC_FLAG_1_TALKED_TO_BOTTLES", 					LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_2, 										"SM_SPECIFIC_FLAG_2", 										LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_3_ALL_SM_ABILITIES_LEARNED, 				"SM_SPECIFIC_FLAG_3_ALL_SM_ABILITIES_LEARNED", 				LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_4, 										"SM_SPECIFIC_FLAG_4", 										LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_5, 										"SM_SPECIFIC_FLAG_5", 										LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_7, 										"SM_SPECIFIC_FLAG_7", 										LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_8_ABILITY_HOLD_A_JUMP_HIGHER_UNLOCKED, 	"SM_SPECIFIC_FLAG_8_ABILITY_HOLD_A_JUMP_HIGHER_UNLOCKED", 	LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_9_ABILITY_FEATHERY_UNLOCKED, 			"SM_SPECIFIC_FLAG_9_ABILITY_FEATHERY_UNLOCKED", 			LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_A_ABILITY_FLIP_UNLOCKED, 				"SM_SPECIFIC_FLAG_A_ABILITY_FLIP_UNLOCKED", 				LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_C, 										"SM_SPECIFIC_FLAG_C", 										LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_E, 										"SM_SPECIFIC_FLAG_E", 										LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_F, 										"SM_SPECIFIC_FLAG_F", 										LEVEL_B_SPIRAL_MOUNTAIN },
    { SM_SPECIFIC_FLAG_10, 										"SM_SPECIFIC_FLAG_10", 										LEVEL_B_SPIRAL_MOUNTAIN },
};
// clang-format on

void GameplayTools_UpdateJinjoCheckboxes(actor_e selectedJinjoId) {
    selectedJinjo = selectedJinjoId;
    for (auto& [jinjoId, jinjoData] : jinjoDataMap) {
        if (jinjoId == selectedJinjoId) {
            std::get<2>(jinjoData) = true;
        } else {
            std::get<2>(jinjoData) = false;
        }
    }
}

void GameplayTools_UpdateSnsCheckboxes(StopNSwop_Item selectedSnsId) {
    selectedSnsItem = selectedSnsId;
    for (auto& [snsId, snsData] : snsDataMap) {
        if (snsId == selectedSnsId) {
            std::get<2>(snsData) = true;
        } else {
            std::get<2>(snsData) = false;
        }
    }
}

void GameplayTools_SpawnPosition() {
    for (int i = 0; i < 2; i++) {
        spawnPosition[i] = playerPosition[i] + spawnOffset[i];
    }
}

void GameplayTools_ObjectSpawner() {
    player_getPosition_s32(playerPosition);
    GameplayTools_SpawnPosition();

    ImGui::SeparatorText("Player Position");
    ImGui::Text("Map ID: %i", gsworld_getMap());

    if (ImGui::BeginTable("SpawnInfoTable", 3)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("PlayerPos", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextColumn();

        ImGui::Text("Pos X: ");
        ImGui::TableNextColumn();
        ImGui::Text(std::to_string(playerPosition[0]).c_str());
        ImGui::TableNextColumn();
        if (UIWidgets::SliderInt("##offsetX", &spawnOffset[0],
                                 UIWidgets::IntSliderOptions()
                                     .Color(THEME_COLOR)
                                     .Min(0)
                                     .Max(200)
                                     .DefaultValue(0)
                                     .Format("Offset X: %i")
                                     .LabelPosition(UIWidgets::LabelPositions::None))) {
            GameplayTools_SpawnPosition();
        }
        ImGui::TableNextColumn();

        ImGui::Text("Pos Y: ");
        ImGui::TableNextColumn();
        ImGui::Text(std::to_string(playerPosition[1]).c_str());
        ImGui::TableNextColumn();
        if (UIWidgets::SliderInt("##offsetY", &spawnOffset[1],
                                 UIWidgets::IntSliderOptions()
                                     .Color(THEME_COLOR)
                                     .Min(0)
                                     .Max(700)
                                     .DefaultValue(0)
                                     .Format("Offset Y: %i")
                                     .LabelPosition(UIWidgets::LabelPositions::None))) {
            GameplayTools_SpawnPosition();
        }
        ImGui::TableNextColumn();

        ImGui::Text("Pos Z: ");
        ImGui::TableNextColumn();
        ImGui::Text(std::to_string(playerPosition[2]).c_str());
        ImGui::TableNextColumn();
        if (UIWidgets::SliderInt("##offsetZ", &spawnOffset[2],
                                 UIWidgets::IntSliderOptions()
                                     .Color(THEME_COLOR)
                                     .Min(0)
                                     .Max(200)
                                     .DefaultValue(0)
                                     .Format("Offset Z: %i")
                                     .LabelPosition(UIWidgets::LabelPositions::None))) {
            GameplayTools_SpawnPosition();
        }

        ImGui::EndTable();
    }

    if (ImGui::BeginTable("ObjectSpawner", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        if (UIWidgets::Button("Spawn Jinjo", { .color = THEME_COLOR })) {
            Actor* newActor = actor_new(spawnPosition, 0, &actorInfoMap.at(selectedJinjo).first,
                                        actorInfoMap.at(selectedJinjo).second);
        }
        ImGui::TableNextColumn();
        for (auto& [jinjoId, jinjoData] : jinjoDataMap) {
            bool isChecked = std::get<2>(jinjoData);
            if (UIWidgets::Checkbox(std::get<0>(jinjoData), &isChecked,
                                    UIWidgets::CheckboxOptions()
                                        .Color(std::get<1>(jinjoData))
                                        .LabelPosition(UIWidgets::LabelPositions::None))) {
                GameplayTools_UpdateJinjoCheckboxes(jinjoId);
            }
            ImGui::SameLine();
        }

        ImGui::TableNextColumn();
        if (UIWidgets::Button("Spawn Jiggy", { .color = THEME_COLOR })) {
            Actor* newActor = actor_new(spawnPosition, 0, &actorInfoMap.at(ACTOR_46_JIGGY).first,
                                        actorInfoMap.at(ACTOR_46_JIGGY).second);
            ActorLocal_Jiggy* actorLocal = (ActorLocal_Jiggy*)&newActor->local;
            actorLocal->index = selectedJiggy;
        }
        ImGui::TableNextColumn();
        RandoCheckId jiggyCheckId = Rando::StaticData::GetCheckByJiggyId(selectedJiggy);
        std::string jiggyText = Ship_ConvertEnumToReadableName(Rando::StaticData::Checks[jiggyCheckId].name);
        jiggyText += ": " + std::to_string(selectedJiggy);

        UIWidgets::SliderInt("##jiggyIndex", &selectedJiggy,
                             UIWidgets::IntSliderOptions()
                                 .Color(THEME_COLOR)
                                 .Min(JIGGY_01_MM_JINJO)
                                 .Max(JIGGY_64_MMM_LOGGO)
                                 .DefaultValue(JIGGY_01_MM_JINJO)
                                 .Format(jiggyText.c_str())
                                 .LabelPosition(UIWidgets::LabelPositions::None));

        ImGui::TableNextColumn();
        if (UIWidgets::Button("Spawn Honeycomb", { .color = THEME_COLOR })) {
            Actor* newActor = actor_new(spawnPosition, 0, &actorInfoMap.at(ACTOR_47_EMPTY_HONEYCOMB).first,
                                        actorInfoMap.at(ACTOR_47_EMPTY_HONEYCOMB).second);
            ActorLocal_EmptyHoneycomb* actorLocal = (ActorLocal_EmptyHoneycomb*)&newActor->local;
            actorLocal->uid = (honeycomb_e)selectedHoneycomb;
        }
        ImGui::TableNextColumn();
        RandoCheckId honeycombCheckId = Rando::StaticData::GetCheckByHoneycombId((honeycomb_e)selectedHoneycomb);
        std::string honeycombText = Ship_ConvertEnumToReadableName(Rando::StaticData::Checks[honeycombCheckId].name);
        honeycombText += ": " + std::to_string(selectedHoneycomb);

        UIWidgets::SliderInt("##honeycombIndex", &selectedHoneycomb,
                             UIWidgets::IntSliderOptions()
                                 .Color(THEME_COLOR)
                                 .Min(HONEYCOMB_1_MM_HILL)
                                 .Max(HONEYCOMB_18_SM_QUARRIES)
                                 .DefaultValue(HONEYCOMB_1_MM_HILL)
                                 .Format(honeycombText.c_str())
                                 .LabelPosition(UIWidgets::LabelPositions::None));

        ImGui::TableNextColumn();
        if (UIWidgets::Button("Spawn Mumbo Token", { .color = THEME_COLOR })) {
            Actor* newActor = actor_new(spawnPosition, 0, &actorInfoMap.at(ACTOR_2D_MUMBO_TOKEN).first,
                                        actorInfoMap.at(ACTOR_2D_MUMBO_TOKEN).second);
            ActorLocal_MumboToken* actorLocal = (ActorLocal_MumboToken*)&newActor->local;
            actorLocal->uid = (mumbotoken_e)selectedToken;
        }
        ImGui::TableNextColumn();
        RandoCheckId tokenCheckId = Rando::StaticData::GetCheckByMumboTokenId((mumbotoken_e)selectedToken);
        std::string tokenText = Ship_ConvertEnumToReadableName(Rando::StaticData::Checks[tokenCheckId].name);
        tokenText += ": " + std::to_string(selectedToken);

        UIWidgets::SliderInt("##tokenIndex", &selectedToken,
                             UIWidgets::IntSliderOptions()
                                 .Color(THEME_COLOR)
                                 .Min(MUMBOTOKEN_01_MM_STUMP_NEAR_CONGA)
                                 .Max(MUMBOTOKEN_73_CCW_WINTER_SIR_SLUSH_BETWEEN_BIG_FLOWER_AND_MUMBOS_SKULL)
                                 .DefaultValue(MUMBOTOKEN_01_MM_STUMP_NEAR_CONGA)
                                 .Format(tokenText.c_str())
                                 .LabelPosition(UIWidgets::LabelPositions::None));

        ImGui::TableNextColumn();
        if (UIWidgets::Button("Spawn Note", { .color = THEME_COLOR })) {
            Actor* newActor = actor_new(spawnPosition, 0, &actorInfoMap.at(ACTOR_51_MUSIC_NOTE).first,
                                        actorInfoMap.at(ACTOR_51_MUSIC_NOTE).second);
        }
        ImGui::TableNextColumn();

        ImGui::TableNextColumn();
        if (UIWidgets::Button("Spawn SNS Egg", { .color = THEME_COLOR })) {
            Actor* newActor = actor_new(spawnPosition, 0, &actorInfoMap.at(ACTOR_25E_SNS_EGG).first,
                                        actorInfoMap.at(ACTOR_25E_SNS_EGG).second);
            newActor->actorTypeSpecificField = selectedSnsItem;
        }
        ImGui::TableNextColumn();
        for (auto& [snsId, snsData] : snsDataMap) {
            bool isChecked = std::get<2>(snsData);
            if (UIWidgets::Checkbox(std::get<0>(snsData), &isChecked,
                                    UIWidgets::CheckboxOptions()
                                        .Color(std::get<1>(snsData))
                                        .LabelPosition(UIWidgets::LabelPositions::None))) {
                GameplayTools_UpdateSnsCheckboxes(snsId);
            }
            ImGui::SameLine();
        }

        ImGui::TableNextColumn();
        if (UIWidgets::Button("Spawn Ice Key", { .color = THEME_COLOR })) {
            Actor* newActor = actor_new(spawnPosition, 0, &actorInfoMap.at(ACTOR_25D_ICE_KEY).first,
                                        actorInfoMap.at(ACTOR_25D_ICE_KEY).second);
        }

        ImGui::EndTable();
    }
}

void DrawGameplayToolsWarpList() {
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

        ImGui::SeparatorText("Common Locations");
        if (ImGui::BeginTable("CommonWarps", 2, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableNextColumn();
            for (auto& [mapId, mapData] : commonWarpMap) {
                if (UIWidgets::Button(mapData.first, { .color = THEME_COLOR })) {
                    func_8031D04C(mapId, mapData.second);
                }
                ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
}

void DrawGrantUnlocks() {
    if (UIWidgets::Button("Unlock Moves", { .color = THEME_COLOR })) {
        ability_setAllLearned(-1);
        ability_setAllUsed(-1);
    }
    if (UIWidgets::Button("Add All Consumables", { .color = THEME_COLOR })) {
        item_set(ITEM_D_EGGS, 100);
        item_set(ITEM_F_RED_FEATHER, 50);
        item_set(ITEM_10_GOLD_FEATHER, 10);
        item_set(ITEM_1C_MUMBO_TOKEN, 25);
    }
}

// Index 0 traces every space; the rest are ANCHOR_FLAGSPACE_* ids offset by one.
static const std::unordered_map<int32_t, const char*> flagTraceSpaces = {
    { 0, "All" },          { 1, "File Progress" }, { 2, "Volatile" }, { 3, "Level Specific" },
    { 4, "Map Specific" }, { 5, "Rando Inf" },
};

void DrawFlagTracer() {
    ImGui::SeparatorText("Flag Tracer");
    ImGui::TextWrapped("Logs every flag write to the console and log file, with the flag's enum name and a marker on "
                       "each map load.");

    UIWidgets::CVarCheckbox("Trace Flag Writes", CVAR_DEVELOPER_TOOLS("FlagTrace"),
                            UIWidgets::CheckboxOptions().Color(THEME_COLOR).DefaultValue(false));

    if (!CVarGetInteger(CVAR_DEVELOPER_TOOLS("FlagTrace"), 0)) {
        return;
    }

    UIWidgets::CVarCombobox("Flag Space", CVAR_DEVELOPER_TOOLS("FlagTraceSpace"), flagTraceSpaces,
                            UIWidgets::ComboboxOptions()
                                .Color(THEME_COLOR)
                                .Tooltip("Limit tracing to a single flag space.")
                                .DefaultIndex(0));
    UIWidgets::CVarCheckbox("Include Call Stacks", CVAR_DEVELOPER_TOOLS("FlagTraceStacks"),
                            UIWidgets::CheckboxOptions()
                                .Color(THEME_COLOR)
                                .DefaultValue(false)
                                .Tooltip("Name the function and line that set each flag."));
}

void DrawMonitoringTools() {
    level_e currentLevel = map_getLevel(gsworld_getMap());
    int32_t mapIndex = 0;

    DrawFlagTracer();

    ImGui::SeparatorText("Map Specific Flags");
    if (ImGui::BeginChild("MapFlagChild")) {
        if (ImGui::BeginTable("MapFlagTable", 2, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableNextColumn();
            for (auto& flagData : mapSpecificFlagList) {
                if (flagData.levelId == currentLevel) {
                    ImGui::PushID(mapIndex);
                    bool flagState = mapSpecificFlags_get(flagData.flagId);
                    if (UIWidgets::Checkbox(
                            "state", &flagState,
                            UIWidgets::CheckboxOptions().LabelPosition(UIWidgets::LabelPositions::None))) {
                        mapSpecificFlags_set(flagData.flagId, !mapSpecificFlags_get(flagData.flagId));
                    }

                    ImGui::TableNextColumn();
                    ImGui::Text(flagData.flagName);
                    ImGui::TableNextColumn();
                    ImGui::PopID();
                }

                mapIndex++;
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
}

void GameplayTools_DrawTabBar() {
    UIWidgets::PushStyleTabs(THEME_COLOR);
    if (ImGui::BeginTabBar("GameplayToolsTabBar")) {
        if (ImGui::BeginTabItem("Spawn Object")) {
            GameplayTools_ObjectSpawner();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Warp")) {
            DrawGameplayToolsWarpList();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Grant Unlocks")) {
            DrawGrantUnlocks();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Monitoring")) {
            DrawMonitoringTools();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    UIWidgets::PopStyleTabs();
}

void GameplayToolsWindow::DrawElement() {
    GameplayTools_DrawTabBar();
}

void GameplayToolsWindow::InitElement() {
    std::get<2>(jinjoDataMap.at(ACTOR_60_JINJO_BLUE)) = true;
    std::get<2>(snsDataMap.at(SNS_ITEM_EGG_YELLOW)) = true;
}
