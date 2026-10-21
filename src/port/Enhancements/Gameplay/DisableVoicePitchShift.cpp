#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#define CVAR_DISABLE_VOICE_PITCH_SHIFT CVAR_ENHANCEMENT("Gameplay.DisableVoicePitchShift")

void RegisterDisableVoicePitchShift_Init() {
    COND_VB_SHOULD(VB_ZOOMBOX_FAST_VOICE, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_DISABLE_VOICE_PITCH_SHIFT, 0),
                   { *should = false; });
}

static RegisterShipInitFunc initFunc(RegisterDisableVoicePitchShift_Init, { CVAR_DISABLE_VOICE_PITCH_SHIFT });
