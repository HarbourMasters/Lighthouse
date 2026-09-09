#include "InputViewer.h"

#define CVAR_INPUT_VIEWER(var) "gInputViewer." var

namespace Prefs::Settings {

namespace InputViewer {

namespace {

// Function-local statics, not globals: the prefs below are globals that borrow these by reference.
const std::map<int32_t, Prefs::EnumEntry>& ButtonOutlineEntries() {
    static const std::map<int32_t, Prefs::EnumEntry> entries = {
        { BUTTON_OUTLINE_ALWAYS_SHOWN, { "always_shown", "Always Shown" } },
        { BUTTON_OUTLINE_NOT_PRESSED, { "not_pressed", "Shown Only While Not Pressed" } },
        { BUTTON_OUTLINE_PRESSED, { "pressed", "Shown Only While Pressed" } },
        { BUTTON_OUTLINE_ALWAYS_HIDDEN, { "always_hidden", "Always Hidden" } },
    };
    return entries;
}

// Same keys and wire names, longer labels: the per-button combos are unlabelled.
const std::map<int32_t, Prefs::EnumEntry>& ButtonOutlineEntriesVerbose() {
    static const std::map<int32_t, Prefs::EnumEntry> entries = {
        { BUTTON_OUTLINE_ALWAYS_SHOWN, { "always_shown", "Outline Always Shown" } },
        { BUTTON_OUTLINE_NOT_PRESSED, { "not_pressed", "Outline Shown Only While Not Pressed" } },
        { BUTTON_OUTLINE_PRESSED, { "pressed", "Outline Shown Only While Pressed" } },
        { BUTTON_OUTLINE_ALWAYS_HIDDEN, { "always_hidden", "Outline Always Hidden" } },
    };
    return entries;
}

const std::map<int32_t, Prefs::EnumEntry>& StickModeEntries() {
    static const std::map<int32_t, Prefs::EnumEntry> entries = {
        { STICK_MODE_ALWAYS_SHOWN, { "always", "Always" } },
        { STICK_MODE_HIDDEN_IN_DEADZONE, { "while_in_use", "While In Use" } },
        { STICK_MODE_ALWAYS_HIDDEN, { "never", "Never" } },
    };
    return entries;
}

} // namespace

// Hundredths: 0.10 to 5.00.
Prefs::Fixed Scale{ PrefSection::SECTION_SETTINGS, "Scale", 100, 100,
                    Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("Scale")).Min(10).Max(500) };
Prefs::Bool EnableDragging{ PrefSection::SECTION_SETTINGS, "EnableDragging", true,
                            Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("EnableDragging")) };
Prefs::Bool ShowBackground{ PrefSection::SECTION_SETTINGS, "ShowBackground", true,
                            Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("ShowBackground")) };

namespace Buttons {

Prefs::Enum OutlineMode{ PrefSection::SECTION_SETTINGS, "ButtonOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                         ButtonOutlineEntries(),
                         Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("ButtonOutlineMode")) };
Prefs::Bool UseGlobalOutlineMode{ PrefSection::SECTION_SETTINGS, "UseGlobalButtonOutlineMode", true,
                                  Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("UseGlobalButtonOutlineMode")) };

Prefs::Bool A{ PrefSection::SECTION_SETTINGS, "ABtn", true, Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("ABtn")) };
Prefs::Enum AOutlineMode{ PrefSection::SECTION_SETTINGS, "ABtnOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                          ButtonOutlineEntriesVerbose(),
                          Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("ABtnOutlineMode")) };

Prefs::Bool B{ PrefSection::SECTION_SETTINGS, "BBtn", true, Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("BBtn")) };
Prefs::Enum BOutlineMode{ PrefSection::SECTION_SETTINGS, "BBtnOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                          ButtonOutlineEntriesVerbose(),
                          Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("BBtnOutlineMode")) };

Prefs::Bool CUp{ PrefSection::SECTION_SETTINGS, "CUp", true, Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("CUp")) };
Prefs::Enum CUpOutlineMode{ PrefSection::SECTION_SETTINGS, "CUpOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                            ButtonOutlineEntriesVerbose(),
                            Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("CUpOutlineMode")) };

Prefs::Bool CRight{ PrefSection::SECTION_SETTINGS, "CRight", true,
                    Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("CRight")) };
Prefs::Enum CRightOutlineMode{ PrefSection::SECTION_SETTINGS, "CRightOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                               ButtonOutlineEntriesVerbose(),
                               Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("CRightOutlineMode")) };

Prefs::Bool CDown{ PrefSection::SECTION_SETTINGS, "CDown", true,
                   Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("CDown")) };
Prefs::Enum CDownOutlineMode{ PrefSection::SECTION_SETTINGS, "CDownOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                              ButtonOutlineEntriesVerbose(),
                              Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("CDownOutlineMode")) };

Prefs::Bool CLeft{ PrefSection::SECTION_SETTINGS, "CLeft", true,
                   Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("CLeft")) };
Prefs::Enum CLeftOutlineMode{ PrefSection::SECTION_SETTINGS, "CLeftOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                              ButtonOutlineEntriesVerbose(),
                              Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("CLeftOutlineMode")) };

Prefs::Bool L{ PrefSection::SECTION_SETTINGS, "LBtn", true, Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("LBtn")) };
Prefs::Enum LOutlineMode{ PrefSection::SECTION_SETTINGS, "LBtnOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                          ButtonOutlineEntriesVerbose(),
                          Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("LBtnOutlineMode")) };

Prefs::Bool R{ PrefSection::SECTION_SETTINGS, "RBtn", true, Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("RBtn")) };
Prefs::Enum ROutlineMode{ PrefSection::SECTION_SETTINGS, "RBtnOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                          ButtonOutlineEntriesVerbose(),
                          Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("RBtnOutlineMode")) };

Prefs::Bool Z{ PrefSection::SECTION_SETTINGS, "ZBtn", true, Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("ZBtn")) };
Prefs::Enum ZOutlineMode{ PrefSection::SECTION_SETTINGS, "ZBtnOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                          ButtonOutlineEntriesVerbose(),
                          Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("ZBtnOutlineMode")) };

Prefs::Bool Start{ PrefSection::SECTION_SETTINGS, "StartBtn", true,
                   Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("StartBtn")) };
Prefs::Enum StartOutlineMode{ PrefSection::SECTION_SETTINGS, "StartBtnOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                              ButtonOutlineEntriesVerbose(),
                              Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("StartBtnOutlineMode")) };

Prefs::Bool Dpad{ PrefSection::SECTION_SETTINGS, "Dpad", false, Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("Dpad")) };
Prefs::Enum DpadOutlineMode{ PrefSection::SECTION_SETTINGS, "DpadOutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                             ButtonOutlineEntriesVerbose(),
                             Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("DpadOutlineMode")) };

Prefs::Bool Mod1{ PrefSection::SECTION_SETTINGS, "Mod1", false, Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("Mod1")) };
Prefs::Enum Mod1OutlineMode{ PrefSection::SECTION_SETTINGS, "Mod1OutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                             ButtonOutlineEntriesVerbose(),
                             Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("Mod1OutlineMode")) };

Prefs::Bool Mod2{ PrefSection::SECTION_SETTINGS, "Mod2", false, Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("Mod2")) };
Prefs::Enum Mod2OutlineMode{ PrefSection::SECTION_SETTINGS, "Mod2OutlineMode", BUTTON_OUTLINE_NOT_PRESSED,
                             ButtonOutlineEntriesVerbose(),
                             Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("Mod2OutlineMode")) };

} // namespace Buttons

namespace AnalogStick {
Prefs::Fixed Movement{ PrefSection::SECTION_SETTINGS, "AnalogStick.Movement", 12, 1,
                       Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogStick.Movement")).Min(0).Max(200) };
Prefs::Enum VisibilityMode{ PrefSection::SECTION_SETTINGS, "AnalogStick.VisibilityMode", STICK_MODE_ALWAYS_SHOWN,
                            StickModeEntries(),
                            Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogStick.VisibilityMode")) };
Prefs::Enum OutlineMode{ PrefSection::SECTION_SETTINGS, "AnalogStick.OutlineMode", STICK_MODE_ALWAYS_SHOWN,
                         StickModeEntries(),
                         Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogStick.OutlineMode")) };

} // namespace AnalogStick

namespace AnalogAngles {
Prefs::Bool Enabled{ PrefSection::SECTION_SETTINGS, "AnalogAngles.Enabled", false,
                     Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("AnalogAngles.Enabled")) };
Prefs::Fixed Offset{ PrefSection::SECTION_SETTINGS, "AnalogAngles.Offset", 7, 1,
                     Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogAngles.Offset")).Min(0).Max(200) };
// Hundredths, drawn as a percentage.
Prefs::Fixed Scale{ PrefSection::SECTION_SETTINGS, "AnalogAngles.Scale", 100, 100,
                    Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogAngles.Scale")).Min(10).Max(500) };
Prefs::Color TextColor{ PrefSection::SECTION_SETTINGS, "AnalogAngles.TextColor", { 255, 255, 255, 255 },
                        Prefs::Options<Prefs::ColorValue>().CVar(CVAR_INPUT_VIEWER("AnalogAngles.TextColor")) };
} // namespace AnalogAngles

namespace RightStick {
Prefs::Fixed Movement{ PrefSection::SECTION_SETTINGS, "RightStick.Movement", 0, 1,
                       Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("RightStick.Movement")).Min(0).Max(400) };
Prefs::Enum VisibilityMode{ PrefSection::SECTION_SETTINGS, "RightStick.VisibilityMode", STICK_MODE_ALWAYS_HIDDEN,
                            StickModeEntries(),
                            Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("RightStick.VisibilityMode")) };
Prefs::Enum OutlineMode{ PrefSection::SECTION_SETTINGS, "RightStick.OutlineMode", STICK_MODE_ALWAYS_HIDDEN,
                         StickModeEntries(),
                         Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("RightStick.OutlineMode")) };
} // namespace RightStick

} // namespace InputViewer

} // namespace Prefs::Settings
