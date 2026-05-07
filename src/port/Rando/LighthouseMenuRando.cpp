#include "port/ui/LighthouseMenu.h"
#include "port/Engine.h"
#include "port/ui/Notification.h"
#include "port/ui/LighthouseModals.h"
#include "port/ResourceHelpers.h"
#include "port/ui/UIWidgets.hpp"

#include "port/Rando/CustomObject/CustomObject.h"

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
    
    // Rando - General
    AddSidebarEntry("Rando", "General", 1);
    WidgetPath path = { "Rando", "General", SECTION_COLUMN_1 };
    
    AddWidget(path, "Enable Rando", WIDGET_SEPARATOR_TEXT);
    
    AddWidget(path, "Enable Rando", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_RANDOMIZER_SETTING("Enable"))
        .Options(CheckboxOptions().Tooltip("Enables Randomizer on the next new save file."));
    
    // Rando - Shuffle Options
    AddSidebarEntry("Rando", "Shuffle Options", 1);
    path = { "Rando", "Shuffle Options", SECTION_COLUMN_1 };
    
    AddWidget(path, "Shuffle Collectables", WIDGET_SEPARATOR_TEXT);
    
    AddWidget(path, "Shuffle Empty Honeycombs", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_EMPTY_HONEYCOMBS].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Empty Honeycombs into the Pool."));
    AddWidget(path, "Shuffle Jiggies", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_JIGGIES].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Jiggies into the Pool."));
    AddWidget(path, "Shuffle Jinjos", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_JINJOS].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Jinjos into the Pool."));
    AddWidget(path, "Shuffle Molehills", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_MOLEHILLS].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles which abilities each Molehill unlocks."));
    AddWidget(path, "Shuffle Mumbo Tokens", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_MUMBO_TOKENS].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Mumbo Tokens into the Pool."));
    AddWidget(path, "Shuffle Music Notes", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SHUFFLE_MUSIC_NOTES].cvar)
        .Options(CheckboxOptions().Tooltip("Shuffles Music Notes into the Pool."));
    
    // Rando - Junk Options
    AddSidebarEntry("Rando", "Junk Options", 1);
    path = { "Rando", "Junk Options", SECTION_COLUMN_1 };
    
    AddWidget(path, "Enable Junk", WIDGET_SEPARATOR_TEXT);
    
    AddWidget(path, "Spawn Junk For Obtained Checks", WIDGET_CVAR_CHECKBOX)
        .CVar(Rando::StaticData::Options[RO_SPAWN_JUNK].cvar)
        .Options(CheckboxOptions().Tooltip("Spawns a junk item in place of an object that has already been collected."))
        .Callback([](WidgetInfo& info) { UpdateJunkList(); });
    
    AddWidget(path, "Junk Selection", WIDGET_SEPARATOR_TEXT);
    
    AddWidget(path, "Honeycomb Refills", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_RANDOMIZER_SETTING("Junk.HealthRefill"))
        .Options(CheckboxOptions().Tooltip("Adds Health Refills to the Junk List."))
        .Callback([](WidgetInfo& info) { UpdateJunkList(); });
    AddWidget(path, "Blue Eggs", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_RANDOMIZER_SETTING("Junk.BlueEggs"))
        .Options(CheckboxOptions().Tooltip("Adds Blue Eggs to the Junk List."))
        .Callback([](WidgetInfo& info) { UpdateJunkList(); });
    AddWidget(path, "Red Feathers", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_RANDOMIZER_SETTING("Junk.RedFeathers"))
        .Options(CheckboxOptions().Tooltip("Adds Red Feathers to the Junk List."))
        .Callback([](WidgetInfo& info) { UpdateJunkList(); });
    AddWidget(path, "Gold Feathers", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_RANDOMIZER_SETTING("Junk.GoldFeathers"))
        .Options(CheckboxOptions().Tooltip("Adds Gold Feathers to the Junk List."))
        .Callback([](WidgetInfo& info) { UpdateJunkList(); });

    // Rando - Check Tracker
    path.sidebarName = "Check Tracker";
    AddSidebarEntry("Rando", path.sidebarName, 1);
    AddWidget(path, "Popout Settings", WIDGET_WINDOW_BUTTON)
        .CVar("gWindows.CheckTrackerSettings")
        .WindowName("Check Tracker Settings");
}

} // namespace LighthouseGui
