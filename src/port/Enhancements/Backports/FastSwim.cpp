// Fast Swim Enhancement
//
// Backports Banjo-Tooie's fast swim mechanic: holding A+B simultaneously
// while underwater combines Banjo's kick paddle (DIVE_SLOW) with Kazooie's
// wing stroke (DIVE_MOVE) for faster swimming. Both animations run at
// independent speeds. The bone modification callback overlays Banjo's leg
// bones from DIVE_SLOW onto the primary DIVE_MOVE animation.

// Include C decomp headers with C linkage before C++ headers to prevent
// transitive includes from giving them C++ name mangling.
extern "C" {
#include "core2/bonetransform.h"
}

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"

struct animation_file_s;
typedef struct animation_file_s AnimationFile;

s32 bs_getState(void);
u32 bakey_held(s32);
f32 time_getDelta(void);
void baanim_setModifyMethod(void (*arg0)(uintptr_t, uintptr_t));
AnimationFile* animBinCache_get(enum asset_e asset_id);
void animationFile_getBoneTransformList(AnimationFile* anim_file, f32 progress, BoneTransformList* bone_transform_list);

// Bone transform accessors
void func_8033A57C(BoneTransformList* self, s32 bone_id, f32 arg2[4]); // get rotation
void func_8033A8F0(BoneTransformList* self, s32 bone_id, f32 arg2[4]); // set rotation
void boneTransformList_getBoneScale(BoneTransformList* self, s32 bone_id, f32 scale[3]);
void boneTransformList_setBoneScale(BoneTransformList* self, s32 bone_id, f32 scale[3]);
void func_8033A6B0(BoneTransformList* self, s32 bone_id, f32 arg2[3]); // get translation
void func_8033A968(BoneTransformList* self, s32 bone_id, f32 arg2[3]); // set translation

extern f32 D_8037D390;
}

// Banjo's leg chain bones — heavily animated in DIVE_SLOW (kicking), mostly static in DIVE_MOVE.
// Bones 14 and 28 are the foot endpoints; included to keep the full chain consistent.
// Right leg: 6, 7, 10, 12, 14
// Left leg:  20, 21, 24, 26, 28
static const s32 sBanjoLegBones[] = { 6, 7, 10, 12, 14, 20, 21, 24, 26, 28 };
static constexpr int BANJO_LEG_BONE_COUNT = sizeof(sBanjoLegBones) / sizeof(sBanjoLegBones[0]);

static bool gFastSwimActive = false;
static f32 gDiveSlowProgress = 0.0f;
static f32 gDiveSlowDuration = 0.75f; // Matches BS_39_DIVE_A playback duration
static BoneTransformList* gSecondaryBones = nullptr;

#define CVAR_NAME CVAR_ENHANCEMENT("Backports.FastSwim")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

// Bone modification callback — chains after Bottles Bonus via baanim_setModifyMethod.
// Samples DIVE_SLOW at independent progress and overlays Banjo's leg bones onto
// the primary DIVE_MOVE animation that drives Kazooie's wing stroke.
static void FastSwim_BoneModifyCallback(uintptr_t arg0, uintptr_t arg1) {
    if (!gFastSwimActive) {
        return;
    }

    auto* primaryBones = reinterpret_cast<BoneTransformList*>(arg0);

    // Advance secondary animation progress independently
    f32 dt = time_getDelta();
    gDiveSlowProgress += dt / gDiveSlowDuration;
    while (gDiveSlowProgress >= 1.0f) {
        gDiveSlowProgress -= 1.0f;
    }

    // Sample Banjo's paddle animation at current progress
    AnimationFile* animFile = animBinCache_get(ASSET_71_ANIM_BSSWIM_DIVE_SLOW);
    if (animFile == nullptr || gSecondaryBones == nullptr) {
        return;
    }
    animationFile_getBoneTransformList(animFile, gDiveSlowProgress, gSecondaryBones);

    // Copy Banjo's leg bone transforms from secondary onto primary
    for (int i = 0; i < BANJO_LEG_BONE_COUNT; i++) {
        s32 boneId = sBanjoLegBones[i];
        f32 quat[4];
        f32 scale[3];
        f32 trans[3];

        func_8033A57C(gSecondaryBones, boneId, quat);
        boneTransformList_getBoneScale(gSecondaryBones, boneId, scale);
        func_8033A6B0(gSecondaryBones, boneId, trans);

        func_8033A8F0(primaryBones, boneId, quat);
        boneTransformList_setBoneScale(primaryBones, boneId, scale);
        func_8033A968(primaryBones, boneId, trans);
    }
}

static void FastSwim_Start() {
    if (gFastSwimActive) {
        return;
    }
    gFastSwimActive = true;
    gDiveSlowProgress = 0.0f;
    if (gSecondaryBones == nullptr) {
        gSecondaryBones = boneTransformList_new();
    }
    baanim_setModifyMethod(FastSwim_BoneModifyCallback);
}

static void FastSwim_Stop() {
    if (!gFastSwimActive) {
        return;
    }
    gFastSwimActive = false;
    baanim_setModifyMethod(nullptr);
}

void RegisterFastSwim_Init() {
    FastSwim_Stop();
    gDiveSlowProgress = 0.0f;

    // State detection — toggle fast swim when in DIVE_B with A held
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVAR, [](IEvent* event) {
        bool inDiveB = (bs_getState() == BS_2C_DIVE_B);
        bool aHeld = (bakey_held(BUTTON_A) != 0);

        if (inDiveB && aHeld) {
            FastSwim_Start();
        } else {
            FastSwim_Stop();
        }
    });

    // Velocity boost — add Banjo's paddle velocity when fast swim is active
    COND_HOOK(OnBeakSwimVelocitySet, EVENT_PRIORITY_NORMAL, CVAR, [](IEvent* event) {
        if (!gFastSwimActive) {
            return;
        }
        auto* ev = reinterpret_cast<OnBeakSwimVelocitySet*>(event);
        *ev->velocity += 120.0f;
    });

    // Cleanup on map transitions
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, CVAR, [](IEvent* event) {
        FastSwim_Stop();
        gDiveSlowProgress = 0.0f;
    });
}

static RegisterShipInitFunc initFunc(RegisterFastSwim_Init, { CVAR_NAME });
