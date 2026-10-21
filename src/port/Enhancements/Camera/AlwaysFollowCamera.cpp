#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/UI/Notification.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"

#include "enums.h"
#include "functions.h"

#define CVAR_FOLLOW_CAMERA CVAR_ENHANCEMENT("Camera.Follow")
#define CVAR CVarGetInteger(CVAR_FOLLOW_CAMERA, 0)

enum FollowCameraMode { FOLLOW_CAMERA_HOLD, FOLLOW_CAMERA_ALWAYS, FOLLOW_CAMERA_TOGGLE };
static bool sToggledOn = false;

void RegisterFollowCamera_Init() {
    sToggledOn = false;

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVAR == FOLLOW_CAMERA_TOGGLE, [](IEvent*) {
        if (!IsDemoMode() && bakey_pressed(BUTTON_R)) {
            sToggledOn = !sToggledOn;
            Notification::Emit({ .prefix = "Follow Camera", .message = sToggledOn ? "on" : "off" });
        }
    });

    COND_VB_SHOULD(VB_CAMERA_FOLLOW, EVENT_PRIORITY_NORMAL, CVAR != FOLLOW_CAMERA_HOLD, {
        if (!IsDemoMode()) {
            *should = CVAR == FOLLOW_CAMERA_ALWAYS || sToggledOn;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterFollowCamera_Init, { CVAR_FOLLOW_CAMERA });
