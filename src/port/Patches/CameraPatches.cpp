// Camera Patches
//
// Cutscene aspect lock and widescreen yaw fixes. The cutscene lock and
// static camera tracking use the event system; the per-frame yaw fix
// is as a direct port_* call from viewport_update().

#include <libultraship/libultraship.h>
#include <libultraship/bridge/consolevariablebridge.h>

#include "port/Engine.h"
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "variables.h"
enum level_e map_getLevel(enum map_e map);
enum map_e gsworld_getMap(void);
}

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern "C" {
extern float sViewportFOVy;
extern float sViewportAspect;
}

// Cutscene aspect lock — force 4:3 during cutscene maps

#define CVAR_AR_ENABLED CVAR_PREFIX_ADVANCED_RESOLUTION ".Enabled"
#define CVAR_AR_COMBO CVAR_PREFIX_ADVANCED_RESOLUTION ".UIComboItem.AspectRatio"
#define CVAR_AR_X CVAR_PREFIX_ADVANCED_RESOLUTION ".AspectRatioX"
#define CVAR_AR_Y CVAR_PREFIX_ADVANCED_RESOLUTION ".AspectRatioY"
#define CVAR_CUTSCENE_ASPECT CVAR_ENHANCEMENT("Graphics.CutsceneAspect")

#define CVAR_CSA_BAK_ACTIVE CVAR_ENHANCEMENT("Graphics.CutsceneAspectBackup.Active")
#define CVAR_CSA_BAK_ENABLED CVAR_ENHANCEMENT("Graphics.CutsceneAspectBackup.Enabled")
#define CVAR_CSA_BAK_COMBO CVAR_ENHANCEMENT("Graphics.CutsceneAspectBackup.Combo")
#define CVAR_CSA_BAK_X CVAR_ENHANCEMENT("Graphics.CutsceneAspectBackup.X")
#define CVAR_CSA_BAK_Y CVAR_ENHANCEMENT("Graphics.CutsceneAspectBackup.Y")

static int32_t sCutsceneAspectActive = 0;

// Restore the user's Advanced Resolution settings from the persisted backup and clear the flag.
static void restoreCutsceneAspectBackup() {
    CVarSetInteger(CVAR_AR_ENABLED, CVarGetInteger(CVAR_CSA_BAK_ENABLED, 0));
    CVarSetInteger(CVAR_AR_COMBO, CVarGetInteger(CVAR_CSA_BAK_COMBO, 3));
    CVarSetFloat(CVAR_AR_X, CVarGetFloat(CVAR_CSA_BAK_X, 16.0f));
    CVarSetFloat(CVAR_AR_Y, CVarGetFloat(CVAR_CSA_BAK_Y, 9.0f));
    CVarClear(CVAR_CSA_BAK_ACTIVE);
    sCutsceneAspectActive = 0;
}

static void ResetCutsceneAspect() {
    if (!sCutsceneAspectActive) {
        return;
    }
    restoreCutsceneAspectBackup();
}

static void updateCutsceneAspect(int32_t mapId) {
    bool isCutscene = (map_getLevel((enum map_e)mapId) == LEVEL_D_CUTSCENE);

    if (isCutscene && !sCutsceneAspectActive) {
        bool enabled = CVarGetInteger(CVAR_AR_ENABLED, 0);
        float arX = CVarGetFloat(CVAR_AR_X, 0.0f);
        float arY = CVarGetFloat(CVAR_AR_Y, 0.0f);
        auto window = Ship::Context::GetRawInstance()->GetWindow();
        uint32_t winW = window->GetWidth();
        uint32_t winH = window->GetHeight();
        float actual = (winH > 0) ? (float)winW / (float)winH : GameEngine_GetAspectRatio();
        if ((arY > 0.0f && (arX / arY) > (4.0f / 3.0f + 0.01f)) || (!enabled && actual > (4.0f / 3.0f + 0.01f))) {
            CVarSetInteger(CVAR_CSA_BAK_ENABLED, CVarGetInteger(CVAR_AR_ENABLED, 0));
            CVarSetInteger(CVAR_CSA_BAK_COMBO, CVarGetInteger(CVAR_AR_COMBO, 3));
            CVarSetFloat(CVAR_CSA_BAK_X, CVarGetFloat(CVAR_AR_X, 16.0f));
            CVarSetFloat(CVAR_CSA_BAK_Y, CVarGetFloat(CVAR_AR_Y, 9.0f));
            CVarSetInteger(CVAR_CSA_BAK_ACTIVE, 1);
            CVarSetInteger(CVAR_AR_ENABLED, 1);
            CVarSetInteger(CVAR_AR_COMBO, 2);
            CVarSetFloat(CVAR_AR_X, 4.0f);
            CVarSetFloat(CVAR_AR_Y, 3.0f);
            sCutsceneAspectActive = 1;
        }
    } else if (!isCutscene && sCutsceneAspectActive) {
        ResetCutsceneAspect();
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
    int32_t curMap = (int32_t)gsworld_getMap();
    for (int i = 0; i < WS_YAW_FIX_COUNT; i++) {
        if (curMap == sWsYawFixes[i].map &&
            (sWsYawFixes[i].node == -1 || sLastStaticCameraNode == sWsYawFixes[i].node)) {
            rotation[1] += sWsYawFixes[i].adjust;
            break;
        }
    }
}

// Event listeners

void RegisterCutsceneAspect() {
    // Boot-time self-heal: if a previous session crashed or quit during a cutscene, the persisted
    // gAdvancedResolution cvars may still hold the forced 4:3. Restore the user's real settings from
    // the backup before anything reads them. This runs from the init process, not a per-frame listener.
    if (CVarGetInteger(CVAR_CSA_BAK_ACTIVE, 0)) {
        restoreCutsceneAspectBackup();
    }
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_CUTSCENE_ASPECT, 0), [](IEvent* event) {
        OnMapLoad* ev = (OnMapLoad*)event;
        updateCutsceneAspect(ev->nextMap);
    });
}

void RegisterCameraPatches_Init() {

    COND_VB_SHOULD(VB_STATIC_CAMERA_SET, EVENT_PRIORITY_NORMAL, true,
                   { sLastStaticCameraNode = *va_arg(args, int32_t*); });
    COND_VB_SHOULD(VB_STATIC_CAMERA_EXIT, EVENT_PRIORITY_NORMAL, true, { sLastStaticCameraNode = -1; });
    COND_VB_SHOULD(VB_CAMERA_LIVE_ASPECT, EVENT_PRIORITY_NORMAL, true, { *should = false; });

    // Bigger frustum — widen the side planes from the actual FOV and aspect
    // ratio, and pad the top/bottom planes to mask vertical cam pop-in.
    // Bail in replayed-input modes (attract demo, Bottles bonus, SnS picture):
    // those recordings are deterministic against the vanilla cull set, and any
    // frustum change shifts which actors tick and desyncs the playback.
    REGISTER_LISTENER(ViewportFrustumUpdate, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        if (IsDemoMode()) {
            return;
        }
        auto* ev = (ViewportFrustumUpdate*)event;
        // Widen the side planes to the actual render aspect, floored at 4:3 so we never
        // cull tighter than vanilla.
        const float kFrustumZ = 45.168514251708984f; // must match the literal in viewport.c
        const float kMargin = 1.10f;                 // ~10% wider than the exact FOV
        float aspect = GameEngine_GetAspectRatio();
        if (aspect < sViewportAspect) {
            aspect = sViewportAspect;
        }
        float halfFovYRad = (sViewportFOVy * 0.5f) * (float)(M_PI / 180.0);
        float halfFovXRad = std::atan(std::tan(halfFovYRad) * aspect * kMargin);
        *ev->frustumX = kFrustumZ / std::tan(halfFovXRad);
        *ev->frustumY = 93.9692611694336f * 1.15f;
    });
}

static RegisterShipInitFunc cutsceneAspectInitFunc(RegisterCutsceneAspect, { CVAR_CUTSCENE_ASPECT });
static RegisterShipInitFunc staticCamInitFunc(RegisterCameraPatches_Init);
