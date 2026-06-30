// Modern control scheme camera
//
// The right stick orbits the camera in yaw around the player (no pitch). It runs
// on the shared orbit core; this file is just the modern-scheme policy.

#include <libultraship/bridge.h>

#include "port/UI/cvar_prefixes.h"
#include "ControlSchemes.h"
#include "ModernCamera.h"
#include "port/Enhancements/Camera/OrbitCameraCore.h"

extern "C" {
extern unsigned char D_8037C061; // current zoom level
extern int D_8037C07C;           // level-3 distance x; 0 means level 3 is unavailable

void func_80290B60(int level); // set the zoom level
float batimer_get(int id);     // timer value
void batimer_set(int id, float t);
void basfx_80299D2C(int sfxId, float pitch, int volume); // play a camera SFX
float time_getDelta(void);
int bs_getState(void);
int balookat_getState(void);    // nonzero while the look-around camera is active
int player_movementGroup(void); // enum bsgroup_e

int ncDynamicCamera_getState(void);

int bainput_should_rotate_camera_left(void);
int bainput_should_rotate_camera_right(void);
int bainput_should_look_first_person_camera(void);

void controller_getRightStick(int controller_index, float dst[2]);
}

#include "enums.h" // BS_CROUCH
#include "port/ShipUtils.h"

namespace {

bool ModernSchemeActive() {
    return CVarGetInteger(CVAR_SETTING("Controls.Scheme"), CONTROL_SCHEME_RETRO) == CONTROL_SCHEME_MODERN;
}

constexpr float kYawDeadzone = 0.2f;
constexpr float kYawEnter = 0.3f;
constexpr float kYawSpeed = 160.0f;
constexpr float kZoomOn = 0.49f;
constexpr float kZoomOff = 0.21f;
constexpr float kPosSmoothRate = 40.0f;

// Flat (pitch-locked) orbit on the shared core.
OrbitCamera sModern = { /*stateId*/ MODERN_ORBIT_CAM_STATE, /*allowPitch*/ 0,
                        /*minPitch*/ 0.0f, /*maxPitch*/ 0.0f, /*smoothRate*/ kPosSmoothRate };

float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

void ReadStickNorm(float& x, float& y) {
    float out[2] = { 0.0f, 0.0f };
    controller_getRightStick(0, out);
    x = clampf(out[0], -1.0f, 1.0f);
    y = clampf(out[1], -1.0f, 1.0f);
}

// Stick X past the deadzone.
float YawInput(float x) {
    if (x > kYawDeadzone) {
        return (x - kYawDeadzone) / (1.0f - kYawDeadzone);
    }
    if (x < -kYawDeadzone) {
        return (x + kYawDeadzone) / (1.0f - kYawDeadzone);
    }
    return 0.0f;
}

// Hand the orbit back to the normal follow camera when the player takes manual
// camera control.
bool ManualCameraControl() {
    return bainput_should_rotate_camera_left() || bainput_should_rotate_camera_right() ||
           bainput_should_look_first_person_camera();
}

} // namespace

extern "C" int port_modernCamera_handleYaw(void) {
    bool schemeOk = ModernSchemeActive() && bs_getState() != BS_CROUCH && !IsDemoMode();
    if (!schemeOk) {
        if (sModern.active) {
            OrbitCamera_Exit(&sModern);
        }
        return 0;
    }

    int state = ncDynamicCamera_getState();

    if (sModern.active) {
        // A scripted/special camera took the state out from under us; drop the
        // orbit and let that camera run.
        if (state != MODERN_ORBIT_CAM_STATE) {
            sModern.active = 0;
            return 0;
        }
        if (ManualCameraControl()) {
            OrbitCamera_Exit(&sModern);
            return 0;
        }
        return 1;
    }

    float x;
    float y;
    ReadStickNorm(x, y);
    if (x > kYawEnter || x < -kYawEnter) {
        OrbitCamera_Enter(&sModern);
        return 1;
    }
    return 0;
}

extern "C" void port_modernCamera_update(void) {
    float dt = time_getDelta();

    // Drive yaw from the stick. Freeze it during look-around so it doesn't drift
    // while the first-person view overrides the camera.
    float yawDelta = 0.0f;
    if (!sModern.justEntered && !balookat_getState()) {
        float x;
        float y;
        ReadStickNorm(x, y);
        yawDelta = YawInput(x) * kYawSpeed * dt * port_cameraInvertXSign();
    }

    OrbitCamera_Update(&sModern, yawDelta, 0.0f);
}

extern "C" int port_camera_suppressVanillaZoom(void) {
    if (IsDemoMode()) {
        return 0;
    }
    int scheme = CVarGetInteger(CVAR_SETTING("Controls.Scheme"), CONTROL_SCHEME_RETRO);
    if (scheme == CONTROL_SCHEME_MODERN) {
        return 1;
    }
    if (scheme == CONTROL_SCHEME_POCKET && bs_getState() == BS_CROUCH) {
        return 1;
    }
    return 0;
}

extern "C" void port_modernCamera_handleZoom(void) {
    if (!ModernSchemeActive() || bs_getState() == BS_CROUCH || IsDemoMode()) {
        return;
    }

    if (balookat_getState() || player_movementGroup() == BSGROUP_4_LOOK) {
        return;
    }

    float sx;
    float sy;
    ReadStickNorm(sx, sy);
    float down = -sy;
    float adown = (down < 0.0f) ? -down : down;

    static bool sArmed = true;

    if (sArmed && adown > kZoomOn && batimer_get(7) <= 0.0f) {
        int32_t level = D_8037C061;
        int32_t next = level;
        if (down > 0.0f) {
            next = level + 1;
            if (next > 3) {
                next = 3;
            }
            if (next == 3 && D_8037C07C == 0) {
                next = 2;
            }
        } else {
            next = level - 1;
            if (next < 1) {
                next = 1;
            }
        }
        if (next != level) {
            bool zoomingIn = next < level;
            int sfxId = zoomingIn ? SFX_12D_CAMERA_ZOOM_CLOSEST : SFX_12E_CAMERA_ZOOM_MEDIUM;
            float pitch = zoomingIn ? 1.0f : ((next == 3) ? 1.2f : 1.0f);
            basfx_80299D2C(sfxId, pitch, 12000);
            func_80290B60(next);
            batimer_set(7, 0.4f);
        }
        sArmed = false;
    }

    if (adown < kZoomOff) {
        sArmed = true;
    }
}

extern "C" float port_cameraInvertXSign(void) {
    return CVarGetInteger(CVAR_ENHANCEMENT("Camera.InvertX"), 0) ? -1.0f : 1.0f;
}

extern "C" float port_cameraInvertYSign(void) {
    return CVarGetInteger(CVAR_ENHANCEMENT("Camera.InvertY"), 0) ? -1.0f : 1.0f;
}
