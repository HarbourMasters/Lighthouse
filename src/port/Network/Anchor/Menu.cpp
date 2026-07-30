#include "Anchor.h"
#include <libultraship/libultraship.h>
#include "port/UI/LighthouseGui.hpp"
#include "port/UI/LighthouseMenu.h"
#include "port/ShipUtils.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/Romhack/RomhackCompat.h"
#include "port/Rando/Rando.h"

namespace LighthouseGui {
extern std::shared_ptr<LighthouseMenu> mLighthouseMenu;
extern std::shared_ptr<AnchorRoomWindow> mAnchorRoomWindow;
} // namespace LighthouseGui

static const char* pvpModes[3] = { "Off", "On", "On + Friendly Fire" };
static std::vector<const char*> teleportModes = { "None", "Team Only", "All" };
static std::vector<const char*> showLocationsModes = { "None", "Team Only", "All" };

void AnchorMainMenu(WidgetInfo& info) {
    auto anchor = Anchor::GetInstance();

    std::string host = CVarGetString(CVAR_REMOTE_ANCHOR("Host"), "anchor.hm64.org");
    uint16_t port = CVarGetInteger(CVAR_REMOTE_ANCHOR("Port"), 43383);
    std::string anchorTeamId = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    std::string anchorRoomId = CVarGetString(CVAR_REMOTE_ANCHOR("RoomId"), "");
    std::string anchorName = CVarGetString(CVAR_REMOTE_ANCHOR("Name"), "");
    bool isFormValid = !host.empty() && port > 1024 && port < 65535 && !anchorRoomId.empty() && !anchorName.empty();

    ImGui::SeparatorText("Connection Settings");

    ImGui::BeginDisabled(anchor->isEnabled);
    ImGui::Text("Host & Port");
    if (UIWidgets::InputString("##Host", &host,
                               UIWidgets::InputOptions()
                                   .Size(ImGui::GetContentRegionAvail() -
                                         ImVec2((ImGui::GetFontSize() * 5 + ImGui::GetStyle().ItemSpacing.x), 0))
                                   .Color(THEME_COLOR))) {
        CVarSetString(CVAR_REMOTE_ANCHOR("Host"), host.c_str());
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }

    ImGui::SameLine();
    UIWidgets::PushStyleInput(THEME_COLOR);
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);
    if (ImGui::InputScalar("##Port", ImGuiDataType_U16, &port)) {
        CVarSetInteger(CVAR_REMOTE_ANCHOR("Port"), port);
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }
    UIWidgets::PopStyleInput();

    ImGui::Text("Name & Color");
    static Color_RGBA8 defaultColor = { 100, 255, 100, 255 };
    UIWidgets::CVarColorPicker("##Color", CVAR_REMOTE_ANCHOR("Color"), defaultColor);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (UIWidgets::InputString("##Name", &anchorName, UIWidgets::InputOptions().Color(THEME_COLOR))) {
        CVarSetString(CVAR_REMOTE_ANCHOR("Name"), anchorName.c_str());
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }
    ImGui::Text("Room ID");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (UIWidgets::InputString("##RoomId", &anchorRoomId,
                               UIWidgets::InputOptions().IsSecret(anchor->isEnabled).Color(THEME_COLOR))) {
        CVarSetString(CVAR_REMOTE_ANCHOR("RoomId"), anchorRoomId.c_str());
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }
    ImGui::Text("Team ID (Items & Flags Shared)");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (UIWidgets::InputString("##TeamId", &anchorTeamId, UIWidgets::InputOptions().Color(THEME_COLOR))) {
        CVarSetString(CVAR_REMOTE_ANCHOR("TeamId"), anchorTeamId.c_str());
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }
    ImGui::Spacing();

    if (UIWidgets::Button("Restore Defaults", UIWidgets::ButtonOptions()
                                                  .Size(ImVec2(ImGui::GetContentRegionAvail().x / 2, 0))
                                                  .Color(UIWidgets::Colors::Red))) {
        CVarSetString(CVAR_REMOTE_ANCHOR("Host"), "anchor.hm64.org");
        CVarSetInteger(CVAR_REMOTE_ANCHOR("Port"), 43383);
        CVarSetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
        CVarSetString(CVAR_REMOTE_ANCHOR("RoomId"), "");
        CVarSetString(CVAR_REMOTE_ANCHOR("Name"), "");
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }

    ImGui::SameLine();

    if (UIWidgets::Button("Global Room", UIWidgets::ButtonOptions()
                                             .Color(UIWidgets::Colors::Blue)
                                             .Tooltip("Always-online public room so you don't have to explore alone. "
                                                      "You'll see other players' characters, but nothing is synced - "
                                                      "no items, flags, PvP, or teleporting."))) {
        CVarSetString(CVAR_REMOTE_ANCHOR("Host"), "anchor.hm64.org");
        CVarSetInteger(CVAR_REMOTE_ANCHOR("Port"), 43383);
        CVarSetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
        CVarSetString(CVAR_REMOTE_ANCHOR("RoomId"), "lh-global");
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }

    ImGui::EndDisabled();

    ImGui::Spacing();

    ImGui::BeginDisabled(!isFormValid);
    const char* buttonLabel = anchor->isEnabled ? "Disable" : "Enable";
    UIWidgets::PushStyleButton(anchor->isEnabled ? UIWidgets::ColorValues.at(UIWidgets::Colors::Red)
                                                 : UIWidgets::ColorValues.at(UIWidgets::Colors::Green));
    if (ImGui::Button(buttonLabel, ImVec2(-1.0f, 0.0f))) {
        if (anchor->isEnabled) {
            CVarClear(CVAR_REMOTE_ANCHOR("Enabled"));
            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
            anchor->Disable();
        } else {
            CVarSetInteger(CVAR_REMOTE_ANCHOR("Enabled"), 1);
            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
            anchor->Enable();
        }
    }
    UIWidgets::PopStyleButton();
    ImGui::EndDisabled();
    ImGui::Spacing();

    UIWidgets::CVarCheckbox(
        "Show Teammate Notifications", CVAR_REMOTE_ANCHOR("Notifications"),
        UIWidgets::CheckboxOptions()
            .DefaultValue(true)
            .Color(THEME_COLOR)
            .Tooltip("Show a notification when a teammate collects a jiggy, opens a level, or (in a "
                     "randomizer) obtains a shuffled check. Randomizer check notifications also require "
                     "\"Send Collection Notifications\" to be enabled in the Randomizer settings."));

    ImGui::Spacing();

    if (!anchor->isEnabled) {
        return;
    }

    if (!anchor->isConnected) {
        ImGui::Text("Connecting...");
        return;
    }

    ImGui::SeparatorText("Current Room");
    ImGui::Text("%s Connected", ICON_FA_CHECK);

    if (!anchor->IsGlobalRoom()) {
        UIWidgets::PushStyleButton(THEME_COLOR);
        if (ImGui::Button("Request Team State")) {
            anchor->SendPacket_RequestTeamState();
            anchor->reloadMapOnTeamState = true;
        }
        UIWidgets::Tooltip("Try this if you are missing items or flags that your team members have collected");
        UIWidgets::PopStyleButton();

        ImGui::SameLine();
    }

    UIWidgets::WindowButton("Toggle Anchor Room Window", CVAR_WINDOW("AnchorRoom"), LighthouseGui::mAnchorRoomWindow);

    ImGui::Spacing();

    if (!LighthouseGui::mAnchorRoomWindow->IsVisible()) {
        LighthouseGui::mAnchorRoomWindow->DrawElement();
    }
}

void AnchorAdminMenu(WidgetInfo& info) {
    auto anchor = Anchor::GetInstance();

    if (!anchor->isEnabled || !anchor->isConnected || anchor->roomState.ownerClientId != anchor->ownClientId ||
        anchor->IsGlobalRoom()) {
        return;
    }

    ImGui::SeparatorText("Room Settings (Admin Only)");

    UIWidgets::PushStyleButton(THEME_COLOR);
    if (ImGui::Button("Clear All Team State")) {
        std::set<std::string> teams;
        for (auto& [clientId, client] : Anchor::GetInstance()->clients) {
            teams.insert(client.teamId);
        }
        for (auto& team : teams) {
            anchor->SendPacket_ClearTeamState(team);
        }
        anchor->roomState.isRomhack = port_isRomhack();
        anchor->roomState.romhackName = Lighthouse::CurrentRomhackLabel();
        anchor->roomState.isRando = IS_RANDO;
        anchor->roomState.seed = IS_RANDO ? (int32_t)RANDO_SEED : 0;
        anchor->lastWarnedRomhackLabel.clear();
        anchor->lastWarnedRandoState.clear();
        anchor->SendPacket_UpdateRoomState();
    }
    UIWidgets::PopStyleButton();

    // if (UIWidgets::CVarCombobox("PvP Mode:", CVAR_REMOTE_ANCHOR("RoomSettings.PvpMode"), pvpModes,
    //                             UIWidgets::ComboboxOptions()
    //                                 .DefaultIndex(1)
    //                                 .LabelPosition(UIWidgets::LabelPositions::Above)
    //                                 .Color(THEME_COLOR))) {
    //     anchor->SendPacket_UpdateRoomState();
    // }
    if (UIWidgets::CVarCombobox("Show Locations For:", CVAR_REMOTE_ANCHOR("RoomSettings.ShowLocationsMode"),
                                showLocationsModes,
                                UIWidgets::ComboboxOptions()
                                    .DefaultIndex(1)
                                    .LabelPosition(UIWidgets::LabelPositions::Above)
                                    .Color(THEME_COLOR))) {
        anchor->SendPacket_UpdateRoomState();
    }
    if (UIWidgets::CVarCombobox("Allow Teleporting To:", CVAR_REMOTE_ANCHOR("RoomSettings.TeleportMode"), teleportModes,
                                UIWidgets::ComboboxOptions()
                                    .DefaultIndex(1)
                                    .LabelPosition(UIWidgets::LabelPositions::Above)
                                    .Color(THEME_COLOR))) {
        anchor->SendPacket_UpdateRoomState();
    }
    if (UIWidgets::CVarCheckbox("Sync Items & Flags", CVAR_REMOTE_ANCHOR("RoomSettings.SyncItemsAndFlags"),
                                UIWidgets::CheckboxOptions().DefaultValue(true).Color(THEME_COLOR))) {
        anchor->SendPacket_UpdateRoomState();
    }
    if (UIWidgets::CVarCheckbox("Share Consumables (Eggs/Feathers)",
                                CVAR_REMOTE_ANCHOR("RoomSettings.ShareConsumables"),
                                UIWidgets::CheckboxOptions().DefaultValue(false).Color(THEME_COLOR))) {
        anchor->SendPacket_UpdateRoomState();
    }
}

void AnchorInstructionsMenu(WidgetInfo& info) {
    auto anchor = Anchor::GetInstance();

    ImGui::SeparatorText("Usage Instructions");

    ImGui::TextWrapped("1. All players involved should start at the file select screen");

    ImGui::TextWrapped("2. Come up with a unique Room ID (this is basically your password) and enter it, along with "
                       "your desired player name and team ID and click Enable");

    ImGui::TextWrapped("3. For Rando: The host should configure the randomizer settings and generate a seed, then get "
                       "in game to create a save file to share.");

    ImGui::TextWrapped("4. All players should now load into their game. IMPORTANT! If using an existing save/seed "
                       "ensure the player with the most progress loads the file first.");

    ImGui::TextWrapped("5. After everyone has loaded in, verify on the network tab that it doesn't warn about anyone "
                       "being on a wrong version or seed.");

    ImGui::Spacing();

    ImGui::TextWrapped(
        "Note: Team ID is used to group players together in the same team, sharing items and flags. Make sure all "
        "players who want to share progress use the same Team ID. All players with the same Team ID should be using "
        "the same randomizer seed, while players on different teams can use different seeds.");
}

#ifdef USE_NETWORKING
void RegisterAnchorMenu() {
    WidgetPath path = { "Network", "Anchor", SECTION_COLUMN_1 };
    LighthouseGui::mLighthouseMenu->AddWidget(path, "AnchorMainMenu", WIDGET_CUSTOM)
        .CustomFunction(AnchorMainMenu)
        .HideInSearch(true);
    path.column = SECTION_COLUMN_2;
    LighthouseGui::mLighthouseMenu->AddWidget(path, "AnchorAdminMenu", WIDGET_CUSTOM)
        .CustomFunction(AnchorAdminMenu)
        .HideInSearch(true);
    LighthouseGui::mLighthouseMenu->AddWidget(path, "AnchorInstructionsMenu", WIDGET_CUSTOM)
        .CustomFunction(AnchorInstructionsMenu)
        .HideInSearch(true);
}

static RegisterMenuInitFunc menuInitFunc(RegisterAnchorMenu);
#endif
