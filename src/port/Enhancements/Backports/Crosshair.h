#pragma once

#ifdef __cplusplus
#include <ship/window/gui/GuiWindow.h>

namespace Crosshair {
class Window final : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;
    void InitElement() override{};
    void DrawElement() override{};
    void Draw() override;
    void UpdateElement() override{};
};
void ShowThisFrame();
} // namespace Crosshair

#endif // __cplusplus
