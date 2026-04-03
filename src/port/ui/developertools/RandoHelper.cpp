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

void RandoHelper_DrawTabBar() {
    UIWidgets::PushStyleTabs(THEME_COLOR);
    if (ImGui::BeginTabBar("RandoHelperTabBar")) {
        if (ImGui::BeginTabItem("Spawn Object")) {
            RandoHelper_ObjectSpawner();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    UIWidgets::PopStyleTabs();
}

void RandoHelperWindow::DrawElement() {
    RandoHelper_DrawTabBar();
}