#pragma once

#include "UIWidgets.hpp"
#include "InputViewer.h"
#include "LighthouseModals.h"

namespace LighthouseGui {
void SetupMenu();
void SetupGuiElements();
void Draw();
void Destroy();
void RegisterPopup(std::string title, std::string message, std::string button1 = "OK", std::string button2 = "",
                   std::function<void()> button1callback = nullptr, std::function<void()> button2callback = nullptr);
size_t PopupsQueued();
UIWidgets::Colors GetMenuThemeColor();
} // namespace LighthouseGui

#define THEME_COLOR LighthouseGui::GetMenuThemeColor()
