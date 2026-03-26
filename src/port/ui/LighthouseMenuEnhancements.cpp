#include "LighthouseMenu.h"

#define CVAR_INT_SHIP_INIT(cvar, val) \
    CVarSetInteger(cvar, val);        \
    ShipInit::Init(cvar);

namespace LighthouseGui {

extern std::shared_ptr<LighthouseMenu> mLighthouseMenu;
using namespace UIWidgets;

void LighthouseMenu::AddMenuEnhancements() {
    // Add Enhancements Menu
    AddMenuEntry("Enhancements", CVAR_SETTING("Menu.EnhancementsSidebarSection"));

    // Enhancements -> Cutscenes
    WidgetPath path = { "Enhancements", "Cutscenes", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Allow Start to Skip Boot Logos", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cutscenes.SkipBootLogos"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Press Start to skip the Rareware and Nintendo logos on boot."));

    AddWidget(path, "Allow Start to Skip Intro Cutscenes", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cutscenes.StartSkipIntro"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Press Start to Skip Intro Cutscenes."));

    // Enhancements -> Graphics
    path = { "Enhancements", "Graphics", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Disable Draw Distance", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Graphics.DrawDistance"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Disables Draw Distance allowing objects to render further."));

    // Enhancements -> Restorations
    path = { "Enhancements", "Restorations", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Restore Return to Lair", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Restorations.ReturnToLair"))
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(CheckboxOptions().Tooltip("Restores the unused Return to Lair option when in Worlds."));

    // Enhancements -> Saving
    path = { "Enhancements", "Saving", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Persist Bottles Bonus", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Saving.PersistBottlesBonus"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Persists Bottle Bonus progress through the save file."));
    AddWidget(path, "Persist Extra Lives", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Saving.PersistExtraLives"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Persists Extra Lives through the save file."));
}

} // namespace LighthouseGui
