#pragma once

// Camera-area occlusion debugger.
//
// BK gates chunks of level geometry on which "camera area" box the camera occupies
// (see modelRender_geoCmd_CAMERA). That portal culling is what hides distant landmark
// scenery like the MM stonehenge. This tool enumerates every camera-area command in the
// current map model, lets you force-draw individual ones live, and dumps the chosen set
// so it can be baked into a per-map allowlist.

#ifdef __cplusplus
#include <libultraship/libultraship.h>

class OcclusionDebugWindow final : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override {}
    void DrawElement() override;
    void UpdateElement() override {}
};

extern "C" {
#endif

// Nonzero while the developer has recording/force-draw enabled (cvar-gated). Used as a
// cheap early-out so the per-command hooks cost nothing when the tool is off.
int OcclusionDebug_IsActive(void);

// Called once before each map-model part is drawn so per-part command indices are stable.
// part: 0 = opaque, 1 = translucent. Part 0 (drawn first each frame) also publishes the
// previous frame's recorded list to the UI and starts a fresh capture.
void OcclusionDebug_BeginPart(int part);

// Geo-command kinds that conditionally cull level geometry. Distant scenery can be hidden
// by any of these, so the debugger instruments them all.
#define OCCLUSION_CMD_CAMERA 0 // geoCmd_CAMERA: gated on which camera-area box the camera is in
#define OCCLUSION_CMD_LOD 1    // geoCmd_LOD: gated on distance band [min, max]
#define OCCLUSION_CMD_UNKE 2   // geoCmd_UnkE: gated on a bounding sphere passing the view frustum

// Called from each conditional cull command. `offset` is the command's byte offset within
// the model bin (a stable per-asset key). `drawnVanilla` is the unmodified recurse decision.
// areaIds/areaCount apply to CAMERA only (NULL/0 otherwise); detail0/detail1 are kind-specific
// extras for display (CAMERA: flags; LOD: min,max). Records the command and returns nonzero if
// the developer has toggled it — or "force draw all" — to draw.
int OcclusionDebug_OnCullCmd(int type, int offset, int drawnVanilla, const unsigned char* areaIds, int areaCount,
                             int detail0, int detail1);

#ifdef __cplusplus
}
#endif
