#include <libultraship/bridge/consolevariablebridge.h>
#include <cmath>

#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Camera/FreeLookCamera.h"

// Vanilla camera / math / input entry points (decomp C API).
extern "C" {
void ncDynamicCamera_getPosition(float dst[3]);
void ncDynamicCamera_setPosition(float src[3]);
void ncDynamicCamera_setRotation(float src[3]);
int ncDynamicCamera_getState(void);
void ncDynamicCamera_setState(int state);

void func_802C02D4(float center[3]);                                                 // camera focus/orbit center
void func_80256E24(float dst[3], float pitch, float yaw, float x, float y, float z); // spherical -> offset
int func_8025801C(float vec[3], float* yaw);                                         // vector -> yaw (degrees)
void func_802BC434(float rotOut[3], float fromPos[3], float targetPos[3]);           // look-at rotation
int func_802BE60C(void);                                                             // swept camera collision + slide

float ml_acosf(float x);
float mlNormalizeAngle(float deg);
float time_getDelta(void);
float gu_sqrtf(float x);

void controller_getRightStick(int controller_index, float dst[2]);

int bainput_should_rotate_camera_left(void);
int bainput_should_rotate_camera_right(void);
int bainput_should_zoom_out_camera(void);
int bainput_should_look_first_person_camera(void);
}

namespace {
#define CVAR_FREELOOK_ENABLED CVAR_ENHANCEMENT("Camera.FreeLook.Enabled")
#define CVAR_FREELOOK_YAW_SENS CVAR_ENHANCEMENT("Camera.FreeLook.YawSensitivity")
#define CVAR_FREELOOK_PITCH_SENS CVAR_ENHANCEMENT("Camera.FreeLook.PitchSensitivity")
#define CVAR_FREELOOK_INVERT_X CVAR_ENHANCEMENT("Camera.FreeLook.InvertX")
#define CVAR_FREELOOK_INVERT_Y CVAR_ENHANCEMENT("Camera.FreeLook.InvertY")
#define CVAR_FREELOOK_SMOOTH_RATE CVAR_ENHANCEMENT("Camera.FreeLook.SmoothRate")
#define CVAR_FREELOOK_MIN_DISTANCE CVAR_ENHANCEMENT("Camera.FreeLook.MinDistance")
#define CVAR_FREELOOK_MAX_DISTANCE CVAR_ENHANCEMENT("Camera.FreeLook.MaxDistance")

// Tuning
constexpr float deadzone = 0.15f;
constexpr float enterThreshold = 0.30f;
constexpr float horizontalSpeed = 160.0f;
constexpr float verticalSpeed = 100.0f;

constexpr float minPitch = -85.0f; // high overhead, looking down
constexpr float maxPitch = 40.0f;  // low, looking up at the player

// position smoothing rate (1/sec); higher = snappier
constexpr float defaultSmoothRate = 40.0f;
constexpr float defaultMinDistance = 120.0f;
constexpr float defaultMaxDistance = 1200.0f;

bool isActive = false;
bool justEntered = false;
float yaw = 0.0f;
float pitch = 0.0f;
float distance = 300.0f;
float smoothPos[3] = { 0.0f, 0.0f, 0.0f };
bool isSmoothValid = false;

float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

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

bool cButtonCameraControl() {
    return bainput_should_rotate_camera_left() || bainput_should_rotate_camera_right() ||
           bainput_should_zoom_out_camera() || bainput_should_look_first_person_camera();
}

void captureFromCurrentCamera() {
    float camPos[3];
    float center[3];
    ncDynamicCamera_getPosition(camPos);
    func_802C02D4(center);

    float diff[3] = { camPos[0] - center[0], camPos[1] - center[1], camPos[2] - center[2] };
    float dist = gu_sqrtf(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
    float minDist = CVarGetFloat(CVAR_FREELOOK_MIN_DISTANCE, defaultMinDistance);
    float maxDist = CVarGetFloat(CVAR_FREELOOK_MAX_DISTANCE, defaultMaxDistance);
    if (maxDist < minDist) {
        maxDist = minDist;
    }
    distance = clampf(dist, minDist, maxDist);

    yaw = 0.0f;
    func_8025801C(diff, &yaw);
    float sinPitch = clampf(-diff[1] / dist, -1.0f, 1.0f);
    float pitchMag = ml_acosf(sinPitch);
    pitch = clampf(sinPitch < 0.0f ? -pitchMag : pitchMag, minPitch, maxPitch);

    justEntered = true;
    isSmoothValid = false;
}

void exitFreeLook() {
    isActive = false;
    if (ncDynamicCamera_getState() == FREELOOK_CAM_STATE) {
        ncDynamicCamera_setState(0xB); // hand back to the normal follow camera
    }
}

} // namespace

extern "C" int port_freeLook_isEnabled(void) {
    return CVarGetInteger(CVAR_FREELOOK_ENABLED, 0) != 0;
}

extern "C" int port_freeLook_handle(void) {
    if (!port_freeLook_isEnabled()) {
        if (isActive) {
            exitFreeLook();
        }
        return 0;
    }

    int state = ncDynamicCamera_getState();

    if (isActive) {
        if (state != FREELOOK_CAM_STATE) {
            isActive = false;
            return 0;
        }
        if (cButtonCameraControl()) {
            exitFreeLook();
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
        captureFromCurrentCamera();
        isActive = true;
        ncDynamicCamera_setState(FREELOOK_CAM_STATE);
        return 1;
    }

    return 0;
}

extern "C" void port_freeLookCamera_update(void) {
    float center[3];
    func_802C02D4(center);

    float dt = time_getDelta();

    if (!justEntered) {
        float stick[2];
        readStick(stick);

        float yawSens = CVarGetFloat(CVAR_FREELOOK_YAW_SENS, 1.0f);
        float pitchSens = CVarGetFloat(CVAR_FREELOOK_PITCH_SENS, 1.0f);
        bool invertX = CVarGetInteger(CVAR_FREELOOK_INVERT_X, 0) != 0;
        bool invertY = CVarGetInteger(CVAR_FREELOOK_INVERT_Y, 0) != 0;

        float yawStep = (invertX ? -stick[0] : stick[0]) * horizontalSpeed * yawSens * dt;
        yaw = mlNormalizeAngle(yaw + yawStep);
        // Default: push up -> camera rises and looks down (overhead). InvertY flips it.
        float pitchStep = (invertY ? stick[1] : -stick[1]) * verticalSpeed * pitchSens * dt;
        pitch = clampf(pitch + pitchStep, minPitch, maxPitch);
    }
    justEntered = false;

    float offset[3];
    func_80256E24(offset, pitch, yaw, 0.0f, 0.0f, distance);

    float pos[3] = { center[0] + offset[0], center[1] + offset[1], center[2] + offset[2] };
    ncDynamicCamera_setPosition(pos);

    // Follow geometry collision like the normal camera does
    func_802BE60C();

    float resolved[3];
    ncDynamicCamera_getPosition(resolved);

    if (!isSmoothValid) {
        smoothPos[0] = resolved[0];
        smoothPos[1] = resolved[1];
        smoothPos[2] = resolved[2];
        isSmoothValid = true;
    } else {
        // Smooth the resolved position to prevent hitching on edges.
        float smoothRate = CVarGetFloat(CVAR_FREELOOK_SMOOTH_RATE, defaultSmoothRate);
        float f = clampf(smoothRate * dt, 0.0f, 1.0f);
        smoothPos[0] += (resolved[0] - smoothPos[0]) * f;
        smoothPos[1] += (resolved[1] - smoothPos[1]) * f;
        smoothPos[2] += (resolved[2] - smoothPos[2]) * f;
    }
    ncDynamicCamera_setPosition(smoothPos);

    // Aim back at the player from the smoothed position
    float rot[3];
    func_802BC434(rot, center, smoothPos);
    ncDynamicCamera_setRotation(rot);
}
