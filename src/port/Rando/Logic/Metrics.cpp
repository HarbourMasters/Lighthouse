#include "Logic.h"
#include "port/UI/UIWidgets.hpp"
#include <imgui.h>
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/ShipUtils.h"

bool seedStatus = false;
std::string statusText = "";
int32_t checksInPool = 0;
int32_t itemsInPool = 0;
int32_t abilitiesInPool = 0;

std::vector<std::string> learnedAbilities;
std::vector<std::string> regionAccessList;
std::vector<std::string> eventAccessList;

#define WIDGET_TEXT_COLOR(color) UIWidgets::ColorValues.at(UIWidgets::Colors::color)

bool metricsInitialized = false;

void Metrics_DrawEventData() {
    if (ImGui::BeginChild("EventData", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        for (auto& event : eventAccessList) {
            ImGui::Text(event.c_str());
        }
        ImGui::EndChild();
    }
}

void Metrics_DrawRegionData() {
    if (ImGui::BeginChild("RegionData", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        for (auto& region : regionAccessList) {
            ImGui::Text(region.c_str());
        }
        ImGui::EndChild();
    }
}

void Metrics_DrawAbilityData() {
    if (ImGui::BeginChild("AbilityData", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        for (auto& ability : learnedAbilities) {
            ImGui::Text(ability.c_str());
        }
        ImGui::EndChild();
    }
}

void Metrics_DrawSeedData() {
    if (ImGui::BeginChild("SeedData", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        ImVec4 statusColor = seedStatus         ? WIDGET_TEXT_COLOR(Green)
                             : checksInPool < 0 ? WIDGET_TEXT_COLOR(Red)
                                                : WIDGET_TEXT_COLOR(Orange);

        if (checksInPool < 0) {
            statusText = "Generation Not Started";
        }

        ImGui::SeparatorText("Seed Data");

        ImGui::TextColored(statusColor, statusText.c_str());
        ImGui::Separator();
        ImGui::Text("Seed ID: %i", randoFinalSeed);
        ImGui::Separator();

        if (ImGui::BeginTable("SeedDataTable", 3, ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Labels", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Values", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            TableCellCenteredText("Total Checks:");
            TableCellCenteredText("Total Items:");

            ImGui::TableNextColumn();
            std::string checkStr = std::to_string(checksInPool).c_str();
            checkStr += " / " + std::to_string(RC_MAX - 1);
            std::string itemStr = std::to_string(itemsInPool).c_str();
            itemStr += " / " + std::to_string(RC_MAX - 1);
            TableCellCenteredText(checkStr.c_str());
            TableCellCenteredText(itemStr.c_str());

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImGui::ProgressBar(((float)checksInPool / (RC_MAX - 1)), ImVec2(-1.0f, 0));
            ImGui::ProgressBar(((float)itemsInPool / (RC_MAX - 1)), ImVec2(-1.0f, 0));
            ImGui::PopStyleColor();

            ImGui::EndTable();
        }

        ImGui::EndChild();
    }
}

void Metrics_DrawTabBar() {
    UIWidgets::PushStyleTabs(WIDGET_COLOR);
    if (ImGui::BeginTabBar("MetricsTab")) {
        if (ImGui::BeginTabItem("Seed Data")) {
            Metrics_DrawSeedData();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Abilities")) {
            Metrics_DrawAbilityData();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Regions")) {
            Metrics_DrawRegionData();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Events")) {
            Metrics_DrawEventData();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    UIWidgets::PopStyleTabs();
}

void RefreshMetrics(std::string text) {
    statusText = text.c_str();

    checksInPool = Rando::Logic::checkPool.size() + Rando::Logic::abilityCheckPool.size() - 1;
    itemsInPool = Rando::Logic::itemPool.size() + Rando::Logic::abilityItemPool.size() - 1;

    seedStatus = checksInPool > 0 && statusText == "Generation Complete" ? true : false;

    learnedAbilities.clear();
    for (int a = ABILITY_0_BARGE; a < ABILITY_13_1ST_NOTEDOOR; a++) {
        if (ability_isUnlocked((ability_e)a)) {
            learnedAbilities.push_back(abilityNameList[a].c_str());
        }
    }

    regionAccessList.clear();
    for (int r = RR_UNKNOWN; r < RR_MAX; r++) {
        if (reachableRegions[(RandoRegionId)r].canAccess) {
            regionAccessList.push_back(Rando::Logic::Regions[(RandoRegionId)r].regionName);
        }
    }

    eventAccessList.clear();
    for (int e = RA_UNKNOWN; e < RA_MAX; e++) {
        if (reachableEvents[(RandoAccessId)e].canAccess) {
            eventAccessList.push_back(std::to_string((RandoAccessId)e));
        }
    }
}

void DrawSeedMetrics() {
    if (!metricsInitialized) {
        metricsInitialized = true;
        RefreshMetrics("Generation Not Started");
    }
    Metrics_DrawTabBar();
}
