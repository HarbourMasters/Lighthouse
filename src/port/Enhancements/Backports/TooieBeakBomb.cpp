#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "bk_time.h"
#include "core1/ml.h"
#include "core1/viewport.h"
#include "core2/ba/physics.h"

extern f32 D_8037D338[3];
}

#define CVAR_NAME CVAR_ENHANCEMENT("Backports.BeakBomb")

namespace {

constexpr f32 kNudgeSpeed = 1000.0f;

s32 sLastFrame = -2;
f32 sFeatherTimer;

} // namespace

void RegisterTooieBeakBomb_Init() {
    COND_VB_SHOULD(VB_BEAK_BOMB_RANGE_END, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NAME, 0), {
        if (!*should || IsDemoMode() || !bakey_held(BUTTON_B)) {
            return;
        }
        const s32 frame = globalTimer_getTime();
        if (frame - sLastFrame > 1) {
            sFeatherTimer = 0.0f;
        }
        sLastFrame = frame;
        sFeatherTimer -= time_getDelta();
        if (sFeatherTimer <= 0.0f) {
            enum bs_e paid = BS_0_NONE;
            func_80346C10(&paid, BS_0_NONE, BS_1_IDLE, ITEM_F_RED_FEATHER, true);
            if (paid == BS_0_NONE) {
                return;
            }
            func_802D8BE4(false);
            sFeatherTimer = 1.0f;
        }
        *should = false;
    });

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NAME, 0), [](IEvent*) {
        if (IsDemoMode() || bs_getState() != BS_2A_BOMB || !bsbfly_bombHitboxActive()) {
            return;
        }
        f32 rotation[3];
        viewport_getRotation_vec3f(rotation);
        f32 nudge[3];
        ml_vec3f_assign(nudge, bastick_getX() * kNudgeSpeed, -bastick_getY() * kNudgeSpeed, 0.0f);
        ml_vec3f_pitch_rotate_copy(nudge, nudge, rotation[0]);
        ml_vec3f_yaw_rotate_copy(nudge, nudge, rotation[1]);
        f32 velocity[3];
        ml_vec3f_add(velocity, D_8037D338, nudge);
        baphysics_set_velocity(velocity);
    });
}

static RegisterShipInitFunc initFunc(RegisterTooieBeakBomb_Init, { CVAR_NAME });
