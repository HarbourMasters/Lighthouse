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

    AddWidget(path, "Skip Jiggy Dance", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cutscenes.SkipJiggyDance"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "Skips the jiggy collection dance, collecting the jiggy immediately like underwater pickups."));

    // Enhancements -> Graphics
    path = { "Enhancements", "Graphics", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Extended Draw Distance", WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("Graphics.DrawDistance"))
        .RaceDisable(false)
        .Options(ComboboxOptions()
                     .Tooltip("Extends the draw distance for objects.\nHigher values render more but cost performance.")
                     .ComboMap({
                         { 0, "Off" },
                         { 1, "25%" },
                         { 2, "50%" },
                         { 3, "75%" },
                         { 4, "100%" },
                     })
                     .DefaultIndex(0));

    AddWidget(path, "Always High Poly Banjo", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Graphics.AlwaysHighPolyBanjo"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Makes Banjo always use the high-polygon model, even in low-detail modes."));

    AddWidget(path, "Original Aspect Ratio In Cutscenes", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Graphics.CutsceneAspect"))
        .Options(CheckboxOptions().Tooltip("Force game to show original aspect ratio during cutscenes to avoid seeing "
                                           "unfinished edges of scene geometry."));

    // Enhancements -> Fixes
    path = { "Enhancements", "Fixes", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Fix Furnace Fun Game Over Dialog", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.FurnaceFunDialog"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Fixes the skull panel game over warning in Furnace Fun to trigger when you "
                                           "have zero extra lives instead of one."));

    AddWidget(path, "Fix Void-Out Game Over", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.VoidOutGameOver"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "Prevents a game over when voiding out with zero extra lives, since void-outs don't cost a life."));

    AddWidget(path, "Fix Mumbo Token: GV Water Pyramid", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.MumboTokenGV"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Lowers the mumbo token in the water pyramid to ground level after the "
                                           "water drains, making it reachable."));

    AddWidget(path, "Fix Mumbo Token: MMM Loggo", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.MumboTokenMMM"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Fixes the MMM Inside Loggo token sharing a collection bitfield index with "
                                           "another token, causing one to despawn."));

    AddWidget(path, "Fix Mumbo Token: CCW Spring", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.MumboTokenCCW"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Fixes the CCW Spring token sharing a collection bitfield index with "
                                           "another token, causing one to despawn."));

    AddWidget(path, "Fix Grunty Defeated Flag Placement", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.GruntyDefeatedFlag"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Delays the Grunty Defeated flag until after the Jinjonator attacks, "
                                           "preventing a false win if the player dies before the hit lands."));

    AddWidget(path, "Fix Cutscene Audio Sync", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fix.CutsceneSync"))
        .RaceDisable(false)
        .Options(
            CheckboxOptions().Tooltip("Compensates for N64 frame stutters during cutscenes so audio stays in sync."));

    AddWidget(path, "Fix Widescreen Camera", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fix.WidescreenCamera"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Adjusts static camera angles in widescreen to prevent skybox "
                                           "exposure at the edges of the screen."));

    // Enhancements -> Restorations
    path = { "Enhancements", "Restorations", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Restore Return to Lair", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Restorations.ReturnToLair"))
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(CheckboxOptions().Tooltip("Restores the unused Return to Lair option when in Worlds."));

    // Enhancements -> Gameplay
    path = { "Enhancements", "Gameplay", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Stop N' Swop at 100%", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Gameplay.StopNSwop100"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Unlocks all Stop N' Swop items when loading a 100% save file."));

    // TODO: All Honeycomb Extensions allows 9 honeycomb health bars instead of the 8 cap,
    // but in 4:3 mode they overlap with the notes sprite in HUD
    AddWidget(path, "All Honeycomb Extensions", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("AllHoneycombExtensions"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "Removes the 3-extension health cap, allowing all 24 honeycombs to grant health bars."));

    AddWidget(path, "Extra Time For GV Water Pyramid", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Gameplay.WaterPyramidTimer"))
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(CheckboxOptions().Tooltip("Adds 4 extra seconds to the GV water pyramid hatch timer."));

    AddWidget(path, "Easier Boggy Races", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("EasierBoggyRaces"))
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(CheckboxOptions().Tooltip("Reduces Boggy's max speed during both sled races in Freezeezy Peak."));

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
