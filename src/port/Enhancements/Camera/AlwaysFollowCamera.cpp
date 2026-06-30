#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#define CVAR_ALWAYS_FOLLOW_CAMERA CVAR_ENHANCEMENT("Camera.AlwaysFollow")
#define CVAR CVarGetInteger(CVAR_ALWAYS_FOLLOW_CAMERA, 0)

void RegisterAlwaysFollowCamera_Init() {
    COND_VB_SHOULD(VB_CAMERA_FOLLOW, EVENT_PRIORITY_NORMAL, CVAR, { *should = true; });
}

static RegisterShipInitFunc initFunc(RegisterAlwaysFollowCamera_Init, { CVAR_ALWAYS_FOLLOW_CAMERA });