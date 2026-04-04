// Camera Patches
//
// Cutscene aspect lock and widescreen yaw fixes. The cutscene lock and
// static camera tracking use the event system; the per-frame yaw fix
// is as a direct port_* call from viewport_update().

#include <libultraship/libultraship.h>
#include <libultraship/bridge/consolevariablebridge.h>

#include "port/Engine.h"
#include "port/ui/cvar_prefixes.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
#include "variables.h"
enum level_e map_getLevel(enum map_e map);
enum map_e map_get(void);
}

// Cutscene aspect lock — force 4:3 during cutscene maps

#define CVAR_AR_ENABLED CVAR_SETTING("AdvancedResolution.Enabled")
#define CVAR_AR_COMBO CVAR_SETTING("AdvancedResolution.UIComboItem.AspectRatio")
#define CVAR_AR_X CVAR_SETTING("AdvancedResolution.AspectRatioX")
#define CVAR_AR_Y CVAR_SETTING("AdvancedResolution.AspectRatioY")

static int32_t sCutsceneAspectActive = 0;
static int32_t sSavedEnabled;
static int32_t sSavedCombo;
static float sSavedX;
static float sSavedY;

static void updateCutsceneAspect(int32_t mapId) {
    int32_t isCutscene = (map_getLevel((enum map_e)mapId) == LEVEL_D_CUTSCENE);

    if (isCutscene && !sCutsceneAspectActive) {
        int32_t enabled = CVarGetInteger(CVAR_AR_ENABLED, 0);
        float arX = CVarGetFloat(CVAR_AR_X, 0.0f);
        float arY = CVarGetFloat(CVAR_AR_Y, 0.0f);
        float actual = GameEngine_GetAspectRatio();
        if ((arY > 0.0f && (arX / arY) > (4.0f / 3.0f + 0.01f)) || (!enabled && actual > (4.0f / 3.0f + 0.01f))) {
            sSavedEnabled = CVarGetInteger(CVAR_AR_ENABLED, 0);
            sSavedCombo = CVarGetInteger(CVAR_AR_COMBO, 3);
            sSavedX = CVarGetFloat(CVAR_AR_X, 16.0f);
            sSavedY = CVarGetFloat(CVAR_AR_Y, 9.0f);
            CVarSetInteger(CVAR_AR_ENABLED, 1);
            CVarSetInteger(CVAR_AR_COMBO, 2);
            CVarSetFloat(CVAR_AR_X, 4.0f);
            CVarSetFloat(CVAR_AR_Y, 3.0f);
            sCutsceneAspectActive = 1;
        }
    } else if (!isCutscene && sCutsceneAspectActive) {
        CVarSetInteger(CVAR_AR_ENABLED, sSavedEnabled);
        CVarSetInteger(CVAR_AR_COMBO, sSavedCombo);
        CVarSetFloat(CVAR_AR_X, sSavedX);
        CVarSetFloat(CVAR_AR_Y, sSavedY);
        sCutsceneAspectActive = 0;
    }
}

// Widescreen yaw fix — per-frame yaw adjustment for certain static cameras in widescreen mode

#define CVAR_WS_CAMERA_FIX CVAR_ENHANCEMENT("Fix.WidescreenCamera")

struct WsYawFix {
    int32_t map;
    int32_t node;
    float adjust;
};

static const WsYawFix sWsYawFixes[] = {
    { MAP_2_MM_MUMBOS_MOUNTAIN, 0x17, -5.0f },
};
static constexpr int WS_YAW_FIX_COUNT = sizeof(sWsYawFixes) / sizeof(sWsYawFixes[0]);

static int32_t sLastStaticCameraNode = -1;

extern "C" void port_camera_applyWsYawFix(float rotation[3]) {
    if (!CVarGetInteger(CVAR_WS_CAMERA_FIX, 1))
        return;
    if (WS_YAW_FIX_COUNT == 0 || sLastStaticCameraNode < 0) {
        return;
    }
    if (GameEngine_GetAspectRatio() <= 1.34f) {
        return;
    }
    int32_t curMap = (int32_t)map_get();
    for (int i = 0; i < WS_YAW_FIX_COUNT; i++) {
        if (curMap == sWsYawFixes[i].map &&
            (sWsYawFixes[i].node == -1 || sLastStaticCameraNode == sWsYawFixes[i].node)) {
            rotation[1] += sWsYawFixes[i].adjust;
            break;
        }
    }
}

// Event listeners

void RegisterCameraPatches_Init() {
    // TODO: swap to COND_HOOK when available
    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnMapLoad* ev = (OnMapLoad*)event;
        updateCutsceneAspect(ev->mapId);
    });

    // TODO: swap to COND_HOOK when available
    COND_VB_SHOULD(VB_STATIC_CAMERA_SET, true, {
        if (ev->id != VB_STATIC_CAMERA_SET)
            return;
        sLastStaticCameraNode = *(int32_t*)args;
    });

    // TODO: swap to COND_HOOK when available
    COND_VB_SHOULD(VB_STATIC_CAMERA_EXIT, true, {
        if (ev->id != VB_STATIC_CAMERA_EXIT)
            return;
        sLastStaticCameraNode = -1;
    });
}

static RegisterShipInitFunc initFunc(RegisterCameraPatches_Init);
