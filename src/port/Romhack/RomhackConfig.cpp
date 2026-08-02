#include "port/Romhack/RomhackConfig.h"
#include "RomhackTable.h"

#include <unordered_map>
#include <string>
#include <cstring>
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <libultraship/libultraship.h>

/*
    RomhackConfig loads and parses the aGameConfig blob from romhack
    o2r archives and exposes the relevant overrides to the decompilation
    (via the port_getRomhack* accessors declared in RomhackConfig.h).
    It is what makes romhacks function properly beyond simple asset swaps.
*/

static bool sGameConfigLoaded = false;
static bool sIsRomhack = false;

static std::unordered_map<int, int> sSceneRemaps;
static std::unordered_map<int, std::pair<int, int>> sReturnToLair;
static std::unordered_map<int, std::pair<int, int>> sMusicAssign;
struct SkyboxLayerData {
    int models[3];
    float scales[3];
    float rotations[3];
};
static std::unordered_map<int, SkyboxLayerData> sSkyboxAssign;
struct SceneDefData {
    int opa;
    int xlu;
    short min[3];
    short max[3];
    float scale;
};
static std::unordered_map<int, SceneDefData> sSceneDefs;

static int sNewGameMap = -1;
static int sStartLevel1 = -1;
static int sStartLevel2 = -1;
static int sKnowAllMoves = -1;
static int sMumboCostTermite = -1;
static int sMumboCostCroc = -1;
static int sMumboCostWalrus = -1;
static int sMumboCostPumpkin = -1;
static int sMumboCostBee = -1;
static int sEggsNormalMax = -1;
static int sEggsCheatomax = -1;
static int sRedFeathersNormalMax = -1;
static int sRedFeathersCheatomax = -1;
static int sGoldFeathersNormalMax = -1;
static int sGoldFeathersCheatomax = -1;
static int sNotesMax = -1;
static int sJiggiesPerWorld = -1;
static int sHoneycombsPerWorld = -1;
static int sExtraHcStart = -1;
static int sWarpExitBanjosHouse = -1;
static int sWarpEnterLair = -1;
static int sSpecialLevel = -1;
static int sHideJiggiesLevel = -1;
static int sHideCollectiblesLevel = -1;
static int sNoteDoors[12] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static int sJiggyCosts[11] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static int sJiggySizes[11] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static int sJiggyFlags[11] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static char* sLevelNames[13] = {};
static std::unordered_map<int, int> sWarpDests;
static std::string sRomName;

// Custom code anchor populated by the CUSTOM_CODE section. Empty when the
// loaded romhack has no detectable injected MIPS code.
static std::string sCustomCodeHashHex;
static uint32_t sCustomCodeRamBase = 0;
static int sCustomCodeKind = -1;

// ROM SHA1 — populated by the ROM_HASH section. Always emitted by Torch for
// any romhack; serves as the fallback identifier when no custom-code blob is
// present.
static std::string sRomHashHex;

static constexpr uint32_t GAMECONFIG_MAGIC = 0x46434B42;

static uint16_t readLE16(const uint8_t* p) {
    return p[0] | (p[1] << 8);
}
static uint32_t readLE32(const uint8_t* p) {
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

enum {
    CCK_NEW_GAME_MAP = 0,
    CCK_START_LEVEL_1,
    CCK_START_LEVEL_2,
    CCK_KNOW_ALL_MOVES,
    CCK_MUMBO_COST_TERMITE,
    CCK_MUMBO_COST_CROC,
    CCK_MUMBO_COST_WALRUS,
    CCK_MUMBO_COST_PUMPKIN,
    CCK_MUMBO_COST_BEE,
    CCK_EGGS_NORMAL_MAX,
    CCK_RED_FEATHERS_NORMAL_MAX,
    CCK_GOLD_FEATHERS_NORMAL_MAX,
    CCK_EGGS_CHEATO_MAX,
    CCK_RED_FEATHERS_CHEATO_MAX,
    CCK_GOLD_FEATHERS_CHEATO_MAX,
    CCK_NOTES_MAX,
    CCK_JIGGIES_PER_WORLD,
    CCK_HONEYCOMBS_PER_WORLD,
    CCK_EXTRA_HC_START,
    CCK_WARP_EXIT_BANJOS_HOUSE,
    CCK_WARP_ENTER_LAIR,
    CCK_SPECIAL_LEVEL,
    CCK_HIDE_JIGGIES_LEVEL,
    CCK_HIDE_COLLECTIBLES_LEVEL,
};

static void LoadGameConfig() {
    if (sGameConfigLoaded) {
        return;
    }
    sGameConfigLoaded = true;

    auto rm = Ship::Context::GetRawInstance()->GetResourceManager();
    auto archive = rm->GetArchiveManager();

    auto res = archive->LoadFile("assets/aGameConfig");
    if (!res || !res->IsLoaded || !res->Buffer) {
        return;
    }

    const uint8_t* data = reinterpret_cast<const uint8_t*>(res->Buffer->data());
    size_t size = res->Buffer->size();

    if (size < 8) {
        SPDLOG_WARN("[GameConfig] File too small ({} bytes)", size);
        return;
    }

    uint32_t magic = readLE32(data);
    if (magic != GAMECONFIG_MAGIC) {
        SPDLOG_WARN("[GameConfig] Bad magic: 0x{:08X} (expected 0x{:08X})", magic, GAMECONFIG_MAGIC);
        return;
    }

    uint16_t version = readLE16(data + 4);
    uint16_t sectionCount = readLE16(data + 6);

    size_t pos = 8;
    if (pos < size) {
        uint8_t nameLen = data[pos++];
        if (pos + nameLen <= size) {
            sRomName.assign(reinterpret_cast<const char*>(data + pos), nameLen);
            pos += nameLen;
        }
    }

    sIsRomhack = true;

    for (uint16_t s = 0; s < sectionCount && pos + 4 <= size; s++) {
        uint16_t secType = readLE16(data + pos);
        uint16_t entryCount = readLE16(data + pos + 2);
        pos += 4;

        switch (secType) {
            case 1: // CODE_CONSTANTS
                for (uint16_t e = 0; e < entryCount && pos + 4 <= size; e++) {
                    uint16_t key = readLE16(data + pos);
                    uint16_t val = readLE16(data + pos + 2);
                    pos += 4;

                    int* target = nullptr;
                    const char* name = "?";
                    switch (key) {
                        case CCK_NEW_GAME_MAP:
                            target = &sNewGameMap;
                            name = "new_game_map";
                            break;
                        case CCK_START_LEVEL_1:
                            target = &sStartLevel1;
                            name = "start_level_1";
                            break;
                        case CCK_START_LEVEL_2:
                            target = &sStartLevel2;
                            name = "start_level_2";
                            break;
                        case CCK_KNOW_ALL_MOVES:
                            target = &sKnowAllMoves;
                            name = "know_all_moves";
                            break;
                        case CCK_MUMBO_COST_TERMITE:
                            target = &sMumboCostTermite;
                            name = "mumbo_cost_termite";
                            break;
                        case CCK_MUMBO_COST_CROC:
                            target = &sMumboCostCroc;
                            name = "mumbo_cost_croc";
                            break;
                        case CCK_MUMBO_COST_WALRUS:
                            target = &sMumboCostWalrus;
                            name = "mumbo_cost_walrus";
                            break;
                        case CCK_MUMBO_COST_PUMPKIN:
                            target = &sMumboCostPumpkin;
                            name = "mumbo_cost_pumpkin";
                            break;
                        case CCK_MUMBO_COST_BEE:
                            target = &sMumboCostBee;
                            name = "mumbo_cost_bee";
                            break;
                        case CCK_EGGS_NORMAL_MAX:
                            target = &sEggsNormalMax;
                            name = "eggs_normal_max";
                            break;
                        case CCK_RED_FEATHERS_NORMAL_MAX:
                            target = &sRedFeathersNormalMax;
                            name = "red_feathers_normal_max";
                            break;
                        case CCK_GOLD_FEATHERS_NORMAL_MAX:
                            target = &sGoldFeathersNormalMax;
                            name = "gold_feathers_normal_max";
                            break;
                        case CCK_EGGS_CHEATO_MAX:
                            target = &sEggsCheatomax;
                            name = "eggs_cheato_max";
                            break;
                        case CCK_RED_FEATHERS_CHEATO_MAX:
                            target = &sRedFeathersCheatomax;
                            name = "red_feathers_cheato_max";
                            break;
                        case CCK_GOLD_FEATHERS_CHEATO_MAX:
                            target = &sGoldFeathersCheatomax;
                            name = "gold_feathers_cheato_max";
                            break;
                        case CCK_NOTES_MAX:
                            target = &sNotesMax;
                            name = "notes_max";
                            break;
                        case CCK_JIGGIES_PER_WORLD:
                            target = &sJiggiesPerWorld;
                            name = "jiggies_per_world";
                            break;
                        case CCK_HONEYCOMBS_PER_WORLD:
                            target = &sHoneycombsPerWorld;
                            name = "honeycombs_per_world";
                            break;
                        case CCK_EXTRA_HC_START:
                            target = &sExtraHcStart;
                            name = "extra_hc_start";
                            break;
                        case CCK_WARP_EXIT_BANJOS_HOUSE:
                            target = &sWarpExitBanjosHouse;
                            name = "warp_exit_banjos_house";
                            break;
                        case CCK_WARP_ENTER_LAIR:
                            target = &sWarpEnterLair;
                            name = "warp_enter_lair";
                            break;
                        case CCK_SPECIAL_LEVEL:
                            target = &sSpecialLevel;
                            name = "special_level";
                            break;
                        case CCK_HIDE_JIGGIES_LEVEL:
                            target = &sHideJiggiesLevel;
                            name = "hide_jiggies_level";
                            break;
                        case CCK_HIDE_COLLECTIBLES_LEVEL:
                            target = &sHideCollectiblesLevel;
                            name = "hide_collectibles_level";
                            break;
                        default:
                            break;
                    }
                    if (target) {
                        *target = val;
                    }
                }
                break;

            case 2: // SCENE_REMAP
                for (uint16_t e = 0; e < entryCount && pos + 4 <= size; e++) {
                    int mapId = readLE16(data + pos);
                    int levelId = readLE16(data + pos + 2);
                    pos += 4;
                    sSceneRemaps[mapId] = levelId;
                }
                break;

            case 3: // RETURN_TO_LAIR
                for (uint16_t e = 0; e < entryCount && pos + 6 <= size; e++) {
                    int idx = data[pos];
                    int map = readLE16(data + pos + 2);
                    int exit = readLE16(data + pos + 4);
                    pos += 6;
                    sReturnToLair[idx] = { map, exit };
                }
                break;

            case 4: // MUSIC
                for (uint16_t e = 0; e < entryCount && pos + 6 <= size; e++) {
                    int mapId = readLE16(data + pos);
                    int track1 = readLE16(data + pos + 2);
                    int track2 = readLE16(data + pos + 4);
                    pos += 6;
                    sMusicAssign[mapId] = { track1, track2 };
                }
                break;

            case 5: // SKYBOX
                for (uint16_t e = 0; e < entryCount && pos + 32 <= size; e++) {
                    int sceneId = readLE16(data + pos);
                    SkyboxLayerData sky = {};
                    for (int layer = 0; layer < 3; layer++) {
                        size_t base = pos + 2 + layer * 10;
                        sky.models[layer] = static_cast<int16_t>(readLE16(data + base));
                        uint32_t scaleBits = readLE32(data + base + 2);
                        uint32_t rotBits = readLE32(data + base + 6);
                        memcpy(&sky.scales[layer], &scaleBits, 4);
                        memcpy(&sky.rotations[layer], &rotBits, 4);
                    }
                    pos += 32;
                    sSkyboxAssign[sceneId] = sky;
                }
                break;

            case 6: // SCENE_DEF
                for (uint16_t e = 0; e < entryCount && pos + 22 <= size; e++) {
                    SceneDefData sd = {};
                    int sceneId = readLE16(data + pos);
                    sd.opa = static_cast<int16_t>(readLE16(data + pos + 2));
                    sd.xlu = static_cast<int16_t>(readLE16(data + pos + 4));
                    for (int b = 0; b < 3; b++) {
                        sd.min[b] = static_cast<int16_t>(readLE16(data + pos + 6 + b * 2));
                    }
                    for (int b = 0; b < 3; b++) {
                        sd.max[b] = static_cast<int16_t>(readLE16(data + pos + 12 + b * 2));
                    }
                    uint32_t scaleBits = readLE32(data + pos + 18);
                    memcpy(&sd.scale, &scaleBits, 4);
                    pos += 22;
                    sSceneDefs[sceneId] = sd;
                }
                break;

            case 7: // NOTE_DOORS
                for (uint16_t e = 0; e < entryCount && pos + 4 <= size; e++) {
                    int idx = data[pos];
                    int threshold = readLE16(data + pos + 2);
                    pos += 4;
                    if (idx >= 0 && idx < 12) {
                        sNoteDoors[idx] = threshold;
                    }
                }
                break;

            case 8: // JIGGY_PUZZLES
                for (uint16_t e = 0; e < entryCount && pos + 6 <= size; e++) {
                    int idx = data[pos];
                    int cost = data[pos + 1];
                    int sizeBits = data[pos + 2];
                    int flag = readLE16(data + pos + 4);
                    pos += 6;
                    if (idx >= 0 && idx < 11) {
                        sJiggyCosts[idx] = cost;
                        sJiggySizes[idx] = sizeBits;
                        sJiggyFlags[idx] = flag;
                    }
                }
                break;

            case 9: // LEVEL_NAMES
                for (uint16_t e = 0; e < entryCount && pos + 2 <= size; e++) {
                    int idx = data[pos];
                    int len = data[pos + 1];
                    pos += 2;
                    if (pos + len > size) {
                        break;
                    }
                    if (idx >= 0 && idx < 13) {
                        if (sLevelNames[idx]) {
                            free(sLevelNames[idx]);
                        }
                        sLevelNames[idx] = static_cast<char*>(malloc(len + 1));
                        memcpy(sLevelNames[idx], data + pos, len);
                        sLevelNames[idx][len] = '\0';
                    }
                    pos += len;
                }
                break;

            case 10: // WARP_DESTINATIONS
                for (uint16_t e = 0; e < entryCount && pos + 4 <= size; e++) {
                    int warpIdx = readLE16(data + pos);
                    int dest = readLE16(data + pos + 2);
                    pos += 4;
                    sWarpDests[warpIdx] = dest;
                }
                break;

            case 11: // CUSTOM_CODE
                if (entryCount >= 1 && pos + 24 <= size) {
                    sCustomCodeRamBase = readLE32(data + pos);
                    char hex[41];
                    for (int i = 0; i < 20; i++) {
                        std::snprintf(hex + i * 2, 3, "%02x", data[pos + 4 + i]);
                    }
                    sCustomCodeHashHex.assign(hex, 40);
                    pos += 24;
                    // Skip any additional entries (forward-compat).
                    for (uint16_t e = 1; e < entryCount && pos + 24 <= size; e++) {
                        pos += 24;
                    }
                } else {
                    SPDLOG_WARN("[GameConfig] CUSTOM_CODE section malformed, skipping");
                    pos += entryCount * 24;
                }
                break;

            case 13: // CUSTOM_CODE_INFO
                if (entryCount >= 1 && pos + 4 <= size) {
                    sCustomCodeKind = readLE16(data + pos);
                }
                pos += entryCount * 4;
                break;

            case 12: // ROM_HASH — single entry: u8 hash[20]
                if (entryCount >= 1 && pos + 20 <= size) {
                    char hex[41];
                    for (int i = 0; i < 20; i++) {
                        std::snprintf(hex + i * 2, 3, "%02x", data[pos + i]);
                    }
                    sRomHashHex.assign(hex, 40);
                    pos += 20;
                    // Skip any additional entries (forward-compat).
                    for (uint16_t e = 1; e < entryCount && pos + 20 <= size; e++) {
                        pos += 20;
                    }
                } else {
                    SPDLOG_WARN("[GameConfig] ROM_HASH section malformed, skipping");
                    pos += entryCount * 20;
                }
                break;

            default:
                SPDLOG_WARN("[GameConfig] Unknown section type {}, aborting parse", secType);
                return;
        }
    }

    if (sCustomCodeHashHex.empty()) {
        // Data-only romhack — BB patches only, no injected MIPS to identify.
        // Fall back to the o2r-derived name for visibility.
        SPDLOG_INFO("[GameConfig] Loaded romhack: {}", sRomName.empty() ? "<unnamed>" : sRomName);
    } else if (const auto* entry = Lighthouse::LookupRomhackEntry(sCustomCodeHashHex.c_str())) {
        SPDLOG_INFO("[GameConfig] Loaded romhack: {}", entry->identifier);
        // No warning when the custom code is just the BB globalized-overlay
        // framework or is already ported. Only injected, not-yet-ported code merits one.
        // isPorted=false means analysis confirmed real un-ported custom code,
        // regardless of how the blob is shipped (Nostalgia 64's lives inside a
        // globalization-kind blob).
        if (!entry->isPorted) {
            SPDLOG_WARN("[GameConfig] {} ships injected custom code that is not ported yet; "
                        "scripted behavior driven by it will be absent.",
                        entry->identifier);
        }
    } else if (sCustomCodeKind == 1 /* BB_GLOBALIZATION */) {
        const char* romId = sRomHashHex.empty() ? nullptr : Lighthouse::LookupRomhackIdentifier(sRomHashHex.c_str());
        SPDLOG_INFO("[GameConfig] Loaded romhack: {}",
                    romId ? romId : (sRomName.empty() ? "<unnamed>" : sRomName.c_str()));
    } else {
        // Custom code present but the SHA1 is not in RomhackTable.h.
        SPDLOG_WARN("[GameConfig] Custom-code SHA1 {} is not identified! This romhack's scripted "
                    "behavior will be absent until a port is added (ramBase=0x{:08X}).",
                    sCustomCodeHashHex, sCustomCodeRamBase);
        SPDLOG_INFO("[GameConfig] Loaded romhack: unidentified custom-code");
    }
}

// All accessors callable from C. Fast path: after first load, vanilla ROMs hit one branch.

#define ROMHACK_GUARD_INT \
    if (!sIsRomhack) {    \
        LoadGameConfig(); \
        if (!sIsRomhack)  \
            return -1;    \
    }
#define ROMHACK_GUARD_ZERO \
    if (!sIsRomhack) {     \
        LoadGameConfig();  \
        if (!sIsRomhack)   \
            return 0;      \
    }
#define ROMHACK_GUARD_NULL \
    if (!sIsRomhack) {     \
        LoadGameConfig();  \
        if (!sIsRomhack)   \
            return NULL;   \
    }

extern "C" bool port_isRomhack(void) {
    LoadGameConfig();
    return sIsRomhack;
}

extern "C" const char* port_getRomhackName(void) {
    LoadGameConfig();
    return sRomName.empty() ? "" : sRomName.c_str();
}

extern "C" int port_getRomhackCustomCodeKind(void) {
    LoadGameConfig();
    if (!sIsRomhack || sCustomCodeHashHex.empty() || sCustomCodeKind < 0) {
        return 0; // none
    }
    return sCustomCodeKind;
}

extern "C" bool port_getRomhackCustomCodeHash(char out_hex[41]) {
    LoadGameConfig();
    if (sCustomCodeHashHex.empty()) {
        return false;
    }
    std::memcpy(out_hex, sCustomCodeHashHex.c_str(), 40);
    out_hex[40] = '\0';
    return true;
}

extern "C" bool port_getRomhackRomHash(char out_hex[41]) {
    LoadGameConfig();
    if (sRomHashHex.empty()) {
        return false;
    }
    std::memcpy(out_hex, sRomHashHex.c_str(), 40);
    out_hex[40] = '\0';
    return true;
}

extern "C" const char* port_getRomhackIdentifier(void) {
    LoadGameConfig();
    // Custom-code blob hash takes precedence — for hacks that ship MIPS code,
    // the blob hash is the most reliable signal of "what code is running" and
    // multiple ROM versions may share one blob.
    if (!sCustomCodeHashHex.empty()) {
        if (const char* id = Lighthouse::LookupRomhackIdentifier(sCustomCodeHashHex.c_str())) {
            return id;
        }
    }
    // Fall back to the ROM SHA1 for data-only hacks (e.g. Banjo-Dreamie) and
    // for custom-code hacks whose blob hash isn't yet in RomhackTable.h.
    if (!sRomHashHex.empty()) {
        return Lighthouse::LookupRomhackIdentifier(sRomHashHex.c_str());
    }
    return nullptr;
}

extern "C" int port_getRomhackSceneRemap(int map_id) {
    ROMHACK_GUARD_INT;
    auto it = sSceneRemaps.find(map_id);
    return (it != sSceneRemaps.end()) ? it->second : -1;
}

extern "C" int port_getRomhackReturnToLair(int level_index, int* out_map, int* out_exit) {
    ROMHACK_GUARD_ZERO;
    auto it = sReturnToLair.find(level_index);
    if (it != sReturnToLair.end()) {
        *out_map = it->second.first;
        *out_exit = it->second.second;
        return 1;
    }
    return 0;
}

extern "C" int port_getRomhackMusic(int map_id, int* out_track1, int* out_track2) {
    ROMHACK_GUARD_ZERO;
    auto it = sMusicAssign.find(map_id);
    if (it != sMusicAssign.end()) {
        *out_track1 = it->second.first;
        *out_track2 = it->second.second;
        return 1;
    }
    return 0;
}

extern "C" int port_getRomhackSkybox(int scene_id) {
    ROMHACK_GUARD_INT;
    auto it = sSkyboxAssign.find(scene_id);
    return (it != sSkyboxAssign.end()) ? it->second.models[0] : -1;
}

extern "C" int port_getRomhackSkyboxFull(int scene_id, int out_models[3], float out_scales[3], float out_rotations[3]) {
    ROMHACK_GUARD_ZERO;
    auto it = sSkyboxAssign.find(scene_id);
    if (it != sSkyboxAssign.end()) {
        for (int i = 0; i < 3; i++) {
            out_models[i] = it->second.models[i];
            out_scales[i] = it->second.scales[i];
            out_rotations[i] = it->second.rotations[i];
        }
        return 1;
    }
    return 0;
}

extern "C" int port_getRomhackSceneDef(int scene_id, int* out_opa, int* out_xlu) {
    ROMHACK_GUARD_ZERO;
    auto it = sSceneDefs.find(scene_id);
    if (it != sSceneDefs.end()) {
        *out_opa = it->second.opa;
        *out_xlu = it->second.xlu;
        return 1;
    }
    return 0;
}

extern "C" int port_getRomhackSceneDefFull(int scene_id, int* out_opa, int* out_xlu, short out_min[3], short out_max[3],
                                           float* out_scale) {
    ROMHACK_GUARD_ZERO;
    auto it = sSceneDefs.find(scene_id);
    if (it != sSceneDefs.end()) {
        *out_opa = it->second.opa;
        *out_xlu = it->second.xlu;
        for (int i = 0; i < 3; i++) {
            out_min[i] = it->second.min[i];
            out_max[i] = it->second.max[i];
        }
        *out_scale = it->second.scale;
        return 1;
    }
    return 0;
}

extern "C" int port_getRomhackNewGameMap(void) {
    ROMHACK_GUARD_INT;
    return sNewGameMap;
}
extern "C" int port_getRomhackStartLevel1(void) {
    ROMHACK_GUARD_INT;
    return sStartLevel1;
}
extern "C" int port_getRomhackStartLevel2(void) {
    ROMHACK_GUARD_INT;
    return sStartLevel2;
}
extern "C" int port_getRomhackMaxEggs(void) {
    ROMHACK_GUARD_INT;
    return sEggsNormalMax;
}
extern "C" int port_getRomhackMaxEggsCheato(void) {
    ROMHACK_GUARD_INT;
    return sEggsCheatomax;
}
extern "C" int port_getRomhackMaxRedFeathers(void) {
    ROMHACK_GUARD_INT;
    return sRedFeathersNormalMax;
}
extern "C" int port_getRomhackMaxRedFeathersCheato(void) {
    ROMHACK_GUARD_INT;
    return sRedFeathersCheatomax;
}
extern "C" int port_getRomhackMaxGoldFeathers(void) {
    ROMHACK_GUARD_INT;
    return sGoldFeathersNormalMax;
}
extern "C" int port_getRomhackMaxGoldFeathersCheato(void) {
    ROMHACK_GUARD_INT;
    return sGoldFeathersCheatomax;
}
extern "C" int port_getRomhackNotesMax(void) {
    ROMHACK_GUARD_INT;
    return sNotesMax;
}
extern "C" int port_getRomhackJiggiesPerWorld(void) {
    ROMHACK_GUARD_INT;
    return sJiggiesPerWorld;
}
extern "C" int port_getRomhackHoneycombsPerWorld(void) {
    ROMHACK_GUARD_INT;
    return sHoneycombsPerWorld;
}
extern "C" int port_getRomhackExtraHcStart(void) {
    ROMHACK_GUARD_INT;
    return sExtraHcStart;
}
extern "C" int port_getRomhackKnowAllMoves(void) {
    ROMHACK_GUARD_INT;
    return sKnowAllMoves;
}
extern "C" int port_getRomhackWarpExitBanjosHouse(void) {
    ROMHACK_GUARD_INT;
    return sWarpExitBanjosHouse;
}
extern "C" int port_getRomhackWarpEnterLair(void) {
    ROMHACK_GUARD_INT;
    return sWarpEnterLair;
}
extern "C" int port_getRomhackSpecialLevel(void) {
    ROMHACK_GUARD_INT;
    return sSpecialLevel;
}
extern "C" int port_getRomhackHideJiggiesLevel(void) {
    ROMHACK_GUARD_INT;
    return sHideJiggiesLevel;
}
extern "C" int port_getRomhackHideCollectiblesLevel(void) {
    ROMHACK_GUARD_INT;
    return sHideCollectiblesLevel;
}

extern "C" int port_getRomhackMumboCost(int transform_id) {
    ROMHACK_GUARD_INT;
    switch (transform_id) {
        case 2:
            return sMumboCostTermite;
        case 5:
            return sMumboCostCroc;
        case 4:
            return sMumboCostWalrus;
        case 3:
            return sMumboCostPumpkin;
        case 6:
            return sMumboCostBee;
        default:
            return -1;
    }
}

extern "C" int port_getRomhackNoteDoor(int door_index) {
    ROMHACK_GUARD_INT;
    if (door_index >= 0 && door_index < 12) {
        return sNoteDoors[door_index];
    }
    return -1;
}

extern "C" int port_getRomhackJiggyPuzzleCost(int puzzle_index) {
    ROMHACK_GUARD_INT;
    if (puzzle_index >= 0 && puzzle_index < 11) {
        return sJiggyCosts[puzzle_index];
    }
    return -1;
}

extern "C" int port_getRomhackJiggyPuzzleSize(int puzzle_index) {
    ROMHACK_GUARD_INT;
    if (puzzle_index >= 0 && puzzle_index < 11) {
        return sJiggySizes[puzzle_index];
    }
    return -1;
}

extern "C" int port_getRomhackJiggyPuzzleFlag(int puzzle_index) {
    ROMHACK_GUARD_INT;
    if (puzzle_index >= 0 && puzzle_index < 11) {
        return sJiggyFlags[puzzle_index];
    }
    return -1;
}

extern "C" const char* port_getRomhackLevelName(int level_index) {
    ROMHACK_GUARD_NULL;
    if (level_index >= 0 && level_index < 13) {
        return sLevelNames[level_index];
    }
    return NULL;
}

extern "C" int port_getRomhackWarpDest(int warp_index) {
    ROMHACK_GUARD_INT;
    auto it = sWarpDests.find(warp_index);
    return (it != sWarpDests.end()) ? it->second : -1;
}

extern "C" void port_clearRomhackWarpDest(int warp_index) {
    LoadGameConfig();
    sWarpDests.erase(warp_index);
}
