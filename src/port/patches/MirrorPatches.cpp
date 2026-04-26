#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"

#include <libultra/gbi.h>
#include <libultra/gu.h>
#include <libultra/convert.h>

extern "C" {
#include "model.h"
}

static bool sMirrorSceneActive = false;
static bool sMirrorAppliedProjection = false;
static bool sMirrorAtCapture = false;

extern "C" {

int port_mirror_active(void) {
    return CVarGetInteger(CVAR_ENHANCEMENT("Modes.MirroredWorld.State"), 0);
}

void port_mirror_beginScene(void) {
    sMirrorSceneActive = true;
    sMirrorAppliedProjection = false;
}

void port_mirror_endScene(void) {
    sMirrorSceneActive = false;
}

// Apply horizontal mirror via projection scale and invert culling.
// guScale(-1,1,1) reverses triangle winding order, so G_EX_INVERT_CULLING
// tells the LUS interpreter to negate the cross product during culling checks.
void port_viewport_applyMirror(Gfx **gfx, Mtx **mtx) {
    if (port_mirror_active() && sMirrorSceneActive) {
        guScale(*mtx, -1.0f, 1.0f, 1.0f);
        gSPMatrix((*gfx)++, OS_PHYSICAL_TO_K0((*mtx)++), G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);
        gSPSetExtraGeometryMode((*gfx)++, G_EX_INVERT_CULLING);
        sMirrorAppliedProjection = true;
    } else if (sMirrorAppliedProjection) {
        gSPClearExtraGeometryMode((*gfx)++, G_EX_INVERT_CULLING);
        sMirrorAppliedProjection = false;
    }
}

// After scene draw, rebuild the projection without the mirror scale so
// HUD elements (speech bubble, jiggy icon, pause menu) render normally.
void port_mirror_undoProjection(Gfx **gfx, Mtx **mtx) {
    if (sMirrorAppliedProjection) {
        extern void viewport_setRenderViewportAndPerspectiveMatrix(Gfx **, Mtx **);
        viewport_setRenderViewportAndPerspectiveMatrix(gfx, mtx);
        sMirrorAppliedProjection = false;
    }
}

// --- Pause menu capture state tracking ---

void port_mirror_markCapture(void) {
    sMirrorAtCapture = port_mirror_active();
}

int port_mirror_shouldFlipPauseBg(void) {
    return port_mirror_active() != sMirrorAtCapture ? 1 : 0;
}

} // extern "C"
