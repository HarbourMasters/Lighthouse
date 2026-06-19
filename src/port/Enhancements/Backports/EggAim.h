#pragma once

#ifdef __cplusplus

#include <ship/window/gui/GuiWindow.h>

class EggAimCrosshairWindow final : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override {};
    void DrawElement() override {};
    void Draw() override;
    void UpdateElement() override {};
};

#endif // __cplusplus
