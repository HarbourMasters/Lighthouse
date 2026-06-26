#include <ship/window/gui/GuiWindow.h>
#include <vector>

#define CVAR_DISPLAY_OVERLAY_MODE "gDisplayOverlay.Mode"
extern const std::vector<const char*> timerDisplayOptions;

class DisplayOverlayWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override;
    void DrawElement() override {};
    void Draw() override;
    void UpdateElement() override{};
};