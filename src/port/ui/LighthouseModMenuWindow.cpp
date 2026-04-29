#include <map>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cctype>

#include <libultraship/libultraship.h>
#include <libultraship/bridge/consolevariablebridge.h>
#include <ship/utils/StringHelper.h>
#include <ship/Context.h>
#include <ship/window/Window.h>
#include <ship/resource/ResourceManager.h>
#include <ship/resource/archive/ArchiveManager.h>
#include <spdlog/spdlog.h>
#include <zip.h>

#include "LighthouseModMenuWindow.h"
#include "LighthouseGui.hpp"
#include "LighthouseMenu.h"
#include "Menu.h"
#include "MenuTypes.h"
#include "UIWidgets.hpp"

std::vector<std::string> enabledModFiles;
std::vector<std::string> disabledModFiles;
std::vector<std::string> unsupportedFiles;
std::map<std::string, std::filesystem::path> filePaths;
static int dragSourceIndex = -1;
static int dragTargetIndex = -1;

namespace LighthouseGui {
extern std::shared_ptr<LighthouseMenu> mLighthouseMenu;
}

static WidgetInfo enableModsWidget;
static WidgetInfo tabHotkeyWidget;

#define CVAR_ENABLED_MODS_NAME CVAR_SETTING("EnabledMods")
#define CVAR_ENABLED_MODS_DEFAULT ""
#define CVAR_ENABLED_MODS_VALUE CVarGetString(CVAR_ENABLED_MODS_NAME, CVAR_ENABLED_MODS_DEFAULT)
#define CVAR_DISABLED_MODS_NAME CVAR_SETTING("DisabledMods")
#define CVAR_DISABLED_MODS_DEFAULT ""
#define CVAR_DISABLED_MODS_VALUE CVarGetString(CVAR_DISABLED_MODS_NAME, CVAR_DISABLED_MODS_DEFAULT)

// "|" was chosen as the separator due to
// it being an invalid character in NTFS
// and being rarely used in ext4
// it is also an ASCII character
// improving portability
#define SEPARATOR "|"

static std::string JoinModList(const std::vector<std::string>& list) {
    std::string s;
    for (const auto& name : list) {
        s += name + SEPARATOR;
    }
    if (!s.empty()) {
        s.pop_back();
    }
    return s;
}

void SetEnabledModsCVarValue() {
    CVarSetString(CVAR_ENABLED_MODS_NAME, JoinModList(enabledModFiles).c_str());
    CVarSetString(CVAR_DISABLED_MODS_NAME, JoinModList(disabledModFiles).c_str());
    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
}

void AfterModChange() {
    std::sort(disabledModFiles.begin(), disabledModFiles.end(), [](const std::string& a, const std::string& b) {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                            [](char c1, char c2) { return std::tolower(c1) < std::tolower(c2); });
    });
}

void ModsPostDragAndDrop() {
    if (dragTargetIndex != -1) {
        std::string file = enabledModFiles[dragSourceIndex];
        enabledModFiles.erase(enabledModFiles.begin() + dragSourceIndex);
        enabledModFiles.insert(enabledModFiles.begin() + dragTargetIndex, file);
        dragTargetIndex = dragSourceIndex = -1;
        AfterModChange();
    }
}

void ModsHandleDragAndDrop(std::vector<std::string>& objectList, int targetIndex, const std::string& itemName,
                           ImGuiDragDropFlags flags = ImGuiDragDropFlags_SourceAllowNullID) {
    if (ImGui::BeginDragDropSource(flags)) {
        ImGui::SetDragDropPayload("DragMove", &targetIndex, sizeof(uint32_t));
        ImGui::Text("Move %s", itemName.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DragMove")) {
            IM_ASSERT(payload->DataSize == sizeof(uint32_t));
            dragSourceIndex = *(const int*)payload->Data;
            dragTargetIndex = targetIndex;
        }
        ImGui::EndDragDropTarget();
    }
}

std::vector<std::string> GetEnabledModsFromCVar() {
    std::string enabledModsCVarValue = CVAR_ENABLED_MODS_VALUE;
    if (enabledModsCVarValue.empty())
        return {};
    return StringHelper::Split(enabledModsCVarValue, SEPARATOR);
}

std::vector<std::string> GetDisabledModsFromCVar() {
    std::string disabledModsCVarValue = CVAR_DISABLED_MODS_VALUE;
    if (disabledModsCVarValue.empty())
        return {};
    return StringHelper::Split(disabledModsCVarValue, SEPARATOR);
}

std::vector<std::string>& GetModFiles(bool enabled) {
    return enabled ? enabledModFiles : disabledModFiles;
}

std::shared_ptr<Ship::ArchiveManager> GetArchiveManager() {
    return Ship::Context::GetInstance()->GetResourceManager()->GetArchiveManager();
}

bool IsValidExtension(std::string extension) {
    // Only .o2r is supported. .otr is intentionally excluded — Lighthouse
    // doesn't ship that legacy format. .zip is excluded because mods are
    // commonly distributed as zips containing nested o2rs.
    return StringHelper::IEquals(extension, ".o2r");
}

// Returns true if the o2r archive at archivePath contains a member named
// `assets/aGameConfig`. Used by the conflict resolver to detect overlays
// that would clash on the BB game-config blob.
static bool ArchiveHasGameConfig(const std::filesystem::path& archivePath) {
    int err = 0;
    zip_t* z = zip_open(archivePath.string().c_str(), ZIP_RDONLY, &err);
    if (z == nullptr) {
        return false;
    }
    bool found = zip_name_locate(z, "assets/aGameConfig", 0) >= 0;
    zip_close(z);
    return found;
}

// Prevent multiple aGameConfig files from loading.
static std::vector<std::string> sQuarantinedConflicts;

static bool DetectAndQuarantineGameConfigConflicts() {
    std::vector<std::string> withConfig;
    for (const auto& name : enabledModFiles) {
        auto it = filePaths.find(name);
        if (it == filePaths.end())
            continue;
        if (ArchiveHasGameConfig(it->second)) {
            withConfig.push_back(name);
        }
    }

    if (withConfig.size() <= 1) {
        return false;
    }

    for (const auto& name : withConfig) {
        SPDLOG_WARN("[ModMenu] Quarantining '{}' due to aGameConfig conflict (multiple romhack overlays enabled)", name);
        auto it = std::find(enabledModFiles.begin(), enabledModFiles.end(), name);
        if (it != enabledModFiles.end()) {
            enabledModFiles.erase(it);
            disabledModFiles.push_back(name);
        }
    }
    sQuarantinedConflicts = withConfig;
    return true;
}

void UpdateModFiles(bool init, bool reset) {
    if (init || reset) {
        enabledModFiles.clear();
        enabledModFiles = GetEnabledModsFromCVar();
        disabledModFiles = GetDisabledModsFromCVar();
    } else {
        disabledModFiles.clear();
    }
    unsupportedFiles.clear();
    filePaths.clear();
    bool changed = false;
    std::string modsPath = Ship::Context::GetPathRelativeToAppDirectory("mods");
    std::map<std::string, std::string> tempMods;
    if (modsPath.length() > 0 && std::filesystem::exists(modsPath)) {
        std::vector<std::filesystem::path> enabledFiles;
        if (std::filesystem::is_directory(modsPath)) {
            for (const std::filesystem::directory_entry& p : std::filesystem::recursive_directory_iterator(
                     modsPath, std::filesystem::directory_options::follow_directory_symlink)) {
                if (p.is_directory()) {
                    continue;
                }
                std::string filename =
                    p.path().filename().generic_string().substr(0, p.path().filename().generic_string().rfind("."));
                std::string extension = p.path().extension().generic_string();
                if (!IsValidExtension(extension)) {
                    continue;
                }
                bool enabled =
                    std::find(enabledModFiles.begin(), enabledModFiles.end(), filename) != enabledModFiles.end();
                bool userDisabled =
                    std::find(disabledModFiles.begin(), disabledModFiles.end(), filename) != disabledModFiles.end();
                if (!enabled && !userDisabled) {
                    tempMods.emplace(p.path().lexically_normal().generic_string(), filename);
                }
                filePaths.emplace(filename, p.path());
            }
            if (tempMods.size() > 0) {
                changed = true;
                for (auto [path, name] : tempMods) {
                    enabledModFiles.push_back(name);
                }
                tempMods.clear();
            }
            // Drop entries whose backing file vanished, before we resolve conflicts.
            auto vanished = [](const std::string& n) { return filePaths.find(n) == filePaths.end(); };
            auto enabledBefore = enabledModFiles.size();
            enabledModFiles.erase(std::remove_if(enabledModFiles.begin(), enabledModFiles.end(), vanished),
                                  enabledModFiles.end());
            if (enabledModFiles.size() != enabledBefore)
                changed = true;
            auto disabledBefore = disabledModFiles.size();
            disabledModFiles.erase(std::remove_if(disabledModFiles.begin(), disabledModFiles.end(), vanished),
                                   disabledModFiles.end());
            if (disabledModFiles.size() != disabledBefore)
                changed = true;

            if (DetectAndQuarantineGameConfigConflicts()) {
                changed = true;
            }

            // Build the disabled list from anything in filePaths that isn't
            // currently in enabledModFiles. Sort comes from AfterModChange().
            for (const auto& [name, _] : filePaths) {
                if (std::find(enabledModFiles.begin(), enabledModFiles.end(), name) == enabledModFiles.end()) {
                    if (std::find(disabledModFiles.begin(), disabledModFiles.end(), name) == disabledModFiles.end()) {
                        disabledModFiles.push_back(name);
                    }
                }
            }
            AfterModChange();

            if (init) {
                for (const std::string& mod : enabledModFiles) {
                    auto it = filePaths.find(mod);
                    if (it == filePaths.end())
                        continue;
                    GetArchiveManager()->AddArchive(it->second.generic_string());
                }
            }
        }
        if (changed) {
            SetEnabledModsCVarValue();
        }
    }
}

void EnableMod(std::string file) {
    auto it = std::find(disabledModFiles.begin(), disabledModFiles.end(), file);
    if (it == disabledModFiles.end())
        return;
    disabledModFiles.erase(it);
    enabledModFiles.insert(enabledModFiles.begin(), file);

    // TODO: runtime changes
    // GetArchiveManager()->AddArchive(file);
    AfterModChange();
}

void DisableMod(std::string file) {
    auto it = std::find(enabledModFiles.begin(), enabledModFiles.end(), file);
    if (it == enabledModFiles.end())
        return;
    enabledModFiles.erase(it);
    disabledModFiles.insert(disabledModFiles.begin(), file);

    // TODO: runtime changes
    // GetArchiveManager()->RemoveArchive(file);
    AfterModChange();
}

static void DrawModInfo(std::string file) {
    ImGui::SameLine();
    ImGui::Text("%s", file.c_str());
}

static void DrawMods(bool enabled) {
    std::vector<std::string>& selectedModFiles = GetModFiles(enabled);
    if (selectedModFiles.empty()) {
        return;
    }

    bool madeAnyChange = false;
    int switchFromIndex = -1;
    int switchToIndex = -1;

    for (size_t i = selectedModFiles.size() - 1; i != SIZE_MAX; i--) {
        std::string file = selectedModFiles[i];
        if (enabled) {
            ImGui::BeginGroup();
        }

        // Move-between-columns toggle: enabled mods get an arrow pointing right
        // (to the disabled list), disabled mods get an arrow pointing left.
        if (UIWidgets::StateButton((file + "_left_right").c_str(), enabled ? ICON_FA_ARROW_RIGHT : ICON_FA_ARROW_LEFT,
                                   ImVec2(25, 25), UIWidgets::ButtonOptions().Color(THEME_COLOR))) {
            if (enabled) {
                DisableMod(file);
            } else {
                EnableMod(file);
            }
        }

        if (enabled) {
            ImGui::SameLine();
            if (i == selectedModFiles.size() - 1) {
                ImGui::BeginDisabled();
            }
            if (UIWidgets::StateButton((file + "_up").c_str(), ICON_FA_ARROW_UP, ImVec2(25, 25),
                                       UIWidgets::ButtonOptions().Color(THEME_COLOR))) {
                madeAnyChange = true;
                switchFromIndex = i;
                switchToIndex = i + 1;
            }
            if (i == selectedModFiles.size() - 1) {
                ImGui::EndDisabled();
            }

            ImGui::SameLine();
            if (i == 0) {
                ImGui::BeginDisabled();
            }
            if (UIWidgets::StateButton((file + "_down").c_str(), ICON_FA_ARROW_DOWN, ImVec2(25, 25),
                                       UIWidgets::ButtonOptions().Color(THEME_COLOR))) {
                madeAnyChange = true;
                switchFromIndex = i;
                switchToIndex = i - 1;
            }
            if (i == 0) {
                ImGui::EndDisabled();
            }
        }

        DrawModInfo(filePaths.at(file).filename().generic_string());
        if (enabled) {
            ImGui::EndGroup();
            ModsHandleDragAndDrop(selectedModFiles, i, file);
        }
    }

    if (enabled) {
        ModsPostDragAndDrop();
    }

    if (madeAnyChange) {
        std::iter_swap(selectedModFiles.begin() + switchFromIndex, selectedModFiles.begin() + switchToIndex);
        AfterModChange();
    }
}

static bool editing = false;

void LighthouseModMenuWindow::DrawElement() {
    LighthouseGui::mLighthouseMenu->MenuDrawItem(enableModsWidget, 200,
                                                 static_cast<UIWidgets::Colors>(LighthouseGui::GetMenuThemeColor()));
    ImGui::SameLine();
    LighthouseGui::mLighthouseMenu->MenuDrawItem(tabHotkeyWidget, 200,
                                                 static_cast<UIWidgets::Colors>(LighthouseGui::GetMenuThemeColor()));

    ImGui::TextColored(
        UIWidgets::ColorValues.at(UIWidgets::Colors::Yellow),
        "Mods are currently not reloaded at runtime. Close and re-open Lighthouse for the changes to take effect.\n"
        "Drag ordering for the enabled list is available.\nMod priority is top to bottom. They override mods listed "
        "below them.");

    auto editOpts = UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Inline).Color(THEME_COLOR);
    editOpts.Disabled(editing);
    editOpts.DisabledTooltip("Already editing...");
    if (UIWidgets::Button("Edit", editOpts)) {
        editing = true;
    }
    if (editing) {
        ImGui::SameLine();
        if (UIWidgets::Button("Cancel", UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Inline))) {
            editing = false;
            UpdateModFiles(false, true);
        }
        ImGui::SameLine();
        if (UIWidgets::Button("Clear List", UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Inline))) {
            LighthouseGui::RegisterPopup("Clear List",
                                         "Clear the current mod list and force a rebuild on next boot.\nClick Apply & "
                                         "Close to save this change.",
                                         "Clear", "Cancel", []() {
                                             enabledModFiles.clear();
                                             AfterModChange();
                                         });
        }
        ImGui::SameLine();
        if (UIWidgets::Button("Apply & Close",
                              UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Inline).Color(THEME_COLOR))) {
            LighthouseGui::RegisterPopup(
                "Apply & Close", "Application currently requires a restart. Save the mod info and close Lighthouse?",
                "Close", "Cancel", []() {
                    SetEnabledModsCVarValue();
                    Ship::Context::GetInstance()->GetConsoleVariables()->Save();
                    Ship::Context::GetInstance()->GetWindow()->Close();
                });
        }
    }
    ImGui::BeginDisabled(!editing);
    if (ImGui::BeginTable("tableMods", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersV)) {
        ImGui::TableSetupColumn("Enabled Mods", ImGuiTableColumnFlags_WidthStretch, 200.0f);
        ImGui::TableSetupColumn("Disabled Mods", ImGuiTableColumnFlags_WidthStretch, 200.0f);
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::TableHeadersRow();
        ImGui::PopItemFlag();
        ImGui::TableNextRow();

        ImGui::TableNextColumn();

        if (ImGui::BeginChild("Enabled Mods", ImVec2(0, -8))) {
            DrawMods(true);

            ImGui::EndChild();
        }

        ImGui::TableNextColumn();

        if (ImGui::BeginChild("Disabled Mods", ImVec2(0, -8))) {
            DrawMods(false);

            ImGui::EndChild();
        }

        ImGui::EndTable();
    }
    ImGui::EndDisabled();
}

void LighthouseModMenuWindow::InitElement() {
    UpdateModFiles(true);
}

static void RegisterModMenuWidgets() {
    enableModsWidget = { .name = "Enable Mods", .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    enableModsWidget.CVar(CVAR_SETTING("AltAssets"))
        .RaceDisable(false)
        .Options(UIWidgets::CheckboxOptions()
                     .DisabledTooltip("Temporarily disabled while editing mods list.")
                     .Color(THEME_COLOR)
                     .Tooltip("Toggle mods. For graphics mods, this means toggling between default and mod graphics.")
                     .DefaultValue(true))
        .PreFunc([&](WidgetInfo& info) {
            auto options = std::static_pointer_cast<UIWidgets::CheckboxOptions>(info.options);
            options->disabled = editing;
        });
    LighthouseGui::mLighthouseMenu->AddSearchWidget({ enableModsWidget, "Settings", "Mod Menu", "Top",
                                                     "alternate assets" });

    tabHotkeyWidget = { .name = "Mods Tab Hotkey", .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    tabHotkeyWidget.CVar(CVAR_SETTING("Mods.AlternateAssetsHotkey"))
        .RaceDisable(false)
        .Options(UIWidgets::CheckboxOptions()
                     .Color(THEME_COLOR)
                     .Tooltip("Allows pressing the Tab key to toggle mods")
                     .DefaultValue(true));
    LighthouseGui::mLighthouseMenu->AddSearchWidget(
        { tabHotkeyWidget, "Settings", "Mod Menu", "Top", "alternate assets tab hotkey" });
}

static RegisterMenuInitFunc menuInitFunc(RegisterModMenuWidgets);

void MaybeShowModConflictPopup() {
    if (sQuarantinedConflicts.empty()) {
        return;
    }
    std::string body =
        "Multiple romhack mods were enabled, each carrying their own game config.\n"
        "Loading more than one would cause problems, so all of them have been\n"
        "disabled for this session.\n\n"
        "The disabled romhacks are:\n";
    for (const auto& name : sQuarantinedConflicts) {
        body += "  - " + name + "\n";
    }
    body += "\nOpen Settings -> Mod Menu, click Edit, and enable exactly one before relaunching.";
    LighthouseGui::RegisterPopup("Romhack Mod Conflict", body, "OK", "", nullptr, nullptr);
    sQuarantinedConflicts.clear();
}

void DisableConflictingModsForPendingExtract() {
    int pending = CVarGetInteger(CVAR_SETTING("Mod.PendingExtract"), 0);
    SPDLOG_INFO("[ModMenu] DisableConflictingModsForPendingExtract: PendingExtract={}", pending);
    if (!pending) {
        return;
    }
    const std::string modsPath = Ship::Context::GetPathRelativeToAppDirectory("mods");
    if (modsPath.empty() || !std::filesystem::is_directory(modsPath)) {
        SPDLOG_WARN("[ModMenu] modsPath empty or not a directory: '{}'", modsPath);
        return;
    }
    SPDLOG_INFO("[ModMenu] modsPath='{}', initial enabled='{}', disabled='{}'", modsPath,
                CVarGetString(CVAR_SETTING("EnabledMods"), ""), CVarGetString(CVAR_SETTING("DisabledMods"), ""));

    auto enabled = GetEnabledModsFromCVar();
    auto disabled = GetDisabledModsFromCVar();
    bool changed = false;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(
             modsPath, std::filesystem::directory_options::follow_directory_symlink)) {
        if (entry.is_directory())
            continue;
        if (!StringHelper::IEquals(entry.path().extension().string(), ".o2r"))
            continue;
        if (!ArchiveHasGameConfig(entry.path()))
            continue;

        std::string basename = entry.path().stem().string();
        auto it = std::find(enabled.begin(), enabled.end(), basename);
        if (it != enabled.end()) {
            enabled.erase(it);
            if (std::find(disabled.begin(), disabled.end(), basename) == disabled.end()) {
                disabled.push_back(basename);
            }
            SPDLOG_INFO("[ModMenu] Pre-extract: disabling existing romhack overlay '{}' so the freshly-generated mod "
                        "boots cleanly",
                        basename);
            changed = true;
        }
    }

    if (changed) {
        // Write the updated lists directly to the CVars. We can't go through
        // the in-memory enabledModFiles/disabledModFiles + SetEnabledModsCVarValue
        // path because UpdateModFiles hasn't run yet and those vectors are stale.
        std::string e, d;
        for (size_t i = 0; i < enabled.size(); i++) {
            if (i)
                e += '|';
            e += enabled[i];
        }
        for (size_t i = 0; i < disabled.size(); i++) {
            if (i)
                d += '|';
            d += disabled[i];
        }
        CVarSetString(CVAR_SETTING("EnabledMods"), e.c_str());
        CVarSetString(CVAR_SETTING("DisabledMods"), d.c_str());
        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }
}
