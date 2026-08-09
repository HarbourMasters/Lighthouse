// Free look camera

#include <cmath>

#include <libultraship/bridge/consolevariablebridge.h>

#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Camera/FreeLookCamera.h"

extern "C" {
// Math helpers
void func_80256E24(float dst[3], float pitch, float yaw, float x, float y, float z);
void ml_vec3f_add(float dst[3], float a[3], float b[3]);
void ml_vec3f_clear(float dst[3]);
float mlNormalizeAngle(float deg);

float time_getDelta(void);

void controller_getRightStick(int controller_index, float dst[2]);

int bainput_should_rotate_camera_left(void);
int bainput_should_rotate_camera_right(void);
int bainput_should_look_first_person_camera(void);

int ncDynamicCamera_getState(void);
void ncDynamicCamera_setState(int state);
void ncDynamicCamera_getPosition(float dst[3]);
void ncDynamicCamera_setRotation(float src[3]);

// Vanilla follow-camera machinery
extern float D_8037DB70;    // dynamicCamB.c: orbit yaw, degrees
extern float D_8037D9C8[3]; // dynamicCamera.c: rotation spring velocity

void func_802C0150(int mode);        // select the focus target
void func_802C0370(void);            // record the pre-move position (anti-flip guard)
void func_802C0394(float target[3]); // record the ideal target (anti-flip guard)
void func_802C03BC(void);            // undo a collision resolve that crossed over
void func_802C0490(float dst[3]);    // focus/orbit center
void func_802C04B0(void);            // re-derive D_8037DB70 from the live camera

float func_802BD8D4(void);                           // target orbit distance (zoom level)
void func_802BE190(float target[3]);                 // position spring toward target
void func_802BE230(float gain, float damp);          // position spring tuning
int func_802BE60C(void);                             // swept camera collision + slide
int func_802BC84C(int mode);                         // occluded-too-long recovery
void func_802BE6FC(float rotOut[3], float focus[3]); // look-at from the live position
}

namespace {
#define CVAR_FREELOOK_ENABLED CVAR_ENHANCEMENT("Camera.FreeLook.Enabled")
#define CVAR_FREELOOK_YAW_SENS CVAR_ENHANCEMENT("Camera.FreeLook.YawSensitivity")
#define CVAR_FREELOOK_PITCH_SENS CVAR_ENHANCEMENT("Camera.FreeLook.PitchSensitivity")
#define CVAR_FREELOOK_INVERT_X CVAR_ENHANCEMENT("Camera.FreeLook.InvertX")
#define CVAR_FREELOOK_INVERT_Y CVAR_ENHANCEMENT("Camera.FreeLook.InvertY")
#define CVAR_FREELOOK_SMOOTH_RATE CVAR_ENHANCEMENT("Camera.FreeLook.SmoothRate")

constexpr float kDeadzone = 0.15f;
constexpr float kEnterThreshold = 0.30f;
constexpr float kYawSpeed = 160.0f;
constexpr float kPitchSpeed = 100.0f;

constexpr float kMinPitch = -85.0f;
constexpr float kMaxPitch = 40.0f;

constexpr float kAimHeightRate = 8.0f;
constexpr float kDefaultSmooth = 40.0f;
constexpr float kSmoothToGain = 0.5f;
constexpr float kDampRatio = 4.0f;

constexpr float kRadToDeg = 57.29577951308232f;

bool sActive = false;
bool sAimValid = false;
float sPitch = 0.0f;
float sAimY = 0.0f;

float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

float SpringGain() {
    return CVarGetFloat(CVAR_FREELOOK_SMOOTH_RATE, kDefaultSmooth) * kSmoothToGain;
}

float ReadStick(float out[2]) {
    controller_getRightStick(0, out);
    float mag = std::sqrt(out[0] * out[0] + out[1] * out[1]);
    if (mag < kDeadzone) {
        out[0] = out[1] = 0.0f;
        return 0.0f;
    }
    if (mag > 1.0f) {
        mag = 1.0f;
    }
    float scaled = (mag - kDeadzone) / (1.0f - kDeadzone);
    float k = scaled / mag;
    out[0] *= k;
    out[1] *= k;
    return scaled;
}

bool ManualCameraControl() {
    return bainput_should_rotate_camera_left() || bainput_should_rotate_camera_right() ||
           bainput_should_look_first_person_camera();
}

float CapturePitch() {
    float camPos[3];
    float focus[3];
    ncDynamicCamera_getPosition(camPos);
    func_802C0490(focus);

    float diff[3] = { camPos[0] - focus[0], camPos[1] - focus[1], camPos[2] - focus[2] };
    float dist = std::sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
    if (dist < 1.0f) {
        return 0.0f;
    }
    return clampf(std::asin(clampf(-diff[1] / dist, -1.0f, 1.0f)) * kRadToDeg, kMinPitch, kMaxPitch);
}

void EnterOrbit() {
    func_802C0150(2);
    func_802BE230(SpringGain(), SpringGain() * kDampRatio);
    func_802C04B0();
    sPitch = CapturePitch();

    ml_vec3f_clear(D_8037D9C8);

    sAimValid = false;
    sActive = true;
    ncDynamicCamera_setState(FREELOOK_CAM_STATE);
}

void ExitOrbit() {
    sActive = false;
    if (ncDynamicCamera_getState() == FREELOOK_CAM_STATE) {
        ncDynamicCamera_setState(0xB);
    }
}

} // namespace

extern "C" int port_freeLook_isEnabled(void) {
    return CVarGetInteger(CVAR_FREELOOK_ENABLED, 0) != 0;
}

extern "C" int port_freeLook_handle(void) {
    if (!port_freeLook_isEnabled()) {
        if (sActive) {
            ExitOrbit();
        }
        return 0;
    }

    int state = ncDynamicCamera_getState();

    if (sActive) {
        if (state != FREELOOK_CAM_STATE) {
            sActive = false;
            return 0;
        }
        if (ManualCameraControl()) {
            ExitOrbit();
            return 0;
        }
        return 1;
    }

    if (ManualCameraControl()) {
        return 0;
    }

    if (state != 0xB && state != 0x1 && state != 0xA) {
        return 0;
    }

    float stick[2];
    if (ReadStick(stick) >= kEnterThreshold) {
        EnterOrbit();
        return 1;
    }

    return 0;
}

extern "C" void port_freeLookCamera_update(void) {
    float dt = time_getDelta();

    float stick[2];
    ReadStick(stick);

    float yawSens = CVarGetFloat(CVAR_FREELOOK_YAW_SENS, 1.0f);
    float pitchSens = CVarGetFloat(CVAR_FREELOOK_PITCH_SENS, 1.0f);
    bool invertX = CVarGetInteger(CVAR_FREELOOK_INVERT_X, 0) != 0;
    bool invertY = CVarGetInteger(CVAR_FREELOOK_INVERT_Y, 0) != 0;

    D_8037DB70 = mlNormalizeAngle(D_8037DB70 + (invertX ? -stick[0] : stick[0]) * kYawSpeed * yawSens * dt);
    sPitch = clampf(sPitch + (invertY ? stick[1] : -stick[1]) * kPitchSpeed * pitchSens * dt, kMinPitch, kMaxPitch);

    float focus[3];
    float offset[3];
    float target[3];

    func_802C0370();
    func_802C0490(focus);
    func_80256E24(offset, sPitch, D_8037DB70, 0.0f, 0.0f, func_802BD8D4());
    ml_vec3f_add(target, focus, offset);

    func_802C0394(target);
    func_802BE190(target);

    if (func_802BE60C()) {
        if (!func_802BC84C(1)) {
            func_802C03BC();
        }
        func_802C04B0();
    }

    func_802C0490(focus);
    if (!sAimValid) {
        sAimY = focus[1];
        sAimValid = true;
    } else {
        sAimY += (focus[1] - sAimY) * clampf(kAimHeightRate * dt, 0.0f, 1.0f);
    }
    focus[1] = sAimY;

    float rot[3];
    func_802BE6FC(rot, focus);
    ncDynamicCamera_setRotation(rot);
}
