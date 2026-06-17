// Modern control scheme camera

#include <SDL2/SDL.h>

#include <ship/Context.h>
#include <ship/controller/controldeck/ControlDeck.h>
#include <ship/controller/physicaldevice/ConnectedPhysicalDeviceManager.h>
#include <libultraship/bridge.h>

#include "port/UI/cvar_prefixes.h"
#include "ControlSchemes.h"
#include "ModernCamera.h"

extern "C" {
extern unsigned char D_8037C061; // current zoom level: 1 (close) .. 3 (far)
extern int D_8037C07C;           // level-3 distance x; 0 means level 3 is unavailable

void func_80290B60(int level); // set the zoom level
float batimer_get(int id);     // timer value (id 7 = zoom cooldown)
void batimer_set(int id, float t);
void basfx_80299D2C(int sfxId, float pitch, int volume); // play a camera SFX
int ncDynamicCamA_func_802C1DB0(float degrees);          // rotate camera by `degrees`, orbiting at zoom distance
float time_getDelta(void);
int bs_getState(void);
int balookat_getState(void);    // nonzero while the look-around camera is active
int player_movementGroup(void); // enum bsgroup_e

void FrameInterpolation_DontInterpolateCamera(void);
}

#include "enums.h" // BS_CROUCH
#include "port/ShipUtils.h"

namespace {

bool ModernSchemeActive() {
    return CVarGetInteger(CVAR_SETTING("Controls.Scheme"), CONTROL_SCHEME_RETRO) == CONTROL_SCHEME_MODERN;
}

void ReadRightStick(int32_t& outX, int32_t& outY) {
    outX = 0;
    outY = 0;
    auto ctx = Ship::Context::GetRawInstance();
    auto controlDeck = ctx != nullptr ? ctx->GetControlDeck() : nullptr;
    auto deviceManager = controlDeck != nullptr ? controlDeck->GetConnectedPhysicalDeviceManager() : nullptr;
    if (deviceManager == nullptr) {
        return;
    }
    for (auto& [instanceId, gamepad] : deviceManager->GetConnectedSDLGamepadsForPort(0)) {
        if (gamepad == nullptr) {
            continue;
        }
        int32_t x = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_RIGHTX);
        int32_t y = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_RIGHTY);
        if ((x < 0 ? -x : x) > (outX < 0 ? -outX : outX)) {
            outX = x;
        }
        if ((y < 0 ? -y : y) > (outY < 0 ? -outY : outY)) {
            outY = y;
        }
    }
}

constexpr float kStickMax = 32767.0f;
constexpr float kYawDeadzone = 0.2f;    // fraction of full deflection
constexpr float kYawSpeed = 600.0f;     // degrees per second at full deflection
constexpr int32_t kZoomOn = 16000;      // ~49% to step zoom
constexpr int32_t kZoomOff = 7000;      // must return inside this to re-arm
constexpr float kYawSnapDegrees = 4.0f; // per-frame turn above this snaps

} // namespace

extern "C" int port_modernCamera_handleYaw(void) {
    if (!ModernSchemeActive() || bs_getState() == BS_CROUCH || IsDemoMode()) {
        return 0;
    }

    int32_t sx = 0;
    int32_t sy = 0;
    ReadRightStick(sx, sy);

    float x = sx / kStickMax;
    if (x > 1.0f) {
        x = 1.0f;
    } else if (x < -1.0f) {
        x = -1.0f;
    }

    if (x > -kYawDeadzone && x < kYawDeadzone) {
        return 0;
    }

    // Rescale past the deadzone so rotation ramps from 0 at the edge to full speed
    float scaled = (x > 0.0f) ? (x - kYawDeadzone) / (1.0f - kYawDeadzone) : (x + kYawDeadzone) / (1.0f - kYawDeadzone);
    float inc = scaled * kYawSpeed * time_getDelta();

    if (inc > kYawSnapDegrees || inc < -kYawSnapDegrees) {
        FrameInterpolation_DontInterpolateCamera();
    }

    ncDynamicCamA_func_802C1DB0(inc);
    return 1;
}

extern "C" int port_camera_suppressVanillaZoom(void) {
    // Demo modes supply their own camera inputs; never suppress them, or the recorded
    // zoom/swivel won't play back correctly.
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

    int32_t sx = 0;
    int32_t sy = 0;
    ReadRightStick(sx, sy);

    static bool sArmed = true;
    int32_t asy = (sy < 0) ? -sy : sy;

    if (sArmed && asy > kZoomOn && batimer_get(7) <= 0.0f) {
        int32_t level = D_8037C061;
        int32_t next = level;
        if (sy > 0) {
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

    if (asy < kZoomOff) {
        sArmed = true;
    }
}
