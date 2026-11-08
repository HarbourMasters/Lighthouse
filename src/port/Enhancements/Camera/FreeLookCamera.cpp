// Free look camera

#include <chrono>
#include <cmath>

#include <libultraship/bridge/consolevariablebridge.h>
#include <ship/Context.h>
#include <ship/window/Window.h>

#include "port/UI/cvar_prefixes.h"
#include "port/Controller/ControlSchemes.h"
#include "port/Enhancements/Camera/FreeLookCamera.h"
#include "port/Enhancements/Camera/CameraShared.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" int globalTimer_getTime(void);

namespace {
using namespace PortCamera;

#define CVAR_FREELOOK_ENABLED CVAR_ENHANCEMENT("Camera.FreeLook.Enabled")
#define CVAR_FREELOOK_YAW_SENS CVAR_ENHANCEMENT("Camera.FreeLook.YawSensitivity")
#define CVAR_FREELOOK_PITCH_SENS CVAR_ENHANCEMENT("Camera.FreeLook.PitchSensitivity")
#define CVAR_FREELOOK_INVERT_X CVAR_ENHANCEMENT("Camera.FreeLook.InvertX")
#define CVAR_FREELOOK_INVERT_Y CVAR_ENHANCEMENT("Camera.FreeLook.InvertY")
#define CVAR_FREELOOK_SMOOTH_RATE CVAR_ENHANCEMENT("Camera.FreeLook.SmoothRate")
#define CVAR_FREELOOK_MOUSE CVAR_ENHANCEMENT("Camera.FreeLook.Mouse")
#define CVAR_FREELOOK_MOUSE_SENS CVAR_ENHANCEMENT("Camera.FreeLook.MouseSensitivity")
#define CVAR_FREELOOK_MOUSE_INVERT_X CVAR_ENHANCEMENT("Camera.FreeLook.MouseInvertX")
#define CVAR_FREELOOK_MOUSE_INVERT_Y CVAR_ENHANCEMENT("Camera.FreeLook.MouseInvertY")
#define CVAR_FREELOOK_EVERYWHERE CVAR_ENHANCEMENT("Camera.FreeLook.Everywhere")

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

// Mouse free look turns the camera by a fixed angle per pixel the mouse moves (not a speed, like
// the stick), so a given hand movement always turns it the same amount: degrees per pixel at 100%
// Mouse Sensitivity.
constexpr float kMouseDegreesPerPixel = 0.2f;
constexpr int kDefaultMouseSensitivity = 50;
// Mouse movement older than this when the camera next asks (a cutscene, a menu, a load) is dropped
// rather than turning the camera all at once.
constexpr double kMouseStaleSeconds = 0.1;

bool sActive = false;
bool sAimValid = false;
float sPitch = 0.0f;
float sAimY = 0.0f;

float sMouseYaw = 0.0f; // degrees the mouse has turned since the last camera update
float sMousePitch = 0.0f;
bool sMouseCaptureApplied = false;
int sPolledFrame = -1; // the game frame the mouse was last read in
bool sMouseMovedThisFrame = false;

float SpringGain() {
    return CVarGetFloat(CVAR_FREELOOK_SMOOTH_RATE, kDefaultSmooth) * kSmoothToGain;
}

float ReadStick(float out[2]) {
    ControlSchemes_GetRightStick(out);
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

// --- mouse -------------------------------------------------------------------------------------

bool MouseEnabled() {
    return port_freeLook_isEnabled() && CVarGetInteger(CVAR_FREELOOK_MOUSE, 0) != 0;
}

// Keeps the window's mouse capture in step with the setting: the mouse is captured (pointer hidden,
// raw movement read) while playing. libultraship frees it while a menu is open and F2 toggles it.
void SyncMouseCapture() {
    const bool want = MouseEnabled();
    if (want == sMouseCaptureApplied) {
        return;
    }
    sMouseCaptureApplied = want;
    auto window = Ship::Context::GetRawInstance()->GetWindow();
    window->SetAutoCaptureMouse(want);
    window->GetMouseStateManager()->UpdateMouseCapture();
}

void ClearMouse() {
    sMouseYaw = 0.0f;
    sMousePitch = 0.0f;
}

// Adds the mouse's movement since the last call to sMouseYaw/sMousePitch. Returns true if it moved.
bool PollMouse() {
    using Clock = std::chrono::steady_clock;
    static Clock::time_point sLastPoll;
    auto window = Ship::Context::GetRawInstance()->GetWindow();
    const Ship::Coords delta = window->GetMouseDelta(); // read every time, so movement doesn't pile up
    const Clock::time_point now = Clock::now();
    const bool stale = std::chrono::duration<double>(now - sLastPoll).count() > kMouseStaleSeconds;
    sLastPoll = now;
    if (!MouseEnabled() || !window->IsMouseCaptured() || stale || (delta.x == 0 && delta.y == 0)) {
        return false;
    }
    const float perPixel =
        kMouseDegreesPerPixel * CVarGetInteger(CVAR_FREELOOK_MOUSE_SENS, kDefaultMouseSensitivity) / 100.0f;
    const bool invertX = CVarGetInteger(CVAR_FREELOOK_MOUSE_INVERT_X, 0) != 0;
    const bool invertY = CVarGetInteger(CVAR_FREELOOK_MOUSE_INVERT_Y, 0) != 0;
    // Mouse right turns the view right (the orbit yaw goes down); mouse up looks up (the pitch goes up,
    // taking the camera lower).
    sMouseYaw += -delta.x * perPixel * (invertX ? -1.0f : 1.0f);
    sMousePitch += -delta.y * perPixel * (invertY ? -1.0f : 1.0f);
    return true;
}

// Reads the mouse once per game frame (the swivel check and the fixed-camera checks all ask).
bool MouseMovedThisFrame() {
    const int frame = globalTimer_getTime();
    if (frame != sPolledFrame) {
        sPolledFrame = frame;
        sMouseMovedThisFrame = PollMouse();
    }
    return sMouseMovedThisFrame;
}

// --- entering and leaving ---------------------------------------------------------------------

bool Everywhere() {
    return CVarGetInteger(CVAR_FREELOOK_EVERYWHERE, 1) != 0;
}

// Camera states free look can take over from: the follow cameras, and with Free Look Everywhere,
// the underwater camera (3) and the fixed cameras of camera-node areas (8: pivot, 0x11: zoom).
bool CanEnterFrom(int state) {
    if (state == 0xB || state == 0x1 || state == 0xA) {
        return true;
    }
    return Everywhere() && (state == 0x3 || state == 0x8 || state == 0x11);
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

bool HandleInput() {
    SyncMouseCapture();
    const bool mouseMoved = MouseMovedThisFrame();

    if (!port_freeLook_isEnabled()) {
        if (sActive) {
            ExitOrbit();
        }
        ClearMouse();
        return false;
    }

    int state = ncDynamicCamera_getState();

    if (sActive) {
        if (state != FREELOOK_CAM_STATE) {
            sActive = false;
            ClearMouse();
            return false;
        }
        if (ManualCameraControl()) {
            ExitOrbit();
            ClearMouse();
            return false;
        }
        return true;
    }

    if (ManualCameraControl()) {
        ClearMouse();
        return false;
    }

    if (!CanEnterFrom(state)) {
        ClearMouse();
        return false;
    }

    // The stick takes over past a threshold; any mouse movement takes over at once.
    float stick[2];
    if (ReadStick(stick) >= kEnterThreshold || mouseMoved) {
        EnterOrbit();
        return true;
    }

    ClearMouse();
    return false;
}

// Free Look Everywhere: asked by the underwater camera and the camera-node (fixed camera) areas
// before they take the camera. True while free look holds the camera, or when the stick or mouse
// asks for it this frame, so they leave it to free look. A C-button camera control hands it back.
bool TakesOverFixedCamera() {
    if (!port_freeLook_isEnabled() || !Everywhere()) {
        return false;
    }
    if (sActive && ncDynamicCamera_getState() == FREELOOK_CAM_STATE) {
        return !ManualCameraControl();
    }
    if (ManualCameraControl()) {
        return false;
    }
    float stick[2];
    return ReadStick(stick) >= kEnterThreshold || MouseMovedThisFrame();
}

void UpdateOrbit() {
    float dt = time_getDelta();

    float stick[2];
    ReadStick(stick);

    float yawSens = CVarGetFloat(CVAR_FREELOOK_YAW_SENS, 1.0f);
    float pitchSens = CVarGetFloat(CVAR_FREELOOK_PITCH_SENS, 1.0f);
    bool invertX = CVarGetInteger(CVAR_FREELOOK_INVERT_X, 0) != 0;
    bool invertY = CVarGetInteger(CVAR_FREELOOK_INVERT_Y, 0) != 0;

    D_8037DB70 = mlNormalizeAngle(D_8037DB70 + (invertX ? -stick[0] : stick[0]) * kYawSpeed * yawSens * dt);
    sPitch = clampf(sPitch + (invertY ? stick[1] : -stick[1]) * kPitchSpeed * pitchSens * dt, kMinPitch, kMaxPitch);

    // The mouse: the angle it moved since the last update, whatever the frame time.
    D_8037DB70 = mlNormalizeAngle(D_8037DB70 + sMouseYaw);
    sPitch = clampf(sPitch + sMousePitch, kMinPitch, kMaxPitch);
    ClearMouse();

    float focus[3];
    float offset[3];
    float target[3];

    func_802C0370();
    func_802C0490(focus);
    func_80256E24(offset, sPitch, D_8037DB70, 0.0f, 0.0f, func_802BD8D4());
    ml_vec3f_add(target, focus, offset);

    func_802C0394(target);
    func_802BE190(target);

    // Keep the camera out of walls, but don't use the follow camera's line-of-sight rescue
    // (func_802BC84C): when the player has been hidden from the camera for a few frames it jumps the
    // camera to a spot beside them, and free look pulls it straight back to where the player put it,
    // over and over, so the view (and the player in it) kept jumping and snapping back.
    if (func_802BE60C()) {
        func_802C03BC();
        func_802C04B0();
    }

    func_802C0490(focus);
    focus[1] = SmoothTowards(sAimY, sAimValid, focus[1], kAimHeightRate, dt);

    float rot[3];
    func_802BE6FC(rot, focus);
    ncDynamicCamera_setRotation(rot);
}

} // namespace

extern "C" int port_freeLook_isEnabled(void) {
    return CVarGetInteger(CVAR_FREELOOK_ENABLED, 0) != 0;
}

void RegisterFreeLookCamera_Init() {
    // Registered whatever the setting, so turning it off mid-orbit still hands the camera back. Low, so it runs
    // before ModernCamera's swivel listener, which defers to it through *should.
    COND_VB_SHOULD(VB_CAMERA_SWIVEL, EVENT_PRIORITY_LOW, true, {
        if (HandleInput()) {
            *should = false;
        }
    });
    COND_HOOK(OnDynamicCameraUpdate, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        if (((OnDynamicCameraUpdate*)event)->state == FREELOOK_CAM_STATE) {
            UpdateOrbit();
        }
    });
    // Free Look Everywhere: the fixed-camera areas and the underwater camera leave the camera to free look.
    COND_VB_SHOULD(VB_CAMERA_FIXED_NODE, EVENT_PRIORITY_NORMAL, true, {
        if (TakesOverFixedCamera()) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_CAMERA_UNDERWATER, EVENT_PRIORITY_NORMAL, true, {
        if (TakesOverFixedCamera()) {
            *should = false;
        }
    });
}

static RegisterShipInitFunc initFreeLookCamera(RegisterFreeLookCamera_Init, {});
