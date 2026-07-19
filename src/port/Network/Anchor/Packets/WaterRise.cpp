#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <map>

extern "C" {
#include "functions.h"
#include "variables.h"
// func_8034C5AC (xform-3 water group) and func_8034E78C (animate its dy) come from functions.h; used for
// the CC remote replay below.
}

/**
 * WATER_RISE
 *
 * A rising water level animates only on the client that triggered it (lair: the switch cutscene in
 * func_802D5628/func_802D5260; CC rings: func_8034E78C on completion). A remote only receives the
 * trailing side effect — the water-level fileprog flag, or the JIGGY_1C spawn — at the END of that
 * animation, so its water snaps. This packet tells teammates the rise STARTED so they animate in step.
 *
 *   kind 0 (lair): the RBB-lobby water subaddie, driven every frame by func_802D5260. p1 = target level
 *     (1-3). A remote stashes it as a "pending level"; func_802D5260 eases toward that until the real
 *     flag catches up (then it's cleared).
 *   kind 1 (CC): the rings water (xform-3 dy) has no per-frame holder, so a remote just plays the same
 *     animated func_8034E78C. p1 = mesh id, p2 = target dy, dur = seconds.
 */

enum { WATERRISE_KIND_LAIR = 0, WATERRISE_KIND_CC = 1 };

// map -> pending target level for the lair water (remote only; cleared once the flag reaches it).
static std::map<int32_t, int32_t> sLairPendingLevel;

static int32_t lairLevelForFlag(int32_t levelFlag) {
    switch (levelFlag) {
        case FILEPROG_23_LAIR_WATER_LEVEL_1: return 1;
        case FILEPROG_25_LAIR_WATER_LEVEL_2: return 2;
        case FILEPROG_27_LAIR_WATER_LEVEL_3: return 3;
        default:                             return 0;
    }
}

// Called from func_802D6264 (the lair switch/cutscene trigger) at the press. If the flag that the cutscene
// will set at its end is a water-level flag, broadcast the rise now so teammates start animating in step
// rather than waiting for that flag to sync ~6s later.
extern "C" void port_lairWater_onRiseTrigger(int32_t waterMap, int32_t levelFlag) {
    int32_t level = lairLevelForFlag(levelFlag);
    if (level == 0) {
        return; // not a water switch (doors, witch switches, etc. use this same trigger)
    }
    Anchor::GetInstance()->SendPacket_WaterRise(waterMap, WATERRISE_KIND_LAIR, level, 0, 0.0f);
}

// func_802D5260 asks for the effective target level: max(flag-derived level, a not-yet-synced pending level
// from a teammate's in-flight rise). Clears the pending once the real flag has caught up.
extern "C" int32_t port_lairWater_targetLevel(int32_t map, int32_t flagLevel) {
    auto it = sLairPendingLevel.find(map);
    if (it == sLairPendingLevel.end()) {
        return flagLevel;
    }
    if (flagLevel >= it->second) {
        sLairPendingLevel.erase(it); // real flag arrived — pending no longer needed
        return flagLevel;
    }
    return it->second;
}

// CC rings: broadcast the animated rise (mesh id, target dy, duration) as the local run completes.
extern "C" void port_ccWater_broadcastRise(int32_t map, int32_t waterId, int32_t targetDy, f32 duration) {
    Anchor::GetInstance()->SendPacket_WaterRise(map, WATERRISE_KIND_CC, waterId, targetDy, duration);
}

void Anchor::SendPacket_WaterRise(s32 map, s32 kind, s32 p1, s32 p2, f32 duration) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }
    nlohmann::json payload;
    payload["type"] = WATER_RISE;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["map"] = map;
    payload["kind"] = kind;
    payload["p1"] = p1;
    payload["p2"] = p2;
    payload["dur"] = duration;
    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_WaterRise(nlohmann::json& payload) {
    if (!roomState.syncItemsAndFlags) {
        return;
    }
    s32 map = payload.at("map").get<s32>();
    s32 kind = payload.at("kind").get<s32>();
    s32 p1 = payload.at("p1").get<s32>();
    s32 p2 = payload.value("p2", (s32)0);
    f32 dur = payload.value("dur", 0.0f);

    if (kind == WATERRISE_KIND_LAIR) {
        int32_t cur = sLairPendingLevel.count(map) ? sLairPendingLevel[map] : 0;
        if (p1 > cur) {
            sLairPendingLevel[map] = p1;
        }
    } else if (kind == WATERRISE_KIND_CC) {
        // Only meaningful while we're actually in the water's map; on later entry func_80388104 sets it
        // correctly (snapped) on its own.
        if ((s32)gsworld_getMap() == map) {
            Struct70s* water = func_8034C5AC(p1);
            if (water != nullptr) {
                func_8034E78C((Struct73s*)water, p2, dur);
            }
        }
    }
}

void RegisterWaterRise_Init() {
    // Pending rises are transient (cleared when their flag lands); drop any leftovers on save load so they
    // never leak across files.
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sLairPendingLevel.clear(); });
}

static RegisterShipInitFunc initWaterRise(RegisterWaterRise_Init, {});
