// Game Fixes
//
// Port functions and event listeners for various bug fixes and corrections.

#include <libultraship/bridge.h>
#include <cstring>
#include "port/ui/cvar_prefixes.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
#include "functions.h"

enum map_e gsworld_getMap(void);
}

// Mumbo token duplicate ID fix
extern "C" int port_fixMumboTokenId(int ret, int pos[3], int map_id) {
    // Token in MMM Inside Loggo shares ID 0x3D with another token
    if (CVarGetInteger(CVAR_ENHANCEMENT("Fixes.MumboTokenMMM"), 0)) {
        if (ret == 0x3D && pos[0] == 424 && pos[1] == 170 && pos[2] == 304 && map_id == MAP_8D_MMM_INSIDE_LOGGO) {
            return 0x74;
        }
    }
    // Token in CCW Spring shares ID 0x5E with another token
    if (CVarGetInteger(CVAR_ENHANCEMENT("Fixes.MumboTokenCCW"), 0)) {
        if (ret == 0x5E && pos[0] == -2649 && pos[1] == 0 && pos[2] == -395 && map_id == MAP_43_CCW_SPRING) {
            return 0x5D;
        }
    }
    return ret;
}

// Honeycomb health cap removal
extern "C" int port_shouldAllowAllHoneycombExtensions(void) {
    return CVarGetInteger(CVAR_ENHANCEMENT("AllHoneycombExtensions"), 0);
}

// Yum-Yum overflow crash: cap dropped collectibles to JP's limit of N on the ground
// at once. Always on (a crash guard, not a toggle). Counts live actors.
extern "C" int port_yumYumDropAllowed(int actorId, int maxOnGround) {
    return actorArray_actorCount((enum actor_e) actorId) < maxOnGround;
}

// Spelling: "Congo" -> "Conga"
extern "C" void port_fixCongaDialog(int textId, char* text) {
    if (!CVarGetInteger(CVAR_ENHANCEMENT("Fixes.CongaText"), 0)) {
        return;
    }
    if (textId != ASSET_B3E_DIALOG_CONGA_MEET_AS_TERMITE || text == NULL) {
        return;
    }
    for (int i = 0; i < 120; i++) {
        if (memcmp(text + i, "CONGO", 5) == 0) {
            text[i + 4] = 'A';
            break;
        }
    }
}

#define CVAR_VOID_OUT CVAR_ENHANCEMENT("Fixes.VoidOutGameOver")
#define CVAR_FF_DIALOG CVAR_ENHANCEMENT("Fixes.FurnaceFunDialog")
#define CVAR_GRUNTY_FLAG CVAR_ENHANCEMENT("Fixes.GruntyDefeatedFlag")
#define CVAR_TOKEN_GV CVAR_ENHANCEMENT("Fixes.MumboTokenGV")
#define CVAR_GNAWTY_ROCK CVAR_ENHANCEMENT("Fixes.GnawtySpringRock")
#define CVAR_FLOWER_REPLANT CVAR_ENHANCEMENT("Fixes.CCWFlowerReplant")
#define CVAR_TERMITE_SLOPES CVAR_ENHANCEMENT("Fixes.TermiteMoundSlopes")
#define CVAR_CLAW_SLIDE CVAR_ENHANCEMENT("Fixes.ClawSwipeSlide")
#define CVAR_BOGGY_RACE CVAR_ENHANCEMENT("Fixes.BoggyRaceGameOver")
#define CVAR_JINJO_SOUND CVAR_ENHANCEMENT("Fixes.JinjoChargeSound")

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
    COND_VB_SHOULD(VB_CCW_GNAWTY_SPRING_ROCK, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_GNAWTY_ROCK, 0),
                   { *should = false; });
}

// CCW flower: prevent the re-plant softlock (v1.1).
void RegisterCCWFlowerReplant_Init() {
    COND_VB_SHOULD(VB_CCW_FLOWER_REPLANT, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_FLOWER_REPLANT, 0),
                   { *should = false; });
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
    COND_VB_SHOULD(VB_BOGGY_RACE_GAME_OVER, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_BOGGY_RACE, 0),
                   { *should = false; });
}

// Grunty fight: stop the Jinjo charge-up sound on hit (v1.1).
void RegisterJinjoChargeSound_Init() {
    COND_VB_SHOULD(VB_JINJO_CHARGE_SOUND, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_JINJO_SOUND, 0),
                   { *should = false; });
}

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
