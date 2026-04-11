// Game Fixes
//
// Port functions and event listeners for various bug fixes and corrections.

#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
#include "functions.h"

enum map_e map_get(void);
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

#define CVAR_VOID_OUT CVAR_ENHANCEMENT("Fixes.VoidOutGameOver")
#define CVAR_FF_DIALOG CVAR_ENHANCEMENT("Fixes.FurnaceFunDialog")
#define CVAR_GRUNTY_FLAG CVAR_ENHANCEMENT("Fixes.GruntyDefeatedFlag")
#define CVAR_TOKEN_GV CVAR_ENHANCEMENT("Fixes.MumboTokenGV")

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
        if (map_get() == MAP_15_GV_WATER_PYRAMID && jiggyscore_isCollected(JIGGY_42_GV_WATER_PYRAMID)) {
            ev->actor->position[1] = 175.0f;
        }
    });
}

static RegisterShipInitFunc initVoidOutFunc(RegisterVoidOutGameOver_Init, { CVAR_VOID_OUT });
static RegisterShipInitFunc initFurnaceFunDialogFunc(RegisterFurnaceFunDialog_Init, { CVAR_FF_DIALOG });
static RegisterShipInitFunc initGruntyDefeatedFlagFunc(RegisterGruntyDefeatedFlag_Init, { CVAR_GRUNTY_FLAG });
static RegisterShipInitFunc initMumboTokenGVFunc(RegisterMumboTokenGV_Init, { CVAR_TOKEN_GV });
