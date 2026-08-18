#include "LighthouseMenu.h"
#include "port/Engine.h"
#include "Notification.h"
#include "LighthouseInputEditorWindow.h"
#include "LighthouseModals.h"
//#include <soh/GameVersions.h>
#include "port/ResourceHelpers.h"
#include "port/DevTools/DevSequences.h"
#include "port/DevTools/ThreadWatchdog.h"
#include "UIWidgets.hpp"
#include <spdlog/fmt/fmt.h>

#include "variables.h"
#include "enums.h"

namespace LighthouseGui {

extern std::shared_ptr<LighthouseMenu> mLighthouseMenu;
extern std::shared_ptr<LighthouseModalWindow> mModalWindow;
using namespace UIWidgets;

static const std::unordered_map<int32_t, const char*> logLevels = {
    { DEBUG_LOG_TRACE, "Trace" }, { DEBUG_LOG_DEBUG, "Debug" }, { DEBUG_LOG_INFO, "Info" },
    { DEBUG_LOG_WARN, "Warn" },   { DEBUG_LOG_ERROR, "Error" }, { DEBUG_LOG_CRITICAL, "Critical" },
    { DEBUG_LOG_OFF, "Off" },
};

// static const std::unordered_map<int32_t, const char*> debugInfoPages = {
//     { DEBUG_PAGE_OBJECTINFO, "Object" }, { DEBUG_PAGE_CHECKSURFACEINFO, "Check Surface" },
//     { DEBUG_PAGE_MAPINFO, "Map" },       { DEBUG_PAGE_STAGEINFO, "Stage" },
//     { DEBUG_PAGE_EFFECTINFO, "Effect" }, { DEBUG_PAGE_ENEMYINFO, "Enemy" },
// };

static const std::unordered_map<int32_t, const char*> language = {
    { 0, "English" },
    { 1, "Japanese" },
};

#ifdef _DEBUG
DebugLogOption defaultLogLevel = DEBUG_LOG_DEBUG;
#else
DebugLogOption defaultLogLevel = DEBUG_LOG_INFO;
#endif

void LighthouseMenu::AddMenuDevTools() {
    // Add Dev Tools Menu
    AddMenuEntry("Dev Tools", CVAR_SETTING("Menu.DevToolsSidebarSection"));

    // General
    AddSidebarEntry("Dev Tools", "General", 3);
    WidgetPath path = { "Dev Tools", "General", SECTION_COLUMN_1 };

    AddWidget(path, "Popout Menu", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("Menu.Popout"))
        .Options(CheckboxOptions().Tooltip("Changes the menu display from overlay to windowed."));
    AddWidget(path, "Log Level", WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_DEVELOPER_TOOLS("LogLevel"))
        .Options(ComboboxOptions()
                     .Tooltip("The log level determines which messages are printed to the console."
                              " This does not affect the log file output")
                     .ComboMap(logLevels)
                     .DefaultIndex(defaultLogLevel))
        .Callback([](WidgetInfo& info) {
            Ship::Context::GetRawInstance()->GetLogger()->set_level(
                (spdlog::level::level_enum)CVarGetInteger(CVAR_DEVELOPER_TOOLS("LogLevel"), defaultLogLevel));
        })
        .PreFunc([](WidgetInfo& info) {
            info.isHidden = mLighthouseMenu->disabledMap.at(DISABLE_FOR_DEBUG_MODE_OFF).active;
        });
#ifdef USE_GBI_TRACE
    AddWidget(path, "GFX Trace Mode", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_DEVELOPER_TOOLS("GFXTrace"))
        .Options(CheckboxOptions().Tooltip(
            "Enables the Gfx trace mode, which will output information about the Gfx commands being run."));
#endif
    AddWidget(path, "Dump Thread State", WIDGET_BUTTON)
        .Callback([](WidgetInfo&) { ThreadWatchdog_DumpNow(); })
        .Options(
            ButtonOptions()
                .Size(Sizes::Inline)
                .Tooltip(
                    "Log a snapshot of the decomp thread heartbeats and pipeline queue state. The watchdog also logs "
                    "this automatically when a thread stops beating."));
    /*AddWidget(path, "Debug Mode", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_DEVELOPER_TOOLS("DebugMode"))
        .Options(CheckboxOptions().Tooltip("Various debug features, including a level selector from the main menu."));*/

    // Sequences
    using namespace Lighthouse::DevTools;
    path.sidebarName = "Sequences";
    AddSidebarEntry("Dev Tools", "Sequences", 1);

    AddWidget(path, "Parades", WIDGET_SEPARATOR_TEXT);
    AddWidget(path, "Furnace Fun Parade", WIDGET_BUTTON)
        .Callback([](WidgetInfo&) { RequestSequence(SEQ_PARADE_FF); })
        .Options(ButtonOptions().Size(Sizes::Inline).Tooltip("Jump to the post-Furnace-Fun character parade."));
    AddWidget(path, "Final Parade", WIDGET_BUTTON)
        .Callback([](WidgetInfo&) { RequestSequence(SEQ_PARADE_FINAL); })
        .Options(ButtonOptions().Size(Sizes::Inline).Tooltip("Jump to the post-Grunty end-credits parade."));

    AddWidget(path, "Cutscenes", WIDGET_SEPARATOR_TEXT);
    AddWidget(path, "Spiral Mountain Ending Sequence", WIDGET_BUTTON)
        .Callback([](WidgetInfo&) { RequestSequence(SEQ_MODE9_BK); })
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(ButtonOptions().Size(Sizes::Inline).Tooltip("Jump to the post-parade demo."));
    AddWidget(path, "Under 100 Ending Scene", WIDGET_BUTTON)
        .Callback([](WidgetInfo&) { RequestCutsceneMap(MAP_20_CS_END_NOT_100); })
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(ButtonOptions().Size(Sizes::Inline).Tooltip("Jump to the not-100 jiggy ending cutscene."));
    AddWidget(path, "All 100 Ending Scene", WIDGET_BUTTON)
        .Callback([](WidgetInfo&) { RequestSequence(SEQ_ENDING_ALL_100); })
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(ButtonOptions().Size(Sizes::Inline).Tooltip("Jump to the 100-jiggy ending cutscene."));
    AddWidget(path, "Game Over Cutscene", WIDGET_BUTTON)
        .Callback([](WidgetInfo&) { RequestSequence(SEQ_GAME_OVER); })
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(
            ButtonOptions().Size(Sizes::Inline).Tooltip("Jump to the Game Over cutscene in Grunty's machine room."));
    AddWidget(path, "Klungo's Machine Room (Unused)", WIDGET_BUTTON)
        .Callback([](WidgetInfo&) { RequestCutsceneMap(MAP_84_CS_UNUSED_MACHINE_ROOM); })
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(
            ButtonOptions().Size(Sizes::Inline).Tooltip("Unused and incomplete cutscene. It has no exit trigger."));

    AddWidget(path, "Attract Demos", WIDGET_SEPARATOR_TEXT);
    static const struct {
        const char* name;
        int demo;
    } kAttractDemos[] = {
        { "Mumbo's Mountain", 0 }, { "Inside Clanker", 1 }, { "Bubblegloop Swamp", 2 }, { "MMM Church", 3 },
        { "Nipper's Shell", 5 },   { "CCW Winter", 6 },     { "RBB Engine Room", 7 },   { "Gobi's Valley", 8 },
    };
    for (const auto& d : kAttractDemos) {
        AddWidget(path, fmt::format("{}", d.name), WIDGET_BUTTON)
            .Callback([demo = d.demo](WidgetInfo&) { RequestSequence(SEQ_ATTRACT_BASE + demo); })
            .Options(ButtonOptions().Size(Sizes::Inline));
    }

    // Save Editor
    path.sidebarName = "Save Editor";
    AddSidebarEntry("Dev Tools", path.sidebarName, 1);
    AddWidget(path, "Popout Save Editor", WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("SaveEditor"))
        .WindowName("Save Editor")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip("Enables the separate Save Editor Window."));

    // Gameplay Tools
    path.sidebarName = "Gameplay Tools";
    AddSidebarEntry("Dev Tools", path.sidebarName, 1);
    AddWidget(path, "Popout Gameplay Tools", WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("GameplayTools"))
        .WindowName("Gameplay Tools")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip("Enables the separate Gameplay Tools Window."));

    // Stats
    path.sidebarName = "Stats";
    AddSidebarEntry("Dev Tools", path.sidebarName, 1);
    AddWidget(path, "Popout Stats", WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("Stats"))
        .WindowName("Stats")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip(
            "Shows the stats window, with your FPS and frametimes, and the OS you're playing on."));

    // Console
    // path.sidebarName = "Console";
    // AddSidebarEntry("Dev Tools", path.sidebarName, 1);
    // AddWidget(path, "Popout Console", WIDGET_WINDOW_BUTTON)
    //    .CVar(CVAR_WINDOW("DevConsole"))
    //    .WindowName("Console##Dev")
    //    .HideInSearch(true)
    //    .Options(WindowButtonOptions().Tooltip("Enables the separate Console Window."));

    path.sidebarName = "Event Debugger";
    AddSidebarEntry("Dev Tools", path.sidebarName, 1);
    AddWidget(path, "Popout Event Debugger", WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("EventDebugger"))
        .WindowName("Event Debugger")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip("Enables the separate Event Debugger Window."));

    path.sidebarName = "Occlusion Debugger";
    AddSidebarEntry("Dev Tools", path.sidebarName, 1);
    AddWidget(path, "Popout Occlusion Debugger", WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("OcclusionDebug"))
        .WindowName("Occlusion Debugger")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip(
            "Enumerates the current map's camera-area (portal occlusion) geometry and lets you force-draw individual "
            "chunks to find what to add to a per-map draw allowlist."));

    // path.sidebarName = "Object Viewer";
    // AddSidebarEntry("Dev Tools", path.sidebarName, 1);
    // AddWidget(path, "Popout Object Viewer", WIDGET_WINDOW_BUTTON)
    //     .CVar(CVAR_WINDOW("ObjectViewer"))
    //     .WindowName("Object Viewer##Dev")
    //     .HideInSearch(true)
    //     .Options(WindowButtonOptions().Tooltip("Enables the separate Object Viewer Window."));
}

} // namespace LighthouseGui
