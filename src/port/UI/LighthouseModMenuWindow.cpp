#include <map>
#include <set>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <memory>
#include <thread>
#include <functional>

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
#include "port/Engine.h"
#include "port/Extractor/GameExtractor.h"
#include "port/GameVersion/BaseGameVersion.h"
#include "port/ResourceHelpers.h"
#include "port/Localization/Language.h"

std::vector<std::string> enabledModFiles;
std::vector<std::string> disabledModFiles;
std::vector<std::string> unsupportedFiles;
std::map<std::string, std::filesystem::path> filePaths;
static int dragSourceIndex = -1;
static int dragTargetIndex = -1;

enum class ModCategory {
    Base,
    Romhack,
    Scoped,
    Shared,
};

// Category for each discovered mod (keyed by basename, mirroring filePaths), plus
// the owning romhack basename for Scoped mods. Rebuilt every UpdateModFiles().
std::map<std::string, ModCategory> modCategory;
std::map<std::string, std::string> modScopeHack;

namespace LighthouseGui {
extern std::shared_ptr<LighthouseMenu> mLighthouseMenu;
}

static WidgetInfo enableModsWidget;
static WidgetInfo tabHotkeyWidget;
static WidgetInfo generateRomhackWidget;

static std::atomic<bool> sInlineExtracting{ false };
static std::atomic<int> sInlineResult{ -1 }; // -1 idle, 0 running, 1 success, 2 failure
static std::atomic<size_t> sInlineCount{ 0 };
static std::atomic<size_t> sInlineTotal{ 0 };
static std::string sInlineFile;
static std::atomic<bool> sInlineLangPack{ false };

static std::vector<std::string> sQuarantinedConflicts;
static std::vector<std::string> sRomhackBaseMismatch;

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

// Romhacks and universally shared mods get special folders
// The lang folder is skipped in scans
#define ROMHACKS_DIR "~romhacks"
#define SHARED_DIR "~shared"
#define LANG_DIR "~lang"

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
    Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
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
    return Ship::Context::GetRawInstance()->GetResourceManager()->GetArchiveManager();
}

bool IsValidExtension(std::string extension) {
    return StringHelper::IEquals(extension, ".o2r");
}

// Ensure the archive being loaded is a BK o2r
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

static bool IsReservedModPath(const std::filesystem::path& modsRoot, const std::filesystem::path& file) {
    std::error_code ec;
    std::filesystem::path rel = std::filesystem::relative(file, modsRoot, ec);
    if (ec) {
        return false;
    }
    auto it = rel.begin();
    return it != rel.end() && it->generic_string() == LANG_DIR;
}

static ModCategory CategorizeMod(const std::filesystem::path& modsRoot, const std::filesystem::path& file,
                                 bool isOverlay, const std::set<std::string>& hackNames, std::string& outScopeHack) {
    outScopeHack.clear();
    if (isOverlay) {
        return ModCategory::Romhack;
    }
    std::error_code ec;
    std::filesystem::path rel = std::filesystem::relative(file, modsRoot, ec);
    if (ec) {
        return ModCategory::Base;
    }
    std::vector<std::string> parts;
    for (const auto& comp : rel) {
        parts.push_back(comp.generic_string());
    }

    if (parts.size() >= 2 && parts[0] == SHARED_DIR) {
        return ModCategory::Shared;
    }
    if (parts.size() >= 2 && hackNames.count(parts[0]) > 0) {
        outScopeHack = parts[0];
        return ModCategory::Scoped;
    }
    return ModCategory::Base;
}

// True if `name` is a discovered romhack overlay (an .o2r carrying an
// aGameConfig). Single source of truth for "is this a romhack", reused by the
// active-hack lookup, the Romhack Menu filter, the scoped-folder check, and the
// conflict quarantine. Valid after the UpdateModFiles() scan populates modCategory.
static bool IsRomhackOverlay(const std::string& name) {
    auto it = modCategory.find(name);
    return it != modCategory.end() && it->second == ModCategory::Romhack;
}

static std::string GetActiveHack() {
    for (const auto& mod : enabledModFiles) {
        if (IsRomhackOverlay(mod)) {
            return mod;
        }
    }
    return "";
}

std::string GetActiveRomhackBasename() {
    return GetActiveHack();
}

static bool ShownInModMenu(const std::string& name, const std::string& activeHack) {
    auto it = modCategory.find(name);
    if (it == modCategory.end()) {
        return false;
    }
    switch (it->second) {
        case ModCategory::Romhack:
            return false;
        case ModCategory::Base:
            return activeHack.empty();
        case ModCategory::Shared:
            return true;
        case ModCategory::Scoped:
            return !activeHack.empty() && modScopeHack[name] == activeHack;
    }
    return false;
}

bool IsScopedModFolderName(const std::string& topLevelName) {
    // A top-level mods/<name>/ folder is a scoped bucket when it shares a name
    // with a romhack overlay.
    return IsRomhackOverlay(topLevelName);
}

static bool DetectAndQuarantineGameConfigConflicts() {
    std::vector<std::string> withConfig;
    for (const auto& name : enabledModFiles) {
        if (IsRomhackOverlay(name)) {
            withConfig.push_back(name);
        }
    }

    if (withConfig.size() <= 1) {
        return false;
    }

    for (const auto& name : withConfig) {
        SPDLOG_WARN("[ModMenu] Quarantining '{}' due to aGameConfig conflict (multiple romhack overlays enabled)",
                    name);
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
    modCategory.clear();
    modScopeHack.clear();
    bool changed = false;
    std::string modsPath = Ship::Context::GetPathRelativeToAppDirectory("mods");
    std::map<std::string, std::string> tempMods;
    if (modsPath.length() > 0 && std::filesystem::exists(modsPath)) {
        std::vector<std::filesystem::path> enabledFiles;
        if (std::filesystem::is_directory(modsPath)) {
            std::vector<std::filesystem::path> candidates;
            std::set<std::string> hackNames;
            std::set<std::string> overlayPaths;
            for (const std::filesystem::directory_entry& p : std::filesystem::recursive_directory_iterator(
                     modsPath, std::filesystem::directory_options::follow_directory_symlink)) {
                if (p.is_directory()) {
                    continue;
                }
                if (!IsValidExtension(p.path().extension().generic_string())) {
                    continue;
                }
                // Skip reserved folders (e.g. mods/~lang/ language packs) — they
                // aren't user-toggleable mods and must not appear in either menu.
                if (IsReservedModPath(modsPath, p.path())) {
                    continue;
                }
                candidates.push_back(p.path());
                if (ArchiveHasGameConfig(p.path())) {
                    hackNames.insert(p.path().stem().generic_string());
                    overlayPaths.insert(p.path().generic_string());
                }
            }

            for (const std::filesystem::path& path : candidates) {
                std::string filename = path.stem().generic_string();
                std::string scopeHack;
                bool isOverlay = overlayPaths.count(path.generic_string()) > 0;
                ModCategory category = CategorizeMod(modsPath, path, isOverlay, hackNames, scopeHack);
                modCategory[filename] = category;
                modScopeHack[filename] = scopeHack;
                bool enabled =
                    std::find(enabledModFiles.begin(), enabledModFiles.end(), filename) != enabledModFiles.end();
                bool userDisabled =
                    std::find(disabledModFiles.begin(), disabledModFiles.end(), filename) != disabledModFiles.end();
                if (!enabled && !userDisabled) {
                    tempMods.emplace(path.lexically_normal().generic_string(), filename);
                }
                filePaths.emplace(filename, path);
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
                const bool baseCompatible = Lighthouse::BaseGameSupportsRomhacks();
                const std::string activeHack = GetActiveHack();

                if (!activeHack.empty()) {
                    std::error_code ec;
                    std::filesystem::create_directories(std::filesystem::path(modsPath) / activeHack, ec);
                }

                auto loadCategory = [&](ModCategory want) {
                    for (const std::string& mod : enabledModFiles) {
                        auto cit = modCategory.find(mod);
                        if (cit == modCategory.end() || cit->second != want)
                            continue;
                        auto it = filePaths.find(mod);
                        if (it == filePaths.end())
                            continue;
                        if (want == ModCategory::Scoped && modScopeHack[mod] != activeHack)
                            continue;
                        if (want == ModCategory::Romhack && !baseCompatible) {
                            SPDLOG_WARN("[ModMenu] Refusing romhack overlay '{}': base bk.o2r is not US v1.0; "
                                        "romhacks require a v1.0 base.",
                                        mod);
                            sRomhackBaseMismatch.push_back(mod);
                            continue;
                        }
                        SPDLOG_INFO("[ModMenu] Loading mod '{}'", it->second.generic_string());
                        GetArchiveManager()->AddArchive(it->second.generic_string());
                    }
                };

                if (activeHack.empty()) {
                    loadCategory(ModCategory::Base);
                }
                loadCategory(ModCategory::Romhack);
                loadCategory(ModCategory::Scoped);
                loadCategory(ModCategory::Shared);

                // Persist-disable the refused romhacks so the mod list reflects
                // reality on the next boot.
                for (const auto& mod : sRomhackBaseMismatch) {
                    auto eit = std::find(enabledModFiles.begin(), enabledModFiles.end(), mod);
                    if (eit != enabledModFiles.end()) {
                        enabledModFiles.erase(eit);
                        disabledModFiles.push_back(mod);
                    }
                }
                if (!sRomhackBaseMismatch.empty()) {
                    AfterModChange();
                    changed = true;
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

using ModFilter = std::function<bool(const std::string&)>;

static void DrawMods(bool enabled, const ModFilter& shown, bool alphabetical) {
    std::vector<std::string>& selectedModFiles = GetModFiles(enabled);

    std::vector<size_t> visible;
    for (size_t i = 0; i < selectedModFiles.size(); i++) {
        if (shown(selectedModFiles[i])) {
            visible.push_back(i);
        }
    }
    if (visible.empty()) {
        return;
    }

    bool madeAnyChange = false;
    size_t switchFromIndex = 0;
    size_t switchToIndex = 0;
    std::string pendingMoveFile;

    for (size_t k = 0; k < visible.size(); k++) {
        size_t vpos = alphabetical ? k : (visible.size() - 1 - k);
        size_t i = visible[vpos];
        std::string file = selectedModFiles[i];
        if (enabled) {
            ImGui::BeginGroup();
        }

        // Move-between-columns toggle: enabled mods get an arrow pointing right
        // (to the disabled list), disabled mods get an arrow pointing left.
        if (UIWidgets::StateButton((file + "_left_right").c_str(), enabled ? ICON_FA_ARROW_RIGHT : ICON_FA_ARROW_LEFT,
                                   ImVec2(25, 25), UIWidgets::ButtonOptions().Color(THEME_COLOR))) {
            pendingMoveFile = file;
        }

        if (enabled && !alphabetical) {
            const bool atTop = (vpos == visible.size() - 1);
            const bool atBottom = (vpos == 0);

            ImGui::SameLine();
            if (atTop) {
                ImGui::BeginDisabled();
            }
            if (UIWidgets::StateButton((file + "_up").c_str(), ICON_FA_ARROW_UP, ImVec2(25, 25),
                                       UIWidgets::ButtonOptions().Color(THEME_COLOR))) {
                madeAnyChange = true;
                switchFromIndex = i;
                switchToIndex = visible[vpos + 1];
            }
            if (atTop) {
                ImGui::EndDisabled();
            }

            ImGui::SameLine();
            if (atBottom) {
                ImGui::BeginDisabled();
            }
            if (UIWidgets::StateButton((file + "_down").c_str(), ICON_FA_ARROW_DOWN, ImVec2(25, 25),
                                       UIWidgets::ButtonOptions().Color(THEME_COLOR))) {
                madeAnyChange = true;
                switchFromIndex = i;
                switchToIndex = visible[vpos - 1];
            }
            if (atBottom) {
                ImGui::EndDisabled();
            }
        }

        DrawModInfo(filePaths.at(file).filename().generic_string());
        if (enabled) {
            ImGui::EndGroup();
            ModsHandleDragAndDrop(selectedModFiles, (int)i, file);
        }
    }

    if (enabled) {
        ModsPostDragAndDrop();
    }

    if (madeAnyChange) {
        std::iter_swap(selectedModFiles.begin() + switchFromIndex, selectedModFiles.begin() + switchToIndex);
        AfterModChange();
    }

    if (!pendingMoveFile.empty()) {
        if (enabled) {
            DisableMod(pendingMoveFile);
        } else {
            EnableMod(pendingMoveFile);
        }
    }
}

static bool editing = false;

static void DrawModManager(const char* tableId, const ModFilter& shown, bool alphabetical = false) {
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
                                         "Disable every mod shown in this list and force a rebuild on next boot.\n"
                                         "Click Apply & Restart to save this change.",
                                         "Clear", "Cancel", [shown]() {
                                             for (auto it = enabledModFiles.begin(); it != enabledModFiles.end();) {
                                                 if (shown(*it)) {
                                                     disabledModFiles.push_back(*it);
                                                     it = enabledModFiles.erase(it);
                                                 } else {
                                                     ++it;
                                                 }
                                             }
                                             AfterModChange();
                                         });
        }
        ImGui::SameLine();
        if (UIWidgets::Button("Apply & Restart",
                              UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Inline).Color(THEME_COLOR))) {
            LighthouseGui::RegisterPopup(
                "Apply & Restart", "Applying mods requires a restart. Save the mod list and relaunch Lighthouse now?",
                "Restart", "Cancel", []() {
                    SetEnabledModsCVarValue();
                    Ship::Context::GetRawInstance()->GetConsoleVariables()->Save();
                    GameEngine::RequestRelaunch();
                    Ship::Context::GetRawInstance()->GetWindow()->Close();
                });
        }
    }
    ImGui::BeginDisabled(!editing);
    if (ImGui::BeginTable(tableId, 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersV)) {
        ImGui::TableSetupColumn("Enabled Mods", ImGuiTableColumnFlags_WidthStretch, 200.0f);
        ImGui::TableSetupColumn("Disabled Mods", ImGuiTableColumnFlags_WidthStretch, 200.0f);
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::TableHeadersRow();
        ImGui::PopItemFlag();
        ImGui::TableNextRow();

        ImGui::TableNextColumn();

        if (ImGui::BeginChild("Enabled Mods", ImVec2(0, -8))) {
            DrawMods(true, shown, alphabetical);

            ImGui::EndChild();
        }

        ImGui::TableNextColumn();

        if (ImGui::BeginChild("Disabled Mods", ImVec2(0, -8))) {
            DrawMods(false, shown, alphabetical);

            ImGui::EndChild();
        }

        ImGui::EndTable();
    }
    ImGui::EndDisabled();
}

void LighthouseModMenuWindow::DrawElement() {
    LighthouseGui::mLighthouseMenu->MenuDrawItem(enableModsWidget, 200,
                                                 static_cast<UIWidgets::Colors>(LighthouseGui::GetMenuThemeColor()));
    ImGui::SameLine();
    LighthouseGui::mLighthouseMenu->MenuDrawItem(tabHotkeyWidget, 200,
                                                 static_cast<UIWidgets::Colors>(LighthouseGui::GetMenuThemeColor()));

    const std::string activeHack = GetActiveHack();
    if (activeHack.empty()) {
        ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Cyan),
                           "No romhack active. Displaying mods compatible with base game.");
    } else {
        ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Cyan),
                           "Displaying eligible mods for mods/~romhacks/%s.o2r.", activeHack.c_str());
    }

    ImGui::TextColored(
        UIWidgets::ColorValues.at(UIWidgets::Colors::Yellow),
        "Mods are currently not reloaded at runtime. Close and re-open Lighthouse for the changes to take effect.\n"
        "Drag ordering for the enabled list is available.\nMod priority is top to bottom. They override mods listed "
        "below them.");

    DrawModManager("tableMods", [activeHack](const std::string& name) { return ShownInModMenu(name, activeHack); });
}

void LighthouseRomhackMenuWindow::DrawElement() {
    LighthouseGui::mLighthouseMenu->MenuDrawItem(generateRomhackWidget, 200,
                                                 static_cast<UIWidgets::Colors>(LighthouseGui::GetMenuThemeColor()));

    ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Yellow),
                       "Enable one romhack here, then use the Mod Menu to manage that hack's mods.\n"
                       "Romhack overlays live in mods/~romhacks/. Changes require a restart, and only one\n"
                       "romhack can be active at a time.");

    // Romhacks are one-at-a-time, so there's no load order to preserve: list them
    // alphabetically instead of by priority.
    DrawModManager(
        "tableRomhacks", [](const std::string& name) { return IsRomhackOverlay(name); }, true);
}

void LighthouseModMenuWindow::InitElement() {
    UpdateModFiles(true);
}

static void RegisterModMenuWidgets() {
    enableModsWidget = { .name = "Enable Mods", .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    enableModsWidget.CVar(CVAR_SETTING("Mods.AlternateAssets"))
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
    LighthouseGui::mLighthouseMenu->AddSearchWidget(
        { enableModsWidget, "Settings", "Mod Menu", "Top", "alternate assets" });

    tabHotkeyWidget = { .name = "Mods Tab Hotkey", .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    tabHotkeyWidget.CVar(CVAR_SETTING("Mods.AlternateAssetsHotkey"))
        .RaceDisable(false)
        .Options(UIWidgets::CheckboxOptions()
                     .Color(THEME_COLOR)
                     .Tooltip("Allows pressing the Tab key to toggle mods")
                     .DefaultValue(true));
    LighthouseGui::mLighthouseMenu->AddSearchWidget(
        { tabHotkeyWidget, "Settings", "Mod Menu", "Top", "alternate assets tab hotkey" });

    generateRomhackWidget = { .name = "Generate Romhack from ROM", .type = WidgetType::WIDGET_BUTTON };
    generateRomhackWidget.RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            LighthouseGui::RegisterPopup(
                "Generate Romhack from ROM",
                "Select a romhack ROM to extract as a mod overlay. Torch will\n"
                "generate a slim mod o2r in your mods/~romhacks/ folder alongside\n"
                "the existing bk.o2r. Lighthouse closes when extraction finishes so\n"
                "the mod loads on the next launch.",
                "Select ROM", "Cancel", []() { RequestInlineModExtraction(); }, nullptr);
        })
        .Options(UIWidgets::ButtonOptions()
                     .Size(UIWidgets::Sizes::Inline)
                     .Tooltip("Pick a romhack ROM and extract it as a slim mod overlay into the mods/~romhacks/ "
                              "folder. Lighthouse closes afterward so the mod loads on the next launch."));
    LighthouseGui::mLighthouseMenu->AddSearchWidget(
        { generateRomhackWidget, "Settings", "Romhack Menu", "Top", "generate romhack rom extract overlay" });
}

static RegisterMenuInitFunc menuInitFunc(RegisterModMenuWidgets);

void MaybeShowModConflictPopup() {
    if (sQuarantinedConflicts.empty()) {
        return;
    }
    std::string body = "Multiple romhack mods were enabled, each carrying their own game config.\n"
                       "Loading more than one would cause problems, so all of them have been\n"
                       "disabled for this session.\n\n"
                       "The disabled romhacks are:\n";
    for (const auto& name : sQuarantinedConflicts) {
        body += "  - " + name + "\n";
    }
    body += "\nOpen Settings -> Romhack Menu, click Edit, and enable exactly one before relaunching.";
    LighthouseGui::RegisterPopup("Romhack Mod Conflict", body, "OK", "", nullptr, nullptr);
    sQuarantinedConflicts.clear();
}

void MaybeShowRomhackBaseMismatchPopup() {
    if (sRomhackBaseMismatch.empty()) {
        return;
    }
    std::string body = "One or more romhack mods were disabled because your base game data\n"
                       "(bk.o2r) is not the US v1.0 version. Romhacks are built from US v1.0 ROMs.\n"
                       "To play romhacks, re-extract bk.o2r from a US v1.0 ROM.\n\n"
                       "Disabled romhacks:\n";
    for (const auto& name : sRomhackBaseMismatch) {
        body += "  - " + name + "\n";
    }
    LighthouseGui::RegisterPopup("Romhack Requires US v1.0", body, "OK", "", nullptr, nullptr);
    sRomhackBaseMismatch.clear();
}

void SetSoleEnabledRomhack(const std::string& keepBasename) {
    const std::string modsPath = Ship::Context::GetPathRelativeToAppDirectory("mods");
    if (modsPath.empty() || !std::filesystem::is_directory(modsPath)) {
        SPDLOG_WARN("[ModMenu] modsPath empty or not a directory: '{}'", modsPath);
        return;
    }
    SPDLOG_INFO("[ModMenu] modsPath='{}', keep='{}', initial enabled='{}', disabled='{}'", modsPath, keepBasename,
                CVarGetString(CVAR_SETTING("EnabledMods"), ""), CVarGetString(CVAR_SETTING("DisabledMods"), ""));

    auto enabled = GetEnabledModsFromCVar();
    auto disabled = GetDisabledModsFromCVar();
    bool changed = false;

    auto eraseFrom = [&](std::vector<std::string>& v, const std::string& name) {
        auto it = std::find(v.begin(), v.end(), name);
        if (it != v.end()) {
            v.erase(it);
            return true;
        }
        return false;
    };

    for (const auto& entry : std::filesystem::recursive_directory_iterator(
             modsPath, std::filesystem::directory_options::follow_directory_symlink)) {
        if (entry.is_directory())
            continue;
        if (!StringHelper::IEquals(entry.path().extension().string(), ".o2r"))
            continue;
        if (!ArchiveHasGameConfig(entry.path()))
            continue;

        std::string basename = entry.path().stem().string();
        if (basename == keepBasename) {
            // The file we just generated: make sure it's enabled (covers the
            // in-place re-extract case where its name was already on the list).
            bool wasDisabled = eraseFrom(disabled, basename);
            if (std::find(enabled.begin(), enabled.end(), basename) == enabled.end()) {
                enabled.push_back(basename);
                changed = true;
            } else if (wasDisabled) {
                changed = true;
            }
            if (wasDisabled) {
                SPDLOG_INFO("[ModMenu] Keeping freshly-generated romhack overlay '{}' enabled", basename);
            }
        } else if (eraseFrom(enabled, basename)) {
            // A different romhack overlay: disable it so the new one is the
            // sole enabled aGameConfig carrier.
            if (std::find(disabled.begin(), disabled.end(), basename) == disabled.end()) {
                disabled.push_back(basename);
            }
            SPDLOG_INFO("[ModMenu] Disabling existing romhack overlay '{}' so the freshly-generated mod boots cleanly",
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
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }
}

bool IsInlineModExtractionBusy() {
    return sInlineExtracting.load();
}

// Kick off the (detached) extraction worker for a ROM the user already picked + loaded.
static void BeginInlineExtraction(std::shared_ptr<GameExtractor> extractor, bool langPack) {
    sInlineFile = extractor->GetRomPath();
    sInlineCount = 0;
    sInlineTotal = 0;
    sInlineResult = 0;
    sInlineLangPack = langPack;
    sInlineExtracting = true;
    std::thread([ex = std::move(extractor)]() mutable {
        const bool ok = ex->GenerateOTR(sInlineCount, sInlineTotal, "bk");
        sInlineResult = ok ? 1 : 2;
        sInlineExtracting = false;
    }).detach();
}

// Shared body of the two public requests: pick a ROM, load it, and start extraction. Language packs
// add a couple of region gates before the common kickoff. The extractor is a shared_ptr so it stays
// alive across the async pick (captured in the callback) and the detached extraction thread.
static void StartInlineRomExtraction(bool langPack) {
    if (sInlineExtracting.load()) {
        return;
    }
    auto extractor = std::make_shared<GameExtractor>();
    extractor->SelectGameFromUI([extractor, langPack](bool ok) {
        if (!ok) {
            return; // cancelled or failed to load
        }
        if (langPack) {
            // Refuse a pack when we already have it as bk.o2r.
            const std::string region = extractor->GetRegionSlug();
            if (region.empty()) {
                LighthouseGui::RegisterPopup("Unrecognized ROM",
                                             "That file isn't a recognized Banjo-Kazooie ROM, so no\n"
                                             "language pack could be made from it.",
                                             "OK", "", nullptr, nullptr);
                return;
            }
            if (region == Lighthouse::BaseRegionSlug()) {
                LighthouseGui::RegisterPopup("Already Have This Language",
                                             "Your base game data (bk.o2r) already provides this region's\n"
                                             "dialog, so there's no need to add it as a language pack.",
                                             "OK", "", nullptr, nullptr);
                return;
            }
            // Re-extracting a region overwrites its existing pack (e.g. to refresh it after an
            // update), so a pre-existing mods/lang/bk<region>.o2r is fine.
            std::error_code ec;
            std::filesystem::create_directories(Ship::Context::GetPathRelativeToAppDirectory("mods/lang"), ec);
            extractor->SetDialogPack(true);
        }
        BeginInlineExtraction(extractor, langPack);
    });
}

void RequestInlineModExtraction() {
    StartInlineRomExtraction(false);
}

void RequestInlineLanguagePackExtraction() {
    StartInlineRomExtraction(true);
}

void DrawInlineModExtraction() {
    if (GameExtractor::sCustomCodePromptRequested.exchange(false)) {
        LighthouseGui::RegisterPopup(
            "Custom Code Romhack Detected",
            "This romhack ships custom code.\n"
            "Lighthouse cannot extract this code, so expected\n"
            "behavior will be missing or broken when playing.\n"
            "\n"
            "Continue extraction anyway?",
            "Continue", "Cancel",
            []() {
                GameExtractor::sCustomCodePromptResult = 1;
                GameExtractor::sCustomCodePromptActive = false;
            },
            []() {
                GameExtractor::sCustomCodePromptResult = 0;
                GameExtractor::sCustomCodePromptActive = false;
            });
    }

    // Completion handling.
    const int result = sInlineResult.load();
    if (result == 1 && sInlineLangPack.exchange(false)) {
        sInlineResult = -1;
        // A language pack is an additive archive (no game re-init like a romhack),
        // so a freshly-extracted pack always loads live: register it and rescan.
        const std::string packPath = GameExtractor::sLastOutputPath;
        if (!packPath.empty() && GetArchiveManager()->AddArchive(packPath) == nullptr) {
            // Shouldn't happen for an o2r we just wrote; if it does, the pack is
            // still in mods/~lang and loads on the next launch.
            SPDLOG_WARN("[Lang] Pack '{}' didn't register live; it will load on the next launch.", packPath);
        }
        Lighthouse::RescanLanguages();
        LighthouseGui::RegisterPopup("Language Pack Added",
                                     "The language pack was added and is ready to use.\n"
                                     "Pick it from Settings -> General -> Languages.",
                                     "OK", "", nullptr, nullptr);
    } else if (result == 1) {
        sInlineResult = -1;
        try {
            std::filesystem::path produced(GameExtractor::sLastOutputPath);
            if (std::filesystem::exists(produced) && ArchiveHasGameConfig(produced)) {
                std::filesystem::path romhacksDir =
                    std::filesystem::path(Ship::Context::GetPathRelativeToAppDirectory("mods")) / ROMHACKS_DIR;
                std::filesystem::path dest = romhacksDir / produced.filename();
                std::error_code same;
                if (!std::filesystem::equivalent(produced, dest, same)) {
                    std::filesystem::create_directories(romhacksDir);
                    std::error_code rec;
                    std::filesystem::remove(dest, rec); // replace a prior overlay of the same name
                    std::filesystem::rename(produced, dest);
                    GameExtractor::sLastOutputPath = dest.generic_string();
                    SPDLOG_INFO("[ModMenu] Moved romhack overlay '{}' into {}/", produced.filename().generic_string(),
                                ROMHACKS_DIR);
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            SPDLOG_WARN("[ModMenu] Could not relocate romhack overlay into {}/: {}", ROMHACKS_DIR, e.what());
        }
        // Make the freshly-generated romhack the sole enabled overlay so the
        // boot-time aGameConfig conflict check doesn't quarantine it.
        const std::string keep = std::filesystem::path(GameExtractor::sLastOutputPath).stem().string();
        SetSoleEnabledRomhack(keep);
        Ship::Context::GetRawInstance()->GetConsoleVariables()->Save();
        if (!Lighthouse::BaseGameSupportsRomhacks()) {
            LighthouseGui::RegisterPopup("Romhack Requires US v1.0",
                                         "The romhack mod was extracted into your mods folder, but your\n"
                                         "base game data (bk.o2r) is not the US v1.0 version.\n\n"
                                         "Romhacks are built from US v1.0 ROMs and will not play correctly\n"
                                         "on a v1.1/PAL/JP base. Re-extract bk.o2r from a US v1.0 ROM,\n"
                                         "then enable this mod from Settings -> Romhack Menu.",
                                         "OK", "", nullptr, nullptr);
        } else {
            LighthouseGui::RegisterPopup(
                "Mod Installed",
                "The romhack mod was extracted into your mods folder.\n"
                "Lighthouse needs to restart to load it.\n\n"
                "Restart now?",
                "Restart", "Later",
                []() {
                    GameEngine::RequestRelaunch();
                    Ship::Context::GetRawInstance()->GetWindow()->Close();
                },
                nullptr);
        }
    } else if (result == 2) {
        sInlineResult = -1;
        std::string body = GameExtractor::sLastError.empty()
                               ? "Extraction failed. Check logs/Lighthouse.log for details."
                               : ("Extraction failed:\n\n" + GameExtractor::sLastError);
        LighthouseGui::RegisterPopup("Extraction Failed", body, "OK", "", nullptr, nullptr);
    }

    const bool wantProgress = sInlineExtracting.load() && !GameExtractor::sCustomCodePromptActive.load();
    if (wantProgress && !ImGui::IsPopupOpen("ROM Extraction")) {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::OpenPopup("ROM Extraction");
    }
    if (!ImGui::IsPopupOpen("ROM Extraction")) {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
    auto color = UIWidgets::ColorValues.at(THEME_COLOR);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(color.x, color.y, color.z, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(color.x, color.y, color.z, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
    if (ImGui::BeginPopupModal("ROM Extraction", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings)) {
        if (!wantProgress) {
            ImGui::CloseCurrentPopup();
        } else {
            const int phase = GameExtractor::sPhase;
            float progress =
                phase == 3 ? 100.0f : (sInlineTotal > 0 ? (float)sInlineCount / (float)sInlineTotal : 0.0f) * 100.0f;
            if (progress > 100.0f) {
                progress = 100.0f;
            }

            const auto filename = std::filesystem::path(sInlineFile).filename().string();
            if (phase == 3) {
                ImGui::Text("Done!");
            } else if (phase >= 1) {
                ImGui::Text("Processing %s... (Step %d/2)", filename.c_str(), phase);
                if (Companion::Instance != nullptr && !Companion::Instance->GetCurrentAssetName().empty()) {
                    auto assetName = Companion::Instance->GetCurrentAssetName();
                    const float maxWidth = 600.0f - ImGui::GetStyle().WindowPadding.x * 2;
                    if (ImGui::CalcTextSize(assetName.c_str()).x > maxWidth) {
                        const std::string ellipsis = "...";
                        const float ellipsisWidth = ImGui::CalcTextSize(ellipsis.c_str()).x;
                        while (assetName.size() > 3 &&
                               ImGui::CalcTextSize(assetName.c_str()).x > maxWidth - ellipsisWidth) {
                            assetName.pop_back();
                        }
                        assetName += ellipsis;
                    }
                    ImGui::Text("%s", assetName.c_str());
                }
            } else {
                ImGui::Text("Starting up...");
            }

            std::string overlay;
            if (sInlineTotal > 0 && sInlineCount > 0) {
                overlay = std::to_string((int)progress) + "%";
            } else if (phase >= 1) {
                overlay = "Reading ROM, please wait...";
            } else {
                overlay = "Starting up...";
            }
            ImGui::ProgressBar(progress / 100.0f, ImVec2(600.0f, 50.0f), overlay.c_str());
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}
