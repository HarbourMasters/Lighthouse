#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"

#include <libultra/controller.h>
#include <libultra/gbi.h>
#include <libultra/gu.h>
#include <libultra/convert.h>

#include <vector>

#include "functions.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
extern "C" {
#include "core2/model.h"
#include "prop.h"
}

static bool sMirrorSceneActive = false;
static bool sMirrorAppliedProjection = false;
static bool sMirrorAtCapture = false;
static bool sMirrorExcludeNextModel = false;
static std::vector<bool> sMirrorBaked; // one per model being drawn, innermost last

extern "C" {

// mlMtx scale function for baking counter-mirror into internal matrix state
extern void mlMtxScale_xyz(f32 x, f32 y, f32 z);

// Animation state in render.c — non-NULL when the current model is animated
extern AnimMtxList* D_8038371C;

} // extern "C"

static bool MirrorActive() {
    return CVarGetInteger(CVAR_ENHANCEMENT("Modes.MirroredWorld.State"), 0);
}

// Apply horizontal mirror via projection scale and invert culling.
// guScale(-1,1,1) reverses triangle winding order, so G_EX_INVERT_CULLING
// tells the LUS interpreter to negate the cross product during culling checks.
static void ApplyMirror(Gfx** gfx, Mtx** mtx) {
    if (MirrorActive() && sMirrorSceneActive) {
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
static void UndoProjection(Gfx** gfx, Mtx** mtx) {
    if (sMirrorAppliedProjection) {
        extern void viewport_setRenderViewportAndPerspectiveMatrix(Gfx**, Mtx**);
        viewport_setRenderViewportAndPerspectiveMatrix(gfx, mtx);
        sMirrorAppliedProjection = false;
    }
}

// --- Pause menu capture state tracking ---

extern "C" void port_mirror_markCapture(void) {
    sMirrorAtCapture = MirrorActive();
}

extern "C" int port_mirror_shouldFlipPauseBg(void) {
    return MirrorActive() != sMirrorAtCapture ? 1 : 0;
}

// --- Per-model counter-mirror for text-bearing objects ---

// Counter-mirror for text-bearing models. Bakes a horizontal flip into the
// mlMtx state so the projection's mirror is cancelled for this model's geometry,
// making text readable. Skips animated models (D_8038371C != NULL) because
// bone transforms use G_MTX_LOAD which would lose the counter-mirror while
// G_EX_INVERT_CULLING is cleared, causing bone geometry to be culled.
static bool BakeCounterScale() {
    if (!sMirrorExcludeNextModel || !sMirrorAppliedProjection)
        return false;
    sMirrorExcludeNextModel = false;
    if (D_8038371C != NULL)
        return false;
    mlMtxScale_xyz(-1.0f, 1.0f, 1.0f);
    return true;
}

static void ExcludeTextSign(IEvent*) {
    sMirrorExcludeNextModel = MirrorActive();
}

void RegisterMirrorPatches_Init() {
    // Registered whatever the mode, so turning it off still restores the projection and culling.
    COND_HOOK(OnControllerRead, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        if (MirrorActive()) {
            auto* pad = static_cast<OSContPad*>(((OnControllerRead*)event)->pad);
            pad->stick_x = -pad->stick_x;
        }
    });
    COND_HOOK(OnWorldDrawBegin, EVENT_PRIORITY_NORMAL, true, [](IEvent*) {
        sMirrorSceneActive = true;
        sMirrorAppliedProjection = false;
    });
    COND_HOOK(OnViewportProjection, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnViewportProjection*)event;
        ApplyMirror(ev->gfx, ev->mtx);
    });
    COND_HOOK(OnWorldDraw, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnWorldDraw*)event;
        sMirrorSceneActive = false;
        UndoProjection(ev->gfx, ev->mtx);
    });
    COND_ID_HOOK(OnActorDrawBegin, ACTOR_2E3_LEVEL_ENTRY_SIGNS, EVENT_PRIORITY_NORMAL, true, ExcludeTextSign);
    COND_ID_HOOK(OnActorDrawBegin, ACTOR_368_5_MUMBO_TOKEN_SIGN, EVENT_PRIORITY_NORMAL, true, ExcludeTextSign);
    COND_ID_HOOK(OnActorDrawBegin, ACTOR_369_20_MUMBO_TOKEN_SIGN, EVENT_PRIORITY_NORMAL, true, ExcludeTextSign);
    COND_ID_HOOK(OnActorDrawBegin, ACTOR_36A_15_MUMBO_TOKEN_SIGN, EVENT_PRIORITY_NORMAL, true, ExcludeTextSign);
    COND_ID_HOOK(OnActorDrawBegin, ACTOR_36B_10_MUMBO_TOKEN_SIGN, EVENT_PRIORITY_NORMAL, true, ExcludeTextSign);
    COND_ID_HOOK(OnActorDrawBegin, ACTOR_36C_25_MUMBO_TOKEN_SIGN, EVENT_PRIORITY_NORMAL, true, ExcludeTextSign);
    COND_HOOK(OnActorDrawEnd, EVENT_PRIORITY_NORMAL, true, [](IEvent*) { sMirrorExcludeNextModel = false; });
    // Counter-mirror text-bearing models so text reads correctly.
    COND_HOOK(OnModelRenderBegin, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        bool baked = BakeCounterScale();
        sMirrorBaked.push_back(baked);
        if (baked) {
            gSPClearExtraGeometryMode((*((OnModelRenderBegin*)event)->gfx)++, G_EX_INVERT_CULLING);
        }
    });
    // Empty when a preset re-registered these between a model's begin and end.
    COND_HOOK(OnModelRenderEnd, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        if (!sMirrorBaked.empty()) {
            if (sMirrorBaked.back()) {
                gSPSetExtraGeometryMode((*((OnModelRenderEnd*)event)->gfx)++, G_EX_INVERT_CULLING);
            }
            sMirrorBaked.pop_back();
        }
    });
}

static RegisterShipInitFunc initMirrorPatches(RegisterMirrorPatches_Init, {});
