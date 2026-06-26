#pragma once
#include <libultraship/libultraship.h>

class GameplayToolsWindow : public Ship::GuiWindow {
public:
    using Ship::GuiWindow::GuiWindow;

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override{};
};