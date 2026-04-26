#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"
#include "port/ShipInit.hpp"
#include "port/enhancements/events/hooks/Events.h"
#include "port/patches/Patches.h"

#define CVAR_MIRRORED_WORLD_MODE CVAR_ENHANCEMENT("Modes.MirroredWorld.Mode")
#define CVAR_MIRRORED_WORLD_STATE CVAR_ENHANCEMENT("Modes.MirroredWorld.State")

void RegisterMirroredWorld_Init() {
    port_mirror_patchTextActors();

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_MIRRORED_WORLD_MODE, 0),
              [](IEvent* event) { CVarSetInteger(CVAR_MIRRORED_WORLD_STATE, 1); });

    if (!CVarGetInteger(CVAR_MIRRORED_WORLD_MODE, 0)) {
        CVarClear(CVAR_MIRRORED_WORLD_STATE);
    }
}

static RegisterShipInitFunc initMirroredWorldFunc(RegisterMirroredWorld_Init, { CVAR_MIRRORED_WORLD_MODE });
