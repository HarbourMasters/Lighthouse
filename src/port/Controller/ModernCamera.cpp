// Modern control scheme camera

#include <cmath>

#include <libultraship/bridge.h>

#include "port/UI/cvar_prefixes.h"
#include "ControlSchemes.h"
#include "ModernCamera.h"
#include "port/Enhancements/Camera/FreeLookCamera.h"

extern "C" {
// Math helpers
void func_80256E24(float dst[3], float pitch, float yaw, float x, float y, float z);
void ml_vec3f_add(float dst[3], float a[3], float b[3]);
void ml_vec3f_clear(float dst[3]);
float mlNormalizeAngle(float deg);

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
void ncDynamicCamera_setState(int state);
void ncDynamicCamera_setRotation(float src[3]);
int func_8029147C(void);       // current ba camera mode
void func_80291488(int mode);  // set the ba camera mode
void func_802914CC(int state); // force a dynamic camera state (mode 1)

int bainput_should_rotate_camera_left(void);
int bainput_should_rotate_camera_right(void);
int bainput_should_look_first_person_camera(void);

void controller_getRightStick(int controller_index, float dst[2]);

// Vanilla follow-camera machinery
extern float D_8037DB70;    // dynamicCamB.c: orbit yaw, degrees
extern float D_8037D9C8[3]; // dynamicCamera.c: rotation spring velocity

void func_802C0150(int mode);        // select the focus target
void func_802C0370(void);            // record the pre-move position (anti-flip guard)
void func_802C0394(float target[3]); // record the ideal target (anti-flip guard)
void func_802C03BC(void);            // undo a collision resolve that crossed over
void func_802C0490(float dst[3]);    // focus/orbit center
void func_802C04B0(void);            // re-derive D_8037DB70 from the live camera

float func_802BD51C(void);                           // target camera height
float func_802BD8D4(void);                           // target orbit distance (zoom level)
void func_802BE190(float target[3]);                 // position spring toward target
void func_802BE230(float gain, float damp);          // position spring tuning
int func_802BE60C(void);                             // swept camera collision + slide
int func_802BC84C(int mode);                         // occluded-too-long recovery
void func_802BE6FC(float rotOut[3], float focus[3]); // look-at from the live position
}

#include "enums.h" // BS_7_CROUCH
#include "port/ShipUtils.h"

namespace {

bool ModernSchemeActive() {
    return CVarGetInteger(CVAR_SETTING("Controls.Scheme"), CONTROL_SCHEME_RETRO) == CONTROL_SCHEME_MODERN &&
           !port_freeLook_isEnabled() && port_rightStickIsMapped();
}

constexpr int kClimbCamState = 0x10;
constexpr int kSwivelCamMode = 8;
constexpr int kFollowCamMode = 2;
constexpr float kYawDeadzone = 0.2f;
constexpr float kYawEnter = 0.3f;
constexpr float kYawSpeed = 160.0f;
constexpr float kZoomOn = 0.49f;
constexpr float kZoomOff = 0.21f;
constexpr float kPosSpringGain = 20.0f;
constexpr float kPosSpringDamp = 80.0f;
constexpr float kHeightRate = 3.0f;
constexpr float kAimHeightRate = 8.0f;

bool sActive = false;
bool sHeightValid = false;
bool sAimValid = false;
float sHeight = 0.0f;
float sAimY = 0.0f;

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

bool Climbing() {
    return player_movementGroup() == BSGROUP_5_CLIMB;
}

// Give the mode back
void ReleaseSwivelMode() {
    if (func_8029147C() != kSwivelCamMode) {
        return;
    }
    if (Climbing()) {
        func_802914CC(kClimbCamState);
    } else {
        func_80291488(kFollowCamMode);
    }
}

// Take the camera over. Everything the vanilla follow camera would set up on init is
// set up here, except that the position spring velocity is deliberately left alone.
void EnterOrbit() {
    func_802C0150(2);
    func_802BE230(kPosSpringGain, kPosSpringDamp);
    func_802C04B0();

    // We drive the rotation directly, so nothing services the rotation spring while we
    // own the camera. Park its velocity so it cannot kick the aim when camB resumes.
    ml_vec3f_clear(D_8037D9C8);

    sHeightValid = false;
    sAimValid = false;
    sActive = true;
    ncDynamicCamera_setState(MODERN_ORBIT_CAM_STATE);
}

void ExitOrbit() {
    sActive = false;
    if (ncDynamicCamera_getState() == MODERN_ORBIT_CAM_STATE) {
        ncDynamicCamera_setState(0xB);
    }
    ReleaseSwivelMode();
}

} // namespace

extern "C" int port_modernCamera_handleYaw(void) {
    bool schemeOk = ModernSchemeActive() && bs_getState() != BS_7_CROUCH && !IsDemoMode();
    if (!schemeOk) {
        if (sActive) {
            ExitOrbit();
        }
        return 0;
    }

    int state = ncDynamicCamera_getState();

    if (sActive) {
        // A scripted/special camera took the state out from under us; drop the
        // orbit and let that camera run.
        if (state != MODERN_ORBIT_CAM_STATE) {
            sActive = false;
            return 0;
        }
        if (ManualCameraControl()) {
            ExitOrbit();
            return 0;
        }
        if (!Climbing()) {
            ReleaseSwivelMode();
        }
        return 1;
    }

    float x;
    float y;
    ReadStickNorm(x, y);
    if (std::fabs(x) > kYawEnter) {
        EnterOrbit();
        if (state == kClimbCamState) {
            func_80291488(kSwivelCamMode);
        }
        return 1;
    }
    return 0;
}

extern "C" void port_modernCamera_update(void) {
    float dt = time_getDelta();

    // Drive yaw from the stick. Freeze it during look-around so it doesn't drift
    // while the first-person view overrides the camera.
    if (!balookat_getState()) {
        float x;
        float y;
        ReadStickNorm(x, y);
        D_8037DB70 = mlNormalizeAngle(D_8037DB70 + YawInput(x) * kYawSpeed * dt * port_cameraInvertXSign());
    }

    // From here down this is ncDynamicCamB_update with our own yaw and height filter.
    float focus[3];
    float offset[3];
    float target[3];

    func_802C0370();
    func_802C0490(focus);
    func_80256E24(offset, 0.0f, D_8037DB70, 0.0f, 0.0f, func_802BD8D4());
    ml_vec3f_add(target, focus, offset);

    float wantHeight = func_802BD51C();
    if (!sHeightValid) {
        sHeight = wantHeight;
        sHeightValid = true;
    } else {
        sHeight += (wantHeight - sHeight) * clampf(kHeightRate * dt, 0.0f, 1.0f);
    }
    target[1] = sHeight;

    func_802C0394(target);
    func_802BE190(target);

    if (func_802BE60C()) {
        // Occluded too long: func_802BC84C relocates the camera outright. Otherwise
        // discard a resolve that pushed the camera back the way it came.
        if (!func_802BC84C(1)) {
            func_802C03BC();
        }
        func_802C04B0(); // the camera slid along geometry; carry the orbit yaw with it
    }

    // Aim straight at the focus from wherever the spring left the camera. The position
    // is already smooth, so the look-at inherits that smoothness exactly and the player
    // stays centered.
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

extern "C" int port_camera_suppressVanillaZoom(void) {
    if (IsDemoMode() || port_freeLook_isEnabled()) {
        return 0;
    }
    int scheme = CVarGetInteger(CVAR_SETTING("Controls.Scheme"), CONTROL_SCHEME_RETRO);
    if (scheme == CONTROL_SCHEME_MODERN) {
        return port_rightStickIsMapped();
    }
    if (scheme == CONTROL_SCHEME_POCKET && bs_getState() == BS_7_CROUCH) {
        return 1;
    }
    return 0;
}

extern "C" void port_modernCamera_handleZoom(void) {
    if (!ModernSchemeActive() || bs_getState() == BS_7_CROUCH || IsDemoMode()) {
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
