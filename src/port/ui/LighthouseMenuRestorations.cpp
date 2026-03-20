#include "LighthouseMenu.h"

#define CVAR_INT_SHIP_INIT(cvar, val) \
    CVarSetInteger(cvar, val);        \
    ShipInit::Init(cvar);
	
namespace LighthouseGui {

extern std::shared_ptr<LighthouseMenu> mLighthouseMenu;
using namespace UIWidgets;

void LighthouseMenu::AddMenuRestorations() {
    // Add Restorations Menu
    AddMenuEntry("Restorations", CVAR_SETTING("Menu.RestorationsSidebarSection"));

    // Restorations -> Restorations
    WidgetPath path = { "Restorations", "Restorations", SECTION_COLUMN_1 };
    AddSidebarEntry("Restorations", path.sidebarName, 1);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Restore Return to Lair", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Restorations.ReturnToLair"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Restores the unused Return to Lair option when in Worlds."));
}

} // namespace LighthouseGui