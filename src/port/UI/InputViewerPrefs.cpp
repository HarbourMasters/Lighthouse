#include "InputViewer.h"

#define CVAR_INPUT_VIEWER(var) "gInputViewer." var

namespace Prefs::Settings {

namespace InputViewer {

static const std::map<int32_t, Prefs::EnumEntry> buttonOutlineEntries = {
    { BUTTON_OUTLINE_ALWAYS_SHOWN, { "always_shown", "Always Shown" } },
    { BUTTON_OUTLINE_NOT_PRESSED, { "not_pressed", "Shown Only While Not Pressed" } },
    { BUTTON_OUTLINE_PRESSED, { "pressed", "Shown Only While Pressed" } },
    { BUTTON_OUTLINE_ALWAYS_HIDDEN, { "always_hidden", "Always Hidden" } },
};

static const std::map<int32_t, Prefs::EnumEntry> buttonOutlineEntriesVerbose = {
    { BUTTON_OUTLINE_ALWAYS_SHOWN, { "always_shown", "Outline Always Shown" } },
    { BUTTON_OUTLINE_NOT_PRESSED, { "not_pressed", "Outline Shown Only While Not Pressed" } },
    { BUTTON_OUTLINE_PRESSED, { "pressed", "Outline Shown Only While Pressed" } },
    { BUTTON_OUTLINE_ALWAYS_HIDDEN, { "always_hidden", "Outline Always Hidden" } },
};

static const std::map<int32_t, Prefs::EnumEntry> stickModeEntries = {
    { STICK_MODE_ALWAYS_SHOWN, { "always", "Always" } },
    { STICK_MODE_HIDDEN_IN_DEADZONE, { "while_in_use", "While In Use" } },
    { STICK_MODE_ALWAYS_HIDDEN, { "never", "Never" } },
};

// Hundredths: 0.10 to 5.00.
Prefs::Fixed Scale{ PrefSection::SECTION_SETTINGS, "InputViewer.Scale", 100, 100,
                    Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("Scale")).Min(10).Max(500) };
Prefs::Bool EnableDragging{ PrefSection::SECTION_SETTINGS, "InputViewer.EnableDragging", true,
                            Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("EnableDragging")) };
Prefs::Bool ShowBackground{ PrefSection::SECTION_SETTINGS, "InputViewer.ShowBackground", true,
                            Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("ShowBackground")) };

namespace Buttons {
std::string confPath = "InputViewer.Buttons.";
Prefs::Enum OutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "OutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                         buttonOutlineEntries,
                         Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("ButtonOutlineMode")) };
Prefs::Bool UseGlobalOutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "UseGlobalOutlineMode", true,
                                  Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("UseGlobalButtonOutlineMode")) };

Prefs::Bool A{ PrefSection::SECTION_SETTINGS, confPath + "A.Show", true,
               Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("ABtn")) };
Prefs::Enum AOutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "ABtnOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                          buttonOutlineEntriesVerbose,
                          Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("ABtnOutlineMode")) };

Prefs::Bool B{ PrefSection::SECTION_SETTINGS, confPath + "B.Show", true,
               Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("BBtn")) };
Prefs::Enum BOutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "BBtnOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                          buttonOutlineEntriesVerbose,
                          Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("BBtnOutlineMode")) };

Prefs::Bool CUp{ PrefSection::SECTION_SETTINGS, confPath + "CUp.Show", true,
                 Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("CUp")) };
Prefs::Enum CUpOutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "CUpOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                            buttonOutlineEntriesVerbose,
                            Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("CUpOutlineMode")) };

Prefs::Bool CRight{ PrefSection::SECTION_SETTINGS, confPath + "CRight.Show", true,
                    Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("CRight")) };
Prefs::Enum CRightOutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "CRightOutlineMode",
                               BUTTON_OUTLINE_NOT_PRESSED, buttonOutlineEntriesVerbose,
                               Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("CRightOutlineMode")) };

Prefs::Bool CDown{ PrefSection::SECTION_SETTINGS, confPath + "CDown.Show", true,
                   Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("CDown")) };
Prefs::Enum CDownOutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "CDownOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                              buttonOutlineEntriesVerbose,
                              Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("CDownOutlineMode")) };

Prefs::Bool CLeft{ PrefSection::SECTION_SETTINGS, confPath + "CLeft.Show", true,
                   Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("CLeft")) };
Prefs::Enum CLeftOutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "CLeft.OutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                              buttonOutlineEntriesVerbose,
                              Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("CLeftOutlineMode")) };

Prefs::Bool L{ PrefSection::SECTION_SETTINGS, confPath + "L.Show", true,
               Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("LBtn")) };
Prefs::Enum LOutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "L.OutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                          buttonOutlineEntriesVerbose,
                          Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("LBtnOutlineMode")) };

Prefs::Bool R{ PrefSection::SECTION_SETTINGS, confPath + "R.Show", true,
               Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("RBtn")) };
Prefs::Enum ROutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "R.OutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                          buttonOutlineEntriesVerbose,
                          Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("RBtnOutlineMode")) };

Prefs::Bool Z{ PrefSection::SECTION_SETTINGS, confPath + "Z.Show", true,
               Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("ZBtn")) };
Prefs::Enum ZOutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "Z.OutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                          buttonOutlineEntriesVerbose,
                          Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("ZBtnOutlineMode")) };

Prefs::Bool Start{ PrefSection::SECTION_SETTINGS, confPath + "Start.Show", true,
                   Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("StartBtn")) };
Prefs::Enum StartOutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "Start.OutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                              buttonOutlineEntriesVerbose,
                              Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("StartBtnOutlineMode")) };

Prefs::Bool Dpad{ PrefSection::SECTION_SETTINGS, confPath + "Dpad.Show", false,
                  Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("Dpad")) };
Prefs::Enum DpadOutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "Dpad.OutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                             buttonOutlineEntriesVerbose,
                             Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("DpadOutlineMode")) };

Prefs::Bool Mod1{ PrefSection::SECTION_SETTINGS, confPath + "Mod1.Show", false,
                  Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("Mod1")) };
Prefs::Enum Mod1OutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "Mod1.OutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                             buttonOutlineEntriesVerbose,
                             Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("Mod1OutlineMode")) };

Prefs::Bool Mod2{ PrefSection::SECTION_SETTINGS, confPath + "Mod2.Show", false,
                  Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("Mod2")) };
Prefs::Enum Mod2OutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "Mod2.OutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                             buttonOutlineEntriesVerbose,
                             Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("Mod2OutlineMode")) };

} // namespace Buttons

namespace AnalogStick {
std::string confPath = "InputViewer.AnalogStick.";
Prefs::Fixed Movement{ PrefSection::SECTION_SETTINGS, confPath + "Movement", 12, 1,
                       Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogStick.Movement")).Min(0).Max(200) };
Prefs::Enum VisibilityMode{ PrefSection::SECTION_SETTINGS, confPath + "VisibilityMode", STICK_MODE_ALWAYS_SHOWN,
                            stickModeEntries,
                            Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogStick.VisibilityMode")) };
Prefs::Enum OutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "OutlineMode", STICK_MODE_ALWAYS_SHOWN,
                         stickModeEntries,
                         Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogStick.OutlineMode")) };

} // namespace AnalogStick

namespace AnalogAngles {
std::string confPath = "InputViewer.AnalogAngles.";
Prefs::Bool Enabled{ PrefSection::SECTION_SETTINGS, confPath + "Enabled", false,
                     Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("AnalogAngles.Enabled")) };
Prefs::Fixed Offset{ PrefSection::SECTION_SETTINGS, confPath + "Offset", 7, 1,
                     Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogAngles.Offset")).Min(0).Max(200) };
// Hundredths, drawn as a percentage.
Prefs::Fixed Scale{ PrefSection::SECTION_SETTINGS, confPath + "Scale", 100, 100,
                    Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogAngles.Scale")).Min(10).Max(500) };
Prefs::Color TextColor{ PrefSection::SECTION_SETTINGS,
                        confPath + "TextColor",
                        { 255, 255, 255, 255 },
                        Prefs::Options<Prefs::ColorValue>().CVar(CVAR_INPUT_VIEWER("AnalogAngles.TextColor")) };
} // namespace AnalogAngles

namespace RightStick {
std::string confPath = "InputViewer.RightStick.";
Prefs::Fixed Movement{ PrefSection::SECTION_SETTINGS, confPath + "Movement", 0, 1,
                       Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("RightStick.Movement")).Min(0).Max(400) };
Prefs::Enum VisibilityMode{ PrefSection::SECTION_SETTINGS, confPath + "VisibilityMode", STICK_MODE_ALWAYS_HIDDEN,
                            stickModeEntries,
                            Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("RightStick.VisibilityMode")) };
Prefs::Enum OutlineMode{ PrefSection::SECTION_SETTINGS, confPath + "OutlineMode", STICK_MODE_ALWAYS_HIDDEN,
                         stickModeEntries,
                         Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("RightStick.OutlineMode")) };
} // namespace RightStick

} // namespace InputViewer

} // namespace Prefs::Settings
