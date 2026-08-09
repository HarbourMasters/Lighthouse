/*
 *  All hack-specific code here is shared
    between Gruntch and Santa's Village.
 */

#include <libultraship/bridge.h>
#include <cmath>

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/Romhack/Shared/HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "macros.h"
#include "actor.h"
#include "core1/ml.h"
#include "core2/timedfunc.h"
#include "core2/modelRender.h"
#include "bk_time.h"

extern ActorInfo chLargeCrocodile;
extern f32 D_8037C5B0[3];
extern PfsManagerControllerData D_80281138[4];
extern f32 cameraPosition[3];
extern f32 cameraRotation[3];
extern f32 D_8037D948[3];
extern f32 D_8037D9C8[3];
extern f32 D_8037D9E0[3];
extern f32 D_8037D9D4, D_8037D9D8, D_8037D9EC, D_8037D9F0;
extern struct {
    u8 unk0;
    u8 level;
} D_80383300;

typedef struct struct_1A_s {
    f32 delay;
    f32 unk4;
    u8* str;
    s16 y;
    u8 portrait;
    u8 unkF;
} PauseTotalsRow;
extern PauseTotalsRow D_8036C520[4];
extern s8 D_8036C5F4[];
}

void TooieJiggyDance_ForceEnable();

// Suppressed dialogs
constexpr int kGruntchSuppressedDialogs[] = {
    ASSET_A6F_DIALOG_RUBEE_MEET,
    ASSET_C8B_DIALOG_GRUNTY_LONG_SWITCH_MISS,
    ASSET_CCF_DIALOG_GNAWTY_MEET_SUMMER,
    ASSET_DA3_DIALOG_EXTRA_LIFE_MEET,
    0xF78,
    ASSET_D9C_DIALOG_MUSIC_NOTE_MEET,
    ASSET_D9D_DIALOG_MUMBO_TOKEN_MEET,
    ASSET_D9E_DIALOG_BLUE_EGG_MEET,
    ASSET_D9F_DIALOG_RED_FEATHER_MEET,
    ASSET_DA0_DIALOG_GOLD_FEATHER_MEET,
    ASSET_DA1_DIALOG_HONEYCOMB_MEET,
    ASSET_DA2_DIALOG_EMPTY_HONEYCOMB_MEET,
};

// ---------------------------------------------------------- Proximity dialogs
constexpr s32 kDialogFlags8A = 0x8A;
constexpr s32 kDialogFlags82 = 0x82;
constexpr s32 kDialogFlagsA = 0x0A;
constexpr f32 kDialogRadius = 300.0f;

// Storybook page dialogs
constexpr ProximityDialogPage kPages37[] = {
    { { -7500.0f, 0.0f, 0.0f },
      kDialogRadius,
      0xA1D,
      kDialogFlags8A,
      0,
      0x8000,
      MUMBOTOKEN_12_CC_TUNNEL_RIGHT_OF_CLANKER },
    { { -5000.0f, 0.0f, 0.0f }, kDialogRadius, 0xA1E, kDialogFlags8A, 0, 0x4000, -1 },
    { { -2500.0f, 0.0f, 0.0f }, kDialogRadius, 0xA29, kDialogFlags8A, 0, 0x2000, -1 },
};

constexpr ProximityDialogPage kPages38[] = {
    { { -7500.0f, 0.0f, 0.0f }, kDialogRadius, 0xA77, kDialogFlags8A, 0, 0x0040, -1 },
};

constexpr ProximityDialogPage kPages3E[] = {
    { { -7500.0f, 0.0f, 0.0f }, kDialogRadius, 0xA70, kDialogFlags8A, 0, 0x1000, -1 },
    { { -5000.0f, 0.0f, 0.0f }, kDialogRadius, 0xA72, kDialogFlags8A, 0, 0x0800, -1 },
    { { -2500.0f, 0.0f, 0.0f }, kDialogRadius, 0xA73, kDialogFlags8A, 0, 0x0400, -1 },
    { { 0.0f, 0.0f, 0.0f }, kDialogRadius, 0xA74, kDialogFlags8A, 0, 0x0200, -1 },
    { { 2500.0f, 0.0f, 0.0f }, kDialogRadius, 0xA75, kDialogFlags8A, 0, 0x0100, -1 },
    { { 5000.0f, 0.0f, 0.0f }, kDialogRadius, 0xA76, kDialogFlags8A, 0, 0x0080, -1 },
};

constexpr ProximityDialogPage kPages3C[] = {
    { { -140.0f, 100.0f, -30.0f }, kDialogRadius, 0xA83, kDialogFlags8A, 1, 0x0080, -1 },
};

constexpr ProximityDialogPage kPages72[] = {
    { { 315.0f, 95.0f, -2085.0f }, 200.0f, 0xA7C, kDialogFlags8A, 1, 0x0200, -1 },
};

constexpr ProximityDialogPage kPages28[] = {
    { { 280.0f, 0.0f, -230.0f },
      kDialogRadius,
      0xA12,
      kDialogFlags8A,
      0,
      0x0000,
      MUMBOTOKEN_11_CC_ABOVE_LEVEL_ENTRANCE },
};

// Gameplay proximity dialogs
constexpr f32 kDialog15Pos[3] = { -568.0f, 128.0f, -180.0f };
constexpr ProximityDialogPage kPages15[] = {
    { { -200.0f, 130.0f, -100.0f }, 220.0f, 0xA82, kDialogFlags8A, 1, 0x0100, -1, kDialog15Pos },
};

constexpr ProximityDialogPage kPages22[] = {
    { { -2800.0f, 590.0f, 800.0f },
      300.0f,
      0xA19,
      kDialogFlags8A,
      1,
      0x1000,
      -1,
      NULL,
      JIGGY_8_MM_ORANGE_PADS,
      -1,
      -1 },
    { { -180.0f, 235.0f, 370.0f }, 700.0f, 0xA1B, kDialogFlags82, 1, 0x0800, -1, NULL, JIGGY_5_MM_HUTS, 0, 0 },
};

constexpr f32 kDialog1BPos[3] = { -15.0f, 798.0f, -360.0f };
constexpr ProximityDialogPage kPages1B[] = {
    { { 0.0f, 720.0f, -360.0f },
      270.0f,
      0xA13,
      kDialogFlagsA,
      1,
      0x8000,
      -1,
      kDialog1BPos,
      -1,
      -1,
      FILEPROG_38_RBB_OPEN },
    { { 0.0f, 720.0f, -360.0f },
      270.0f,
      0xA7D,
      kDialogFlagsA,
      1,
      0x4000,
      -1,
      kDialog1BPos,
      -1,
      FILEPROG_38_RBB_OPEN,
      -1 },
};

constexpr ProximityDialogPage kPages29[] = {
    { { 230.0f, 660.0f, 100.0f }, 200.0f, 0xA1C, kDialogFlags82, 1, 0x2000, -1, NULL },
};

static bool GruntchDialogGate15() {
    if (jiggyscore_isCollected(JIGGY_34_LAIR_MM_WITCH_SWITCH) ||
        !fileProgressFlag_get(FILEPROG_18_MM_WITCH_SWITCH_JIGGY_PRESSED)) {
        return false;
    }
    fileProgressFlag_set(FILEPROG_88_TRIED_LOGGO_AS_BEAR, 1);
    return true;
}

static bool Gruntch_HeadHasNoticedPlayer();

static bool GruntchDialogGate1B() {
    if (!Gruntch_HeadHasNoticedPlayer()) {
        return false;
    }
    if (ProximityDialogs_IsShown(1, 0xC000) && player_isStable()) {
        f32 reArm[3] = { 0.0f, 720.0f, -360.0f };
        if (ml_vec3f_distance(reArm, D_8037C5B0) < 170.0f && D_80281138[0].face_button[1] == 1) {
            ProximityDialogs_ClearShown(1, 0xC000);
        }
    }
    return true;
}

// Storybook pages
constexpr ProximityDialogMap kGruntchStorybookPages[] = {
    { MAP_37_RBB_CONTAINER_1, kPages37, ARRAY_COUNT(kPages37), nullptr },
    { MAP_38_RBB_CONTAINER_3, kPages38, ARRAY_COUNT(kPages38), nullptr },
    { MAP_3E_RBB_CONTAINER_2, kPages3E, ARRAY_COUNT(kPages3E), nullptr },
};

// Map the player warps to when resuming save
constexpr StorybookConfig kGruntchStorybook = {
    kGruntchStorybookPages,
    ARRAY_COUNT(kGruntchStorybookPages),
    0x2802,
    MUMBOTOKEN_12_CC_TUNNEL_RIGHT_OF_CLANKER,
};

// Gameplay dialogs
constexpr ProximityDialogMap kGruntchGameplayDialogs[] = {
    { MAP_3C_RBB_KITCHEN, kPages3C, ARRAY_COUNT(kPages3C), nullptr },
    { MAP_28_MMM_EGG_ROOM, kPages28, ARRAY_COUNT(kPages28), nullptr },
    { MAP_72_GL_BGS_LOBBY, kPages72, ARRAY_COUNT(kPages72), nullptr },
    { MAP_15_GV_WATER_PYRAMID, kPages15, ARRAY_COUNT(kPages15), GruntchDialogGate15 },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, kPages1B, ARRAY_COUNT(kPages1B), GruntchDialogGate1B },
    { MAP_22_CC_INSIDE_CLANKER, kPages22, ARRAY_COUNT(kPages22), nullptr },
    { MAP_29_MMM_NOTE_ROOM, kPages29, ARRAY_COUNT(kPages29), nullptr },
};

// ---------------------------------------------------------- Conditional actors
static bool sConditionalActorsEnabled = false;

extern "C" void romhack_RewriteActorSpawn(void* actorInfo, u32* flags) {
    if (!sConditionalActorsEnabled || actorInfo == NULL || flags == NULL) {
        return;
    }
    ActorInfo* info = (ActorInfo*)actorInfo;
    switch (info->actorId) {
        case 0x131:
            *flags = (*flags & ~0x02u) | 0x40u;
            break;
        case 0xF:
        case 0xF1:
        case ACTOR_340_XMAS_TREE_ICE:
            *flags |= 0x400u;
            info->draw_distance = 0x8000;
            break;
        default:
            break;
    }
}

// Draw the Giant Christmas Tree only if it's been "purchased"
static void Gruntch_EnableConditionalActors() {
    sConditionalActorsEnabled = true;
    COND_VB_SHOULD(VB_XMAS_TREE_ICE_UPDATE, EVENT_PRIORITY_NORMAL, sConditionalActorsEnabled, {
        Actor* self = va_arg(args, Actor*);
        if (gsworld_getMap() == MAP_1B_MMM_MAD_MONSTER_MANSION && self != NULL && self->marker != NULL &&
            self->marker->propPtr != NULL) {
            bool open = fileProgressFlag_get(FILEPROG_38_RBB_OPEN) != 0;
            self->marker->propPtr->unk8_3 = open;
            self->unk38_31 = !open;
            *should = false;
        }
    });

    // Force Boggy to be visible
    COND_VB_SHOULD(VB_BOGGY_HOME_VISIBLE, EVENT_PRIORITY_NORMAL, sConditionalActorsEnabled, { *should = true; });
}

// Game-Over respawns
static void Gruntch_EnableVoidOutRespawn() {
    REGISTER_VB_SHOULD(VB_VOID_OUT_RESPAWN_TRANSITION, EVENT_PRIORITY_NORMAL, {
        s32 map = va_arg(args, s32);
        s32 exit = va_arg(args, s32);
        if (D_80383300.level == 1) {
            map = MAP_28_MMM_EGG_ROOM;
            exit = 2;
        } else if (D_80383300.level == 0xA) {
            map = MAP_6C_GL_RED_CAULDRON_ROOM;
            exit = 5;
        }
        transitionToMap((enum map_e)map, exit, 1);
        *should = false;
    });
}

// Firing an egg spikes the stealth meter
static void Gruntch_EnableEggNoise() {
    REGISTER_VB_SHOULD(VB_EGG_FIRE_SFX, EVENT_PRIORITY_NORMAL, {
        s32 slot = va_arg(args, s32);
        s32* rate = va_arg(args, s32*);
        switch (slot) {
            case 0:
                StealthNoise_AddBurst(0.3f, 0.2f);
                break;
            case 1:
                StealthNoise_AddBurst(0.65f, 0.3f);
                *rate = 0x6D60;
                break;
            case 2:
                StealthNoise_AddBurst(0.65f, 0.4f);
                break;
            default:
                break;
        }
        (void)should;
    });
}

// Lair music persists through various maps
constexpr int kMusicGroupLair[] = { 0x6A, 0x6C, 0x6F, 0x71, 0x6B, 0x15 };
constexpr int kMusicGroupVillage[] = { 0x1B, 0x22, 0x0C };
constexpr WarpMusicGroup kGruntchMusicGroups[] = {
    { kMusicGroupLair, ARRAY_COUNT(kMusicGroupLair) },
    { kMusicGroupVillage, ARRAY_COUNT(kMusicGroupVillage) },
};

// Banjo & Kazooie don't rebound when hitting windows with Rat-A-Tap Rap
static void Gruntch_EnableWindowRapNoRebound() {
    REGISTER_VB_SHOULD(VB_BUMP_REBOUNDS_PLAYER, EVENT_PRIORITY_NORMAL, {
        ActorMarker* marker = va_arg(args, ActorMarker*);
        if (marker != NULL && marker->id == MARKER_107_ENGINE_ROOM_DOOR) {
            *should = false;
        }
    });
}

// ------------------------------------------------------- Jiggy consolidation
constexpr int kJiggyToMM[] = { 0x14, 0x20, 0x2E, 0x4A };
constexpr int kJiggyToMMM[] = { 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3E };
constexpr JiggyRelocation kGruntchJiggyRelocations[] = {
    { LEVEL_1_MUMBOS_MOUNTAIN, kJiggyToMM, ARRAY_COUNT(kJiggyToMM) },
    { LEVEL_A_MAD_MONSTER_MANSION, kJiggyToMMM, ARRAY_COUNT(kJiggyToMMM) },
};

// Pause-menu totals layout for Gruntch and Santa's Village
static void Gruntch_EnablePauseTotalsLayout() {
    static const s16 kRowY[4] = { 46, 85, -100, 122 };
    static const s8 kScrollStep[4] = { 2, 2, 3, 3 };
    for (int i = 0; i < 4; i++) {
        D_8036C520[i].y = kRowY[i];
        D_8036C5F4[i] = kScrollStep[i];
    }
}

static void Gruntch_EnableJiggyTally() {
    HackShared_EnableJiggyRelocation(kGruntchJiggyRelocations);
    port_overrideRomhackJiggiesPerWorld(10);

    // Hide Mt Grumpit's notes and jiggies
    REGISTER_VB_SHOULD(VB_PAUSEMENU_ROW_VISIBLE, EVENT_PRIORITY_NORMAL, {
        s32 lvl = va_arg(args, s32);
        s32 row = va_arg(args, s32);
        if (lvl == LEVEL_2_TREASURE_TROVE_COVE) {
            *should = (row != 0 && row != 1);
        }
    });
}

// The second act stays locked until the RBB level-open flag is set
static void Gruntch_EnableActGate() {
    REGISTER_VB_SHOULD(VB_WARP_DISPATCH, EVENT_PRIORITY_NORMAL, {
        if (va_arg(args, int) == 0xAA && !fileProgressFlag_get(FILEPROG_38_RBB_OPEN)) {
            *should = false;
        }
    });
}

// Mumbo's token dialogs are rendered by the hack once his jiggy is collected
static void Gruntch_EnableMumboTokenDialogs() {
    REGISTER_VB_SHOULD(VB_OVERRIDE_DIALOG_SHOW, EVENT_PRIORITY_LOW, {
        const s32 textId = va_arg(args, s32);
        if ((textId != ASSET_DAA_DIALOG_MUMBO_HAS_ENOUGH_TOKENS &&
             textId != ASSET_DAB_DIALOG_MUMBO_NOT_ENOUGH_TOKENS) ||
            gsworld_getMap() != MAP_48_FP_MUMBOS_SKULL || !jiggyscore_isCollected((enum jiggy_e)3)) {
            return;
        }
        func_80324E38(0.0f, 3);
        *should = true;
    });
}

// ------------------------------------------------------- Santa's head-look
namespace {

constexpr f32 kHeadYawCone = 80.0f;
constexpr f32 kHeadYawRate = 0.5f;
constexpr f32 kHeadPitchRate = 0.67f;
constexpr f32 kHeadPitchFree = 330.0f;
constexpr f32 kHeadPitchMin = 40.0f;
constexpr f32 kHeadPitchMax = 185.0f;
constexpr f32 kHeadPitchBias = 55.0f;
constexpr f32 kHeadBobSpeed = 5.0f;
constexpr f32 kHeadBobAmount = 0.5f;
constexpr f32 kHeadBobOffset = 2.0f;
constexpr f32 kHeadBobPeriod = 62.831853f;
constexpr f32 kHeadDeadZone = 1.0f;
constexpr f32 kHeadShadowY = 732.0f;
constexpr f32 kHeadShadowPitch = 3.0f;
constexpr f32 kHeadShadowScale = 0.4f;
constexpr s32 kHeadShadowAlpha = 0xF1;
constexpr s32 kHeadTrackDialogA = 0xA13;
constexpr s32 kHeadTrackDialogB = 0xA7D;

struct HeadLookState {
    bool initialised;
    bool noticedPlayer;
    f32 restYaw;
    f32 restPitch;
    f32 restRoll;
    f32 forward[3];
    f32 basePosition[3];
    f32 bobPhase;
    void (*savedUpdate)(Actor*);
    Actor* (*savedDraw)(ActorMarker*, Gfx**, Mtx**, Vtx**);
};

HeadLookState sHead{};

f32 HeadLook_radToDeg(f32 radians) {
    return (f32)((f64)radians * 57.29577951308232);
}

f32 HeadLook_angleDelta(f32 from, f32 to) {
    while (from >= 180.0f) {
        from -= 360.0f;
    }
    while (to >= 180.0f) {
        to -= 360.0f;
    }
    return (to < from) ? -(from - to) : (to - from);
}

f32 HeadLook_approach(f32 current, f32 target, f32 rate) {
    f32 delta = HeadLook_angleDelta(current, target);
    if (delta >= 180.0f) {
        delta -= 360.0f;
    }
    if (fabsf(delta) < kHeadDeadZone) {
        return mlNormalizeAngle(current);
    }
    return mlNormalizeAngle(current + delta * rate * 0.5f);
}

f32 HeadLook_vecAngle(const f32 a[3], const f32 b[3]) {
    const f32 lenA = sqrtf(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    const f32 lenB = sqrtf(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]);
    const f32 cosine = (b[0] * a[0] + b[1] * a[1] + b[2] * a[2]) / (lenB * lenA);
    if (cosine > 1.0f) {
        return (f32)acos(1.0);
    }
    if (cosine < -1.0f) {
        return (f32)acos(-1.0);
    }
    return (f32)acos((f64)cosine);
}

f32 HeadLook_vecAngleAbout(const f32 a[3], const f32 b[3], const f32 axis[3]) {
    const f32 cross[3] = {
        b[1] * a[2] - a[1] * b[2],
        b[2] * a[0] - a[2] * b[0],
        a[1] * b[0] - b[1] * a[0],
    };
    const f32 sine = cross[0] * axis[0] + cross[1] * axis[1] + cross[2] * axis[2];
    const f32 cosine = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    return atan2f(sine, cosine);
}

void HeadLook_rotateVector(f32 pitchDeg, f32 yawDeg, const f32 in[3], f32 out[3]) {
    const f32 pitch = (f32)((f64)(pitchDeg / 180.0f) * 3.141592653589793);
    const f32 yaw = (f32)((f64)(yawDeg / 180.0f) * 3.141592653589793);
    const f32 sinPitch = sinf(pitch);
    const f32 cosPitch = cosf(pitch);
    const f32 sinYaw = sinf(yaw);
    const f32 cosYaw = cosf(yaw);
    const f32 x = in[0], y = in[1], z = in[2];

    out[0] = x * cosYaw + (z * cosPitch + y * sinPitch) * sinYaw;
    out[1] = y * cosPitch - z * sinPitch;
    out[2] = (z * cosPitch + y * sinPitch) * cosYaw - x * sinYaw;
}

void HeadLook_capture(Actor* self) {
    sHead.initialised = true;
    sHead.restPitch = self->pitch + kHeadPitchBias;
    self->pitch = sHead.restPitch;
    sHead.restYaw = self->yaw;
    sHead.restRoll = self->roll;

    const f32 unitZ[3] = { 0.0f, 0.0f, 1.0f };
    HeadLook_rotateVector(0.0f, sHead.restYaw, unitZ, sHead.forward);

    for (int i = 0; i < 3; i++) {
        sHead.basePosition[i] = self->position[i];
    }
}

void HeadLook_update(Actor* self) {
    if (!sHead.initialised) {
        HeadLook_capture(self);
    }

    sHead.noticedPlayer = false;
    if (sHead.savedUpdate != nullptr) {
        sHead.savedUpdate(self);
    }

    const f32 axisY[3] = { 0.0f, 1.0f, 0.0f };
    const f32 flatToPlayer[3] = { D_8037C5B0[0] - self->position[0], 0.0f, D_8037C5B0[2] - self->position[2] };
    const f32 yawDeg = HeadLook_radToDeg(HeadLook_vecAngleAbout(flatToPlayer, sHead.forward, axisY));
    const f32 yaw = mlNormalizeAngle(yawDeg + sHead.restYaw);

    bool tracking = fabsf(yawDeg) <= kHeadYawCone;
    if (!tracking) {
        const s32 textId = gcdialog_getCurrentTextId();
        tracking = (textId == kHeadTrackDialogA || textId == kHeadTrackDialogB);
    }

    f32 pitch;
    if (tracking) {
        sHead.noticedPlayer = true;
        self->yaw = HeadLook_approach(self->yaw, yaw, kHeadYawRate);

        const f32 toPlayer[3] = { D_8037C5B0[0] - self->position[0], D_8037C5B0[1] - self->position[1],
                                  D_8037C5B0[2] - self->position[2] };
        const f32 reference[3] = { toPlayer[0], toPlayer[1] - D_8037C5B0[1], toPlayer[2] };
        pitch = mlNormalizeAngle(sHead.restPitch - HeadLook_radToDeg(HeadLook_vecAngle(toPlayer, reference)));
    } else {
        self->yaw = HeadLook_approach(self->yaw, sHead.restYaw, kHeadYawRate);
        pitch = 0.0f;
    }

    if (pitch <= kHeadPitchFree) {
        if (pitch < kHeadPitchMin) {
        } else if (pitch >= kHeadPitchMax) {
            pitch = kHeadPitchFree;
        } else {
            pitch = kHeadPitchMin;
        }
    }
    self->pitch = HeadLook_approach(self->pitch, pitch, kHeadPitchRate);

    sHead.bobPhase += time_getDelta() * kHeadBobSpeed;
    if (sHead.bobPhase > kHeadBobPeriod) {
        sHead.bobPhase -= kHeadBobPeriod;
    }
    self->position[1] = sHead.basePosition[1] + kHeadBobOffset + sinf(sHead.bobPhase) * kHeadBobAmount;
}

Actor* HeadLook_draw(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx) {
    Actor* self = nullptr;
    if (sHead.savedDraw != nullptr) {
        self = sHead.savedDraw(marker, gfx, mtx, vtx);
    }
    Actor* actor = marker_getActor(marker);
    if (actor == NULL) {
        return self;
    }

    modelRender_setDepthMode(MODEL_RENDER_DEPTH_COMPARE);
    BKModelBin* shadow = (BKModelBin*)assetcache_get(ASSET_3BF_MODEL_PLAYER_SHADOW);
    f32 position[3] = { actor->position[0], kHeadShadowY, actor->position[2] };
    f32 rotation[3] = { kHeadShadowPitch, 0.0f, 0.0f };
    modelRender_setAlpha(kHeadShadowAlpha);
    modelRender_draw(gfx, mtx, position, rotation, kHeadShadowScale, NULL, shadow);
    return self;
}

} // namespace

static bool Gruntch_HeadHasNoticedPlayer() {
    return sHead.noticedPlayer;
}

static void Gruntch_EnableHeadLook() {
    sHead.savedUpdate = chLargeCrocodile.update_func;
    sHead.savedDraw = chLargeCrocodile.draw_func;
    chLargeCrocodile.update_func = HeadLook_update;
    chLargeCrocodile.draw_func = HeadLook_draw;

    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent*) {
        sHead.initialised = false;
        sHead.noticedPlayer = false;
        sHead.bobPhase = 0.0f;
    });
}

// ------------------------------------------------------------ Stealth section
extern "C" void warp_rbbExitBoomBoxContainer(NodeProp*, ActorMarker*);

constexpr s32 kWarpRbbExitBoomBoxContainer = 182;

constexpr StealthNoiseConfig kGruntchStealth = {
    MAP_3C_RBB_KITCHEN,           -500.0f, 3600.0f, 0.0f, 3100.0f, 0xA7B, kWarpRbbExitBoomBoxContainer,
    warp_rbbExitBoomBoxContainer, 0x13,
};

// ------------------------------------------------------- Patch registration
void RegisterGruntchPatches() {
    TooieJiggyDance_ForceEnable();
    Storybook_Enable(kGruntchStorybook);
    ProximityDialogs_Enable(kGruntchGameplayDialogs);
    StealthNoise_Enable(kGruntchStealth);
    Gruntch_EnableActGate();
    Gruntch_EnableConditionalActors();
    Gruntch_EnableHeadLook();
    HackShared_EnableMumboReward();
    Gruntch_EnableVoidOutRespawn();
    Gruntch_EnableEggNoise();
    HackShared_EnableWarpMusicGroups(kGruntchMusicGroups);
    Gruntch_EnableWindowRapNoRebound();
    Gruntch_EnablePauseTotalsLayout();
    Gruntch_EnableJiggyTally();
    HackShared_EnableDialogSuppression(kGruntchSuppressedDialogs);
    Gruntch_EnableMumboTokenDialogs();
    HackShared_EnableForceAbilitiesUsed(kAllUsedAbilities);
}
