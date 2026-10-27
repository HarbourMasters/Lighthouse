// Beak Bomb Aiming

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"
#include "port/UI/Crosshair.h"
#include "BeakBombAiming.h"

#include <algorithm>

extern "C" {
#include "enums.h"
#include "functions.h"
#include "bk_time.h"
#include "core1/ml.h"
#include "core2/anctrl.h"
#include "core2/yaw.h"
#include "core2/ba/physics.h"

void func_802A339C(f32 arg0[3], f32 arg1[3], f32 arg2);
}

#define CVAR_NAME CVAR_ENHANCEMENT("Gameplay.BeakBombAiming")

namespace {

constexpr f32 kHoldPoint = 0.6f;
constexpr f32 kTurnRate = 120.0f;
constexpr f32 kElevationRate = 90.0f;
constexpr f32 kMaxElevation = 80.0f;
constexpr f32 kEyeHeight = 60.0f; // baModel_setYDisplacement in flight
constexpr f32 kEyeAhead = 120.0f;
constexpr s32 kFlyCamera = 4;

f32 sYaw;
f32 sElevation;

void StartAiming(const f32 launch[3]) {
    ml_horizontal_and_vertical_angles(0.0f, 0.0f, 0.0f, launch[0], launch[1], launch[2], &sElevation, &sYaw);
    // Below the horizon comes back as 270 to 360.
    sElevation = std::clamp(sElevation > 180.0f ? sElevation - 360.0f : sElevation, -kMaxElevation, kMaxElevation);
    func_802914CC(BEAK_BOMB_AIM_CAM_STATE);
}

void Steer(f32 launch[3]) {
    const f32 delta = time_getDelta();
    sYaw = mlNormalizeAngle(sYaw - bastick_getX() * kTurnRate * delta);
    sElevation = std::clamp(sElevation - bastick_getY() * kElevationRate * delta, -kMaxElevation, kMaxElevation);
    f32 rotation[3];
    func_802A339C(launch, rotation, 4200.0f);
    yaw_setIdeal(rotation[1] + 180.0f);
    pitch_setIdeal(rotation[0]);
    yaw_applyIdeal();
    pitch_applyIdeal();
    baphysics_set_velocity(launch);
}

} // namespace

extern "C" void BeakBombAiming_UpdateCamera(void) {
    // Hit mid-aim into a state that leaves the camera alone.
    if (bs_getState() != BS_2A_BOMB) {
        func_802914CC(kFlyCamera);
        return;
    }
    f32 position[3];
    f32 ahead[3];
    player_getPosition(position);
    func_80256E24(ahead, -sElevation, sYaw, 0.0f, 0.0f, kEyeAhead);
    ml_vec3f_add(position, position, ahead);
    position[1] += kEyeHeight;
    f32 rotation[3] = { sElevation, mlNormalizeAngle(sYaw + 180.0f), 0.0f };
    ncDynamicCamera_setPosition(position);
    ncDynamicCamera_setRotation(rotation);
}

void RegisterBeakBombAiming_Init() {
    COND_VB_SHOULD(VB_BEAK_BOMB_WINDUP, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NAME, 0), {
        AnimCtrl* windup = va_arg(args, AnimCtrl*);
        f32* launch = va_arg(args, f32*);
        if (IsDemoMode() || !bakey_held(BUTTON_B) || anctrl_getAnimTimer(windup) < kHoldPoint) {
            return;
        }
        if (ncDynamicCamera_getState() != BEAK_BOMB_AIM_CAM_STATE) {
            StartAiming(launch);
        }
        Steer(launch);
        Crosshair::ShowThisFrame();
        anctrl_setAnimTimer(windup, kHoldPoint);
        *should = false;
    });
}

static RegisterShipInitFunc initFunc(RegisterBeakBombAiming_Init, { CVAR_NAME });
