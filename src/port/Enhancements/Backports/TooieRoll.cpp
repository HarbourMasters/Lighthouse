// Tooie Roll
//
// Backports Banjo-Tooie's roll (banjo-tooie src/overlays/bs/twirl.c)

#include "core2/bonetransform.h"

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "animation.h"
#include "core2/anctrl.h"
#include "core2/animationfile.h"
#include "core2/anim/commoncache.h"
#include "core2/yaw.h"
#include "core2/ba/physics.h"

void baanim_setModifyMethod(void (*arg0)(uintptr_t, uintptr_t));
}

#define CVAR_NAME CVAR_ENHANCEMENT("Backports.Roll")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

namespace {

constexpr f32 kRollDuration = 0.9f;
constexpr f32 kHandoff = 0.8011f;

// Tooie's roll keeps Banjo's arms tucked, elbows bent, right up to the handoff, and the run blends in
// from there. BK's tucks them the same way, then flings them out from 0.6 to stand up, and blending
// the run in from that flourish whips one arm over his shoulder. Hold the arms at BK's own tucked pose
// instead while the rest of the roll plays.
constexpr f32 kArmsTucked = 0.6f;
constexpr s32 kArmBones[] = { 8, 14, 16, 22, 28, 30 };

BoneTransformList* sTuckedPose = nullptr;
bool sHoldingArms = false;

void ReleaseArms() {
    if (sHoldingArms) {
        sHoldingArms = false;
        baanim_setModifyMethod(nullptr);
    }
}

void HoldArms(uintptr_t arg0, uintptr_t arg1) {
    Animation* anim = anctrl_getAnimPtr(baanim_getAnimCtrlPtr());
    // Once the next animation starts, its blend has already taken the held pose as its start.
    if (anim_getIndex(anim) != ASSET_4F_ANIM_BSTWIRL) {
        ReleaseArms();
        return;
    }
    if (anim_getTimer(anim) < kArmsTucked) {
        return;
    }
    auto* pose = reinterpret_cast<BoneTransformList*>(arg0);
    for (s32 bone : kArmBones) {
        pose->ptr[bone] = sTuckedPose->ptr[bone];
    }
}

} // namespace

void RegisterTooieRoll_Init() {
    ReleaseArms();

    COND_HOOK(OnRollInit, EVENT_PRIORITY_NORMAL, CVAR, [](IEvent* event) {
        yaw_setUpdateState(YAW_STATE_3_BOUNDED);
        yaw_setVelocityBounded(300.0f, 6.5f);
        func_8029957C(1);
        baphysics_set_type(BA_PHYSICS_NORMAL);

        if (sTuckedPose == nullptr) {
            sTuckedPose = boneTransformList_new();
        }
        animationfilebin_getBoneTransformList(animcommoncache_get(ASSET_4F_ANIM_BSTWIRL), kArmsTucked, sTuckedPose);
        sHoldingArms = true;
        baanim_setModifyMethod(HoldArms);
    });

    // Vanilla brakes here and plays the stand-up at 2.5s. Tooie hands off to landing instead, which goes
    // straight into a walk or run while the stick is held and otherwise lets the roll play out.
    COND_HOOK(OnRollUpdate, EVENT_PRIORITY_NORMAL, CVAR, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnRollUpdate*>(event);
        AnimCtrl* aCtrl = baanim_getAnimCtrlPtr();
        if (anctrl_isAt(aCtrl, kHandoff)) {
            anctrl_setDuration(aCtrl, kRollDuration);
            *ev->nextState = BS_20_LANDING;
        }
    });

    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, CVAR, [](IEvent* event) { ReleaseArms(); });
}

static RegisterShipInitFunc initFunc(RegisterTooieRoll_Init, { CVAR_NAME });
