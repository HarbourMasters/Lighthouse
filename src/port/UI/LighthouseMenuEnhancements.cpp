#include "LighthouseMenu.h"
#include "port/Enhancements/Trackers/DisplayOverlay.h"

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
    AddSidebarEntry("Enhancements", path.sidebarName, 2);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Allow Start to Skip Boot Logos", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cutscenes.SkipBootLogos"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Press Start to skip the Rareware and Nintendo logos on boot."));

    AddWidget(path, "Allow Start to Skip Intro Cutscenes", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cutscenes.StartSkipIntro"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Press Start to Skip Intro Cutscenes."));

    AddWidget(path, "Allow Start to Skip Misc Cutscenes", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cutscenes.SkipMiscCutscenes"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Press Start to skip the Gruntilda's Lair and Game Over cutscenes."));

    AddWidget(path, "Skip Jiggy Dance", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cutscenes.SkipJiggyDance"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Skips the jiggy collection dance, collecting the jiggy immediately. "
                                           "Takes priority over the Tooie Jiggy Animation backport."));

    AddWidget(path, "Skip Clucker Cutscene", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cutscenes.SkipCluckerCutscene"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Skips the cutscene that plays when first defeating a Clucker."));

    // Enhancements -> Graphics
    path = { "Enhancements", "Graphics", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 2);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Disable LOD", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Graphics.DisableLOD"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Forces maximum model detail everywhere."));

    AddWidget(path, "Original Aspect Ratio In Cutscenes", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Graphics.CutsceneAspect"))
        .Options(CheckboxOptions().Tooltip("Forces game to show original aspect ratio during cutscenes to avoid seeing "
                                           "unfinished edges of scene geometry."));

    AddWidget(path, "Extended Draw Distance: %dx", WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("Graphics.DrawDistance"))
        .RaceDisable(false)
        .Options(IntSliderOptions().Min(1).Max(6).DefaultValue(1).ShowButtons(true).Format("").Tooltip(
            "Multiplies the draw distance for objects.\n"
            "Higher values render more but cost performance."));

    // Enhancements -> Camera
    path = { "Enhancements", "Camera", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 2);

    path.column = SECTION_COLUMN_1;

    // General Camera Settings
    AddWidget(path, "Camera Settings", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Invert Camera X", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Camera.InvertX"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Inverts horizontal camera."));

    AddWidget(path, "Invert Camera Y", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Camera.InvertY"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Inverts vertical camera in first-person."));

    path.column = SECTION_COLUMN_2;

    // Free Look Camera
    AddWidget(path, "Free Look Camera", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Free Look (Right Stick)", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Camera.FreeLook.Enabled"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "Use the right stick to freely orbit the camera around Banjo (yaw and pitch). "
            "The camera holds its angle until you use a C-button camera control, which returns "
            "to the normal camera. While enabled, the right stick no longer acts as the C-buttons."));

    AddWidget(path, "Free Look Yaw Sensitivity", WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_ENHANCEMENT("Camera.FreeLook.YawSensitivity"))
        .RaceDisable(false)
        .Options(FloatSliderOptions().Min(0.25f).Max(3.0f).DefaultValue(1.0f).Step(0.1f).Format("%.1f").Tooltip(
            "Horizontal (left/right) free look speed."));

    AddWidget(path, "Free Look Pitch Sensitivity", WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_ENHANCEMENT("Camera.FreeLook.PitchSensitivity"))
        .RaceDisable(false)
        .Options(FloatSliderOptions().Min(0.25f).Max(3.0f).DefaultValue(1.0f).Step(0.1f).Format("%.1f").Tooltip(
            "Vertical (up/down) free look speed."));

    AddWidget(path, "Free Look Invert X", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Camera.FreeLook.InvertX"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Inverts the horizontal free look direction."));

    AddWidget(path, "Free Look Invert Y", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Camera.FreeLook.InvertY"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Inverts the vertical free look direction."));

    AddWidget(path, "Free Look Smoothing", WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_ENHANCEMENT("Camera.FreeLook.SmoothRate"))
        .RaceDisable(false)
        .Options(FloatSliderOptions().Min(8.0f).Max(60.0f).DefaultValue(40.0f).Step(1.0f).Format("%.0f").Tooltip(
            "How quickly the camera settles when sliding along geometry. "
            "Lower is smoother but floatier; higher is snappier but can hitch on walls."));

    AddWidget(path, "Follow Camera", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Camera.Follow"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "The camera will always follow Banjo and Kazooie when available, without requiring the player to hold R"));

    // Enhancements -> Modes
    path = { "Enhancements", "Modes", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 2);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Mirrored World", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Modes.MirroredWorld.Mode"))
        .Callback([](WidgetInfo& info) {
            if (CVarGetInteger(CVAR_ENHANCEMENT("Modes.MirroredWorld.Mode"), 0)) {
                CVarSetInteger(CVAR_ENHANCEMENT("Modes.MirroredWorld.State"), 1);
            } else {
                CVarClear(CVAR_ENHANCEMENT("Modes.MirroredWorld.State"));
            }
        })
        .Options(CheckboxOptions().Tooltip("Mirrors the world horizontally. Inverts left/right controls to match."));

    // Enhancements -> Fixes
    path = { "Enhancements", "Fixes", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 2);
    path.column = SECTION_COLUMN_1;

    // Column 1: Progression

    // Game Over Section
    AddWidget(path, "Game Over", WIDGET_SEPARATOR_TEXT);

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

    AddWidget(path, "Fix Boggy Race Game Over", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.BoggyRaceGameOver"))
        .RaceDisable(false)
        .Options(CheckboxOptions().DefaultValue(true).Tooltip(
            "Losing Boggy's race with no extra lives reloads the race instead of "
            "triggering a game over."));

    // Missable Collectibles Section
    AddWidget(path, "Missable Collectibles", WIDGET_SEPARATOR_TEXT);

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

    // Softlocks Section
    AddWidget(path, "Softlocks", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Fix CCW Flower Replant Softlock", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.CCWFlowerReplant"))
        .RaceDisable(false)
        .Options(CheckboxOptions().DefaultValue(true).Tooltip(
            "Prevents re-planting the CCW Spring flower after it's already planted."));

    // Column 2: Behavior & Presentation
    path.column = SECTION_COLUMN_2;

    // Gameplay Behavior Section
    AddWidget(path, "Gameplay Behavior", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Fix CCW Gnawty Rock (Spring)", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.GnawtySpringRock"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Makes Gnawty's rock indestructible in CCW Spring."));

    AddWidget(path, "Fix Termite Mound Slopes", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.TermiteMoundSlopes"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Makes slopes in the Mumbo's Mountain termite mound slide instantly."));

    AddWidget(path, "Fix Early Claw Swipe During Slide", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.ClawSwipeSlide"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Prevents a claw swipe from triggering mid-slide."));

    AddWidget(path, "Fix Grunty Defeated Flag Placement", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.GruntyDefeatedFlag"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Delays the Grunty Defeated flag until after the Jinjonator attacks, "
                                           "preventing a false win if the player dies before the hit lands."));

    AddWidget(path, "Fix Bouncing Grunty", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.GruntyBounce"))
        .RaceDisable(false)
        .Options(CheckboxOptions().DefaultValue(true).Tooltip(
            "Stops Wonderwing from launching Grunty off her platform in the final battle."));

    // Audio Section
    AddWidget(path, "Audio", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Fix Grunty Jinjo Charge Sound", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.JinjoChargeSound"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Stops the Jinjo charge-up sound the instant it hits Grunty."));

    AddWidget(path, "Fix Cutscene Audio Sync", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fix.CutsceneSync"))
        .RaceDisable(false)
        .Options(
            CheckboxOptions().Tooltip("Compensates for N64 frame stutters during cutscenes so audio stays in sync."));

    // Visual Section
    AddWidget(path, "Visual", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Fix Widescreen Camera", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fix.WidescreenCamera"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Adjusts static camera angles in widescreen to prevent skybox "
                                           "exposure at the edges of the screen."));

    AddWidget(path, "Fix Conga's Name", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Fixes.CongaText"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Corrects a spelling error when meeting Conga as a termite."));

    // Enhancements -> Restorations
    path = { "Enhancements", "Restorations", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 2);
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
    AddSidebarEntry("Enhancements", path.sidebarName, 2);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Difficulty", WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("Gameplay.Difficulty"))
        .RaceDisable(false)
        .Options(ComboboxOptions()
                     .Tooltip("Scales the damage taken from enemies and hazards.")
                     .ComboMap({
                         { 1, "Normal" },
                         { 2, "Hard" },
                         { 3, "Brutal" },
                         { 4, "One-Hit" },
                     })
                     .DefaultIndex(1));

    AddWidget(path, "Skip Spiral Mountain Tutorial", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Gameplay.SkipSMTutorial"))
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(CheckboxOptions().Tooltip(
            "Start in the Lair with all basic moves and the six empty honeycombs collected."));

    AddWidget(path, "Furnace Fun Moves", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Gameplay.FurnaceFunMoves"))
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(CheckboxOptions().Tooltip(
            "Start new files with the moves of an N64 Furnace Fun Moves (FFM) setup file. Egg Firing, "
            "Flight, and Wonderwing stay unlearned so Bottles still teaches them with their free eggs "
            "and feathers. Has no effect on existing files."));

    AddWidget(path, "Stop N' Swop at 100%", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Gameplay.StopNSwop100"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Unlocks all Stop N' Swop items when loading a 100% save file."));

    AddWidget(path, "Disable Snacker Spawn", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Gameplay.DisableSnackerSpawn"))
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(CheckboxOptions().Tooltip(
            "Prevents Snacker the shark from spawning in Treasure Trove Cove and Rusty Bucket Bay."));

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

    AddWidget(path, "Easier Mr Vile", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("EasierMrVile"))
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) {
            if (mLighthouseMenu->disabledMap.at(DISABLE_FOR_ROMHACK).active) {
                info.activeDisables.push_back(DISABLE_FOR_ROMHACK);
            }
        })
        .Options(CheckboxOptions().Tooltip(
            "Reduces Mr Vile's max speed during all three phases of his mini game in Bubblegloop Swamp."));

    // Enhancements -> Tooie Backports
    path = { "Enhancements", "Tooie Backports", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 2);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Tooie Jiggy Animation", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Backports.JiggyAnimation"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "Replaces the jiggy collection dance with a Banjo-Tooie style animation. Has no effect while "
            "Skip Jiggy Dance (under Cutscenes) is on."));

    AddWidget(path, "Honeyback Health Regen", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Backports.Honeyback"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "Backports Banjo-Tooie's Honeyback: once all 24 empty honeycombs are collected, your health "
            "slowly refills one honeycomb at a time after a short pause when you stop taking damage."));

    AddWidget(path, "Fast Swimming", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Backports.FastSwim"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "Hold A+B while underwater to combine Banjo's kick with Kazooie's wing stroke for faster swimming."));

    AddWidget(path, "First-Person Egg Aim", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Backports.EggAim"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "Backports Banjo-Tooie's Egg Aim to fire eggs while in first-person camera view."));

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

    AddWidget(path, "Note Collection Retention", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Gameplay.NoteRetention"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "Notes you've already collected stay collected and don't respawn when you revisit a level. "
            "Collection is always tracked; this toggle controls whether collected notes are skipped on "
            "load. Note-door totals still use the vanilla per-level high score."));

    AddWidget(path, "Jinjo Collection Retention", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Gameplay.JinjoRetention"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "Jinjos you've already collected stay collected across visits instead of resetting each time "
            "you enter a level, so you no longer need all five in one go. Collection is always tracked; "
            "this toggle controls whether collected jinjos are skipped on load and your progress is "
            "restored."));

    // Enhancements -> Cheats
    path = { "Enhancements", "Cheats", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 2);
    path.column = SECTION_COLUMN_1;

    // Column 1: Stats & Consumables

    // Stats Section
    AddWidget(path, "Player Stats", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Infinite Health", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.InfiniteHealth"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Prevents health from decreasing."));

    AddWidget(path, "Infinite Air", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.InfiniteAir"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Prevents air from decreasing while underwater."));

    AddWidget(path, "Infinite Lives", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.InfiniteLives"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Keeps lives at 9 (max displayable)."));

    // Consumables Section
    AddWidget(path, "Consumables", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Infinite Eggs", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.InfiniteEggs"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Refills eggs to your current max capacity when below max."));

    AddWidget(path, "Infinite Red Feathers", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.InfiniteRedFeathers"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Refills red feathers to your current max capacity when below max."));

    AddWidget(path, "Infinite Gold Feathers", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.InfiniteGoldFeathers"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Refills gold feathers to your current max capacity when below max."));

    AddWidget(path, "Infinite Boots & Sneakers", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.InfiniteBootsSneakers"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Wading Boots and Turbo Talon Sneakers timers never expire."));

    // Column 2: Movement & Transformations
    path.column = SECTION_COLUMN_2;

    // Movement Section
    AddWidget(path, "Movement", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Hold L to Levitate", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.Levitate"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Hold L to levitate upward with gravity disabled."));

    AddWidget(path, "D-pad Talon Trot Cycling", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.TalonTrotCycle"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "While in Talon Trot: D-pad Right cycles forward (Normal→Boots→Sneakers), D-pad Left cycles backward."));

    // Transformations Section
    AddWidget(path, "Transformations", WIDGET_SEPARATOR_TEXT);

    AddWidget(path, "Fast Transformation", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.FastTransform"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Speeds up Mumbo transformation animation by 3x."));

    AddWidget(path, "D-pad Cycle Transform", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.CycleTransform"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip("Press D-pad Up/Down to cycle through transformation forms.\nUp: Forward "
                                           "(Banjo→Mumbo→...→Wishy→Banjo), Down: Backward."));

    AddWidget(path, "No Mumbo Untransform", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Cheats.NoMumboUntransform"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            "Disables Mumbo untransforming you when going too far and skips his warning dialog."));

    path = { "Enhancements", "Trackers", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, "Gameplay Timer", WIDGET_SEPARATOR_TEXT);
    AddWidget(path, "Time Display", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        int32_t currentIndex = CVarGetInteger(CVAR_DISPLAY_OVERLAY_MODE, TIMER_DISPLAY_NONE);
        const char* widgetLabel = timerDisplayOptions[currentIndex];

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        UIWidgets::PushStyleCombobox(WIDGET_COLOR);
        if (ImGui::BeginCombo("##gameplayTimerMode", widgetLabel)) {
            for (int i = 0; i < timerDisplayOptions.size(); i++) {
                const bool isSelected = (currentIndex == i);

                if (ImGui::Selectable(timerDisplayOptions[i], isSelected)) {
                    CVarSetInteger(CVAR_DISPLAY_OVERLAY_MODE, i);
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        UIWidgets::PopStyleCombobox();
    });
    AddWidget(path, "Hide Window Background", WIDGET_CVAR_CHECKBOX)
        .CVar("gDisplayOverlay.Background")
        .Options(CheckboxOptions().Tooltip("Hides the background of the Display Overlay window."));
    AddWidget(path, "Scale: %.1fx", WIDGET_CVAR_SLIDER_FLOAT)
        .CVar("gDisplayOverlay.Scale")
        .Options(FloatSliderOptions()
                     .Tooltip("Adjust the Scale for the Display Overlay window.")
                     .Min(1.0f)
                     .Max(5.0f)
                     .DefaultValue(1.0f)
                     .Format("%.1f")
                     .Step(0.1f));
}

} // namespace LighthouseGui
