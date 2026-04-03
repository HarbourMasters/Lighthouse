#include "port/ui/LighthouseMenu.h"
#include "port/Engine.h"
#include "port/ui/Notification.h"
#include "port/ui/LighthouseInputEditorWindow.h"
#include "port/ui/LighthouseModals.h"
#include "port/ResourceHelpers.h"
#include "port/ui/UIWidgets.hpp"

extern "C" {
#include "variables.h"
}

namespace LighthouseGui {

extern std::shared_ptr<LighthouseMenu> mLighthouseMenu;
extern std::shared_ptr<LighthouseModalWindow> mModalWindow;
using namespace UIWidgets;

void LighthouseMenu::AddMenuRando() {

    // Add Rando Menu
    AddMenuEntry("Rando", CVAR_SETTING("Menu.RandoSidebarSection"));

    // General
    AddSidebarEntry("Rando", "General", 1);
    WidgetPath path = { "Rando", "General", SECTION_COLUMN_1 };

    AddWidget(path, "Enable Rando", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Enable Rando", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_RANDOMIZER_SETTING("Enable"))
        .Options(CheckboxOptions().Tooltip("Enables Randomizer on the next new save file."));
}

} // namespace LighthouseGui