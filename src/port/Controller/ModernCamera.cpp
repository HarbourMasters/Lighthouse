// Modern control scheme camera

#include <libultraship/bridge.h>

#include "port/UI/cvar_prefixes.h"
#include "ControlSchemes.h"
#include "ModernCamera.h"

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

// Vanilla camera / math entry points used to drive the orbit state.
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
float func_802BD8D4(void);                                                           // target orbit distance (zoom level)
float func_802BD51C(void);                                                           // target camera height

float mlNormalizeAngle(float deg);
float gu_sqrtf(float x);

int bainput_should_rotate_camera_left(void);
int bainput_should_rotate_camera_right(void);
int bainput_should_look_first_person_camera(void);

void controller_getRightStick(int controller_index, float dst[2]);

void FrameInterpolation_DontInterpolateCamera(void);
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
constexpr float kYawSnapDegrees = 4.0f;
constexpr float kDistanceRate = 8.0f; // 1/sec; how fast distance chases the zoom-level target
// 1/sec; vertical follow. Matched to the vanilla camera, whose Y lerp is target*dt*2, so the
// camera stays mostly put during a jump (Banjo rises in frame) instead of choppily chasing the
// parabola up and back down.
constexpr float kHeightRate = 2.0f;
constexpr float kPosSmoothRate = 40.0f;

// Orbit state. Captured on entry, then yaw is driven continuously by the stick
// while distance/height track the vanilla zoom-level targets.
bool sOrbitActive = false;
bool sJustEntered = false;
float sYaw = 0.0f;
float sDistance = 300.0f;
float sHeight = 0.0f;
bool sSmoothValid = false;
float sSmoothPos[3] = { 0.0f, 0.0f, 0.0f };

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

void CaptureFromCurrentCamera() {
    float camPos[3];
    float center[3];
    ncDynamicCamera_getPosition(camPos);
    func_802C02D4(center);

    float diff[3] = { camPos[0] - center[0], camPos[1] - center[1], camPos[2] - center[2] };
    sDistance = gu_sqrtf(diff[0] * diff[0] + diff[2] * diff[2]);
    sHeight = camPos[1];
    sYaw = 0.0f;
    func_8025801C(diff, &sYaw);

    sJustEntered = true;
    sSmoothValid = false;
}

void ExitOrbit() {
    sOrbitActive = false;
    if (ncDynamicCamera_getState() == MODERN_ORBIT_CAM_STATE) {
        ncDynamicCamera_setState(0xB);
    }
}

} // namespace

extern "C" int port_modernCamera_handleYaw(void) {
    bool schemeOk = ModernSchemeActive() && bs_getState() != BS_CROUCH && !IsDemoMode();
    if (!schemeOk) {
        if (sOrbitActive) {
            ExitOrbit();
        }
        return 0;
    }

    int state = ncDynamicCamera_getState();

    if (sOrbitActive) {
        // A scripted/special camera took the state out from under us; drop the
        // orbit and let that camera run.
        if (state != MODERN_ORBIT_CAM_STATE) {
            sOrbitActive = false;
            return 0;
        }
        if (ManualCameraControl()) {
            ExitOrbit();
            return 0;
        }
        return 1;
    }

    float x;
    float y;
    ReadStickNorm(x, y);
    if (x > kYawEnter || x < -kYawEnter) {
        CaptureFromCurrentCamera();
        sOrbitActive = true;
        ncDynamicCamera_setState(MODERN_ORBIT_CAM_STATE);
        return 1;
    }
    return 0;
}

extern "C" void port_modernCamera_update(void) {
    float dt = time_getDelta();
    float center[3];
    func_802C02D4(center);

    // Drive yaw from the stick. Freeze it during look-around so it doesn't drift
    // while the first-person view overrides the camera.
    if (!sJustEntered && !balookat_getState()) {
        float x;
        float y;
        ReadStickNorm(x, y);
        float inc = -YawInput(x) * kYawSpeed * dt;
        if (inc > kYawSnapDegrees || inc < -kYawSnapDegrees) {
            FrameInterpolation_DontInterpolateCamera();
        }
        sYaw = mlNormalizeAngle(sYaw + inc);
    }
    sJustEntered = false;

    // Track the vanilla zoom-level distance/height so the right-stick zoom still
    // works smoothly while orbiting. Height tracks more slowly than distance so
    // vertical motion (jumps) reads as natural rather than choppy.
    float distTrack = clampf(kDistanceRate * dt, 0.0f, 1.0f);
    float heightTrack = clampf(kHeightRate * dt, 0.0f, 1.0f);
    sDistance += (func_802BD8D4() - sDistance) * distTrack;
    sHeight += (func_802BD51C() - sHeight) * heightTrack;

    float offset[3];
    func_80256E24(offset, 0.0f, sYaw, 0.0f, 0.0f, sDistance);
    float pos[3] = { center[0] + offset[0], sHeight, center[2] + offset[2] };
    ncDynamicCamera_setPosition(pos);

    // Resolve geometry.
    func_802BE60C();

    float resolved[3];
    ncDynamicCamera_getPosition(resolved);

    if (!sSmoothValid) {
        sSmoothPos[0] = resolved[0];
        sSmoothPos[1] = resolved[1];
        sSmoothPos[2] = resolved[2];
        sSmoothValid = true;
    } else {
        float f = clampf(kPosSmoothRate * dt, 0.0f, 1.0f);
        sSmoothPos[0] += (resolved[0] - sSmoothPos[0]) * f;
        sSmoothPos[1] += (resolved[1] - sSmoothPos[1]) * f;
        sSmoothPos[2] += (resolved[2] - sSmoothPos[2]) * f;
    }
    ncDynamicCamera_setPosition(sSmoothPos);

    float rot[3];
    func_802BC434(rot, center, sSmoothPos);
    ncDynamicCamera_setRotation(rot);
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
