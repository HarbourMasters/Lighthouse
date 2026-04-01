#include <libultraship/libultraship.h>
#include <libultraship/bridge/consolevariablebridge.h>
#include <spdlog/spdlog.h>

extern "C" {
#include "functions.h"
#include "variables.h"
#include "port/Engine.h"
}

// Force 4:3 during cutscene maps to prevent skybox exposure, then restore
// the user's original setting when leaving the cutscene.

#define CVAR_AR_ENABLED "gSettings.AdvancedResolution.Enabled"
#define CVAR_AR_COMBO "gSettings.AdvancedResolution.UIComboItem.AspectRatio"
#define CVAR_AR_X "gSettings.AdvancedResolution.AspectRatioX"
#define CVAR_AR_Y "gSettings.AdvancedResolution.AspectRatioY"

static int32_t sCutsceneAspectActive = 0;
static int32_t sSavedEnabled;
static int32_t sSavedCombo;
static float sSavedX;
static float sSavedY;

extern "C" int32_t port_camera_isCutsceneAspectActive(void) {
    return sCutsceneAspectActive;
}

extern "C" void port_camera_updateCutsceneAspect(int32_t mapId) {
    int32_t isCutscene = (map_getLevel((enum map_e)mapId) == LEVEL_D_CUTSCENE);

    if (isCutscene && !sCutsceneAspectActive) {
        int32_t enabled = CVarGetInteger(CVAR_AR_ENABLED, 0);
        float arX = CVarGetFloat(CVAR_AR_X, 0.0f);
        float arY = CVarGetFloat(CVAR_AR_Y, 0.0f);
        float actual = GameEngine_GetAspectRatio();
        // Lock if aspect ratio is wider than 4:3
        if ((arY > 0.0f && (arX / arY) > (4.0f / 3.0f + 0.01f)) || (!enabled && actual > (4.0f / 3.0f + 0.01f))) {
            sSavedEnabled = CVarGetInteger(CVAR_AR_ENABLED, 0);
            sSavedCombo = CVarGetInteger(CVAR_AR_COMBO, 3);
            sSavedX = CVarGetFloat(CVAR_AR_X, 16.0f);
            sSavedY = CVarGetFloat(CVAR_AR_Y, 9.0f);
            CVarSetInteger(CVAR_AR_ENABLED, 1);
            CVarSetInteger(CVAR_AR_COMBO, 2); // Original (4:3)
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

// For non-cutscene maps where specific static camera nodes expose the skybox
// in widescreen. Called from viewport_update() right before the view matrix
// is built. Matches by map + camera node index for precision.

struct WsYawFix {
    int32_t map;
    int32_t node; // camera node index, or -1 for any
    float adjust; // yaw offset in degrees
};

// { map, node, adjust }
static const WsYawFix sWsYawFixes[] = {
    { MAP_2_MM_MUMBOS_MOUNTAIN, 0x17, -5.0f }, // Bottles beak buster molehill (yaw=264.26)
};
static constexpr int WS_YAW_FIX_COUNT = sizeof(sWsYawFixes) / sizeof(sWsYawFixes[0]);

static int32_t sLastStaticCameraNode = -1;

extern "C" void port_camera_setStaticNode(int32_t nodeIndex) {
    sLastStaticCameraNode = nodeIndex;
}

extern "C" void port_camera_clearStaticNode(void) {
    sLastStaticCameraNode = -1;
}

extern "C" void port_camera_applyWsYawFix(float rotation[3]) {
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
