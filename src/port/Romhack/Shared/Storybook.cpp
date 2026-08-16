#include <cstring>
#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "macros.h"
#include "actor.h"
#include "core1/ml.h"

extern OSContPad pfsManagerContPadData[4];
extern PfsManagerControllerData D_80281138[4];
extern Struct_core1_10A00_1 D_80281250[4];
extern f32 D_8037C5B0[3];
extern f32 player_position[3];
}

namespace {

bool IsStorybookMap(enum map_e map);
f32 StoryStickAxis(s32 raw);
void StorybookInputCapture();
void StorybookPageUpdate();
void ApplyStorybookIntroHooks();

// Engine tuning
constexpr f32 kPageStep = 2500.0f;
constexpr f32 kStickClamp = 80.0f;
constexpr f32 kPushThreshold = 0.5f;
constexpr f32 kCenterThreshold = 0.3f;
constexpr s32 kDebounceTicks = 50;

// Per-hack data
const ProximityDialogMap* sPages = nullptr;
s32 sPageCount = 0;
s32 sResumeDest = 0;
s32 sSeenToken = -1;

bool sStorybookEnabled = false;
s32 sStoryTick = 0;
s32 sStoryLastStep = -kDebounceTicks - 1;
s32 sStoryStickX = 0;
s32 sStoryStickY = 0;

bool IsStorybookMap(enum map_e map) {
    for (s32 i = 0; i < sPageCount; i++) {
        if ((enum map_e)sPages[i].map == map) {
            return true;
        }
    }
    return false;
}

f32 StoryStickAxis(s32 raw) {
    f32 v = (f32)raw;
    if (v > kStickClamp) {
        v = kStickClamp;
    } else if (v < -kStickClamp) {
        v = -kStickClamp;
    }
    return v / kStickClamp;
}

void StorybookInputCapture() {
    if (!IsStorybookMap(gsworld_getMap())) {
        return;
    }
    sStoryStickX = pfsManagerContPadData[0].stick_x;
    sStoryStickY = pfsManagerContPadData[0].stick_y;
    memset(&D_80281138[0], 0, sizeof(D_80281138[0]));
    memset(&D_80281250[0], 0, sizeof(D_80281250[0]));
    pfsManagerContPadData[0].stick_x = 0;
    pfsManagerContPadData[0].stick_y = 0;
    pfsManagerContPadData[0].button = 0;
}

void StorybookPageUpdate() {
    sStoryTick++;
    if (!IsStorybookMap(gsworld_getMap())) {
        return;
    }
    D_8037C5B0[1] = 0.0f;
    D_8037C5B0[2] = 0.0f;

    if (!gcdialog_hasCurrentTextId()) {
        const f32 x = StoryStickAxis(sStoryStickX);
        const f32 y = StoryStickAxis(sStoryStickY);
        if (y > -kCenterThreshold && y < kCenterThreshold && x > kPushThreshold &&
            sStoryLastStep + kDebounceTicks < sStoryTick) {
            sStoryLastStep = sStoryTick;
            D_8037C5B0[0] += kPageStep;
        }
    }
    player_position[0] = D_8037C5B0[0];
}

void ApplyStorybookIntroHooks() {
    // First storybook writes a mumbo token to indicate that it's been seen
    COND_VB_SHOULD(VB_GAMESELECT_START_NEW_GAME, EVENT_PRIORITY_NORMAL, sStorybookEnabled, {
        (void)args; // gamenum is passed but unused
        const bool seen = sSeenToken >= 0 && mumboscore_get((enum mumbotoken_e)sSeenToken);
        *should = !seen;
    });

    // Storybook mode freezes all input except the left stick's right movement
    COND_HOOK(OnControllerUpdate, EVENT_PRIORITY_NORMAL, sStorybookEnabled, [](IEvent*) { StorybookInputCapture(); });

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, sStorybookEnabled, [](IEvent*) {
        ProximityDialogs_Run(sPages, sPageCount);
        StorybookPageUpdate();
    });
}

} // namespace

extern "C" void Storybook_ResumeWarp(NodeProp* node, ActorMarker* marker) {
    (void)marker;
    func_8031CC8C(node, sResumeDest);
}

extern "C" void* port_getRomhackResumeWarpFunc(void) {
    return sStorybookEnabled ? (void*)&Storybook_ResumeWarp : nullptr;
}

void Storybook_Enable(const StorybookConfig& cfg) {
    sPages = cfg.pages;
    sPageCount = cfg.pageCount;
    sResumeDest = cfg.resumeDest;
    sSeenToken = cfg.newGameSeenToken;
    sStorybookEnabled = true;
    ApplyStorybookIntroHooks();
}
