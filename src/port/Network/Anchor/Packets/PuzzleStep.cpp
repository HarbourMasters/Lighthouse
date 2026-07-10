#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <array>
#include <map>
#include <vector>

extern "C" {
#include "functions.h"
// Catch-up tick for the TTC treasure hunt (treasurehunt.c): its progress is a bare global with no
// always-present actor to poll from, so the anchor frame hook drives the replay of teammate steps.
void chTreasurehunt_netTick(void);
}

#include "port/Patches/Patches.h"

/**
 * PUZZLE_STEP
 *
 * Live + temporary-persistence sync of multi-step world puzzles whose progress lives only in
 * actor-local data with no flag of its own (BGS Tanktup's four legs, the croctus feed chain, the
 * nested pink eggs). Each completed sub-step is one bit; the per-puzzle progress is an OR-merged
 * bitmask kept in an in-memory map (never saved), broadcast team-wide, and restored at spawn. Each
 * actor polls its puzzle's mask every update and replays any teammate steps it hasn't applied yet.
 *
 * Puzzles are keyed by (map, puzzleId). puzzleId is a stable per-puzzle constant (see Patches.h);
 * keying by map too lets the small ids stay unique. Bits are monotonic, so OR is the only merge.
 * Cleared on save load so it never leaks across files.
 */

namespace {
std::map<std::array<int32_t, 2>, int32_t> sBits;
} // namespace

extern "C" int32_t port_puzzleStep_get(int32_t puzzleId) {
    auto it = sBits.find({ (int32_t)gsworld_getMap(), puzzleId });
    return it != sBits.end() ? it->second : 0;
}

extern "C" int32_t port_puzzleStep_getForMap(int32_t map, int32_t puzzleId) {
    auto it = sBits.find({ map, puzzleId });
    return it != sBits.end() ? it->second : 0;
}

extern "C" void port_puzzleStep_orBits(int32_t puzzleId, int32_t bits) {
    int32_t map = (int32_t)gsworld_getMap();
    std::array<int32_t, 2> key = { map, puzzleId };
    int32_t before = sBits.count(key) ? sBits[key] : 0;
    int32_t after = before | bits;
    // Idempotent by identity: the bitmask is the registration. If these bits are already set (we're
    // replaying a teammate's step we already recorded, by re-running the same local break/feed path),
    // nothing changed, so don't re-broadcast. No "am I replaying" guard flag is needed for dedup.
    if (after == before) {
        return;
    }
    sBits[key] = after;
    Anchor::GetInstance()->SendPacket_PuzzleStep(puzzleId, after, map);
}

void Anchor::SendPacket_PuzzleStep(s32 puzzleId, s32 bits, s32 map) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = PUZZLE_STEP;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["puzzle"] = puzzleId;
    payload["bits"] = bits;
    payload["map"] = map;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_PuzzleStep(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s32 puzzleId = payload.at("puzzle").get<s32>();
    s32 bits = payload.at("bits").get<s32>();
    s32 map = payload.at("map").get<s32>();

    // Record (OR-merge) regardless of our map; the actors replay each new step themselves on their
    // next update, both live (in-map) and at spawn (in-memory persistence on the next visit).
    sBits[{ map, puzzleId }] |= bits;
}

// Authoritative team-state snapshot/restore (UpdateTeamState.cpp). Flat [map, puzzleId, bits].
std::vector<int32_t> port_puzzleStep_snapshot() {
    std::vector<int32_t> flat;
    flat.reserve(sBits.size() * 3);
    for (const auto& [key, bits] : sBits) {
        flat.push_back(key[0]);
        flat.push_back(key[1]);
        flat.push_back(bits);
    }
    return flat;
}

void port_puzzleStep_restore(const std::vector<int32_t>& flat) {
    sBits.clear();
    for (size_t i = 0; i + 3 <= flat.size(); i += 3) {
        sBits[{ flat[i], flat[i + 1] }] = flat[i + 2];
    }
}

void RegisterPuzzleStep_Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sBits.clear(); });
    // The treasure hunt's progress lives in a global (CH_TREASUREHUNT_PUZZLE_CURRENT_STEP) with no
    // actor spawned until the first X is busted, so nothing exists to poll the mask from an update
    // func. Tick it from the frame hook instead; it no-ops unless the shared mask is ahead.
    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        if (gsworld_getMap() == MAP_7_TTC_TREASURE_TROVE_COVE) {
            chTreasurehunt_netTick();
        }
    });
}

static RegisterShipInitFunc initPuzzleStep(RegisterPuzzleStep_Init, {});
