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
}

static RegisterShipInitFunc initMirroredWorldFunc(RegisterMirroredWorld_Init, { CVAR_MIRRORED_WORLD_MODE });
