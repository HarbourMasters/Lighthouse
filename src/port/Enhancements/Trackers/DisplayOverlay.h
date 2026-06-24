#include <ship/window/gui/GuiWindow.h>

#define CVAR_DISPLAY_OVERLAY_MODE "gDisplayOverlay.Mode"

class DisplayOverlayWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override{};
};