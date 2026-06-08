#include "LighthouseMenu.h"
#include "port/Engine.h"
#include "Notification.h"
#include "LighthouseInputEditorWindow.h"
#include "LighthouseModals.h"
#include "port/ResourceHelpers.h"
#include "UIWidgets.hpp"
#include <spdlog/fmt/fmt.h>

extern "C" {
#include "variables.h"
}

namespace LighthouseGui {

void LighthouseMenu::AddMenuNetwork() {
    // Add Network Menu
    AddMenuEntry("Network", CVAR_SETTING("Menu.NetworkSidebarSection"));
    WidgetPath path;

#ifndef USE_NETWORKING
    path = { "Network", "Info", SECTION_COLUMN_1 };
    AddSidebarEntry("Network", path.sidebarName, 2);

    AddWidget(path,
              ICON_FA_EXCLAMATION_TRIANGLE " The Network features are unavailable because SoH was compiled without "
                                           "network support (\"ENABLE_REMOTE_CONTROL\" build flag).",
              WIDGET_TEXT)
        .Options(UIWidgets::TextOptions().Color(UIWidgets::Colors::Orange));
    return;
#endif
    AddSidebarEntry("Network", "Anchor", 2);
}

} // namespace LighthouseGui
