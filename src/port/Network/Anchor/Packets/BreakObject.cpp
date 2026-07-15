#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <array>
#include <set>

extern "C" {
#include "functions.h"
}

#include "port/Patches/Patches.h"

/**
 * BREAK_OBJECT
 *
 * Live + temporary-persistence sync of a non-persistent breakable (glass windows, etc.) that
 * breaks + despawns with no synced flag of its own. These objects are static, so their spawn
 * position is identical on every client; we key them by (map, marker, position).
 *
 * The break is recorded in an in-memory set (never written to the save) and broadcast team-wide,
 * so every client remembers it for the session. A teammate in the same map replays the real break
 * live; everyone else applies it at spawn (the object despawns on (re)load if it was broken). The
 * set is cleared on save load so it never leaks across files.
 */

namespace {
// (map, marker, x, y, z) of every non-persistent breakable the team has broken this session.
std::set<std::array<int32_t, 5>> sBroken;
} // namespace

extern "C" int32_t port_breakable_isBroken(int32_t map, int32_t markerId, int32_t x, int32_t y, int32_t z) {
    return sBroken.count({ map, markerId, x, y, z }) != 0 ? 1 : 0;
}

// Snapshot/restore for the authoritative team-state sync (UpdateTeamState.cpp). Flat [map, marker,
// x, y, z] tuples. Restore overwrites — a client adopting team state takes that session's set.
std::vector<int32_t> port_breakable_snapshotBroken() {
    std::vector<int32_t> flat;
    flat.reserve(sBroken.size() * 5);
    for (const auto& e : sBroken) {
        flat.insert(flat.end(), e.begin(), e.end());
    }
    return flat;
}

void port_breakable_restoreBroken(const std::vector<int32_t>& flat) {
    sBroken.clear();
    for (size_t i = 0; i + 5 <= flat.size(); i += 5) {
        sBroken.insert({ flat[i], flat[i + 1], flat[i + 2], flat[i + 3], flat[i + 4] });
    }
}

// Occupancy sweep (Anchor::SweepUnoccupiedLevelState): vanilla never persists these breaks, so
// once a level has no players left in it, its records must go or it stays broken all session.
void port_breakable_clearForLevel(int32_t levelId) {
    std::erase_if(sBroken,
                  [levelId](const std::array<int32_t, 5>& e) { return (int32_t)map_getLevel((enum map_e)e[0]) == levelId; });
}

void Anchor::SendPacket_BreakObject(s16 markerId, s32 x, s32 y, s32 z, s32 map, bool replay) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = BREAK_OBJECT;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["marker"] = markerId;
    payload["x"] = x;
    payload["y"] = y;
    payload["z"] = z;
    payload["map"] = map;
    // replay=false: the object replays its own break by polling the broken set each frame (CC
    // grates run a break/rise state machine that func_802D31AC can't reproduce), so we only want
    // the receiver to record it, not invoke the generic remote-break handler.
    payload["replay"] = replay;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_BreakObject(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 markerId = payload.at("marker").get<s16>();
    s32 x = payload.at("x").get<s32>();
    s32 y = payload.at("y").get<s32>();
    s32 z = payload.at("z").get<s32>();
    s32 map = payload.at("map").get<s32>();
    bool replay = payload.contains("replay") ? payload.at("replay").get<bool>() : true;

    // Remember it for the session regardless of where we are (so it despawns at spawn when we
    // next load that map)...
    sBroken.insert({ map, markerId, x, y, z });
    // ...and replay the real break live if we're currently in that map. Self-polled breakables
    // (replay=false) skip this — they notice the recorded break on their next update themselves.
    if (replay && (s32)gsworld_getMap() == map) {
        port_breakable_remoteBreakAt(markerId, x, y, z);
    }
}

// C-callable broadcast trigger from the shared collision-die handler in game.c. Records our own
// break too (so it persists for us on revisit) and broadcasts it to the team.
extern "C" void port_breakable_broadcastBreak(int32_t markerId, int32_t x, int32_t y, int32_t z) {
    s32 map = (s32)gsworld_getMap();
    // Idempotent by identity: the broken set is the registration that dedupes. If this object is
    // already recorded (we're replaying a teammate's break we recorded on receipt, or a deferred
    // break fires after the first), don't re-broadcast. Callers therefore never need an
    // "am I replaying" guard flag — the set, not a global bool, prevents the echo.
    if (sBroken.count({ map, markerId, x, y, z }) != 0) {
        return;
    }
    sBroken.insert({ map, markerId, x, y, z });
    Anchor::GetInstance()->SendPacket_BreakObject((s16)markerId, x, y, z, map);
}

// Same as broadcastBreak but for objects that replay their own break by polling port_breakable_isBroken
// each update (CC grates), so the receiver records the break without running the generic remote-break
// handler. Persistence + team-state still ride the shared broken set.
extern "C" void port_breakable_recordBreak(int32_t markerId, int32_t x, int32_t y, int32_t z) {
    s32 map = (s32)gsworld_getMap();
    if (sBroken.count({ map, markerId, x, y, z }) != 0) {
        return;
    }
    sBroken.insert({ map, markerId, x, y, z });
    Anchor::GetInstance()->SendPacket_BreakObject((s16)markerId, x, y, z, map, false);
}

void RegisterBreakObject_Init() {
    // Never written to the save — drop it on save load so it can't leak across files.
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sBroken.clear(); });
}

static RegisterShipInitFunc initBreakObject(RegisterBreakObject_Init, {});
