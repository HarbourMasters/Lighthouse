#include <libultraship/libultraship.h>
#include <libultraship/bridge/consolevariablebridge.h>

#include "port/Engine.h"
#include "port/Patches/Patches.h"
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"

#include <map>

#include "enums.h"
#include "functions.h"
#include "variables.h"
#include "actor.h"

#define CVAR_WS_CAMERA_FIX CVAR_ENHANCEMENT("Fix.WidescreenCamera")

struct WsXluScaleFix {
    int32_t map;
    float scale;
};

struct HiddenCutsceneActor {
    int32_t map;
    int32_t modelId;
    float delay = 0.0f;
};

struct SpawnAnchor {
    float position[3];
    float elapsed;
    bool moved;
};

static const WsXluScaleFix sWsXluScaleFixes[] = {
    { .map = MAP_1F_CS_START_RAREWARE, .scale = 1.1f },
};
static const HiddenCutsceneActor sHiddenCutsceneActors[] = {
    { .map = MAP_1F_CS_START_RAREWARE, .modelId = ASSET_3ED_MODEL_BUZZBOMB },
    { .map = MAP_1E_CS_START_NINTENDO, .modelId = ASSET_353_MODEL_BIGBUTT, .delay = 0.2f },
    { .map = MAP_1E_CS_START_NINTENDO, .modelId = ASSET_354_MODEL_SMALL_BULL, .delay = 0.7f },
    { .map = MAP_1E_CS_START_NINTENDO, .modelId = ASSET_369_MODEL_CONCERT_FROG, .delay = 0.2f },
};
static constexpr int HIDDEN_CUTSCENE_ACTOR_COUNT = sizeof(sHiddenCutsceneActors) / sizeof(sHiddenCutsceneActors[0]);
static constexpr float kHiddenMoveThreshold = 1.0f;
static std::map<ActorMarker*, SpawnAnchor> sSpawnAnchors;
static constexpr int WS_XLU_SCALE_FIX_COUNT = sizeof(sWsXluScaleFixes) / sizeof(sWsXluScaleFixes[0]);

void RegisterWidescreenXluScale() {
    COND_HOOK(MapModelXluScale, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_WS_CAMERA_FIX, 1), [](IEvent* event) {
        float aspectScale = port_wsCameraYawScale();
        if (aspectScale <= 0.0f) {
            return;
        }
        auto* ev = (MapModelXluScale*)event;
        for (int i = 0; i < WS_XLU_SCALE_FIX_COUNT; i++) {
            if (ev->map == sWsXluScaleFixes[i].map) {
                *ev->scale *= 1.0f + (sWsXluScaleFixes[i].scale - 1.0f) * aspectScale;
                break;
            }
        }
    });
}

static const HiddenCutsceneActor* findHiddenCutsceneActor(int32_t map, int32_t modelId) {
    for (int i = 0; i < HIDDEN_CUTSCENE_ACTOR_COUNT; i++) {
        if (map == sHiddenCutsceneActors[i].map && modelId == sHiddenCutsceneActors[i].modelId) {
            return &sHiddenCutsceneActors[i];
        }
    }
    return nullptr;
}

void RegisterHiddenCutsceneActors() {
    COND_VB_SHOULD(VB_CUTSCENE_ACTOR_DRAW, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_WS_CAMERA_FIX, 1), {
        Actor* actor = va_arg(args, Actor*);
        if (actor == nullptr || actor->marker == nullptr) {
            return;
        }
        const HiddenCutsceneActor* hidden = findHiddenCutsceneActor((int32_t)gsworld_getMap(), actor->marker->modelId);
        if (hidden == nullptr) {
            return;
        }

        auto entry = sSpawnAnchors.find(actor->marker);
        if (entry == sSpawnAnchors.end()) {
            SpawnAnchor anchor;
            for (int i = 0; i < 3; i++) {
                anchor.position[i] = actor->position[i];
            }
            anchor.elapsed = 0.0f;
            anchor.moved = false;
            entry = sSpawnAnchors.emplace(actor->marker, anchor).first;
        }
        SpawnAnchor& anchor = entry->second;

        if (!anchor.moved) {
            float dx = actor->position[0] - anchor.position[0];
            float dy = actor->position[1] - anchor.position[1];
            float dz = actor->position[2] - anchor.position[2];
            if ((dx * dx + dy * dy + dz * dz) > (kHiddenMoveThreshold * kHiddenMoveThreshold)) {
                anchor.moved = true;
            } else {
                *should = false;
                return;
            }
        }

        if (anchor.elapsed < hidden->delay) {
            anchor.elapsed += time_getDelta();
            *should = false;
        }
    });

    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_WS_CAMERA_FIX, 1),
              [](IEvent* event) { sSpawnAnchors.clear(); });
}

static RegisterShipInitFunc widescreenXluScaleInitFunc(RegisterWidescreenXluScale, { CVAR_WS_CAMERA_FIX });
static RegisterShipInitFunc hiddenCutsceneActorsInitFunc(RegisterHiddenCutsceneActors, { CVAR_WS_CAMERA_FIX });
