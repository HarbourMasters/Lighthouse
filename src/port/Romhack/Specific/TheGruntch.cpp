/*
 *  All hack-specific code here is shared
    between Gruntch and Santa's Village.
 */

#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/Shared/HackShared.h"
#include "port/Romhack/Shared/Storybook.h"
#include "port/Romhack/Shared/ProximityDialogs.h"
#include "port/Romhack/Shared/StealthNoise.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "macros.h"
#include "actor.h"
#include "core1/ml.h"
#include "core2/timedfunc.h"
#include "bk_time.h"

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

static bool GruntchDialogGate1B() {
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

// --------------------------------------------------------------- Mumbo reward
namespace {
constexpr f32 kJiggyDelay = 4.15f;
constexpr f32 kStateDelay = 3.85f;
constexpr f32 kStateValue = 1.1f;
constexpr f32 kCameraReleaseDelay = 0.5f;
constexpr s32 kRewardJiggy = 3;
constexpr s32 kCameraMotor = 1;
constexpr s32 kLeaveHutText = 0xA7E;
constexpr s32 kLeaveHutFlags = 0x02;

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
            sMumboState = -1.0f; // sentinel: expired, restore next frame
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

static void Gruntch_EnableMumboReward() {
    sMumboRewardEnabled = true;
    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, [](IEvent*) { MumboReward_updateCamera(); });
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
constexpr s32 kMusicGroupLair[] = { 0x6A, 0x6C, 0x6F, 0x71, 0x6B, 0x15 };
constexpr s32 kMusicGroupVillage[] = { 0x1B, 0x22, 0x0C };

static bool SameMusicGroup(const s32* group, int count, s32 a, s32 b) {
    bool hasA = false, hasB = false;
    for (int i = 0; i < count; i++) {
        hasA = hasA || group[i] == a;
        hasB = hasB || group[i] == b;
    }
    return hasA && hasB;
}

// Banjo & Kazooie don't rebound when hitting windows with Rat-A-Tap Rap
static void Gruntch_EnableWindowRapNoRebound() {
    REGISTER_VB_SHOULD(VB_BUMP_REBOUNDS_PLAYER, EVENT_PRIORITY_NORMAL, {
        ActorMarker* marker = va_arg(args, ActorMarker*);
        if (marker != NULL && marker->id == MARKER_107_ENGINE_ROOM_DOOR) {
            *should = false;
        }
    });
}

static void Gruntch_EnableWarpMusic() {
    REGISTER_VB_SHOULD(VB_WARP_KEEPS_MUSIC, EVENT_PRIORITY_NORMAL, {
        const s32 dest = va_arg(args, s32);
        const s32 cur = gsworld_getMap();
        if (SameMusicGroup(kMusicGroupVillage, ARRAY_COUNT(kMusicGroupVillage), cur, dest) ||
            SameMusicGroup(kMusicGroupLair, ARRAY_COUNT(kMusicGroupLair), cur, dest)) {
            musicKeepsPlaying();
        }
        (void)should;
    });
}

// ------------------------------------------------------- Jiggy consolidation
constexpr s32 kJiggyToMM[] = { 0x14, 0x20, 0x2E, 0x4A };
constexpr s32 kJiggyToMMM[] = { 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3E };

static bool JiggyMovedFromVanillaLevel(s32 id) {
    for (s32 m : kJiggyToMM) {
        if (m == id) {
            return true;
        }
    }
    for (s32 m : kJiggyToMMM) {
        if (m == id) {
            return true;
        }
    }
    return false;
}

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
    REGISTER_VB_SHOULD(VB_JIGGYSCORE_LEVEL_TOTAL, EVENT_PRIORITY_NORMAL, {
        s32 lvl = va_arg(args, s32);
        s32* result = va_arg(args, s32*);
        s32 cnt = 0;
        if (lvl >= 1 && lvl <= 0xA) {
            for (s32 id = (lvl - 1) * 10 + 1; id <= lvl * 10; id++) {
                if (!JiggyMovedFromVanillaLevel(id) && jiggyscore_isCollected((enum jiggy_e)id)) {
                    cnt++;
                }
            }
            if (lvl == 1) {
                for (s32 id : kJiggyToMM) {
                    if (jiggyscore_isCollected((enum jiggy_e)id)) {
                        cnt++;
                    }
                }
            } else if (lvl == 0xA) {
                for (s32 id : kJiggyToMMM) {
                    if (jiggyscore_isCollected((enum jiggy_e)id)) {
                        cnt++;
                    }
                }
            }
        }
        *result = cnt;
        *should = false;
    });

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
    Gruntch_EnableMumboReward();
    Gruntch_EnableVoidOutRespawn();
    Gruntch_EnableEggNoise();
    Gruntch_EnableWarpMusic();
    Gruntch_EnableWindowRapNoRebound();
    Gruntch_EnablePauseTotalsLayout();
    Gruntch_EnableJiggyTally();
    HackShared_EnableDialogSuppression(kGruntchSuppressedDialogs);
    HackShared_EnableForceAbilitiesUsed(kAllUsedAbilities);
}
