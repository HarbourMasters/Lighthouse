#pragma once

// Camera-area occlusion debugger.
//
// Listens on OnGeoCull (fired per conditional cull command in the map model) to enumerate
// every culled geometry chunk in the current map, lets you force-draw individual ones live,
// and dumps the chosen set so it can be baked into the draw-distance enhancement. All wiring
// is internal — render.c only fires OnGeoCull via port_geoCullDraw (see GeoCull.h).

#include <libultraship/libultraship.h>

class OcclusionDebugWindow final : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override {
    }
    void DrawElement() override;
    void UpdateElement() override {
    }
};
