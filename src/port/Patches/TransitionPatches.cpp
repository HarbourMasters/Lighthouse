#include <libultraship/libultraship.h>

#include <libultra/gbi.h>
#include <libultra/gu.h>

#include "port/Engine.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Patches/Patches.h"

#include "enums.h"

// Falling-pieces transition uids.
#define TRANSITION_FALLING_PIECES_IN 0x10
#define TRANSITION_FALLING_PIECES_OUT 0x11

// Cached transition state.
static int32_t sTransitionModelId = 0;
static int32_t sTransitionUid = 0;
static int32_t sTransitionSubstate = 0;

extern "C" int port_shouldCaptureTransition(void) {
    if (sTransitionModelId != ASSET_467_MODEL_TRANSITION_FALLING_JIGGIES) {
        return 0;
    }
    // IN captures through substates 0-2.
    // OUT snapshots only at substate 2.
    if (sTransitionUid == TRANSITION_FALLING_PIECES_IN) {
        return sTransitionSubstate <= 2;
    }
    return sTransitionSubstate == 2;
}

void RegisterTransitionPatches_Init() {
    // Cache the pushed transition state for the capture decision.
    REGISTER_LISTENER(OnTransitionStateUpdate, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (OnTransitionStateUpdate*)event;
        sTransitionModelId = ev->modelId;
        sTransitionUid = ev->uid;
        sTransitionSubstate = ev->substate;
    });

    // Widescreen transition scaling. For the falling jiggy pieces, X-scale the
    // projection so the pieces fill a non-4:3 viewport; the captured-FB UVs
    // (port_patchTransitionModel) map 1:1 to match. Other transitions get a small
    // overscan. Structural widescreen fix, always on.
    REGISTER_LISTENER(OnTransitionModelScale, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (OnTransitionModelScale*)event;
        float aspectRatio = GameEngine_GetAspectRatio() / (320.0f / 240.0f);
        bool isJigsaw = (ev->uid == TRANSITION_FALLING_PIECES_IN || ev->uid == TRANSITION_FALLING_PIECES_OUT);

        if (isJigsaw && aspectRatio > 1.01f) {
            *ev->scale = 1.0f;
            Mtx* xScaleMtx = (*ev->mtx)++;
            guScale(xScaleMtx, aspectRatio, 1.0f, 1.0f);
            gSPMatrix((*ev->gfx)++, xScaleMtx, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
        } else {
            *ev->scale = (aspectRatio > 1.01f) ? aspectRatio + 0.1f : 1.0f;
        }
    });
}

static RegisterShipInitFunc transitionPatchesInitFunc(RegisterTransitionPatches_Init);
