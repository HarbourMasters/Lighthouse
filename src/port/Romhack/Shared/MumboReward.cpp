#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/RomhackConfig.h"
#include "HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "actor.h"
#include "core2/timedfunc.h"
#include "bk_time.h"

extern f32 cameraPosition[3];
extern f32 cameraRotation[3];
extern f32 D_8037D948[3];
extern f32 D_8037D9C8[3];
extern f32 D_8037D9E0[3];
extern f32 D_8037D9D4, D_8037D9D8, D_8037D9EC, D_8037D9F0;
}

namespace {
constexpr f32 kJiggyDelay = 4.15f;
constexpr f32 kStateDelay = 3.85f;
constexpr f32 kStateValue = 1.1f;
constexpr f32 kCameraReleaseDelay = 0.5f;
constexpr s32 kRewardJiggy = 3;
constexpr s32 kCameraMotor = 1;
constexpr s32 kLeaveHutText = 0xA7E;
constexpr s32 kLeaveHutFlags = 0x02;
// Shown instead of the whole purchase path once the Jiggy has already been taken.
constexpr s32 kAlreadyRewardedText = 0xA81;
constexpr s32 kAlreadyRewardedFlags = 0x02;

f32 sJiggyPos[3] = { 0.0f, 500.0f, -95.0f };
f32 sMumboState = 0.0f;
bool sMumboRewardEnabled = false;
bool sLeaveHutPending = false;

void MumboReward_spawnJiggy() {
    jiggy_spawn((enum jiggy_e)kRewardJiggy, sJiggyPos);
    func_802BB3DC(kCameraMotor, 3.0f, 1.0f);
    timedFunc_set_1(kCameraReleaseDelay, (GenFunction_1)func_802BB41C, kCameraMotor);
}

void MumboReward_setState() {
    sMumboState = kStateValue;
}
} // namespace

extern "C" s32 romhack_mumboTransform(s32 transformId) {
    if (!sMumboRewardEnabled || gsworld_getMap() != MAP_48_FP_MUMBOS_SKULL) {
        return player_transform((enum transformation_e)transformId);
    }
    timedFunc_set_0(kJiggyDelay, MumboReward_spawnJiggy);
    timedFunc_set_0(kStateDelay, MumboReward_setState);
    sLeaveHutPending = true;
    return 1;
}

extern "C" s32 romhack_mumboWishwashyId(void) {
    return sMumboRewardEnabled ? 9 : TRANSFORM_7_WISHWASHY;
}

extern "C" s32 romhack_mumboRandomEventsAllowed(void) {
    return sMumboRewardEnabled ? 0 : 1;
}

// Mumbo's reward shows a static camera
namespace {
constexpr f32 kPanPosition[3] = { -250.0f, 194.0f, 147.0f };
constexpr f32 kPanRotation[3] = { 35.0f, 315.0f, 0.0f };

bool sPanSaved = false;
f32 sPanSavedPosition[3];
f32 sPanSavedRotation[3];

void MumboReward_updatePendingDialog() {
    if (!sLeaveHutPending) {
        return;
    }
    const s32 state = bs_getState();
    if (state == BS_74_UNKNOWN || state == BS_20_LANDING || state == BS_44_JIG_JIGGY) {
        return;
    }
    gcdialog_showDialog(kLeaveHutText, kLeaveHutFlags, NULL, NULL, NULL, NULL);
    sLeaveHutPending = false;
}

void MumboReward_updateCamera() {
    if (!sMumboRewardEnabled) {
        return;
    }
    MumboReward_updatePendingDialog();
    if (sMumboState > 0.0f) {
        if (!sPanSaved) {
            sPanSaved = true;
            for (int i = 0; i < 3; i++) {
                sPanSavedPosition[i] = cameraPosition[i];
                sPanSavedRotation[i] = cameraRotation[i];
            }
        }
        for (int i = 0; i < 3; i++) {
            cameraPosition[i] = kPanPosition[i];
            cameraRotation[i] = kPanRotation[i];
            D_8037D948[i] = kPanPosition[i];
            D_8037D9C8[i] = 0.0f;
            D_8037D9E0[i] = 0.0f;
        }
        D_8037D9D4 = 0.0f;
        D_8037D9D8 = 0.0f;
        D_8037D9EC = 0.0f;
        D_8037D9F0 = 0.0f;

        sMumboState -= time_getDelta();
        if (sMumboState == 0.0f) {
            sMumboState = -1.0f;
        }
    } else if (sMumboState < 0.0f && sPanSaved) {
        for (int i = 0; i < 3; i++) {
            cameraPosition[i] = sPanSavedPosition[i];
            cameraRotation[i] = sPanSavedRotation[i];
        }
        sPanSaved = false;
        sMumboState = 0.0f;
    }
}
} // namespace

void HackShared_EnableMumboReward() {
    sMumboRewardEnabled = true;
    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, [](IEvent*) { MumboReward_updateCamera(); });

    // Pressing B once the Jiggy is already collected
    REGISTER_VB_SHOULD(VB_MUMBO_HUT_INTERACT, EVENT_PRIORITY_NORMAL, {
        if (gsworld_getMap() != MAP_48_FP_MUMBOS_SKULL || !jiggyscore_isCollected((enum jiggy_e)kRewardJiggy)) {
            return;
        }
        gcdialog_showDialog(kAlreadyRewardedText, kAlreadyRewardedFlags, NULL, NULL, NULL, NULL);
        fileProgressFlag_set(FILEPROG_92_PAID_WALRUS_COST, TRUE);
        fileProgressFlag_set(FILEPROG_DC_HAS_HAD_ENOUGH_TOKENS_BEFORE, TRUE);
        *should = false;
    });
}
