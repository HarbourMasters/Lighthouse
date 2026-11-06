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
// Map the lair water was last leveled for, so re-entry snaps instead of ramping.
static s32 sLairWaterMap = -1;

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

// The effective target level: max(flag-derived level, pending level from a teammate's in-flight rise).
// Clears the pending once the real flag catches up.
static int32_t lairTargetLevel(int32_t map, int32_t flagLevel) {
    auto it = sLairPendingLevel.find(map);
    if (it == sLairPendingLevel.end() || !Anchor::GetInstance()->IsWorldSyncActive()) {
        return flagLevel;
    }
    if (flagLevel >= it->second) {
        sLairPendingLevel.erase(it);
        return flagLevel;
    }
    return it->second;
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
        // The switch cutscene plays in one of the three water rooms; all three share the water.
        for (int32_t waterMap : { MAP_76_GL_640_NOTE_DOOR, MAP_77_GL_RBB_LOBBY, MAP_78_GL_RBB_AND_MMM_PUZZLE }) {
            int32_t cur = sLairPendingLevel.count(waterMap) ? sLairPendingLevel[waterMap] : 0;
            if (p1 > cur) {
                sLairPendingLevel[waterMap] = p1;
            }
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

// A teammate's rise levels our water to its pending level, and while synced the water ramps there rather than
// jumping. Draining stays vanilla.
static void LairWaterLevel(IEvent* event) {
    auto* ev = (OnLairWaterLevel*)event;
    const s32 map = (s32)gsworld_getMap();
    const bool draining = levelSpecificFlags_get(LEVEL_FLAG_3C_LAIR_UNKNOWN);
    if (!draining) {
        const s32 flagLevel = fileProgressFlag_get(FILEPROG_27_LAIR_WATER_LEVEL_3)   ? 3
                              : fileProgressFlag_get(FILEPROG_25_LAIR_WATER_LEVEL_2) ? 2
                              : fileProgressFlag_get(FILEPROG_23_LAIR_WATER_LEVEL_1) ? 1
                                                                                     : 0;
        *ev->height = ev->levels[lairTargetLevel(map, flagLevel)];
    }
    if (sLairWaterMap != map || draining || !Anchor::GetInstance()->IsWorldSyncActive()) {
        sLairWaterMap = map;
        return;
    }
    const f32 step = 240.0f * time_getDelta();
    const f32 diff = *ev->height - ev->current;
    if (diff > step) {
        *ev->height = ev->current + step;
    } else if (diff < -step) {
        *ev->height = ev->current - step;
    }
}

void RegisterWaterRise_Init() {
    // At boot, with the rest of this file: a teammate's rise counts only while world sync is live, and the sends do
    // nothing until a save is loaded in a syncing room.
    COND_HOOK(OnSaveLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) { sLairPendingLevel.clear(); });
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) { sLairWaterMap = -1; });
    // A water-level flag landing settles a pending rise in all three rooms, wherever we are: outside them, switch 3's
    // flag clears again at once, and its rise would otherwise wait for us forever.
    COND_HOOK(OnGameFlagSet, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnGameFlagSet*)event;
        const int32_t level = lairLevelForFlag(ev->index);
        if (ev->flagSpace == ANCHOR_FLAGSPACE_FILE_PROGRESS && ev->value != 0 && level != 0) {
            std::erase_if(sLairPendingLevel, [level](const auto& kv) { return kv.second <= level; });
        }
    });
    COND_HOOK(OnLairWaterLevel, EVENT_PRIORITY_NORMAL, true, LairWaterLevel);
    // If the flag the cutscene will set is a water-level flag, broadcast the rise now.
    COND_HOOK(OnCutsceneWarp, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnCutsceneWarp*)event;
        const int32_t level = lairLevelForFlag(ev->progressFlag);
        if (level != 0) {
            Anchor::GetInstance()->SendPacket_WaterRise(ev->map, WATERRISE_KIND_LAIR, level, 0, 0.0f);
        }
    });
    COND_HOOK(OnWaterRise, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnWaterRise*)event;
        Anchor::GetInstance()->SendPacket_WaterRise(ev->map, WATERRISE_KIND_CC, ev->waterId, ev->targetDy,
                                                    ev->duration);
    });
}

static RegisterShipInitFunc initWaterRise(RegisterWaterRise_Init, {});
