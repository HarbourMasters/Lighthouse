#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"

#include <libultra/gbi.h>
#include <libultra/gu.h>
#include <libultra/convert.h>

#include "functions.h"
extern "C" {
#include "model.h"
#include "prop.h"
}

static bool sMirrorSceneActive = false;
static bool sMirrorAppliedProjection = false;
static bool sMirrorAtCapture = false;
static bool sMirrorExcludeNextModel = false;

// --- Original draw function pointers (saved before patching) ---
typedef Actor* (*ActorDrawFunc)(ActorMarker*, Gfx**, Mtx**, Vtx**);

static ActorDrawFunc sOrig_lair_func_80387560 = nullptr;
static ActorDrawFunc sOrig_actor_draw = nullptr;

extern "C" {

// ActorInfo externs for text-bearing models (must be inside extern "C" for correct linkage)
extern ActorInfo D_80393354;    // level entry signs (model 0x563)
extern ActorInfo chMumboSign5;  // 5 mumbo token sign (0x301)
extern ActorInfo chMumboSign10; // 10 mumbo token sign (0x302)
extern ActorInfo chMumboSign15; // 15 mumbo token sign (0x303)
extern ActorInfo chMumboSign20; // 20 mumbo token sign (0x304)
extern ActorInfo chMumboSign25; // 25 mumbo token sign (0x305)

// Custom draw functions used by text-bearing actors (not in functions.h)
extern Actor* lair_func_80387560(ActorMarker*, Gfx**, Mtx**, Vtx**);

// mlMtx scale function for baking counter-mirror into internal matrix state
extern void mlMtxScale_xyz(f32 x, f32 y, f32 z);

// Animation state in render.c — non-NULL when the current model is animated
extern AnimMtxList* D_8038371C;

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
void port_viewport_applyMirror(Gfx** gfx, Mtx** mtx) {
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
void port_mirror_undoProjection(Gfx** gfx, Mtx** mtx) {
    if (sMirrorAppliedProjection) {
        extern void viewport_setRenderViewportAndPerspectiveMatrix(Gfx**, Mtx**);
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

// --- Per-model counter-mirror for text-bearing objects ---

void port_mirror_setExclude(void) {
    sMirrorExcludeNextModel = true;
}

void port_mirror_clearExclude(void) {
    sMirrorExcludeNextModel = false;
}

// Counter-mirror for text-bearing models. Bakes a horizontal flip into the
// mlMtx state so the projection's mirror is cancelled for this model's geometry,
// making text readable. Skips animated models (D_8038371C != NULL) because
// bone transforms use G_MTX_LOAD which would lose the counter-mirror while
// G_EX_INVERT_CULLING is cleared, causing bone geometry to be culled.
int port_mirror_bakeCounterScale(void) {
    if (!sMirrorExcludeNextModel || !sMirrorAppliedProjection)
        return 0;
    sMirrorExcludeNextModel = false;
    if (D_8038371C != NULL)
        return 0;
    mlMtxScale_xyz(-1.0f, 1.0f, 1.0f);
    return 1;
}

// --- Wrapper draw functions ---

static Actor* mirror_excluded_lair_func_80387560(ActorMarker* m, Gfx** g, Mtx** t, Vtx** v) {
    if (port_mirror_active())
        port_mirror_setExclude();
    Actor* r = sOrig_lair_func_80387560(m, g, t, v);
    port_mirror_clearExclude();
    return r;
}

static Actor* mirror_excluded_actor_draw(ActorMarker* m, Gfx** g, Mtx** t, Vtx** v) {
    if (port_mirror_active())
        port_mirror_setExclude();
    Actor* r = sOrig_actor_draw(m, g, t, v);
    port_mirror_clearExclude();
    return r;
}

// Patch ActorInfo draw_func pointers for text-bearing models.
// Called once at init. Wrappers check port_mirror_active() so they're
// no-ops when mirror mode is off.
void port_mirror_patchTextActors(void) {
    // Level entry signs
    sOrig_lair_func_80387560 = (ActorDrawFunc)lair_func_80387560;
    D_80393354.draw_func = mirror_excluded_lair_func_80387560;

    // Mumbo token signs (5 variants: 5, 10, 15, 20, 25 tokens)
    sOrig_actor_draw = (ActorDrawFunc)actor_draw;
    chMumboSign5.draw_func = mirror_excluded_actor_draw;
    chMumboSign10.draw_func = mirror_excluded_actor_draw;
    chMumboSign15.draw_func = mirror_excluded_actor_draw;
    chMumboSign20.draw_func = mirror_excluded_actor_draw;
    chMumboSign25.draw_func = mirror_excluded_actor_draw;
}

} // extern "C"
