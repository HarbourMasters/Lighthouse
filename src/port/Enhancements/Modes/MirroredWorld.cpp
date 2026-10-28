#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Patches/Patches.h"

#define CVAR_MIRRORED_WORLD_MODE CVAR_ENHANCEMENT("Modes.MirroredWorld.Mode")
#define CVAR_MIRRORED_WORLD_STATE CVAR_ENHANCEMENT("Modes.MirroredWorld.State")

void RegisterMirroredWorld_Init() {
    MirrorPatches_PatchTextActors();

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_MIRRORED_WORLD_MODE, 0),
              [](IEvent* event) { CVarSetInteger(CVAR_MIRRORED_WORLD_STATE, 1); });

    COND_HOOK(OnControllerRead, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_MIRRORED_WORLD_MODE, 0), [](IEvent* event) {
        if (MirrorPatches_IsActive()) {
            auto* pad = (OSContPad*)((OnControllerRead*)event)->pad;
            pad->stick_x = -pad->stick_x;
        }
    });

    if (!CVarGetInteger(CVAR_MIRRORED_WORLD_MODE, 0)) {
        CVarClear(CVAR_MIRRORED_WORLD_STATE);
    }

    COND_HOOK(OnWorldDrawBegin, EVENT_PRIORITY_NORMAL, true, [](IEvent*) { MirrorPatches_BeginScene(); });
    COND_HOOK(OnWorldDrawEnd, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnWorldDrawEnd*)event;
        MirrorPatches_EndScene();
        MirrorPatches_UndoProjection(ev->gfx, ev->mtx);
    });
    COND_HOOK(OnViewportProjection, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnViewportProjection*)event;
        MirrorPatches_ApplyViewport(ev->gfx, ev->mtx);
    });
    COND_HOOK(ModelCounterMirror, EVENT_PRIORITY_NORMAL, true,
              [](IEvent* event) { *((ModelCounterMirror*)event)->applied = MirrorPatches_BakeCounterScale(); });
    COND_HOOK(OnPauseCapture, EVENT_PRIORITY_NORMAL, true, [](IEvent*) { MirrorPatches_MarkCapture(); });
    COND_VB_SHOULD(VB_PAUSE_BG_MIRRORED, EVENT_PRIORITY_NORMAL, true, { *should = MirrorPatches_ShouldFlipPauseBg(); });
}

static RegisterShipInitFunc initMirroredWorldFunc(RegisterMirroredWorld_Init, { CVAR_MIRRORED_WORLD_MODE });
