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

#include "functions.h"
extern "C" {
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

/**
 * Every bitmask entry is written by one vanilla actor that lives in one known map, and
 * some entries are derived from -- or applied back to -- raw map/level flag indices whose
 * meaning is map-relative. A romhack that relocates such an actor makes it read one map's
 * flag and republish it as another's. To avoid complications, each puzzle id is scoped to
 * its actor's home: outside it every leg is inert.
 *
 * Vanilla is unaffected, and a romhack keeps live sync for every puzzle it didn't relocate.
 */
struct PuzzleHome {
    std::set<int32_t> maps;
    int32_t level = -1; // home is a whole level (romhacks may remap a level's maps)
};

static const std::map<int32_t, PuzzleHome>& puzzleHomes() {
    static const std::map<int32_t, PuzzleHome> homes = {
        { ANCHOR_PUZZLE_BGS_TANKTUP, { { MAP_D_BGS_BUBBLEGLOOP_SWAMP } } },
        { ANCHOR_PUZZLE_BGS_CROCTUS, { { MAP_D_BGS_BUBBLEGLOOP_SWAMP } } },
        // The egg chain's map isn't pinned down in the decomp; its readback only drives
        // actor state, so the whole level is safe.
        { ANCHOR_PUZZLE_BGS_PINKEGG, { {}, LEVEL_4_BUBBLEGLOOP_SWAMP } },
        { ANCHOR_PUZZLE_CC_CLANKER_TEETH, { { MAP_B_CC_CLANKERS_CAVERN } } },
        { ANCHOR_PUZZLE_GV_JINXY_DOOR, { { MAP_12_GV_GOBIS_VALLEY } } },
        { ANCHOR_PUZZLE_MM_JUJU, { { MAP_2_MM_MUMBOS_MOUNTAIN } } },
        { ANCHOR_PUZZLE_TTC_NIPPER, { { MAP_7_TTC_TREASURE_TROVE_COVE } } },
        { ANCHOR_PUZZLE_TTC_BLUBBER, { { MAP_7_TTC_TREASURE_TROVE_COVE } } },
        { ANCHOR_PUZZLE_TTC_XHUNT, { { MAP_7_TTC_TREASURE_TROVE_COVE } } },
        { ANCHOR_PUZZLE_FP_TREE_ICE, { { MAP_53_FP_CHRISTMAS_TREE } } },
        { ANCHOR_PUZZLE_FP_PRESENTS, { { MAP_27_FP_FREEZEEZY_PEAK } } },
        { ANCHOR_PUZZLE_FP_SNOWBUTTONS, { { MAP_27_FP_FREEZEEZY_PEAK } } },
        { ANCHOR_PUZZLE_FP_SLUSHES, { { MAP_27_FP_FREEZEEZY_PEAK } } },
        { ANCHOR_PUZZLE_RBB_ENGINE_FANS, { { MAP_34_RBB_ENGINE_ROOM } } },
        // Tutorial choreography follows the level, not the map.
        { ANCHOR_PUZZLE_SM_TUTORIAL, { {}, LEVEL_B_SPIRAL_MOUNTAIN } },
    };
    return homes;
}

static bool mirrorAllowedFor(int32_t puzzleId, int32_t map) {
    auto it = puzzleHomes().find(puzzleId);
    if (it == puzzleHomes().end()) {
        return false; // unknown id: fail inert
    }
    if (it->second.maps.count(map)) {
        return true;
    }
    return it->second.level >= 0 && (int32_t)map_getLevel((enum map_e)map) == it->second.level;
}

static bool mirrorLive(int32_t puzzleId) {
    return mirrorAllowedFor(puzzleId, (int32_t)gsworld_getMap());
}

extern "C" int32_t port_puzzleStep_get(int32_t puzzleId) {
    if (!mirrorLive(puzzleId)) {
        return 0;
    }
    auto it = sPuzzleBits.find({ (int32_t)gsworld_getMap(), puzzleId });
    return it != sPuzzleBits.end() ? it->second : 0;
}

extern "C" int32_t port_puzzleStep_getForMap(int32_t map, int32_t puzzleId) {
    if (!mirrorAllowedFor(puzzleId, map)) {
        return 0;
    }
    auto it = sPuzzleBits.find({ map, puzzleId });
    return it != sPuzzleBits.end() ? it->second : 0;
}

// For level-homed puzzles: a relocated actor writes under the romhack's map
// key, which a map-keyed read can't know. ORs the puzzle's bits across the level.
extern "C" int32_t port_puzzleStep_getForLevel(int32_t levelId, int32_t puzzleId) {
    int32_t bits = 0;
    for (const auto& [key, val] : sPuzzleBits) {
        if (key[1] == puzzleId && mirrorAllowedFor(puzzleId, key[0]) &&
            (int32_t)map_getLevel((enum map_e)key[0]) == levelId) {
            bits |= val;
        }
    }
    return bits;
}

extern "C" void port_puzzleStep_orBits(int32_t puzzleId, int32_t bits) {
    if (!mirrorLive(puzzleId)) {
        return;
    }
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
    if (!mirrorAllowedFor(puzzleId, map)) {
        return;
    }
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
        if (!mirrorAllowedFor(flat[i + 1], flat[i])) {
            continue;
        }
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
    if (!mirrorLive(puzzleId)) {
        return 0;
    }
    auto it = sPuzzlePos.find({ (int32_t)gsworld_getMap(), puzzleId });
    return (it != sPuzzlePos.end() && it->second.count(puzzlePosHash(x, y, z))) ? 1 : 0;
}

extern "C" void port_puzzlePos_mark(int32_t puzzleId, int32_t x, int32_t y, int32_t z) {
    if (!mirrorLive(puzzleId)) {
        return;
    }
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
        // Still consume the run's hashes when filtered so the stream stays aligned.
        const bool allowed = mirrorAllowedFor(puzzleId, map);
        for (int32_t j = 0; j < count && i < flat.size(); j++, i++) {
            if (allowed) {
                sPuzzlePos[{ map, puzzleId }].insert(flat[i]);
            }
        }
    }
}

/**
 * PUZZLE_COUNT
 *
 * Companion to PUZZLE_STEP for count-based progress (Eyrie's fed worms, Nabnut's acorns).
 * Not home-scoped: counters never touch flag indices (keyed by the real map, applied only
 * to actor state), and scoping them would brick a relocated Eyrie/Nabnut.
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
