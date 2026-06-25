#include <libultraship/bridge/consolevariablebridge.h>

#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Camera/FreeLookCamera.h"
#include "port/Enhancements/Camera/OrbitCameraCore.h"

// Vanilla camera / input entry points (decomp C API).
extern "C" {
int ncDynamicCamera_getState(void);

float time_getDelta(void);
float gu_sqrtf(float x);

void controller_getRightStick(int controller_index, float dst[2]);

int bainput_should_rotate_camera_left(void);
int bainput_should_rotate_camera_right(void);
int bainput_should_look_first_person_camera(void);
}

namespace {
#define CVAR_FREELOOK_ENABLED CVAR_ENHANCEMENT("Camera.FreeLook.Enabled")
#define CVAR_FREELOOK_YAW_SENS CVAR_ENHANCEMENT("Camera.FreeLook.YawSensitivity")
#define CVAR_FREELOOK_PITCH_SENS CVAR_ENHANCEMENT("Camera.FreeLook.PitchSensitivity")
#define CVAR_FREELOOK_INVERT_X CVAR_ENHANCEMENT("Camera.FreeLook.InvertX")
#define CVAR_FREELOOK_INVERT_Y CVAR_ENHANCEMENT("Camera.FreeLook.InvertY")
#define CVAR_FREELOOK_SMOOTH_RATE CVAR_ENHANCEMENT("Camera.FreeLook.SmoothRate")

// Tuning
constexpr float deadzone = 0.15f;
constexpr float enterThreshold = 0.30f;
constexpr float horizontalSpeed = 160.0f;
constexpr float verticalSpeed = 100.0f;

constexpr float minPitch = -85.0f; // high overhead, looking down
constexpr float maxPitch = 40.0f;  // low, looking up at the player

// position smoothing rate (1/sec); higher = snappier
constexpr float defaultSmoothRate = 40.0f;

// Pitch-enabled orbit on the shared core. Distance tracks the vanilla zoom level
// (same as the modern-scheme camera) so the normal follow distances are honored.
OrbitCamera sFreeLook = { /*stateId*/ FREELOOK_CAM_STATE, /*allowPitch*/ 1,
                          /*minPitch*/ minPitch, /*maxPitch*/ maxPitch, /*smoothRate*/ defaultSmoothRate };

// Right stick with a radial deadzone, rescaled so motion ramps from 0 at the
// deadzone edge to 1 at full deflection.
float readStick(float out[2]) {
    controller_getRightStick(0, out);
    float mag = gu_sqrtf(out[0] * out[0] + out[1] * out[1]);
    if (mag < deadzone) {
        out[0] = out[1] = 0.0f;
        return 0.0f;
    }
    if (mag > 1.0f) {
        mag = 1.0f;
    }
    float scaled = (mag - deadzone) / (1.0f - deadzone);
    float k = scaled / mag;
    out[0] *= k;
    out[1] *= k;
    return scaled;
}

// Explicit camera commands hand the frame back to the normal camera. Zoom is not
// one of them: it adjusts the vanilla zoom level, which free look tracks, so the
// player can re-zoom without dropping out of free look.
bool cButtonCameraControl() {
    return bainput_should_rotate_camera_left() || bainput_should_rotate_camera_right() ||
           bainput_should_look_first_person_camera();
}

} // namespace

extern "C" int port_freeLook_isEnabled(void) {
    return CVarGetInteger(CVAR_FREELOOK_ENABLED, 0) != 0;
}

extern "C" int port_freeLook_handle(void) {
    if (!port_freeLook_isEnabled()) {
        if (sFreeLook.active) {
            OrbitCamera_Exit(&sFreeLook);
        }
        return 0;
    }

    int state = ncDynamicCamera_getState();

    if (sFreeLook.active) {
        if (state != FREELOOK_CAM_STATE) {
            sFreeLook.active = 0;
            return 0;
        }
        if (cButtonCameraControl()) {
            OrbitCamera_Exit(&sFreeLook);
            return 0;
        }
        return 1; // hold the angle; consume the frame
    }

    if (cButtonCameraControl()) {
        return 0;
    }

    // Only enter from a normal follow/orbit camera.
    if (state != 0xB && state != 0x1 && state != 0xA) {
        return 0;
    }

    float stick[2];
    if (readStick(stick) >= enterThreshold) {
        OrbitCamera_Enter(&sFreeLook);
        return 1;
    }

    return 0;
}

extern "C" void port_freeLookCamera_update(void) {
    float dt = time_getDelta();
    sFreeLook.smoothRate = CVarGetFloat(CVAR_FREELOOK_SMOOTH_RATE, defaultSmoothRate);

    float yawDelta = 0.0f;
    float pitchDelta = 0.0f;
    if (!sFreeLook.justEntered) {
        float stick[2];
        readStick(stick);

        float yawSens = CVarGetFloat(CVAR_FREELOOK_YAW_SENS, 1.0f);
        float pitchSens = CVarGetFloat(CVAR_FREELOOK_PITCH_SENS, 1.0f);
        bool invertX = CVarGetInteger(CVAR_FREELOOK_INVERT_X, 0) != 0;
        bool invertY = CVarGetInteger(CVAR_FREELOOK_INVERT_Y, 0) != 0;

        yawDelta = (invertX ? -stick[0] : stick[0]) * horizontalSpeed * yawSens * dt;
        // Default: push up -> camera rises and looks down (overhead). InvertY flips it.
        pitchDelta = (invertY ? stick[1] : -stick[1]) * verticalSpeed * pitchSens * dt;
    }

    OrbitCamera_Update(&sFreeLook, yawDelta, pitchDelta);
}
