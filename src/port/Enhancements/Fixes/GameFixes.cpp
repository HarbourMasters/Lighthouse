// Game Fixes
//
// Port functions and event listeners for various bug fixes and corrections.

#include <libultraship/bridge.h>
#include <cstring>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#include "functions.h"
extern "C" {
#include "enums.h"

enum map_e gsworld_getMap(void);
}

// Check if in parade
extern "C" int port_isInCharacterParade(void) {
    return volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE) != 0;
}

#define CVAR_VOID_OUT CVAR_ENHANCEMENT("Fixes.VoidOutGameOver")
#define CVAR_FF_DIALOG CVAR_ENHANCEMENT("Fixes.FurnaceFunDialog")
#define CVAR_GRUNTY_FLAG CVAR_ENHANCEMENT("Fixes.GruntyDefeatedFlag")
#define CVAR_TOKEN_GV CVAR_ENHANCEMENT("Fixes.MumboTokenGV")
#define CVAR_TOKEN_MMM CVAR_ENHANCEMENT("Fixes.MumboTokenMMM")
#define CVAR_TOKEN_CCW CVAR_ENHANCEMENT("Fixes.MumboTokenCCW")
#define CVAR_GNAWTY_ROCK CVAR_ENHANCEMENT("Fixes.GnawtySpringRock")
#define CVAR_FLOWER_REPLANT CVAR_ENHANCEMENT("Fixes.CCWFlowerReplant")
#define CVAR_TERMITE_SLOPES CVAR_ENHANCEMENT("Fixes.TermiteMoundSlopes")
#define CVAR_CLAW_SLIDE CVAR_ENHANCEMENT("Fixes.ClawSwipeSlide")
#define CVAR_BOGGY_RACE CVAR_ENHANCEMENT("Fixes.BoggyRaceGameOver")
#define CVAR_JINJO_SOUND CVAR_ENHANCEMENT("Fixes.JinjoChargeSound")
#define CVAR_GRUNTY_BOUNCE CVAR_ENHANCEMENT("Fixes.GruntyBounce")
#define CVAR_CONGA_TEXT CVAR_ENHANCEMENT("Fixes.CongaText")
#define CVAR_STUMP_RUMBLE CVAR_ENHANCEMENT("Fixes.ChimpyStumpRumble")

void RegisterVoidOutGameOver_Init() {
    COND_VB_SHOULD(VB_VOID_OUT_GAME_OVER, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_VOID_OUT, 0),
                   { *should = false; });
}

void RegisterFurnaceFunDialog_Init() {
    COND_HOOK(OnFurnaceFunDialog, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_FF_DIALOG, 0), [](IEvent* event) {
        auto* ev = reinterpret_cast<OnFurnaceFunDialog*>(event);
        *ev->lifeThreshold = 0;
    });
}

// v1.1 fix: defeated flag is moved from chfinalboss_setBossDefeated to the
// post-Jinjonator point so death between freeing the Jinjonator and the boss
// defeat doesn't lose progress.
void RegisterGruntyDefeatedFlag_Init() {
    COND_HOOK(OnGruntyJinjonatorComplete, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_GRUNTY_FLAG, 0),
              [](IEvent* event) { fileProgressFlag_set(FILEPROG_FC_DEFEAT_GRUNTY, true); });

    COND_VB_SHOULD(VB_GRUNTY_DEFEATED_FLAG_BOSS, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_GRUNTY_FLAG, 0),
                   { *should = false; });
}

// Move this token to the floor when jiggy is collected so it is no longer missable
void RegisterMumboTokenGV_Init() {
    COND_HOOK(OnMumboTokenUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_TOKEN_GV, 0), [](IEvent* event) {
        auto* ev = reinterpret_cast<OnMumboTokenUpdate*>(event);
        if (gsworld_getMap() == MAP_15_GV_WATER_PYRAMID && jiggyscore_isCollected(JIGGY_42_GV_WATER_PYRAMID)) {
            ev->actor->position[1] = 175.0f;
        }
    });
}

// CCW Gnawty rock: indestructible in Spring (v1.1).
void RegisterGnawtySpringRock_Init() {
    COND_VB_SHOULD(VB_CCW_GNAWTY_SPRING_ROCK, EVENT_PRIORITY_NORMAL, !CVarGetInteger(CVAR_GNAWTY_ROCK, 0),
                   { *should = true; });
}

// CCW flower: prevent the re-plant softlock (v1.1). Default on to prevent softlock.
void RegisterCCWFlowerReplant_Init() {
    COND_VB_SHOULD(VB_CCW_FLOWER_REPLANT, EVENT_PRIORITY_NORMAL, !CVarGetInteger(CVAR_FLOWER_REPLANT, 1),
                   { *should = true; });
}

// Termite mound: instant slide on slopes (v1.1).
void RegisterTermiteMoundSlopes_Init() {
    COND_VB_SHOULD(VB_TERMITE_MOUND_SLOPES, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_TERMITE_SLOPES, 0),
                   { *should = false; });
}

// Claw swipe: suppress claw during a slide (v1.1).
void RegisterClawSwipeSlide_Init() {
    COND_VB_SHOULD(VB_CLAW_SWIPE_SLIDE, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_CLAW_SLIDE, 0),
                   { *should = false; });
}

// Boggy race: reload instead of game over at 0 lives (v1.1).
void RegisterBoggyRaceGameOver_Init() {
    COND_VB_SHOULD(VB_BOGGY_RACE_GAME_OVER, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_BOGGY_RACE, 1),
                   { *should = false; });
}

// Grunty fight: stop the Jinjo charge-up sound on hit (v1.1).
void RegisterJinjoChargeSound_Init() {
    COND_VB_SHOULD(VB_JINJO_CHARGE_SOUND, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_JINJO_SOUND, 0),
                   { *should = false; });
}

// Grunty fight: don't let Grunty bounce when hit with wonderwing (v1.1)
void RegisterGruntyBounce_Init() {
    COND_VB_SHOULD(VB_ENEMY_BECOME_BUNDLE, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_GRUNTY_BOUNCE, 1), {
        int actorId = va_arg(args, int);
        if (actorId == ACTOR_38B_GRUNTILDA_FINAL_BOSS) {
            *should = false;
        }
    });
}

// Yum-Yum overflow crash guard: cap dropped collectibles to JP's on-ground limit. Always on
// (structural crash fix, not a toggle); varargs carry (actorId, maxOnGround) from the decomp.
void RegisterYumYumDrop_Init() {
    REGISTER_VB_SHOULD(VB_YUMYUM_DROP, EVENT_PRIORITY_NORMAL, {
        int actorId = va_arg(args, int);
        int maxOnGround = va_arg(args, int);
        *should = actorArray_actorCount((enum actor_e)actorId) < maxOnGround;
    });
}

// Spelling: "Congo" -> "Conga" when the Conga-as-termite dialog bin loads.
void RegisterCongaDialog_Init() {
    COND_HOOK(OnDialogLoaded, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_CONGA_TEXT, 0), [](IEvent* event) {
        auto* ev = reinterpret_cast<OnDialogLoaded*>(event);
        if (ev->textId != ASSET_B3E_DIALOG_CONGA_MEET_AS_TERMITE || ev->text == nullptr) {
            return;
        }
        for (int i = 0; i < 120; i++) {
            if (memcmp(ev->text + i, "CONGO", 5) == 0) {
                ev->text[i + 4] = 'A';
                break;
            }
        }
    });
}

// MM Chimpy stump: Chimpy respawns and walks off again on every entry to MM once his jiggy has
// spawned, so the stump drops in low and grinds back up with the rumble sfx each time. Mute the
// rumble for those replays only; the first rise (jiggy spawns 2.9s after the shake starts, so it
// isn't marked spawned yet) keeps its sound.
static bool IsChimpyWalkOffReplay() {
    return jiggyscore_isSpawned(JIGGY_9_MM_CHIMPY) &&
           !mapSpecificFlags_get(MM_SPECIFIC_FLAG_2_ORANGE_HAS_BEEN_RETURNED);
}

void RegisterChimpyStumpRumble_Init() {
    COND_VB_SHOULD(VB_MM_CHIMPY_STUMP_RUMBLE, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_STUMP_RUMBLE, 1),
                   { *should = !jiggyscore_isSpawned(JIGGY_9_MM_CHIMPY); });
    COND_VB_SHOULD(VB_MM_CHIMPY_NOISE, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_STUMP_RUMBLE, 1),
                   { *should = !IsChimpyWalkOffReplay(); });
    COND_VB_SHOULD(VB_SPLINE_PATH_SFX, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_STUMP_RUMBLE, 1), {
        Actor* pathWalker = va_arg(args, Actor*);
        if (pathWalker != NULL && pathWalker->actor_info != NULL && pathWalker->actor_info->actorId == ACTOR_F_CHIMPY &&
            IsChimpyWalkOffReplay()) {
            *should = false;
        }
    });
}

// Mumbo token duplicate-id fix: rewrite the resolved id for the two tokens that share an id.
void RegisterMumboTokenIdResolve_Init() {
    COND_HOOK(OnMumboTokenIdResolve, EVENT_PRIORITY_NORMAL,
              CVarGetInteger(CVAR_TOKEN_MMM, 0) || CVarGetInteger(CVAR_TOKEN_CCW, 0), [](IEvent* event) {
                  auto* ev = reinterpret_cast<OnMumboTokenIdResolve*>(event);
                  // MMM Inside Loggo token shares id 0x3D with another token.
                  if (CVarGetInteger(CVAR_TOKEN_MMM, 0) && *ev->tokenId == 0x3D && ev->position[0] == 424 &&
                      ev->position[1] == 170 && ev->position[2] == 304 && ev->mapId == MAP_8D_MMM_INSIDE_LOGGO) {
                      *ev->tokenId = 0x74;
                  }
                  // CCW Spring token shares id 0x5E with another token.
                  if (CVarGetInteger(CVAR_TOKEN_CCW, 0) && *ev->tokenId == 0x5E && ev->position[0] == -2649 &&
                      ev->position[1] == 0 && ev->position[2] == -395 && ev->mapId == MAP_43_CCW_SPRING) {
                      *ev->tokenId = 0x5D;
                  }
              });
}

// Map savestates and demo playback
static bool isPlaybackMode(s32 mode) {
    return (mode == GAME_MODE_5_UNKNOWN) || (mode == GAME_MODE_6_FILE_PLAYBACK) || (mode == GAME_MODE_7_ATTRACT_DEMO) ||
           (mode == GAME_MODE_8_BOTTLES_BONUS) || (mode == GAME_MODE_9_BANJO_AND_KAZOOIE) ||
           (mode == GAME_MODE_A_SNS_PICTURE);
}

void RegisterMapSavestatePlayback_Init() {
    REGISTER_VB_SHOULD(VB_MAP_SAVESTATE_USE, EVENT_PRIORITY_NORMAL, {
        s32 mode = va_arg(args, s32);
        if (isPlaybackMode(mode)) {
            *should = false;
        }
    });
}

static RegisterShipInitFunc initMapSavestatePlaybackFunc(RegisterMapSavestatePlayback_Init);
static RegisterShipInitFunc initVoidOutFunc(RegisterVoidOutGameOver_Init, { CVAR_VOID_OUT });
static RegisterShipInitFunc initFurnaceFunDialogFunc(RegisterFurnaceFunDialog_Init, { CVAR_FF_DIALOG });
static RegisterShipInitFunc initGruntyDefeatedFlagFunc(RegisterGruntyDefeatedFlag_Init, { CVAR_GRUNTY_FLAG });
static RegisterShipInitFunc initMumboTokenGVFunc(RegisterMumboTokenGV_Init, { CVAR_TOKEN_GV });
static RegisterShipInitFunc initGnawtySpringRockFunc(RegisterGnawtySpringRock_Init, { CVAR_GNAWTY_ROCK });
static RegisterShipInitFunc initCCWFlowerReplantFunc(RegisterCCWFlowerReplant_Init, { CVAR_FLOWER_REPLANT });
static RegisterShipInitFunc initTermiteMoundSlopesFunc(RegisterTermiteMoundSlopes_Init, { CVAR_TERMITE_SLOPES });
static RegisterShipInitFunc initClawSwipeSlideFunc(RegisterClawSwipeSlide_Init, { CVAR_CLAW_SLIDE });
static RegisterShipInitFunc initBoggyRaceGameOverFunc(RegisterBoggyRaceGameOver_Init, { CVAR_BOGGY_RACE });
static RegisterShipInitFunc initJinjoChargeSoundFunc(RegisterJinjoChargeSound_Init, { CVAR_JINJO_SOUND });
static RegisterShipInitFunc initGruntyBounceFunc(RegisterGruntyBounce_Init, { CVAR_GRUNTY_BOUNCE });
static RegisterShipInitFunc initYumYumDropFunc(RegisterYumYumDrop_Init);
static RegisterShipInitFunc initCongaDialogFunc(RegisterCongaDialog_Init, { CVAR_CONGA_TEXT });
static RegisterShipInitFunc initChimpyStumpRumbleFunc(RegisterChimpyStumpRumble_Init, { CVAR_STUMP_RUMBLE });
static RegisterShipInitFunc initMumboTokenIdResolveFunc(RegisterMumboTokenIdResolve_Init,
                                                        { CVAR_TOKEN_MMM, CVAR_TOKEN_CCW });
