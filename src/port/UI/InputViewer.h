#pragma once

#include <libultraship/libultraship.h>

#define CVAR_INPUT_VIEWER(var) "gInputViewer." var

typedef enum {
    BUTTON_OUTLINE_ALWAYS_SHOWN,
    BUTTON_OUTLINE_NOT_PRESSED,
    BUTTON_OUTLINE_PRESSED,
    BUTTON_OUTLINE_ALWAYS_HIDDEN
} ButtonOutlineMode;

typedef enum {
    STICK_MODE_ALWAYS_SHOWN,
    STICK_MODE_HIDDEN_IN_DEADZONE,
    STICK_MODE_ALWAYS_HIDDEN,
} StickMode;

class InputViewer final : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void Draw() override;
    void InitElement() override{};
    void DrawElement() override;
    void UpdateElement() override{};

    InputViewer();
    ~InputViewer();

private:
    void RenderButton(std::string btn, std::string btnOutline, int state, ImVec2 size, int outlineMode);
};

class InputViewerSettingsWindow final : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override;
    void UpdateElement() override{};

    InputViewerSettingsWindow();
    ~InputViewerSettingsWindow();
};

// Prefs
#include "port/Prefs/Pref.h"

namespace Prefs::Settings {

namespace InputViewer {
extern Prefs::Fixed Scale;
extern Prefs::Bool EnableDragging;
extern Prefs::Bool ShowBackground;

namespace Buttons {
extern Prefs::Enum OutlineMode;
extern Prefs::Bool UseGlobalOutlineMode;

extern Prefs::Bool A;
extern Prefs::Enum AOutlineMode;
extern Prefs::Bool B;
extern Prefs::Enum BOutlineMode;
extern Prefs::Bool CUp;
extern Prefs::Enum CUpOutlineMode;
extern Prefs::Bool CRight;
extern Prefs::Enum CRightOutlineMode;
extern Prefs::Bool CDown;
extern Prefs::Enum CDownOutlineMode;
extern Prefs::Bool CLeft;
extern Prefs::Enum CLeftOutlineMode;
extern Prefs::Bool L;
extern Prefs::Enum LOutlineMode;
extern Prefs::Bool R;
extern Prefs::Enum ROutlineMode;
extern Prefs::Bool Z;
extern Prefs::Enum ZOutlineMode;
extern Prefs::Bool Start;
extern Prefs::Enum StartOutlineMode;
extern Prefs::Bool Dpad;
extern Prefs::Enum DpadOutlineMode;
extern Prefs::Bool Mod1;
extern Prefs::Enum Mod1OutlineMode;
extern Prefs::Bool Mod2;
extern Prefs::Enum Mod2OutlineMode;
} // namespace Buttons

namespace AnalogAngles {
extern Prefs::Bool Enabled;
extern Prefs::Fixed Offset;
extern Prefs::Fixed Scale;
extern Prefs::Color TextColor;
} // namespace AnalogAngles

namespace AnalogStick {
extern Prefs::Fixed Movement;
extern Prefs::Enum VisibilityMode;
extern Prefs::Enum OutlineMode;
} // namespace AnalogStick

namespace RightStick {
extern Prefs::Fixed Movement;
extern Prefs::Enum VisibilityMode;
extern Prefs::Enum OutlineMode;
} // namespace RightStick

} // namespace InputViewer

} // namespace Prefs::Settings