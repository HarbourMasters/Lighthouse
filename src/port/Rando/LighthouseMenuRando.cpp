#include "port/UI/LighthouseMenu.h"
#include "port/Engine.h"
#include "port/UI/Notification.h"
#include "port/UI/LighthouseModals.h"
#include "port/ResourceHelpers.h"
#include "port/UI/UIWidgets.hpp"

#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/Spoiler/Spoiler.h"

#include <spdlog/fmt/fmt.h>

#include "variables.h"

const char* logicModes[2] = {
    "Glitchless",
    "No Logic",
};

namespace LighthouseGui {

extern std::shared_ptr<LighthouseMenu> mLighthouseMenu;
extern std::shared_ptr<LighthouseModalWindow> mModalWindow;
using namespace UIWidgets;

void LighthouseMenu::AddMenuRando() {

    // Add Rando Menu
    AddMenuEntry("Rando", CVAR_SETTING("Menu.RandoSidebarSection"));

    // Rando - General
    AddSidebarEntry("Rando", "General", 1);
    WidgetPath path = { "Rando", "General", SECTION_COLUMN_1 };

    AddWidget(path, "Settings", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Enable Rando", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_RANDOMIZER_SETTING("Enable"))
        .Options(CheckboxOptions().Tooltip("Enables Randomizer on the next new save file."));
    AddWidget(path, "Logic Mode", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        int32_t currentIndex = CVarGetInteger(Rando::StaticData::Options[RO_LOGIC].cvar, 0);
        const char* widgetLabel = logicModes[currentIndex];

        ImGui::SetNextItemWidth(ImGui::GetContentRegionMax().x * 0.5f);
        UIWidgets::PushStyleCombobox(WIDGET_COLOR);
        if (ImGui::BeginCombo("##randoLogicMode", widgetLabel)) {
            for (int i = 0; i < IM_ARRAYSIZE(logicModes); i++) {
                const bool isSelected = (currentIndex == i);

                if (ImGui::Selectable(logicModes[i], isSelected)) {
                    CVarSetInteger(Rando::StaticData::Options[RO_LOGIC].cvar, i);
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        UIWidgets::PopStyleCombobox();
    });
    AddWidget(path, "Load Existing Spoiler Log", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_RANDOMIZER_SETTING("UseExistingLog"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Uses a Spoiler Log in the randomizer folder."));
    AddWidget(path, "Available Spoiler Logs", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        std::vector<const char*> spoilerLogPtrs;
        spoilerLogPtrs.reserve(Rando::Spoiler::spoilerLogs.size());
        for (const auto& log : Rando::Spoiler::spoilerLogs) {
            spoilerLogPtrs.push_back(log.c_str());
        }

        ImGui::BeginDisabled(!CVarGetInteger(CVAR_RANDOMIZER_SETTING("UseExistingLog"), 0));
        if (UIWidgets::CVarCombobox("Seed", CVAR_RANDOMIZER_SETTING("SpoilerFileIndex"), spoilerLogPtrs,
                                    { .labelPosition = UIWidgets::LabelPositions::None, .color = WIDGET_COLOR })) {
            if (CVarGetInteger(CVAR_RANDOMIZER_SETTING("SpoilerFileIndex"), 0) == 0) {
                CVarSetString(CVAR_RANDOMIZER_SETTING("SpoilerFile"), "");
            } else {
                std::string spoilerName =
                    Rando::Spoiler::spoilerLogs[CVarGetInteger(CVAR_RANDOMIZER_SETTING("SpoilerFileIndex"), 0)];
                CVarSetString(CVAR_RANDOMIZER_SETTING("SpoilerFile"), spoilerName.c_str());
            }
        }
        ImGui::SameLine();
        if (UIWidgets::Button(ICON_FA_REFRESH, UIWidgets::ButtonOptions()
                                                   .Color(WIDGET_COLOR)
                                                   .Size(ImVec2(32.0f, 32.0f))
                                                   .Tooltip("Refreshes the list of Spoiler Logs."))) {
            Rando::Spoiler::RefreshSpoilerLogs();
        }
        ImGui::EndDisabled();
    });

    AddWidget(path, "Send Collection Notifications", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_RANDOMIZER_SETTING("RandoNotifications"))
        .Options(CheckboxOptions().Tooltip("Sends notifications when you collect a Rando Item.").DefaultValue(1));

    AddWidget(path, "Manual Seed Options", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Manual Seed ID", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        // TODO: Seeded Glitchless Generation
        ImGui::BeginDisabled(true);
        UIWidgets::CVarCheckbox("Use Manual Seed Input", CVAR_RANDOMIZER_SETTING("ManualInput"),
                                UIWidgets::CheckboxOptions().Color(WIDGET_COLOR));

        UIWidgets::PushStyleSlider();
        static char seed[256];
        std::string stringSeed = CVarGetString(CVAR_RANDOMIZER_SETTING("InputSeed"), "");
        strcpy(seed, stringSeed.c_str());

        // ImGui::BeginDisabled(!CVarGetInteger(CVAR_RANDOMIZER_SETTING("ManualInput"), 0));
        ImGui::InputText("##Seed", seed, sizeof(seed), ImGuiInputTextFlags_CallbackAlways,
                         [](ImGuiInputTextCallbackData* data) {
                             CVarSetString(CVAR_RANDOMIZER_SETTING("InputSeed"), data->Buf);
                             return 0;
                         });
        if (stringSeed.length() < 1) {
            ImGui::SameLine(17.0f);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.4f), "Leave blank for random seed");
        }
        // ImGui::EndDisabled();
        ImGui::EndDisabled();

        UIWidgets::PopStyleSlider();
    });

    AddWidget(path, "Seed Metrics", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Metrics", WIDGET_CUSTOM)
        .CustomFunction([](WidgetInfo& info) { DrawSeedMetrics(); })
        .HideInSearch(true);

    // Rando - Shuffle Options
    AddSidebarEntry("Rando", "Shuffle Options", 2);
    path = { "Rando", "Shuffle Options", SECTION_COLUMN_1 };

    AddWidget(path, "Base Shuffles", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Shuffle Empty Honeycombs", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_EMPTY_HONEYCOMBS].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Empty Honeycombs into the Pool."));
    AddWidget(path, "Shuffle Jiggies", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_JIGGIES].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Jiggies into the Pool."));
    AddWidget(path, "Shuffle Jinjos", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_JINJOS].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Jinjos into the Pool."));
    AddWidget(path, "Shuffle Mumbo Tokens", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_MUMBO_TOKENS].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Mumbo Tokens into the Pool."));
    AddWidget(path, "Shuffle Music Notes", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_MUSIC_NOTES].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Music Notes into the Pool."));

    AddWidget(path, "Advanced Shuffles", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Shuffle Beehive Contents", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_BEEHIVE_HONEYCOMBS].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Contents of Beehives into the Pool."));
    AddWidget(path, "Shuffle Blue Eggs", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_BLUE_EGGS].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Blue Eggs into the Pool."));
    AddWidget(path, "Shuffle Extra Lives", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_EXTRA_LIVES].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Extra Lives into the Pool."));
    AddWidget(path, "Shuffle Molehills", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_MOLEHILLS].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles which abilities each Molehill unlocks."));

    path.column = SECTION_COLUMN_2;

    AddWidget(path, "Win Conditions", WIDGET_SEPARATOR_TEXT);
    AddWidget(path, "Stop n Swop Hunt", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_STOP_N_SWOP].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Stop n Swop items into the Pool, collect all of them to win."));

    // Rando - Starting Loadout
    AddSidebarEntry("Rando", "Starting Loadout", 1);
    path = { "Rando", "Starting Loadout", SECTION_COLUMN_1 };

    AddWidget(path, "Starting Abilities", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Abilities", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        std::string abilityToolTip;
        bool defaultValue = false;

        ImGui::PushID(SECTION_COLUMN_1);
        if (UIWidgets::Button("Enable All", UIWidgets::ButtonOptions()
                                                .Color(UIWidgets::Colors::Green)
                                                .Size(ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))) {
            for (auto& [abilityId, abilityInfo] : abilityLoadoutMap) {
                if (abilityId == ABILITY_A_HOLD_A_JUMP_HIGHER || abilityId == ABILITY_13_1ST_NOTEDOOR) {
                    continue;
                }
                CVarSetInteger(abilityInfo.second, true);
            }
            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        }
        ImGui::SameLine();
        if (UIWidgets::Button("Disable All", UIWidgets::ButtonOptions().Color(UIWidgets::Colors::Red))) {
            for (auto& [abilityId, abilityInfo] : abilityLoadoutMap) {
                if (abilityId == ABILITY_A_HOLD_A_JUMP_HIGHER || abilityId == ABILITY_13_1ST_NOTEDOOR) {
                    continue;
                }
                CVarSetInteger(abilityInfo.second, false);
            }
            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        }
        ImGui::PopID();

        if (ImGui::BeginTable("Abilities Table", 3, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableNextColumn();

            for (auto& [abilityId, abilityInfo] : abilityLoadoutMap) {
                abilityToolTip = fmt::format("Start with {} unlocked.", abilityInfo.first);
                defaultValue =
                    abilityId == ABILITY_A_HOLD_A_JUMP_HIGHER || abilityId == ABILITY_13_1ST_NOTEDOOR ? true : false;

                ImGui::BeginDisabled(abilityId == ABILITY_A_HOLD_A_JUMP_HIGHER || abilityId == ABILITY_13_1ST_NOTEDOOR);
                UIWidgets::CVarCheckbox(abilityInfo.first, abilityInfo.second,
                                        UIWidgets::CheckboxOptions()
                                            .Color(WIDGET_COLOR)
                                            .Tooltip(abilityToolTip.c_str())
                                            .DefaultValue(defaultValue));
                ImGui::EndDisabled();
                ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
    });

    AddWidget(path, "Starting Consumables", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Consumables", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        std::string itemToolTip;
        bool defaultValue = false;

        ImGui::PushID(SECTION_COLUMN_2);
        if (UIWidgets::Button("Enable All", UIWidgets::ButtonOptions()
                                                .Color(UIWidgets::Colors::Green)
                                                .Size(ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))) {
            for (auto& [itemId, itemInfo] : itemLoadoutMap) {
                CVarSetInteger(itemInfo.second, true);
            }
            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        }
        ImGui::SameLine();
        if (UIWidgets::Button("Disable All", UIWidgets::ButtonOptions().Color(UIWidgets::Colors::Red))) {
            for (auto& [itemId, itemInfo] : itemLoadoutMap) {
                CVarSetInteger(itemInfo.second, false);
            }
            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        }
        ImGui::PopID();

        if (ImGui::BeginTable("Items Table", 3, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableNextColumn();

            for (auto& [itemId, itemInfo] : itemLoadoutMap) {
                itemToolTip = fmt::format("Start with max {}.", itemInfo.first);

                UIWidgets::CVarCheckbox(itemInfo.first, itemInfo.second,
                                        UIWidgets::CheckboxOptions().Color(WIDGET_COLOR).Tooltip(itemToolTip.c_str()));
                ImGui::TableNextColumn();
            }
            ImGui::EndTable();
        }
    });

    // Rando - Check Tracker
    path.sidebarName = "Check Tracker";
    AddSidebarEntry("Rando", path.sidebarName, 1);
    AddWidget(path, "Popout Settings", WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("CheckTrackerSettings"))
        .WindowName("Check Tracker Settings")
        .HideInSearch(true);
}

} // namespace LighthouseGui
