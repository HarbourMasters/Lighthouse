#include "Warps.h"
#include "port/ui/UIWidgets.hpp"

#include <imgui.h>
#include <libultraship/libultraship.h>
#include "port/ui/LighthouseGui.hpp"

#include "enums.h"

extern "C" {
void func_8031D04C(enum map_e arg0, s32 exit_id);
}

static int32_t mapId = 0;
static int32_t exitId = 0;

static const char* mapNames[] = {
    "Mumbo's Mountain",
    "Treasure Trove Cove",
    "Clanker's Cavern",
    "Bubblegloop Swamp",
    "Freezeezy Peak",
    "Gobi's Valley",
    "Click Clock Wood",
    "Rusty Bucket Bay",
    "Mad Monster Mansion",
    "Spiral Mountain",
    "Cutscene",
    "Gruntilda's Lair",
    "Boss Arena",
};

static const std::vector<int32_t> mapIdList = {
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

void DrawWarpList() {
    ImGui::SeparatorText("Custom Warp Selector");
    UIWidgets::Combobox("Map Select", &mapId, mapNames, { .color = THEME_COLOR });
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
    if (UIWidgets::Button("Mumbo's Mountain Warp Pad", { .color = THEME_COLOR })) {
        func_8031D04C(MAP_2_MM_MUMBOS_MOUNTAIN, 5);
    }
    if (UIWidgets::Button("Mr. Vile's Chamber", { .color = THEME_COLOR })) {
        // Exit 2 matches the Furnace Fun minigame warp table entry for this map
        func_8031D04C(MAP_10_BGS_MR_VILE, 2);
    }
}

void WarpsWindow::DrawElement() {
    DrawWarpList();
}
