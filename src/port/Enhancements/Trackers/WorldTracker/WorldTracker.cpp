#include "WorldTracker.h"
#include "port/Enhancements/Retention/Retention.h"
#include "port/Rando/ShuffleBehavior/ObjectBehavior.h"
#include "port/Save/Types.h"
#include "port/ShipUtils.h"
#include "port/UI/UIWidgets.hpp"
#include "fast/Fast3dGui.h"
#include "functions.h"

#define CVAR_NAME_SHOW_WORLD_TRACKER "gWindows.WorldTracker"
#define CVAR_NAME_SHOW_CURRENT_LEVEL "gRando.WorldTracker.ShowCurrentLevel"
#define CVAR_NAME_SHOW_TOTAL_COLLECTED "gRando.WorldTracker.ShowTotalCollected"
#define CVAR_NAME_SEPARATE_TOTAL_COLLECTED "gRando.CheckTracker.SeparateCollectedChecks"

#define CVAR_SHOW_WORLD_TRACKER CVarGetInteger(CVAR_NAME_SHOW_WORLD_TRACKER, 0)
#define CVAR_SHOW_CURRENT_LEVEL CVarGetInteger(CVAR_NAME_SHOW_CURRENT_LEVEL, 0)
#define CVAR_SHOW_TOTAL_COLLECTED CVarGetInteger(CVAR_NAME_SHOW_TOTAL_COLLECTED, 0)
#define CVAR_SHOW_SEPARATE_TOTAL_COLLECTED CVarGetInteger(CVAR_NAME_SEPARATE_TOTAL_COLLECTED, 0)

extern "C" {
extern u8 D_80385FF0[0xE];
#define MUMBO_TOKEN_COUNT 126
#define MUMBOSCORE_SIZE (((MUMBO_TOKEN_COUNT - 1 + 7) & ~7) / 8)
extern u8 sMumboTokenScore[MUMBOSCORE_SIZE];
}

bool worldTrackerPopoutState = false;
ImVec4 worldTrackerBG = ImVec4{ 0, 0, 0, 0.5f };
static ImVec2 imageSize = ImVec2(32.0f, 32.0f);

std::vector<const char*> jinjoTextureNameList = { "Blue Jinjo", "Green Jinjo", "Orange Jinjo", "Pink Jinjo",
                                                  "Yellow Jinjo" };

std::vector<actor_e> orderedJinjoActorList = { ACTOR_60_JINJO_BLUE, ACTOR_62_JINJO_GREEN, ACTOR_5F_JINJO_ORANGE,
                                               ACTOR_61_JINJO_PINK, ACTOR_5E_JINJO_YELLOW };

namespace LighthouseGui {
extern std::shared_ptr<WorldTracker::WorldTrackerWindow> mWorldTrackerWindow;
}

ImTextureID GetWorldTrackerTexture(std::string textureName) {
    auto gui = std::dynamic_pointer_cast<Fast::Fast3dGui>(Ship::Context::GetRawInstance()->GetWindow()->GetGui());

    return gui->GetTextureByName(textureName);
}

void WorldTracker_PushImageButtonStyle() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
}

void WorldTracker_PopImageButtonStyle() {
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(1);
}

void WorldTracker_DrawTotals() {
    bool isEmbedded = !CVAR_SHOW_SEPARATE_TOTAL_COLLECTED;

    if (!isEmbedded) {
        ImGui::Begin("SplitWorldTrackerTotals", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
    }
    if (ImGui::BeginTable("World Tracker Totals", 4, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextColumn();
        ImGui::Image(GetWorldTrackerTexture("Music Note"), imageSize);
        ImGui::SameLine();
        TableCellCenteredSetCursorPosY(imageSize.y);
        ImGui::Text("%i / 900", WorldTracker::worldTrackerTotal.noteLevelTotal);

        ImGui::TableNextColumn();
        ImGui::Image(GetWorldTrackerTexture("Jiggy"), imageSize);
        ImGui::SameLine();
        TableCellCenteredSetCursorPosY(imageSize.y);
        ImGui::Text("%i / 100", WorldTracker::worldTrackerTotal.jiggyLevelTotal);

        ImGui::TableNextColumn();
        ImGui::Image(GetWorldTrackerTexture("Empty Honeycomb"), imageSize);
        ImGui::SameLine();
        TableCellCenteredSetCursorPosY(imageSize.y);
        ImGui::Text("%i / 24", WorldTracker::worldTrackerTotal.honeycombLevelTotal);

        ImGui::TableNextColumn();
        ImGui::Image(GetWorldTrackerTexture("Mumbo Token"), imageSize);
        ImGui::SameLine();
        TableCellCenteredSetCursorPosY(imageSize.y);
        ImGui::Text("%i / 116", WorldTracker::worldTrackerTotal.tokenLevelTotal);

        ImGui::EndTable();
    }

    for (int i = 0; i < 5; i++) {
        ImGui::Image(GetWorldTrackerTexture(jinjoTextureNameList[i]), imageSize);
        ImGui::SameLine();
        TableCellCenteredSetCursorPosY(imageSize.y);
        ImGui::Text("%i / 9", WorldTracker::worldTrackerTotal.hasJinjo[i]);
        if (i != 4) {
            ImGui::SameLine();
        }
    }
    if (!isEmbedded) {
        ImGui::End();
    }
}

void WorldTracker_DrawJinjos(level_e levelId) {
    for (int i = 0; i < 5; i++) {
        bool hasJinjo = WorldTracker::worldTrackerObject[levelId].hasJinjo[i];
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::ImageButton(std::to_string(i).c_str(), GetWorldTrackerTexture(jinjoTextureNameList[i]), imageSize,
                           ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, hasJinjo ? 1 : 0.4f));
        ImGui::PopItemFlag();
        if (i != 4) {
            ImGui::SameLine();
        }
    }
}

void WorldTracker_DrawWorldObject(level_e levelId) {
    ImGui::PushID(levelId);
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,
                        ImVec2(ImGui::GetStyle().CellPadding.x, ImGui::GetStyle().CellPadding.y + 2.0f));
    std::string levelName = worldNameList[levelId - 1].c_str();
    int32_t maxEHoneycombs = levelId == LEVEL_B_SPIRAL_MOUNTAIN ? 6 : 2;

    WorldTracker_PushImageButtonStyle();
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0.5f));
    if (ImGui::CollapsingHeader(levelName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(20.0f);
        if (ImGui::BeginTable(levelName.c_str(), 4, ImGuiTableFlags_SizingFixedFit)) {
            if (levelId != LEVEL_B_SPIRAL_MOUNTAIN && levelId != LEVEL_6_LAIR) {
                ImGui::TableNextColumn();
                ImGui::Image(GetWorldTrackerTexture("Music Note"), imageSize);
                ImGui::SameLine();
                TableCellCenteredSetCursorPosY(imageSize.y);
                ImGui::Text("%i / 100", WorldTracker::worldTrackerObject[levelId].noteLevelTotal);
            }

            if (levelId != LEVEL_B_SPIRAL_MOUNTAIN) {
                ImGui::TableNextColumn();
                ImGui::Image(GetWorldTrackerTexture("Jiggy"), imageSize);
                ImGui::SameLine();
                TableCellCenteredSetCursorPosY(imageSize.y);
                ImGui::Text("%i / 10", WorldTracker::worldTrackerObject[levelId].jiggyLevelTotal);
            }

            if (levelId != LEVEL_6_LAIR) {
                ImGui::TableNextColumn();
                ImGui::Image(GetWorldTrackerTexture("Empty Honeycomb"), imageSize);
                ImGui::SameLine();
                TableCellCenteredSetCursorPosY(imageSize.y);
                ImGui::Text("%i / %i", WorldTracker::worldTrackerObject[levelId].honeycombLevelTotal, maxEHoneycombs);
            }

            if (levelId != LEVEL_B_SPIRAL_MOUNTAIN) {
                ImGui::TableNextColumn();
                ImGui::Image(GetWorldTrackerTexture("Mumbo Token"), imageSize);
                ImGui::SameLine();
                TableCellCenteredSetCursorPosY(imageSize.y);
                ImGui::Text("%i / %i", WorldTracker::worldTrackerObject[levelId].tokenLevelTotal,
                            kWorlds[levelId - 1].mumboCount);
            }

            ImGui::EndTable();
        }
        if (levelId != LEVEL_B_SPIRAL_MOUNTAIN && levelId != LEVEL_6_LAIR) {
            WorldTracker_DrawJinjos(levelId);
        }
        ImGui::Unindent(20.0f);
    }
    ImGui::PopStyleColor(2);
    WorldTracker_PopImageButtonStyle();

    ImGui::PopStyleVar(1);
    ImGui::PopID();
}

void WorldTracker_DrawTracker() {
    if (gsworld_getMap() == MAP_91_FILE_SELECT) {
        ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Orange), "No File Selected...");
    } else {
        if (CVAR_SHOW_TOTAL_COLLECTED) {
            WorldTracker_DrawTotals();
        }
        if (ImGui::BeginChild("WorldTrackerChild")) {
            if (CVAR_SHOW_CURRENT_LEVEL) {
                level_e currentLevel = map_getLevel(gsworld_getMap());
                if (currentLevel < LEVEL_1_MUMBOS_MOUNTAIN || currentLevel > LEVEL_B_SPIRAL_MOUNTAIN) {
                    ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Orange),
                                       "No World Data for this level...");
                } else {
                    WorldTracker_DrawWorldObject(currentLevel);
                }
            } else {
                for (int i = LEVEL_1_MUMBOS_MOUNTAIN; i <= LEVEL_B_SPIRAL_MOUNTAIN; i++) {
                    WorldTracker_DrawWorldObject((level_e)i);
                }
            }
            ImGui::EndChild();
        }
    }
}

namespace WorldTracker {

WorldTrackerObject worldTrackerObject[LEVEL_C_BOSS];
WorldTrackerObject worldTrackerTotal;

void ClearWorldTrackerTotals() {
    worldTrackerTotal = {
        .noteLevelTotal = 0,
        .jiggyLevelTotal = 0,
        .honeycombLevelTotal = 0,
        .tokenLevelTotal = 0,
    };

    for (int i = 0; i < 5; i++) {
        worldTrackerTotal.hasJinjo[i] = 0;
    }
}

void UpdateWorldTrackerTotals(WorldTrackerObject levelTrackerObject) {
    worldTrackerTotal.noteLevelTotal += levelTrackerObject.noteLevelTotal;
    worldTrackerTotal.jiggyLevelTotal += levelTrackerObject.jiggyLevelTotal;
    worldTrackerTotal.honeycombLevelTotal += levelTrackerObject.honeycombLevelTotal;
    worldTrackerTotal.tokenLevelTotal += levelTrackerObject.tokenLevelTotal;

    for (int i = 0; i < 5; i++) {
        worldTrackerTotal.hasJinjo[i] += levelTrackerObject.hasJinjo[i];
    }
}

void UpdateWorldTracker() {
    ClearWorldTrackerTotals();

    for (int i = LEVEL_1_MUMBOS_MOUNTAIN; i <= LEVEL_B_SPIRAL_MOUNTAIN; i++) {
        int32_t tokenMaxCount = kWorlds[i - 1].mumboStart + kWorlds[i - 1].mumboCount;
        int32_t collectedTokens = 0;
        uint8_t collectedJinjos = collectedBits(i);

        worldTrackerObject[i].noteLevelTotal = D_80385FF0[i];
        worldTrackerObject[i].jiggyLevelTotal = jiggyscore_leveltotal(i);
        worldTrackerObject[i].honeycombLevelTotal = honeycombscore_get_level_total((level_e)i);

        for (int m = kWorlds[i - 1].mumboStart; m < tokenMaxCount; m++) {
            if ((sMumboTokenScore[(m - 1) / 8] & (1 << (m & 7))) != 0) {
                collectedTokens++;
            }
        }
        worldTrackerObject[i].tokenLevelTotal = collectedTokens;

        for (int j = 0; j < 5; j++) {
            int32_t jinjoIndex =
                (GetJinjoActorMarkerId((actor_e)(int32_t)(ACTOR_5E_JINJO_YELLOW + j)) - MARKER_5A_JINJO_BLUE);
            u8 jinjoBit = jinjoBitFromActor(ACTOR_5E_JINJO_YELLOW + j);
            worldTrackerObject[i].hasJinjo[jinjoIndex] = (collectedJinjos & jinjoBit) != 0 ? 1 : 0;
        }

        if (CVAR_SHOW_TOTAL_COLLECTED) {
            UpdateWorldTrackerTotals(worldTrackerObject[i]);
        }
    }
}

void WorldTrackerWindow::Draw() {
    if (!CVAR_SHOW_WORLD_TRACKER) {
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, worldTrackerBG);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, worldTrackerBG);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, worldTrackerBG);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);

    ImGui::SetNextWindowSize(ImVec2(485.0f, 500.0f), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("World Tracker", nullptr)) {
        worldTrackerBG.w = ImGui::IsWindowDocked() ? 1.0f : worldTrackerBG.w;
        WorldTracker_DrawTracker();
    }

    ImGui::End();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(1);
}

void SettingsWindow::DrawElement() {
    if (CVarGetInteger("gWindows.WorldTracker", 0)) {
        worldTrackerPopoutState = true;
        UIWidgets::WindowButton("Return World Tracker", "gWindows.WorldTracker", LighthouseGui::mWorldTrackerWindow,
                                { .size = UIWidgets::Sizes::Inline, .color = UIWidgets::Colors::Red });
    } else {
        worldTrackerPopoutState = false;
        UIWidgets::WindowButton("Popout World Tracker", "gWindows.WorldTracker", LighthouseGui::mWorldTrackerWindow,
                                { .size = UIWidgets::Sizes::Inline, .color = UIWidgets::Colors::Green });
    }

    if (ImGui::BeginTable("SettingsTable", 2)) {
        ImGui::TableSetupColumn("col1", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("col2", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextColumn();

        ImGui::SeparatorText("World Tracker");
        if (!worldTrackerPopoutState) {
            if (ImGui::BeginChild("EmbeddedWorldTrackerChild")) {
                WorldTracker_DrawTracker();
                ImGui::EndChild();
            }
        } else {
            ImGui::TextColored(UIWidgets::ColorValues.at(WIDGET_COLOR), "Tracker popped out");
        }

        ImGui::TableNextColumn();
        ImGui::SeparatorText("Window Settings");
        UIWidgets::CVarCheckbox("Only Show Current Level", CVAR_NAME_SHOW_CURRENT_LEVEL);
        if (UIWidgets::CVarCheckbox("Display Game Total", CVAR_NAME_SHOW_TOTAL_COLLECTED)) {
            UpdateWorldTracker();
        }
        ImGui::BeginDisabled(!CVAR_SHOW_TOTAL_COLLECTED);
        UIWidgets::CVarCheckbox("Separate Total Collected Checks", CVAR_NAME_SEPARATE_TOTAL_COLLECTED);
        ImGui::EndDisabled();

        ImGui::EndTable();
    }
}

void Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { WorldTracker::UpdateWorldTracker(); })
    REGISTER_LISTENER(OnActorCollisionEnd, EVENT_PRIORITY_NORMAL,
                      [](IEvent* event) { WorldTracker::UpdateWorldTracker(); })
}

} // namespace WorldTracker
