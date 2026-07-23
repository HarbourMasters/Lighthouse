#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <array>
#include <map>
#include <set>
#include <vector>

extern "C" {
#include "functions.h"
void chTreasurehunt_netTick(void);
}

#include "port/Patches/Patches.h"

/**
 * PUZZLE_STEP
 *
 * Syncs multi-step world puzzles with no flag of their own (Tanktup's legs, croctus feed, pink
 * eggs). Progress is an OR-merged bitmask keyed by (map, puzzleId); never saved.
 */

std::map<std::array<int32_t, 2>, int32_t> sPuzzleBits;
std::map<std::array<int32_t, 2>, int32_t> sPuzzleCounts;
// PUZZLE_POS: (map, puzzleId) -> set of spawn-position hashes for positional puzzles (see below).
std::map<std::array<int32_t, 2>, std::set<int32_t>> sPuzzlePos;

static int32_t puzzlePosHash(int32_t x, int32_t y, int32_t z) {
    uint32_t h = (uint32_t)x * 73856093u ^ (uint32_t)y * 19349663u ^ (uint32_t)z * 83492791u;
    return (int32_t)((h & 0x7FFFFFFFu) | 1u); // never 0 (the "no positional hash" sentinel)
}

extern "C" int32_t port_puzzleStep_get(int32_t puzzleId) {
    auto it = sPuzzleBits.find({ (int32_t)gsworld_getMap(), puzzleId });
    return it != sPuzzleBits.end() ? it->second : 0;
}

extern "C" int32_t port_puzzleStep_getForMap(int32_t map, int32_t puzzleId) {
    auto it = sPuzzleBits.find({ map, puzzleId });
    return it != sPuzzleBits.end() ? it->second : 0;
}

extern "C" void port_puzzleStep_orBits(int32_t puzzleId, int32_t bits) {
    int32_t map = (int32_t)gsworld_getMap();
    std::array<int32_t, 2> key = { map, puzzleId };
    int32_t before = sPuzzleBits.count(key) ? sPuzzleBits[key] : 0;
    int32_t after = before | bits;
    if (after == before) {
        return;
    }
    sPuzzleBits[key] = after;
    Anchor::GetInstance()->SendPacket_PuzzleStep(puzzleId, after, map);
}

void Anchor::SendPacket_PuzzleStep(s32 puzzleId, s32 bits, s32 map, s32 phash) {
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
    // A positional puzzle ships its member's hash instead of a bit; 0 = an ordinary bitmask step.
    if (phash != 0) {
        payload["phash"] = phash;
    }

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_PuzzleStep(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s32 puzzleId = payload.at("puzzle").get<s32>();
    s32 map = payload.at("map").get<s32>();
    s32 phash = payload.value("phash", (s32)0);
    if (phash != 0) {
        sPuzzlePos[{ map, puzzleId }].insert(phash);
        return;
    }
    sPuzzleBits[{ map, puzzleId }] |= payload.at("bits").get<s32>();
}

std::vector<int32_t> port_puzzleStep_snapshot() {
    std::vector<int32_t> flat;
    flat.reserve(sPuzzleBits.size() * 3);
    for (const auto& [key, bits] : sPuzzleBits) {
        flat.push_back(key[0]);
        flat.push_back(key[1]);
        flat.push_back(bits);
    }
    return flat;
}

void port_puzzleStep_restore(const std::vector<int32_t>& flat) {
    sPuzzleBits.clear();
    for (size_t i = 0; i + 3 <= flat.size(); i += 3) {
        sPuzzleBits[{ flat[i], flat[i + 1] }] = flat[i + 2];
    }
}

void port_puzzleStep_clearForLevel(int32_t levelId) {
    std::erase_if(sPuzzleBits,
                  [levelId](const auto& kv) { return (int32_t)map_getLevel((enum map_e)kv.first[0]) == levelId; });
    std::erase_if(sPuzzleCounts,
                  [levelId](const auto& kv) { return (int32_t)map_getLevel((enum map_e)kv.first[0]) == levelId; });
    std::erase_if(sPuzzlePos,
                  [levelId](const auto& kv) { return (int32_t)map_getLevel((enum map_e)kv.first[0]) == levelId; });
}

/**
 * PUZZLE_POS
 *
 * Positional companion to PUZZLE_STEP for steps keyed by spawn position instead of a bit index
 * (FP's Sir Slushes). Rides the same PUZZLE_STEP packet via the phash field.
 */

extern "C" int32_t port_puzzlePos_isMarked(int32_t puzzleId, int32_t x, int32_t y, int32_t z) {
    auto it = sPuzzlePos.find({ (int32_t)gsworld_getMap(), puzzleId });
    return (it != sPuzzlePos.end() && it->second.count(puzzlePosHash(x, y, z))) ? 1 : 0;
}

extern "C" void port_puzzlePos_mark(int32_t puzzleId, int32_t x, int32_t y, int32_t z) {
    int32_t map = (int32_t)gsworld_getMap();
    int32_t hash = puzzlePosHash(x, y, z);
    auto& set = sPuzzlePos[{ map, puzzleId }];
    if (!set.insert(hash).second) {
        return;
    }
    Anchor::GetInstance()->SendPacket_PuzzleStep(puzzleId, 0, map, hash);
}

// Flat [map, puzzleId, count, hashes...] runs.
std::vector<int32_t> port_puzzlePos_snapshot() {
    std::vector<int32_t> flat;
    for (const auto& [key, set] : sPuzzlePos) {
        flat.push_back(key[0]);
        flat.push_back(key[1]);
        flat.push_back((int32_t)set.size());
        for (int32_t h : set) {
            flat.push_back(h);
        }
    }
    return flat;
}

void port_puzzlePos_restore(const std::vector<int32_t>& flat) {
    sPuzzlePos.clear();
    size_t i = 0;
    while (i + 3 <= flat.size()) {
        int32_t map = flat[i], puzzleId = flat[i + 1], count = flat[i + 2];
        i += 3;
        auto& set = sPuzzlePos[{ map, puzzleId }];
        for (int32_t j = 0; j < count && i < flat.size(); j++, i++) {
            set.insert(flat[i]);
        }
    }
}

/**
 * PUZZLE_COUNT
 *
 * Companion to PUZZLE_STEP for count-based progress (Eyrie's fed worms, Nabnut's acorns).
 */

extern "C" int32_t port_puzzleCount_get(int32_t counterId) {
    auto it = sPuzzleCounts.find({ (int32_t)gsworld_getMap(), counterId });
    return it != sPuzzleCounts.end() ? it->second : 0;
}

extern "C" void port_puzzleCount_add(int32_t counterId, int32_t delta) {
    int32_t map = (int32_t)gsworld_getMap();
    sPuzzleCounts[{ map, counterId }] += delta;
    Anchor::GetInstance()->SendPacket_PuzzleCount(counterId, delta, map);
}

void Anchor::SendPacket_PuzzleCount(s32 counterId, s32 delta, s32 map) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = PUZZLE_COUNT;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["counter"] = counterId;
    payload["delta"] = delta;
    payload["map"] = map;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_PuzzleCount(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s32 counterId = payload.at("counter").get<s32>();
    s32 delta = payload.at("delta").get<s32>();
    s32 map = payload.at("map").get<s32>();

    sPuzzleCounts[{ map, counterId }] += delta;
}

std::vector<int32_t> port_puzzleCount_snapshot() {
    std::vector<int32_t> flat;
    flat.reserve(sPuzzleCounts.size() * 3);
    for (const auto& [key, count] : sPuzzleCounts) {
        flat.push_back(key[0]);
        flat.push_back(key[1]);
        flat.push_back(count);
    }
    return flat;
}

void port_puzzleCount_restore(const std::vector<int32_t>& flat) {
    sPuzzleCounts.clear();
    for (size_t i = 0; i + 3 <= flat.size(); i += 3) {
        sPuzzleCounts[{ flat[i], flat[i + 1] }] = flat[i + 2];
    }
}

void RegisterPuzzleStep_Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        sPuzzleBits.clear();
        sPuzzleCounts.clear();
        sPuzzlePos.clear();
    });
    // Treasure hunt has no actor to poll until the first X is busted.
    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        if (gsworld_getMap() == MAP_7_TTC_TREASURE_TROVE_COVE) {
            chTreasurehunt_netTick();
        }
    });
}

static RegisterShipInitFunc initPuzzleStep(RegisterPuzzleStep_Init, {});
