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
    "Mumbo's Mountain", "Treasure Trove Cove", "Clanker's Cavern", "Bubblegloop Swamp",   "Freezeezy Peak",
    "Gobi's Valley",    "Click Clock Wood",    "Rusty Bucket Bay", "Mad Monster Mansion", "Spiral Mountain",
    "Cutscene",         "Gruntilda's Lair",    "Boss Arena",
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

// Curated quick-warp destinations, presented as a single combobox.
struct CuratedWarp {
    const char* label;
    enum map_e map;
    int32_t exit;
};

static const std::vector<CuratedWarp> curatedWarps = {
    { "Mumbo's Mountain - Warp Pad", MAP_2_MM_MUMBOS_MOUNTAIN, 5 },
    // Exit 2 matches the Furnace Fun minigame warp table entry for this map
    { "Mr. Vile's Chamber", MAP_10_BGS_MR_VILE, 2 },
    // Click Clock Wood hub spawn points (exits 0, 5, 6 and 8-20 are omitted)
    { "Click Clock Wood - Spring", MAP_40_CCW_HUB, 2 },
    { "Click Clock Wood - Summer Exit", MAP_40_CCW_HUB, 3 },
    { "Click Clock Wood - Fall Exit", MAP_40_CCW_HUB, 4 },
    { "Click Clock Wood - Winter Exit", MAP_40_CCW_HUB, 1 },
    { "Click Clock Wood - Entrance Pad", MAP_40_CCW_HUB, 7 },
    // Each season's start (exit 1 of the season map, matching warp_ccwEnter*)
    { "Click Clock Wood - Spring Start", MAP_43_CCW_SPRING, 1 },
    { "Click Clock Wood - Summer Start", MAP_44_CCW_SUMMER, 1 },
    { "Click Clock Wood - Autumn Start", MAP_45_CCW_AUTUMN, 1 },
    { "Click Clock Wood - Winter Start", MAP_46_CCW_WINTER, 1 },
};

static int32_t curatedWarpId = 0;

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
    static const std::vector<const char*> curatedWarpNames = [] {
        std::vector<const char*> names;
        for (const CuratedWarp& warp : curatedWarps) {
            names.push_back(warp.label);
        }
        return names;
    }();
    UIWidgets::Combobox("Quick Warp", &curatedWarpId, curatedWarpNames, { .color = THEME_COLOR });
    if (UIWidgets::Button("Warp to Selected Location", { .color = THEME_COLOR })) {
        const CuratedWarp& warp = curatedWarps[curatedWarpId];
        func_8031D04C(warp.map, warp.exit);
    }
}

void WarpsWindow::DrawElement() {
    DrawWarpList();
}
