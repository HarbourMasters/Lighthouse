#include "RandoHelper.h"
#include "port/ui/UIWidgets.hpp"
#include "port/ui/Notification.h"
#include "port/ShipUtils.h"

#include <string>
#include <imgui.h>
#include <libultraship/libultraship.h>
#include "port/ui/LighthouseGui.hpp"
#include "port/ui/cvar_prefixes.h"

#include "enums.h"
#include "prop.h"
#include "actor.h"

extern "C" {
void jiggy_spawn(enum jiggy_e jiggy_id, f32 pos[3]);
void player_getPosition(f32 dst[3]);
void player_getPosition_s32(s32 arg0[3]);
Actor* actor_new(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags);
Actor* actor_spawnWithYaw_s32(enum actor_e id, s32 (*pos)[3], s32 rot);
void func_8031D04C(enum map_e arg0, s32 exit_id);

void item_set(s32 item, s32 val);
void ability_setAllLearned(s32 val);
void ability_setAllUsed(s32 val);

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

int32_t playerPosition[3];
int32_t spawnOffset[3];
int32_t spawnPosition[3];
int32_t mapId = 0;
int32_t exitId = 0;

const char* mapNames[] = {
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

void RandoHelper_SpawnPosition() {
    for (int i = 0; i < 2; i++) {
        spawnPosition[i] = playerPosition[i] + spawnOffset[i];
    }
}

void RandoHelper_ObjectSpawner() {
    player_getPosition_s32(playerPosition);
    RandoHelper_SpawnPosition();

    ImGui::SeparatorText("Spawn Data");
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
            RandoHelper_SpawnPosition();
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
            .Max(200)
            .DefaultValue(0)
            .Format("Offset Y: %i")
            .LabelPosition(UIWidgets::LabelPositions::None))) {
            RandoHelper_SpawnPosition();
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
            RandoHelper_SpawnPosition();
        }

        ImGui::EndTable();
    }

    if (UIWidgets::Button("Spawn Jinjo", { .color = THEME_COLOR })) {
        Actor* newActor = actor_spawnWithYaw_s32(ACTOR_60_JINJO_BLUE, &spawnPosition, 0);
    }

    if (UIWidgets::Button("Spawn Jiggy", { .color = THEME_COLOR })) {
        Actor* newActor = actor_spawnWithYaw_s32(ACTOR_46_JIGGY, &spawnPosition, 0);
        ActorLocal_Jiggy* actorLocal = (ActorLocal_Jiggy*)&newActor->local;
        actorLocal->index = JIGGY_03_MM_MUMBOS_SKULL;
    }

    if (UIWidgets::Button("Spawn Honeycomb", { .color = THEME_COLOR })) {
        Actor* newActor = actor_spawnWithYaw_s32(ACTOR_47_EMPTY_HONEYCOMB, &spawnPosition, 0);
        ActorLocal_EmptyHoneycomb* actorLocal = (ActorLocal_EmptyHoneycomb*)&newActor->local;
        actorLocal->uid = HONEYCOMB_1_MM_HILL;
    }

    if (UIWidgets::Button("Spawn Mumbo Token", { .color = THEME_COLOR })) {
        Actor* newActor = actor_spawnWithYaw_s32(ACTOR_2D_MUMBO_TOKEN, &spawnPosition, 0);
        ActorLocal_MumboToken* actorLocal = (ActorLocal_MumboToken*)&newActor->local;
        actorLocal->uid = MUMBOTOKEN_01_MM_STUMP_NEAR_CONGA;
    }

    if (UIWidgets::Button("Spawn Note", { .color = THEME_COLOR })) {
        Actor* newActor = actor_spawnWithYaw_s32(ACTOR_51_MUSIC_NOTE, &spawnPosition, 0);
    }
}

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

void RandoHelper_DrawTabBar() {
    UIWidgets::PushStyleTabs(THEME_COLOR);
    if (ImGui::BeginTabBar("RandoHelperTabBar")) {
        if (ImGui::BeginTabItem("Spawn Object")) {
            RandoHelper_ObjectSpawner();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Warp")) {
            DrawWarpList();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Grant Unlocks")) {
            DrawGrantUnlocks();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    UIWidgets::PopStyleTabs();
}

void RandoHelperWindow::DrawElement() {
    RandoHelper_DrawTabBar();
}