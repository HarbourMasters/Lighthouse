// Frame Pacing Patches
//
// Cutscene stutter tables are sourced from BanjoRecomp's analysis:
// github.com/BanjoRecomp/BanjoRecomp/blob/main/patches/timing_patches.c

#include <libultraship/bridge/consolevariablebridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#define CVAR_CUTSCENE_SYNC CVAR_ENHANCEMENT("Fix.CutsceneSync")
#define CVAR_GV_LOBBY_FRAMERATE CVAR_ENHANCEMENT("Fix.GVLobbyFramerate")

extern "C" {

#include "enums.h"
#include "core2/nc/camera.h"

int getGameMode(void);
enum map_e gsworld_getMap(void);
s32 ncCamera_getType(void);
bool func_802BB270(void);
void viMgr_func_8024BF94(s32 viPerTick);

// Demo Display Pacing

static int sDemoViCount = 0;
static constexpr int kMaxDemoViCount = 0xF;

int port_getDemoViCount(void) {
    return sDemoViCount;
}

void port_setDemoViCount(int viCount) {
    sDemoViCount = (viCount > kMaxDemoViCount) ? kMaxDemoViCount : viCount;
}

int port_getDemoDisplayViCount(int rawViCount) {
    if (getGameMode() == GAME_MODE_A_SNS_PICTURE) {
        switch (gsworld_getMap()) {
            case MAP_7F_FP_WOZZAS_CAVE:
            case MAP_92_GV_SNS_CHAMBER:
                return 3;
            default:
                break;
        }
    }
    return rawViCount;
}

// Cutscene Stutter Compensation

static int sConcertStartFrames[] = { 269, 521, 583, 663, 769, 959, 1155, 1182, 1214 };
static int sConcertDurations[] = { 4, 4, 4, 4, 4, 4, 4, 4, 4 };

static int sLairDingpotStartFrames[] = {
    258, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000
};
static int sLairDingpotDurations[] = { 6, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };

static int sCutsceneCounter = 0;
static int sCutsceneNextStutter = 0;
static int sCutsceneLagIndex = 0;
static int sCutsceneExtraVis = 0;

static bool shouldLagCutscene(int* startFrames, int* durations, int count) {
    if (sCutsceneNextStutter == -1) {
        return false;
    }

    if (sCutsceneNextStutter < startFrames[0]) {
        sCutsceneNextStutter = startFrames[0];
    }

    if (sCutsceneCounter >= sCutsceneNextStutter &&
        sCutsceneCounter < sCutsceneNextStutter + durations[sCutsceneLagIndex]) {
        return true;
    }

    if (sCutsceneCounter > sCutsceneNextStutter) {
        sCutsceneLagIndex++;
        if (sCutsceneLagIndex >= count) {
            sCutsceneNextStutter = -1;
        } else {
            sCutsceneNextStutter = startFrames[sCutsceneLagIndex];
        }
    }

    return false;
}

static void resetCutsceneTimings(void) {
    sCutsceneCounter = 0;
    sCutsceneNextStutter = 0;
    sCutsceneLagIndex = 0;
    sCutsceneExtraVis = 0;
}

// Advances the stutter tables by one cutscene frame. Called from func_802E4384,
// the single point where a tick commits its time delta, so every consumer of
// port_getCutsceneExtraVis() sees the same answer for that tick.
void port_tickCutsceneStutter(void) {
    sCutsceneExtraVis = 0;

    if (!CVarGetInteger(CVAR_CUTSCENE_SYNC, 1))
        return;

    switch (gsworld_getMap()) {
        case MAP_1E_CS_START_NINTENDO:
            if (shouldLagCutscene(sConcertStartFrames, sConcertDurations,
                                  (int)(sizeof(sConcertStartFrames) / sizeof(sConcertStartFrames[0])))) {
                sCutsceneExtraVis = 1;
            }
            sCutsceneCounter++;
            break;
        case MAP_7B_CS_INTRO_GL_DINGPOT_1:
            if (shouldLagCutscene(sLairDingpotStartFrames, sLairDingpotDurations,
                                  (int)(sizeof(sLairDingpotStartFrames) / sizeof(sLairDingpotStartFrames[0])))) {
                sCutsceneExtraVis = 1;
            }
            sCutsceneCounter++;
            break;
        default:
            break;
    }
}

int port_getCutsceneExtraVis(void) {
    return sCutsceneExtraVis;
}

} // extern "C"

void RegisterFramePacingPatches_Init() {
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        (void)event;
        resetCutsceneTimings();
    });
}

void RegisterGVLobbyFramerate_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_GV_LOBBY_FRAMERATE, 1), [](IEvent* event) {
        (void)event;
        if (gsworld_getMap() != MAP_6E_GL_GV_LOBBY) {
            return;
        }
        const bool scripted = (ncCamera_getType() == CAMERA_TYPE_3_STATIC) || !func_802BB270();
        viMgr_func_8024BF94(scripted ? 3 : 2);
    });
}

static RegisterShipInitFunc initFunc(RegisterFramePacingPatches_Init);
static RegisterShipInitFunc initGVLobbyFramerateFunc(RegisterGVLobbyFramerate_Init, { CVAR_GV_LOBBY_FRAMERATE });
