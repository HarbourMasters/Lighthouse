#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#define CVAR_FOLLOW_CAMERA CVAR_ENHANCEMENT("Camera.Follow")
#define CVAR CVarGetInteger(CVAR_FOLLOW_CAMERA, 0)

void RegisterFollowCamera_Init() {
    COND_VB_SHOULD(VB_CAMERA_FOLLOW, EVENT_PRIORITY_NORMAL, CVAR, { *should = true; });
}

static RegisterShipInitFunc initFunc(RegisterFollowCamera_Init, { CVAR_FOLLOW_CAMERA });