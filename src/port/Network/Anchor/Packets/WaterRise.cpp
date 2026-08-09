#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <map>

#include "functions.h"
extern "C" {
#include "variables.h"
// func_8034C5AC: xform-3 water group. func_8034E78C: animate its dy.
}

/**
 * WATER_RISE
 *
 * A rising water level animates only on the client that triggered it (lair: switch cutscene in
 * func_802D5628/func_802D5260; CC rings: func_8034E78C). Remotes otherwise only get the trailing
 * fileprog flag / JIGGY_1C spawn at the end, snapping the water. This packet signals the rise start.
 *
 *   kind 0 (lair): RBB-lobby water subaddie, driven every frame by func_802D5260. p1 = target level
 *     (1-3), stashed by a remote as a "pending level" until the real flag catches up.
 *   kind 1 (CC): rings water (xform-3 dy), no per-frame holder; remote replays func_8034E78C.
 *     p1 = mesh id, p2 = target dy, dur = seconds.
 */

enum { WATERRISE_KIND_LAIR = 0, WATERRISE_KIND_CC = 1 };

// map -> pending target level for the lair water (remote only; cleared once the flag reaches it).
static std::map<int32_t, int32_t> sLairPendingLevel;

static int32_t lairLevelForFlag(int32_t levelFlag) {
    switch (levelFlag) {
        case FILEPROG_23_LAIR_WATER_LEVEL_1:
            return 1;
        case FILEPROG_25_LAIR_WATER_LEVEL_2:
            return 2;
        case FILEPROG_27_LAIR_WATER_LEVEL_3:
            return 3;
        default:
            return 0;
    }
}

// Called from func_802D6264 (the lair switch/cutscene trigger). If the flag the cutscene will set is
// a water-level flag, broadcast the rise now.
extern "C" void port_lairWater_onRiseTrigger(int32_t waterMap, int32_t levelFlag) {
    int32_t level = lairLevelForFlag(levelFlag);
    if (level == 0) {
        return; // not a water switch (doors, witch switches, etc. use this same trigger)
    }
    Anchor::GetInstance()->SendPacket_WaterRise(waterMap, WATERRISE_KIND_LAIR, level, 0, 0.0f);
}

// func_802D5260 asks for the effective target level: max(flag-derived level, pending level from a
// teammate's in-flight rise). Clears the pending once the real flag catches up.
extern "C" int32_t port_lairWater_targetLevel(int32_t map, int32_t flagLevel) {
    auto it = sLairPendingLevel.find(map);
    if (it == sLairPendingLevel.end()) {
        return flagLevel;
    }
    if (flagLevel >= it->second) {
        sLairPendingLevel.erase(it);
        return flagLevel;
    }
    return it->second;
}

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
        // Only replay while in the water's map; on later entry func_80388104 snaps it on its own.
        if ((s32)gsworld_getMap() == map) {
            Struct70s* water = func_8034C5AC(p1);
            if (water != nullptr) {
                func_8034E78C((Struct73s*)water, p2, dur);
            }
        }
    }
}

void RegisterWaterRise_Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sLairPendingLevel.clear(); });
}

static RegisterShipInitFunc initWaterRise(RegisterWaterRise_Init, {});
