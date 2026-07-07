#include "CheckTracker.h"
#include "port/Rando/Logic/Logic.h"
#include "port/ShipUtils.h"
#include "port/UI/UIWidgets.hpp"
#include <cstring>

#define DEFAULT_LOGIC_COLOR \
    Color_RGBA8 {           \
        200, 200, 200, 255  \
    }
#define DEFAULT_COLLECTED_COLOR \
    Color_RGBA8 {               \
        100, 255, 100, 255      \
    }
#define DEFAULT_SKIPPED_COLOR \
    Color_RGBA8 {             \
        255, 100, 255, 255    \
    }
#define DEFAULT_ITEM_COLOR \
    Color_RGBA8 {          \
        79, 0, 221, 255    \
    }

#define CVAR_NAME_SHOW_CHECK_TRACKER "gWindows.CheckTracker"
#define CVAR_NAME_ENABLE_FLOATING_WINDOW "gRando.CheckTracker.Floating"
#define CVAR_NAME_CHECK_TRACKER_OPACITY "gRando.CheckTracker.Opacity"
#define CVAR_NAME_CHECK_TRACKER_SCALE "gRando.CheckTracker.Scale"
#define CVAR_NAME_SHOW_CURRENT_LEVEL "gRando.CheckTracker.ShowCurrentLevel"
#define CVAR_NAME_HIDE_COMPLETED_WORLD "gRando.CheckTracker.HideCompletedWorld"
#define CVAR_NAME_SHOW_COLLECTED_CHECKS "gRando.CheckTracker.ShowCollectedChecks"
#define CVAR_NAME_SHOW_WORLD_CHECKS "gRando.CheckTracker.ShowWorldChecks"
#define CVAR_NAME_SHOW_LOGIC "gRando.CheckTracker.ShowLogic"
#define CVAR_NAME_SEPARATE_COLLECTED_CHECKS "gRando.CheckTracker.SeparateCollectedChecks"
#define CVAR_NAME_COLLECTED_CHECKS_OPACITY "gRando.CheckTracker.CollectedChecksOpacity"
#define CVAR_NAME_COLLECTED_CHECKS_SCALE "gRando.CheckTracker.CollectedChecksScale"
#define CVAR_NAME_HIDE_COLLECTED "gRando.CheckTracker.HideCollected"
#define CVAR_NAME_LOGIC_COLOR "gRando.CheckTracker.LogicColor"
#define CVAR_NAME_COLLECTED_COLOR "gRando.CheckTracker.CollectedColor"
#define CVAR_NAME_SKIPPED_COLOR "gRando.CheckTracker.SkippedColor"
#define CVAR_NAME_HIDE_SKIPPED "gRando.CheckTracker.HideSkipped"
#define CVAR_NAME_ITEM_COLOR "gRando.CheckTracker.ItemColor"

#define CVAR_SHOW_CHECK_TRACKER CVarGetInteger(CVAR_NAME_SHOW_CHECK_TRACKER, 0)
#define CVAR_ENABLE_FLOATING_WINDOW CVarGetInteger(CVAR_NAME_ENABLE_FLOATING_WINDOW, 0)
#define CVAR_CHECK_TRACKER_OPACITY CVarGetFloat(CVAR_NAME_CHECK_TRACKER_OPACITY, 0.5f)
#define CVAR_CHECK_TRACKER_SCALE CVarGetFloat(CVAR_NAME_CHECK_TRACKER_SCALE, 1.0f)
#define CVAR_SHOW_CURRENT_LEVEL CVarGetInteger(CVAR_NAME_SHOW_CURRENT_LEVEL, 0)
#define CVAR_HIDE_COMPLETED_WORLD CVarGetInteger(CVAR_NAME_HIDE_COMPLETED_WORLD, 0)
#define CVAR_SHOW_COLLECTED_CHECKS CVarGetInteger(CVAR_NAME_SHOW_COLLECTED_CHECKS, 0)
#define CVAR_SHOW_WORLD_CHECKS CVarGetInteger(CVAR_NAME_SHOW_WORLD_CHECKS, 0)
#define CVAR_SHOW_LOGIC CVarGetInteger(CVAR_NAME_SHOW_LOGIC, 0)
#define CVAR_SHOW_SEPARATE_COLLECTED_CHECKS CVarGetInteger(CVAR_NAME_SEPARATE_COLLECTED_CHECKS, 0)
#define CVAR_COLLECTED_CHECKS_OPACITY CVarGetFloat(CVAR_NAME_COLLECTED_CHECKS_OPACITY, 0.5f)
#define CVAR_COLLECTED_CHECKS_SCALE CVarGetFloat(CVAR_NAME_COLLECTED_CHECKS_SCALE, 1.0f)
#define CVAR_HIDE_COLLECTED CVarGetInteger(CVAR_NAME_HIDE_COLLECTED, 0)
#define CVAR_LOGIC_COLOR CVarGetColor(CVAR_NAME_LOGIC_COLOR ".Value", DEFAULT_LOGIC_COLOR)
#define CVAR_COLLECTED_COLOR CVarGetColor(CVAR_NAME_COLLECTED_COLOR ".Value", DEFAULT_COLLECTED_COLOR)
#define CVAR_SKIPPED_COLOR CVarGetColor(CVAR_NAME_SKIPPED_COLOR ".Value", DEFAULT_SKIPPED_COLOR)
#define CVAR_HIDE_SKIPPED CVarGetInteger(CVAR_NAME_HIDE_SKIPPED, 0)
#define CVAR_ITEM_COLOR CVarGetColor(CVAR_NAME_ITEM_COLOR ".Value", DEFAULT_ITEM_COLOR)

extern "C" {
enum map_e level_get_main_map(enum level_e level_id);
enum map_e gsworld_getMap(void);
enum level_e map_getLevel(enum map_e map);
}

namespace LighthouseGui {
extern std::shared_ptr<Rando::CheckTracker::CheckTrackerWindow> mRandoCheckTrackerWindow;
}

std::vector<std::tuple<const char*, Color_RGBA8, const char*>> defaultCheckColorList = {
    { CVAR_NAME_LOGIC_COLOR, DEFAULT_LOGIC_COLOR, "Out of Logic" },
    { CVAR_NAME_COLLECTED_COLOR, DEFAULT_COLLECTED_COLOR, "Check Collected" },
    { CVAR_NAME_SKIPPED_COLOR, DEFAULT_SKIPPED_COLOR, "Check Skipped" },
    { CVAR_NAME_ITEM_COLOR, DEFAULT_ITEM_COLOR, "Obtained Item" },
};

std::map<RandoCheckId, std::string> checkNames;

Rando::StaticData::RandoLogicData reachableRegions[RR_MAX];
Rando::StaticData::RandoLogicData reachableEvents[RA_MAX];
Rando::StaticData::RandoLogicData reachableChecks[RC_MAX];

bool isCheckTrackerInitialized = false;
bool checkTrackerPopoutState = false;
ImVec4 checkTrackerBG = ImVec4{ 0, 0, 0, 0.5f };
ImVec4 collectedChecksBG = ImVec4{ 0, 0, 0, 0.5f };
float checkTrackerScale = 1.0f;
float collectedChecksScale = 1.0f;

std::string totalCheckCount;
int32_t worldCollected = 0;
int32_t worldTotalShuffled = 0;

bool expandToggle = true;
bool expandState = true;

ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;

std::string GetTotalCheckCount() {
    std::string totalChecks;
    uint32_t collected = 0;
    uint32_t totalShuffled = 0;
    for (auto& entry : Rando::Logic::shuffledPool) {
        if (entry.obtained || entry.skipped) {
            collected++;
        }
        if (entry.isShuffled) {
            totalShuffled++;
        }
    }
    totalChecks = std::to_string(collected);
    totalChecks += " of ";
    totalChecks += std::to_string(totalShuffled);
    return totalChecks;
}

void CreateCheckStringList() {
    checkNames.clear();
    for (auto& entry : Rando::Logic::shuffledPool) {
        checkNames.insert({ entry.randoCheckId, Ship_ConvertEnumToReadableName(entry.name) });
    }
}

void UpdateWorldCheckCount(level_e world) {
    worldCollected = 0;
    worldTotalShuffled = 0;

    for (auto& entry : Rando::Logic::shuffledPool) {
        if (Rando::StaticData::Checks[entry.randoCheckId].worldId != world) {
            continue;
        }
        if (entry.obtained || entry.skipped) {
            worldCollected++;
        }
        if (entry.isShuffled) {
            worldTotalShuffled++;
        }
    }
}

std::string GetWorldCheckString(level_e world) {
    std::string worldCheckString;

    worldCheckString = std::to_string(worldCollected);
    worldCheckString += " / ";
    worldCheckString += std::to_string(worldTotalShuffled);
    return worldCheckString;
}

void DrawCheckTrackerCount() {
    if (CVAR_SHOW_COLLECTED_CHECKS) {
        totalCheckCount = GetTotalCheckCount();
        if (CVAR_SHOW_SEPARATE_COLLECTED_CHECKS) {
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, collectedChecksBG);
            ImGui::PushStyleColor(ImGuiCol_TitleBg, collectedChecksBG);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, collectedChecksBG);
            if (ImGui::Begin("CheckCount", nullptr,
                             ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing |
                                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
                ImGui::SetWindowFontScale(collectedChecksScale);
                ImGui::Text("Checks: %s", totalCheckCount.c_str());
                ImGui::End();
            }
            ImGui::PopStyleColor(3);
        } else {
            ImGui::Text("Checks: %s", totalCheckCount.c_str());
        }
    }
}

void DrawCheckTrackerList() {
    if (Rando::Logic::shuffledPool.empty()) {
        return;
    }

    if (!isCheckTrackerInitialized) {
        CreateCheckStringList();
        isCheckTrackerInitialized = true;
    }

    if (CVAR_SHOW_COLLECTED_CHECKS && !CVAR_SHOW_SEPARATE_COLLECTED_CHECKS) {
        DrawCheckTrackerCount();
    }

    for (int i = LEVEL_1_MUMBOS_MOUNTAIN; i <= LEVEL_B_SPIRAL_MOUNTAIN; i++) {

        if (CVAR_SHOW_CURRENT_LEVEL && i != map_getLevel(gsworld_getMap())) {
            continue;
        }

        UpdateWorldCheckCount((level_e)i);
        if (CVAR_HIDE_COMPLETED_WORLD && worldTotalShuffled == worldCollected) {
            continue;
        }

        std::string headerName = port_mapName(level_get_main_map((level_e)i));
        if (CVAR_SHOW_WORLD_CHECKS) {
            headerName += " ";
            headerName += GetWorldCheckString((level_e)i);
        }

        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0.5f));

        if (expandState != expandToggle) {
            ImGui::SetNextItemOpen(expandToggle);
        }

        if (ImGui::CollapsingHeader(headerName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent(20.0f);
            if (ImGui::BeginTable("CheckTrackerTable", 1)) {

                ImGui::TableNextColumn();
                for (auto& entry : Rando::Logic::shuffledPool) {
                    if (Rando::StaticData::Checks[entry.randoCheckId].worldId != i) {
                        continue;
                    }

                    if (CVAR_HIDE_COLLECTED && entry.obtained) {
                        continue;
                    }

                    if (CVAR_HIDE_SKIPPED && entry.skipped) {
                        continue;
                    }

                    ImVec4 checkTextColor = entry.obtained ? VecFromRGBA8(CVAR_COLLECTED_COLOR)
                                                           : UIWidgets::ColorValues.at(UIWidgets::Colors::White);

                    ImVec4 itemTextColor = entry.obtained ? VecFromRGBA8(CVAR_ITEM_COLOR)
                                                          : UIWidgets::ColorValues.at(UIWidgets::Colors::Indigo);
                    if (entry.skipped) {
                        checkTextColor = itemTextColor = VecFromRGBA8(CVAR_SKIPPED_COLOR);
                    }

                    if (!entry.obtained && CVAR_SHOW_LOGIC) {
                        checkTextColor = Rando::Logic::CanAccessCheck(entry.randoCheckId)
                                             ? UIWidgets::ColorValues.at(UIWidgets::Colors::White)
                                             : VecFromRGBA8(CVAR_LOGIC_COLOR);
                    }

                    ImGui::BeginGroup();
                    ImGui::TextColored(checkTextColor, checkNames.at(entry.randoCheckId).c_str());
                    if (entry.obtained) {
                        ImGui::SameLine();
                        RandoItemId randoItemId = Rando::Logic::GetShuffledObject(entry.randoCheckId).randoItemId;
                        const std::string& randoItemName =
                            Rando::StaticData::Checks[entry.randoCheckId].randoCheckType == RCTYPE_MOLEHILL
                                ? abilityNameList[entry.randoCollectionId]
                                : Rando::StaticData::Items[randoItemId].name;
                        ImGui::TextColored(itemTextColor, "(%s)", randoItemName.c_str());
                    } else if (entry.skipped) {
                        ImGui::SameLine();
                        ImGui::TextColored(itemTextColor, "(Skipped)");
                    }
                    ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 0));
                    ImGui::EndGroup();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::IsItemHovered()
                                                                          ? IM_COL32(255, 255, 0, 128)
                                                                          : IM_COL32(255, 255, 255, 0));
                    if (ImGui::IsItemClicked()) {
                        entry.skipped = !entry.skipped;
                    }
                    ImGui::TableNextColumn();
                }
                ImGui::EndTable();
            }
            ImGui::Unindent(20.0f);
        }
        ImGui::PopStyleColor(2);
        ImGui::PopID();
    }
}

namespace Rando {

namespace CheckTracker {

void CheckTrackerWindow::Draw() {
    if (!CVAR_SHOW_CHECK_TRACKER) {
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, checkTrackerBG);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, checkTrackerBG);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, checkTrackerBG);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);

    ImGui::SetNextWindowSize(ImVec2(485.0f, 500.0f), ImGuiCond_FirstUseEver);

    if (CVAR_SHOW_COLLECTED_CHECKS && CVAR_SHOW_SEPARATE_COLLECTED_CHECKS) {
        DrawCheckTrackerCount();
    }

    if (ImGui::Begin("CheckTracker", nullptr, windowFlags)) {
        checkTrackerBG.w = ImGui::IsWindowDocked() ? 1.0f : CVAR_CHECK_TRACKER_OPACITY;
        ImGui::SetWindowFontScale(checkTrackerScale);

        if (gsworld_getMap() == MAP_91_FILE_SELECT) {
            ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Orange), "No Rando File Selected...");
        } else {
            if (ImGui::BeginChild("CheckTrackerChild")) {
                DrawCheckTrackerList();
                expandState = expandToggle;
                ImGui::EndChild();
            }
        }
    }

    ImGui::End();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(1);
}

void SettingsWindow::DrawElement() {
    windowFlags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;
    if (CVAR_ENABLE_FLOATING_WINDOW) {
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
    }

    if (CVarGetInteger("gWindows.CheckTracker", 0)) {
        checkTrackerPopoutState = true;
        UIWidgets::WindowButton("Return Check Tracker", "gWindows.CheckTracker",
                                LighthouseGui::mRandoCheckTrackerWindow,
                                { .size = UIWidgets::Sizes::Inline, .color = UIWidgets::Colors::Red });
    } else {
        checkTrackerPopoutState = false;
        UIWidgets::WindowButton("Popout Check Tracker", "gWindows.CheckTracker",
                                LighthouseGui::mRandoCheckTrackerWindow,
                                { .size = UIWidgets::Sizes::Inline, .color = UIWidgets::Colors::Green });
    }
    if (ImGui::BeginTable("SettingsTable", 2)) {
        ImGui::TableSetupColumn("col1", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("col2", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextColumn();

        ImGui::SeparatorText("Check Tracker");
        if (!checkTrackerPopoutState) {
            if (ImGui::BeginChild("EmbeddedCheckTrackerChild")) {
                DrawCheckTrackerList();
                ImGui::EndChild();
            }
        } else {
            ImGui::TextColored(UIWidgets::ColorValues.at(WIDGET_COLOR), "Tracker popped out");
        }

        ImGui::TableNextColumn();
        ImGui::SeparatorText("Window Settings");
        if (ImGui::BeginChild("WindowSettingsChild")) {
            UIWidgets::CVarCheckbox("Only Show Current Level", CVAR_NAME_SHOW_CURRENT_LEVEL);
            UIWidgets::CVarCheckbox("Dim Out of Logic Checks", CVAR_NAME_SHOW_LOGIC);
            UIWidgets::CVarCheckbox("Hide Completed Worlds", CVAR_NAME_HIDE_COMPLETED_WORLD);
            UIWidgets::CVarCheckbox("Hide Collected Checks", CVAR_NAME_HIDE_COLLECTED);
            UIWidgets::CVarCheckbox("Hide Skipped Checks", CVAR_NAME_HIDE_SKIPPED);
            UIWidgets::CVarCheckbox("Display Total Collected Checks", CVAR_NAME_SHOW_COLLECTED_CHECKS);
            UIWidgets::CVarCheckbox("Display Total World Checks", CVAR_NAME_SHOW_WORLD_CHECKS);

            ImGui::BeginDisabled(!CVAR_SHOW_COLLECTED_CHECKS);
            UIWidgets::CVarCheckbox("Separate Total Collected Checks", CVAR_NAME_SEPARATE_COLLECTED_CHECKS);
            ImGui::EndDisabled();
            ImGui::BeginDisabled(!CVAR_SHOW_SEPARATE_COLLECTED_CHECKS || !CVAR_SHOW_COLLECTED_CHECKS);
            if (UIWidgets::CVarSliderFloat("  ", CVAR_NAME_COLLECTED_CHECKS_OPACITY,
                                           {
                                               .format = "Opacity: %.1f",
                                               .step = 0.01f,
                                               .min = 0.0f,
                                               .max = 1.0f,
                                               .defaultValue = 0.5f,
                                               .labelPosition = UIWidgets::LabelPositions::None,
                                               .color = WIDGET_COLOR,
                                           })) {
                collectedChecksBG.w = CVAR_COLLECTED_CHECKS_OPACITY;
            }

            if (UIWidgets::CVarSliderFloat("    ", CVAR_NAME_COLLECTED_CHECKS_SCALE,
                                           {
                                               .format = "Scale: %.1f",
                                               .step = 0.10f,
                                               .min = 0.7f,
                                               .max = 2.5f,
                                               .defaultValue = 1.0f,
                                               .labelPosition = UIWidgets::LabelPositions::None,
                                               .color = WIDGET_COLOR,
                                           })) {
                collectedChecksScale = CVAR_COLLECTED_CHECKS_SCALE;
            }

            ImGui::EndDisabled();

            UIWidgets::CVarCheckbox("Toggle Floating Window", CVAR_NAME_ENABLE_FLOATING_WINDOW);

            if (UIWidgets::Button(
                    "Expand/Collapse All Levels",
                    UIWidgets::ButtonOptions{}.Color(WIDGET_COLOR).Size(ImVec2(ImGui::GetContentRegionAvail().x, 0)))) {
                expandToggle = !expandToggle;
            }

            if (UIWidgets::CVarSliderFloat("", CVAR_NAME_CHECK_TRACKER_OPACITY,
                                           {
                                               .format = "Opacity: %.1f",
                                               .step = 0.01f,
                                               .min = 0.0f,
                                               .max = 1.0f,
                                               .defaultValue = 0.5f,
                                               .labelPosition = UIWidgets::LabelPositions::None,
                                               .color = WIDGET_COLOR,
                                           })) {
                checkTrackerBG.w = CVAR_CHECK_TRACKER_OPACITY;
            }

            if (UIWidgets::CVarSliderFloat(" ", CVAR_NAME_CHECK_TRACKER_SCALE,
                                           {
                                               .format = "Scale: %.1f",
                                               .step = 0.10f,
                                               .min = 0.7f,
                                               .max = 2.5f,
                                               .defaultValue = 1.0f,
                                               .labelPosition = UIWidgets::LabelPositions::None,
                                               .color = WIDGET_COLOR,
                                           })) {
                checkTrackerScale = CVAR_CHECK_TRACKER_SCALE;
            }

            int16_t checkColorIndex = 0;
            for (auto& [cvar, color, label] : defaultCheckColorList) {
                std::string cvarText = cvar;
                cvarText += ".Value";
                std::string colorText = label;
                colorText += " Color";
                std::string widgetLabel = "##";
                widgetLabel += std::to_string(checkColorIndex);

                ImGui::PushID(checkColorIndex);
                UIWidgets::CVarColorPicker(widgetLabel.c_str(), cvar, color, true);
                ImGui::SameLine();
                if (UIWidgets::Button(ICON_FA_REFRESH, { .size = ImVec2(32.0f, 32.0f), .color = WIDGET_COLOR })) {
                    CVarSetColor(cvarText.c_str(), color);
                    Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
                }
                ImGui::SameLine();
                ImGui::Text(colorText.c_str());
                ImGui::PopID();
                checkColorIndex++;
            }
            ImGui::EndChild();
        }
        ImGui::EndTable();
    }
}

void Init() {
    checkTrackerPopoutState = CVarGetInteger("gWindows.CheckTracker", 0);
    checkTrackerBG = { 0, 0, 0, CVAR_CHECK_TRACKER_OPACITY };
    collectedChecksBG = { 0, 0, 0, CVAR_COLLECTED_CHECKS_OPACITY };
    checkTrackerScale = CVAR_CHECK_TRACKER_SCALE;
    collectedChecksScale = CVAR_COLLECTED_CHECKS_SCALE;
}

} // namespace CheckTracker
} // namespace Rando