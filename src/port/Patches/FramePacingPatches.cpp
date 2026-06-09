// Frame Pacing Patches
//
// Cutscene stutter tables are sourced from BanjoRecomp's analysis:
// github.com/BanjoRecomp/BanjoRecomp/blob/main/patches/timing_patches.c

#include <libultraship/bridge/consolevariablebridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#define CVAR_CUTSCENE_SYNC CVAR_ENHANCEMENT("Fix.CutsceneSync")

extern "C" {

#include "enums.h"

int getGameMode(void);
enum map_e gsworld_getMap(void);

// Demo Display Pacing

static int sDemoViCount = 0;

int port_getDemoViCount(void) {
    return sDemoViCount;
}

void port_setDemoViCount(int viCount) {
    sDemoViCount = viCount;
}

int port_getDemoDisplayViCount(int rawViCount) {
    int displayViCount = rawViCount;
    int game_mode = getGameMode();

    if (game_mode == GAME_MODE_8_BOTTLES_BONUS) {
        displayViCount = 3;
    } else if (game_mode == GAME_MODE_A_SNS_PICTURE) {
        switch (gsworld_getMap()) {
            case MAP_7F_FP_WOZZAS_CAVE:
            case MAP_92_GV_SNS_CHAMBER:
                displayViCount = 3;
                break;
            default:
                break;
        }
    }

    return displayViCount;
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
}

int port_getCutsceneExtraVis(void) {
    if (!CVarGetInteger(CVAR_CUTSCENE_SYNC, 1))
        return 0;

    int extra = 0;

    switch (gsworld_getMap()) {
        case MAP_1E_CS_START_NINTENDO:
            if (shouldLagCutscene(sConcertStartFrames, sConcertDurations,
                                  (int)(sizeof(sConcertStartFrames) / sizeof(sConcertStartFrames[0])))) {
                extra = 1;
            }
            sCutsceneCounter++;
            break;
        case MAP_7B_CS_INTRO_GL_DINGPOT_1:
            if (shouldLagCutscene(sLairDingpotStartFrames, sLairDingpotDurations,
                                  (int)(sizeof(sLairDingpotStartFrames) / sizeof(sLairDingpotStartFrames[0])))) {
                extra = 1;
            }
            sCutsceneCounter++;
            break;
        default:
            break;
    }

    return extra;
}

int port_getInterpolationFpsCap(void) {
    if (!CVarGetInteger(CVAR_CUTSCENE_SYNC, 1))
        return 0;

    switch (gsworld_getMap()) {
        // [port] Cap concert to 30 fps
        case MAP_1E_CS_START_NINTENDO:
            return 30;
        default:
            return 0;
    }
}

} // extern "C"

void RegisterFramePacingPatches_Init() {
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        (void)event;
        resetCutsceneTimings();
    });
}

static RegisterShipInitFunc initFunc(RegisterFramePacingPatches_Init);
