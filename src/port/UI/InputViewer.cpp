#include "InputViewer.h"

#include <libultraship/bridge/consolevariablebridge.h>
#include <libultraship/libultra/controller.h>
#include <ship/Context.h>
#include <fast/Fast3dGui.h>
#include <libultraship/controller/controldeck/ControlDeck.h>
#include "port/UI/cvar_prefixes.h"
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <cmath>

#include "UIWidgets.hpp"
#include "LighthouseGui.hpp"

using namespace UIWidgets;

static std::shared_ptr<Fast::Fast3dGui> GetFast3dGui() {
    return std::static_pointer_cast<Fast::Fast3dGui>(Ship::Context::GetRawInstance()->GetWindow()->GetGui());
}

InputViewer::~InputViewer() {
    SPDLOG_TRACE("destruct input viewer");
}

void InputViewer::RenderButton(std::string btnTexture, std::string btnOutlineTexture, int state, ImVec2 size,
                               int outlineMode) {
    const ImVec2 pos = ImGui::GetCursorPos();
    ImGui::SetNextItemAllowOverlap();
    // Render Outline based on settings
    if (outlineMode == BUTTON_OUTLINE_ALWAYS_SHOWN || (outlineMode == BUTTON_OUTLINE_NOT_PRESSED && !state) ||
        (outlineMode == BUTTON_OUTLINE_PRESSED && state)) {
        ImGui::Image(GetFast3dGui()->GetTextureByName(btnOutlineTexture), size, ImVec2(0, 0), ImVec2(1.0f, 1.0f));
    }
    // Render button if pressed
    if (state) {
        ImGui::SetCursorPos(pos);
        ImGui::SetNextItemAllowOverlap();
        ImGui::Image(GetFast3dGui()->GetTextureByName(btnTexture), size, ImVec2(0, 0), ImVec2(1.0f, 1.0f));
    }
}

void InputViewer::Draw() {
    if (!IsVisible()) {
        return;
    }
    DrawElement();
    // Sync up the IsVisible flag if it was changed by ImGui
    SyncVisibilityConsoleVariable();
}

void InputViewer::DrawElement() {
    static bool sButtonTexturesLoaded = false;
    if (!sButtonTexturesLoaded) {
        GetFast3dGui()->LoadTextureFromRawImage("Input-Viewer-Background",
                                                "textures/buttons/InputViewerBackground.png");
        GetFast3dGui()->LoadTextureFromRawImage("A-Btn", "textures/buttons/ABtn.png");
        GetFast3dGui()->LoadTextureFromRawImage("B-Btn", "textures/buttons/BBtn.png");
        GetFast3dGui()->LoadTextureFromRawImage("L-Btn", "textures/buttons/LBtn.png");
        GetFast3dGui()->LoadTextureFromRawImage("R-Btn", "textures/buttons/RBtn.png");
        GetFast3dGui()->LoadTextureFromRawImage("Z-Btn", "textures/buttons/ZBtn.png");
        GetFast3dGui()->LoadTextureFromRawImage("Start-Btn", "textures/buttons/StartBtn.png");
        GetFast3dGui()->LoadTextureFromRawImage("C-Left", "textures/buttons/CLeft.png");
        GetFast3dGui()->LoadTextureFromRawImage("C-Right", "textures/buttons/CRight.png");
        GetFast3dGui()->LoadTextureFromRawImage("C-Up", "textures/buttons/CUp.png");
        GetFast3dGui()->LoadTextureFromRawImage("C-Down", "textures/buttons/CDown.png");
        GetFast3dGui()->LoadTextureFromRawImage("Analog-Stick", "textures/buttons/AnalogStick.png");
        GetFast3dGui()->LoadTextureFromRawImage("Dpad-Left", "textures/buttons/DPadLeft.png");
        GetFast3dGui()->LoadTextureFromRawImage("Dpad-Right", "textures/buttons/DPadRight.png");
        GetFast3dGui()->LoadTextureFromRawImage("Dpad-Up", "textures/buttons/DPadUp.png");
        GetFast3dGui()->LoadTextureFromRawImage("Dpad-Down", "textures/buttons/DPadDown.png");
        GetFast3dGui()->LoadTextureFromRawImage("Modifier-1", "textures/buttons/Mod1.png");
        GetFast3dGui()->LoadTextureFromRawImage("Modifier-2", "textures/buttons/Mod2.png");
        GetFast3dGui()->LoadTextureFromRawImage("Right-Stick", "textures/buttons/RightStick.png");
        GetFast3dGui()->LoadTextureFromRawImage("A-Btn Outline", "textures/buttons/ABtnOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("B-Btn Outline", "textures/buttons/BBtnOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("L-Btn Outline", "textures/buttons/LBtnOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("R-Btn Outline", "textures/buttons/RBtnOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("Z-Btn Outline", "textures/buttons/ZBtnOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("Start-Btn Outline", "textures/buttons/StartBtnOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("C-Left Outline", "textures/buttons/CLeftOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("C-Right Outline", "textures/buttons/CRightOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("C-Up Outline", "textures/buttons/CUpOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("C-Down Outline", "textures/buttons/CDownOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("Analog-Stick Outline", "textures/buttons/AnalogStickOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("Dpad-Left Outline", "textures/buttons/DPadLeftOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("Dpad-Right Outline", "textures/buttons/DPadRightOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("Dpad-Up Outline", "textures/buttons/DPadUpOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("Dpad-Down Outline", "textures/buttons/DPadDownOutline.png");
        GetFast3dGui()->LoadTextureFromRawImage("Modifier-1 Outline", "textures/buttons/Mod1Outline.png");
        GetFast3dGui()->LoadTextureFromRawImage("Modifier-2 Outline", "textures/buttons/Mod2Outline.png");
        GetFast3dGui()->LoadTextureFromRawImage("Right-Stick Outline", "textures/buttons/RightStickOutline.png");
        sButtonTexturesLoaded = true;
    }

    ImVec2 mainPos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetMainViewport()->WorkSize;

#ifdef __WIIU__
    const float scale = Prefs::Settings::InputViewer::Scale.Float() * 2.0f;
#else
    const float scale = Prefs::Settings::InputViewer::Scale.Float();
#endif
    const int showAnalogAngles = Prefs::Settings::InputViewer::AnalogAngles::Enabled;
    const float angleTextScale = Prefs::Settings::InputViewer::AnalogAngles::Scale.Float();
    const int buttonOutlineMode = Prefs::Settings::InputViewer::Buttons::OutlineMode;
    const bool useGlobalOutlineMode = Prefs::Settings::InputViewer::Buttons::UseGlobalOutlineMode;

    ImVec2 bgSize = GetFast3dGui()->GetTextureSize("Input-Viewer-Background");
    ImVec2 scaledBGSize = ImVec2(bgSize.x * scale, bgSize.y * scale);

    ImGui::SetNextWindowSize(
        ImVec2(scaledBGSize.x + 20,
               scaledBGSize.y + (showAnalogAngles ? ImGui::CalcTextSize("X").y : 0) * scale * angleTextScale + 20));
    ImGui::SetNextWindowContentSize(
        ImVec2(scaledBGSize.x, scaledBGSize.y + (showAnalogAngles ? 15 : 0) * scale * angleTextScale));
    ImGui::SetNextWindowPos(ImVec2(mainPos.x + size.x - scaledBGSize.x - 30, mainPos.y + size.y - scaledBGSize.y - 30),
                            ImGuiCond_FirstUseEver);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    OSContPad* pads =
        std::dynamic_pointer_cast<LUS::ControlDeck>(Ship::Context::GetRawInstance()->GetControlDeck())->GetPads();

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground |
                                   ImGuiWindowFlags_NoFocusOnAppearing;

    if (!Prefs::Settings::InputViewer::EnableDragging) {
        windowFlags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove;
    }

    if (pads != nullptr && ImGui::Begin("Input Viewer", nullptr, windowFlags)) {
        ImGui::SetCursorPos(ImVec2(10, 10));
        const ImVec2 aPos = ImGui::GetCursorPos();

        if (Prefs::Settings::InputViewer::ShowBackground) {
            ImGui::SetNextItemAllowOverlap();
            // Background
            ImGui::Image(GetFast3dGui()->GetTextureByName("Input-Viewer-Background"), scaledBGSize, ImVec2(0, 0),
                         ImVec2(1.0f, 1.0f));
        }

        // A/B
        if (Prefs::Settings::InputViewer::Buttons::B) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("B-Btn", "B-Btn Outline", pads[0].button & BTN_B, scaledBGSize,
                         useGlobalOutlineMode ? buttonOutlineMode
                                              : Prefs::Settings::InputViewer::Buttons::BOutlineMode.Get());
        }
        if (Prefs::Settings::InputViewer::Buttons::A) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("A-Btn", "A-Btn Outline", pads[0].button & BTN_A, scaledBGSize,
                         useGlobalOutlineMode ? buttonOutlineMode
                                              : Prefs::Settings::InputViewer::Buttons::AOutlineMode.Get());
        }

        // C buttons
        if (Prefs::Settings::InputViewer::Buttons::CUp) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("C-Up", "C-Up Outline", pads[0].button & BTN_CUP, scaledBGSize,
                         useGlobalOutlineMode ? buttonOutlineMode
                                              : Prefs::Settings::InputViewer::Buttons::CUpOutlineMode.Get());
        }
        if (Prefs::Settings::InputViewer::Buttons::CLeft) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("C-Left", "C-Left Outline", pads[0].button & BTN_CLEFT, scaledBGSize,
                         useGlobalOutlineMode ? buttonOutlineMode
                                              : Prefs::Settings::InputViewer::Buttons::CLeftOutlineMode.Get());
        }
        if (Prefs::Settings::InputViewer::Buttons::CRight) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("C-Right", "C-Right Outline", pads[0].button & BTN_CRIGHT, scaledBGSize,
                         useGlobalOutlineMode ? buttonOutlineMode
                                              : Prefs::Settings::InputViewer::Buttons::CRightOutlineMode.Get());
        }
        if (Prefs::Settings::InputViewer::Buttons::CDown) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("C-Down", "C-Down Outline", pads[0].button & BTN_CDOWN, scaledBGSize,
                         useGlobalOutlineMode ? buttonOutlineMode
                                              : Prefs::Settings::InputViewer::Buttons::CDownOutlineMode.Get());
        }

        // L/R/Z
        if (Prefs::Settings::InputViewer::Buttons::L) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("L-Btn", "L-Btn Outline", pads[0].button & BTN_L, scaledBGSize,
                         useGlobalOutlineMode ? buttonOutlineMode
                                              : Prefs::Settings::InputViewer::Buttons::LOutlineMode.Get());
        }
        if (Prefs::Settings::InputViewer::Buttons::R) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("R-Btn", "R-Btn Outline", pads[0].button & BTN_R, scaledBGSize,
                         useGlobalOutlineMode ? buttonOutlineMode
                                              : Prefs::Settings::InputViewer::Buttons::ROutlineMode.Get());
        }
        if (Prefs::Settings::InputViewer::Buttons::Z) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("Z-Btn", "Z-Btn Outline", pads[0].button & BTN_Z, scaledBGSize,
                         useGlobalOutlineMode ? buttonOutlineMode
                                              : Prefs::Settings::InputViewer::Buttons::ZOutlineMode.Get());
        }

        // Start
        if (Prefs::Settings::InputViewer::Buttons::Start) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("Start-Btn", "Start-Btn Outline", pads[0].button & BTN_START, scaledBGSize,
                         useGlobalOutlineMode ? buttonOutlineMode
                                              : Prefs::Settings::InputViewer::Buttons::StartOutlineMode.Get());
        }

        // Dpad
        if (Prefs::Settings::InputViewer::Buttons::Dpad) {
            const int dpadOutlineMode =
                useGlobalOutlineMode ? buttonOutlineMode : Prefs::Settings::InputViewer::Buttons::DpadOutlineMode.Get();
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("Dpad-Left", "Dpad-Left Outline", pads[0].button & BTN_DLEFT, scaledBGSize, dpadOutlineMode);
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("Dpad-Right", "Dpad-Right Outline", pads[0].button & BTN_DRIGHT, scaledBGSize,
                         dpadOutlineMode);
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("Dpad-Up", "Dpad-Up Outline", pads[0].button & BTN_DUP, scaledBGSize, dpadOutlineMode);
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            RenderButton("Dpad-Down", "Dpad-Down Outline", pads[0].button & BTN_DDOWN, scaledBGSize, dpadOutlineMode);
        }

        const bool analogStickIsInDeadzone = !pads[0].stick_x && !pads[0].stick_y;
        const bool rightStickIsInDeadzone = !pads[0].right_stick_x && !pads[0].right_stick_y;

        // Analog Stick
        const int analogOutlineMode = Prefs::Settings::InputViewer::AnalogStick::OutlineMode;
        const int32_t maxStickDistance = Prefs::Settings::InputViewer::AnalogStick::Movement;
        if (analogOutlineMode == STICK_MODE_ALWAYS_SHOWN ||
            (analogOutlineMode == STICK_MODE_HIDDEN_IN_DEADZONE && !analogStickIsInDeadzone)) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            ImGui::Image(GetFast3dGui()->GetTextureByName("Analog-Stick Outline"), scaledBGSize, ImVec2(0, 0),
                         ImVec2(1.0f, 1.0f));
        }
        const int analogStickMode = Prefs::Settings::InputViewer::AnalogStick::VisibilityMode;
        if (analogStickMode == STICK_MODE_ALWAYS_SHOWN ||
            (analogStickMode == STICK_MODE_HIDDEN_IN_DEADZONE && !analogStickIsInDeadzone)) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(
                ImVec2(aPos.x + maxStickDistance * ((float)(pads[0].stick_x) / MAX_AXIS_RANGE) * scale,
                       aPos.y - maxStickDistance * ((float)(pads[0].stick_y) / MAX_AXIS_RANGE) * scale));
            ImGui::Image(GetFast3dGui()->GetTextureByName("Analog-Stick"), scaledBGSize, ImVec2(0, 0),
                         ImVec2(1.0f, 1.0f));
        }

        // Right Stick
        const int32_t maxRightStickDistance = Prefs::Settings::InputViewer::RightStick::Movement;
        const int rightOutlineMode = Prefs::Settings::InputViewer::RightStick::OutlineMode;
        if (rightOutlineMode == STICK_MODE_ALWAYS_SHOWN ||
            (rightOutlineMode == STICK_MODE_HIDDEN_IN_DEADZONE && !rightStickIsInDeadzone)) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(aPos);
            ImGui::Image(GetFast3dGui()->GetTextureByName("Right-Stick Outline"), scaledBGSize, ImVec2(0, 0),
                         ImVec2(1.0f, 1.0f));
        }
        const int rightStickMode = Prefs::Settings::InputViewer::RightStick::VisibilityMode;
        if (rightStickMode == STICK_MODE_ALWAYS_SHOWN ||
            (rightStickMode == STICK_MODE_HIDDEN_IN_DEADZONE && !rightStickIsInDeadzone)) {
            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorPos(
                ImVec2(aPos.x + maxRightStickDistance * ((float)(pads[0].right_stick_x) / MAX_AXIS_RANGE) * scale,
                       aPos.y - maxRightStickDistance * ((float)(pads[0].right_stick_y) / MAX_AXIS_RANGE) * scale));
            ImGui::Image(GetFast3dGui()->GetTextureByName("Right-Stick"), scaledBGSize, ImVec2(0, 0),
                         ImVec2(1.0f, 1.0f));
        }

        // Analog stick angle text
        if (showAnalogAngles) {
            ImGui::SetCursorPos(ImVec2(aPos.x + 10 + Prefs::Settings::InputViewer::AnalogAngles::Offset * scale,
                                       scaledBGSize.y + aPos.y + 10));
            // Scale font with input viewer scale
            float oldFontScale = ImGui::GetFont()->Scale;
            ImGui::GetFont()->Scale *= scale * angleTextScale;
            ImGui::PushFont(ImGui::GetFont());

            // Render text
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  VecFromRGBA8(Prefs::Settings::InputViewer::AnalogAngles::TextColor.Value()));
            ImGui::Text("X: %-3d  Y: %-3d", pads[0].stick_x, pads[0].stick_y);
            // Restore original color
            ImGui::PopStyleColor();
            // Restore original font scale
            ImGui::GetFont()->Scale = oldFontScale;
            ImGui::PopFont();
        }

        ImGui::End();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

InputViewerSettingsWindow::~InputViewerSettingsWindow() {
    SPDLOG_TRACE("destruct input viewer settings window");
}

void InputViewerSettingsWindow::DrawElement() {
    // gInputViewer.Scale
    PrefSlider("Input Viewer Scale: %.0f%%", SliderOptions()
                                                 .Setting(&Prefs::Settings::InputViewer::Scale)
                                                 .Display(SliderDisplay::Percentage)
                                                 .Color(THEME_COLOR)
                                                 .ShowButtons(true)
                                                 .Tooltip("Sets the on screen size of the input viewer"));

    // gInputViewer.EnableDragging
    PrefCheckbox("Enable Dragging",
                 CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::EnableDragging));

    UIWidgets::Separator(true, true);

    // gInputViewer.ShowBackground
    PrefCheckbox("Show Background Layer",
                 CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::ShowBackground));

    UIWidgets::Separator(true, true);

    PushStyleHeader(THEME_COLOR);
    if (ImGui::CollapsingHeader("Buttons")) {

        // gInputViewer.ButtonOutlineMode
        PrefCombobox(
            "Button Outlines/Backgrounds",
            ComboboxOptions({ { .disabled = !Prefs::Settings::InputViewer::Buttons::UseGlobalOutlineMode,
                                .disabledTooltip = "Disabled because Global Button Outline is off" } })
                .Color(THEME_COLOR)
                .Setting(&Prefs::Settings::InputViewer::Buttons::OutlineMode)
                .Tooltip("Sets the desired visibility behavior for the button outline/background layers. Useful for "
                         "custom input viewers."));

        // gInputViewer.UseGlobalButtonOutlineMode
        PrefCheckbox(
            "Use for all buttons",
            CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::UseGlobalOutlineMode));

        UIWidgets::Separator();

        bool useIndividualOutlines = !Prefs::Settings::InputViewer::Buttons::UseGlobalOutlineMode;

        // gInputViewer.ABtn
        PrefCheckbox("Show A-Button Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::A));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::A) {
            ImGui::Indent();
            PrefCombobox(
                "##ABtnOutline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::AOutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.BBtn
        PrefCheckbox("Show B-Button Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::B));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::B) {
            ImGui::Indent();
            PrefCombobox(
                "##BBtnOutline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::BOutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.CUp
        PrefCheckbox("Show C-Up Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::CUp));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::CUp) {
            ImGui::Indent();
            PrefCombobox(
                "##CUpOutline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::CUpOutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.CRight
        PrefCheckbox("Show C-Right Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::CRight));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::CRight) {
            ImGui::Indent();
            PrefCombobox("##CRightOutline", ComboboxOptions()
                                                .Color(THEME_COLOR)
                                                .Setting(&Prefs::Settings::InputViewer::Buttons::CRightOutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.CDown
        PrefCheckbox("Show C-Down Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::CDown));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::CDown) {
            ImGui::Indent();
            PrefCombobox(
                "##CDownOutline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::CDownOutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.CLeft
        PrefCheckbox("Show C-Left Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::CLeft));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::CLeft) {
            ImGui::Indent();
            PrefCombobox(
                "##CLeftOutline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::CLeftOutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.LBtn
        PrefCheckbox("Show L-Button Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::L));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::L) {
            ImGui::Indent();
            PrefCombobox(
                "##LBtnOutline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::LOutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.RBtn
        PrefCheckbox("Show R-Button Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::R));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::R) {
            ImGui::Indent();
            PrefCombobox(
                "##RBtnOutline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::ROutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.ZBtn
        PrefCheckbox("Show Z-Button Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::Z));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::Z) {
            ImGui::Indent();
            PrefCombobox(
                "##ZBtnOutline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::ZOutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.StartBtn
        PrefCheckbox("Show Start Button Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::Start));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::Start) {
            ImGui::Indent();
            PrefCombobox(
                "##StartBtnOutline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::StartOutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.Dpad
        PrefCheckbox("Show D-Pad Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::Dpad));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::Dpad) {
            ImGui::Indent();
            PrefCombobox(
                "##DpadOutline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::DpadOutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.Mod1
        PrefCheckbox("Show Modifier Button 1 Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::Mod1));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::Mod1) {
            ImGui::Indent();
            PrefCombobox(
                "##Mmod1Outline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::Mod1OutlineMode));
            ImGui::Unindent();
        }
        // gInputViewer.Mod2
        PrefCheckbox("Show Modifier Button 2 Layers",
                     CheckboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::Mod2));
        if (useIndividualOutlines && Prefs::Settings::InputViewer::Buttons::Mod2) {
            ImGui::Indent();
            PrefCombobox(
                "##Mod2Outline",
                ComboboxOptions().Color(THEME_COLOR).Setting(&Prefs::Settings::InputViewer::Buttons::Mod2OutlineMode));
            ImGui::Unindent();
        }

        UIWidgets::Separator(true, true);
    }

    if (ImGui::CollapsingHeader("Analog Stick")) {
        // gInputViewer.AnalogStick.VisibilityMode
        PrefCombobox(
            "Analog Stick Visibility",
            ComboboxOptions()
                .Color(THEME_COLOR)
                .Setting(&Prefs::Settings::InputViewer::AnalogStick::VisibilityMode)
                .Tooltip(
                    "Determines the conditions under which the moving layer of the analog stick texture is visible."));

        // gInputViewer.AnalogStick.OutlineMode
        PrefCombobox(
            "Analog Stick Outline/Background Visibility",
            ComboboxOptions()
                .Color(THEME_COLOR)
                .Setting(&Prefs::Settings::InputViewer::AnalogStick::OutlineMode)
                .Tooltip(
                    "Determines the conditions under which the analog stick outline/background texture is visible."));
        // gInputViewer.AnalogStick.Movement
        PrefSlider("Analog Stick Movement: %dpx",
                   SliderOptions()
                       .Setting(&Prefs::Settings::InputViewer::AnalogStick::Movement)
                       .Color(THEME_COLOR)
                       .ShowButtons(true)
                       .Tooltip("Sets the distance to move the analog stick in the input viewer. Useful for custom "
                                "input viewers."));
        UIWidgets::Separator(true, true);
    }

    if (ImGui::CollapsingHeader("Additional (\"Right\") Stick")) {
        // gInputViewer.RightStick.VisibilityMode
        PrefCombobox(
            "Right Stick Visibility",
            ComboboxOptions()
                .Color(THEME_COLOR)
                .Setting(&Prefs::Settings::InputViewer::RightStick::VisibilityMode)
                .Tooltip(
                    "Determines the conditions under which the moving layer of the right stick texture is visible."));

        PrefCombobox(
            "Right Stick Outline/Background Visibility",
            ComboboxOptions()
                .Color(THEME_COLOR)
                .Setting(&Prefs::Settings::InputViewer::RightStick::OutlineMode)
                .Tooltip(
                    "Determines the conditions under which the right stick outline/background texture is visible."));

        PrefSlider(
            "Right Stick Movement: %dpx",
            SliderOptions()
                .Setting(&Prefs::Settings::InputViewer::RightStick::Movement)
                .Color(THEME_COLOR)
                .ShowButtons(true)
                .Tooltip(
                    "Sets the distance to move the right stick in the input viewer. Useful for custom input viewers."));
        UIWidgets::Separator(true, true);
    }

    if (ImGui::CollapsingHeader("Analog Angle Values")) {
        PrefCheckbox("Show Analog Stick Angle Values",
                     CheckboxOptions()
                         .Color(THEME_COLOR)
                         .Tooltip("Displays analog stick angle values in the input viewer")
                         .Setting(&Prefs::Settings::InputViewer::AnalogAngles::Enabled));
        if (Prefs::Settings::InputViewer::AnalogAngles::Enabled) {
            PrefColorPicker("Text Color", ColorPickerOptions()
                                              .Color(THEME_COLOR)
                                              .Setting(&Prefs::Settings::InputViewer::AnalogAngles::TextColor)
                                              .UseAlpha()
                                              .ShowRandom()
                                              .ShowReset());

            PrefSlider("Angle Text Scale: %.0f%%", SliderOptions()
                                                       .Setting(&Prefs::Settings::InputViewer::AnalogAngles::Scale)
                                                       .Color(THEME_COLOR)
                                                       .IsPercentage()
                                                       .ShowButtons(true));

            PrefSlider("Angle Text Offset: %dpx",
                       SliderOptions()
                           .Setting(&Prefs::Settings::InputViewer::AnalogAngles::Offset)
                           .Color(THEME_COLOR)
                           .ShowButtons(true)
                           .Tooltip("Sets the distance to move the right stick in the input viewer. Useful for "
                                    "custom input viewers."));
        }
    }
    PopStyleHeader();
}
