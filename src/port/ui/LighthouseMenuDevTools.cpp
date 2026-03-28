#include "LighthouseMenu.h"
#include "port/Engine.h"
#include "Notification.h"
#include "LighthouseInputEditorWindow.h"
#include "LighthouseModals.h"
//#include <soh/GameVersions.h>
#include "port/ResourceHelpers.h"
#include "UIWidgets.hpp"
#include <spdlog/fmt/fmt.h>

extern "C" {
#include "variables.h"
}

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
            Ship::Context::GetInstance()->GetLogger()->set_level(
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
    /*AddWidget(path, "Debug Mode", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_DEVELOPER_TOOLS("DebugMode"))
        .Options(CheckboxOptions().Tooltip("Various debug features, including a level selector from the main menu."));*/

    AddWidget(path, "Infinite Health", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_DEVELOPER_TOOLS("InfiniteHealth"))
        .Options(CheckboxOptions().Tooltip("Prevents health from decreasing."));

    // Save Editor
    path.sidebarName = "Save Editor";
    AddSidebarEntry("Dev Tools", path.sidebarName, 1);
    AddWidget(path, "Popout Save Editor", WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("SaveEditor"))
        .WindowName("Save Editor")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip("Enables the separate Save Editor Window."));

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

    // path.sidebarName = "Object Viewer";
    // AddSidebarEntry("Dev Tools", path.sidebarName, 1);
    // AddWidget(path, "Popout Object Viewer", WIDGET_WINDOW_BUTTON)
    //     .CVar(CVAR_WINDOW("ObjectViewer"))
    //     .WindowName("Object Viewer##Dev")
    //     .HideInSearch(true)
    //     .Options(WindowButtonOptions().Tooltip("Enables the separate Object Viewer Window."));
}

} // namespace LighthouseGui
