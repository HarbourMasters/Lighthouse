#ifndef RANDO_CHECK_TRACKER_H
#define RANDO_CHECK_TRACKER_H

#include "port/Rando/Rando.h"
#include <ship/window/gui/GuiWindow.h>
#include <nlohmann/json.hpp>

namespace Rando {

namespace CheckTracker {

void Init();
void LoadFromPreset(const nlohmann::json& info);

class CheckTrackerWindow : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override{};
    void Draw() override;
    void UpdateElement() override{};
};

class SettingsWindow : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override;
    void UpdateElement() override{};
};

} // namespace CheckTracker

} // namespace Rando

#endif // RANDO_CHECK_TRACKER_H