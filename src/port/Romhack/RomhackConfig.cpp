#include "port/Romhack/RomhackConfig.h"
#include "RomhackTable.h"
#include "factories/bk64/ConfigFactory.h"
#include "port/Localization/Language.h"

#include <unordered_map>
#include <unordered_set>
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

// Indexed by BK64::CodeConstantKey.
static int* const sConstants[] = {
    &sNewGameMap,       &sStartLevel1,          &sStartLevel2,           &sKnowAllMoves,
    &sMumboCostTermite, &sMumboCostCroc,        &sMumboCostWalrus,       &sMumboCostPumpkin,
    &sMumboCostBee,     &sEggsNormalMax,        &sRedFeathersNormalMax,  &sGoldFeathersNormalMax,
    &sEggsCheatomax,    &sRedFeathersCheatomax, &sGoldFeathersCheatomax, &sNotesMax,
    &sJiggiesPerWorld,  &sHoneycombsPerWorld,   &sExtraHcStart,          &sWarpExitBanjosHouse,
    &sWarpEnterLair,    &sSpecialLevel,         &sHideJiggiesLevel,      &sHideCollectiblesLevel,
};
static_assert(std::size(sConstants) == static_cast<size_t>(BK64::CodeConstantKey::COUNT));

static std::string HexString(const uint8_t* bytes, size_t count) {
    std::string out;
    for (size_t i = 0; i < count; i++) {
        char hex[3];
        std::snprintf(hex, sizeof(hex), "%02x", bytes[i]);
        out += hex;
    }
    return out;
}

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

    BK64::GameConfigData config;
    std::string error;
    if (!BK64::ReadGameConfig(*res->Buffer, config, error)) {
        SPDLOG_WARN("[GameConfig] {}", error);
        return;
    }
    sRomName = config.romName;
    sIsRomhack = true;

    for (const auto& constant : config.constants) {
        if (constant.key < std::size(sConstants)) {
            *sConstants[constant.key] = constant.value;
        }
    }
    for (const auto& entry : config.worldByMap) {
        sSceneRemaps[entry.map] = entry.level;
    }
    for (const auto& entry : config.returnToLair) {
        sReturnToLair[entry.world] = { entry.map, entry.exit };
    }
    for (const auto& entry : config.music) {
        sMusicAssign[entry.map] = { entry.track, entry.alternate };
    }
    for (const auto& entry : config.skybox) {
        SkyboxLayerData& sky = sSkyboxAssign[entry.map];
        for (int layer = 0; layer < 3; layer++) {
            sky.models[layer] = entry.layers[layer].model;
            sky.scales[layer] = entry.layers[layer].scale;
            sky.rotations[layer] = entry.layers[layer].spin;
        }
    }
    for (const auto& entry : config.levelModels) {
        SceneDefData& sd = sSceneDefs[entry.map];
        sd.opa = entry.opaque;
        sd.xlu = entry.translucent;
        for (int b = 0; b < 3; b++) {
            sd.min[b] = entry.cubeMin[b];
            sd.max[b] = entry.cubeMax[b];
        }
        sd.scale = entry.scale;
    }
    for (const auto& entry : config.noteDoors) {
        if (entry.door < 12) {
            sNoteDoors[entry.door] = entry.notes;
        }
    }
    for (const auto& entry : config.puzzles) {
        if (entry.picture < 11) {
            sJiggyCosts[entry.picture] = entry.cost;
            sJiggySizes[entry.picture] = entry.size;
            sJiggyFlags[entry.picture] = entry.flag;
        }
    }
    for (const auto& entry : config.titles) {
        if (entry.index < 13) {
            free(sLevelNames[entry.index]);
            sLevelNames[entry.index] = static_cast<char*>(malloc(entry.text.size() + 1));
            memcpy(sLevelNames[entry.index], entry.text.c_str(), entry.text.size() + 1);
        }
    }

    // Validate warp destinations
    std::unordered_set<uint32_t> setupIds;
    if (auto files = archive->ListFiles("assets/setup/*")) {
        for (const auto& path : *files) {
            uint32_t id = 0;
            if (Lighthouse::AssetHexFromPath(path, id)) {
                setupIds.insert(id);
            }
        }
    }
    for (const auto& entry : config.warps) {
        if (!setupIds.contains((entry.dest >> 8) + 0x71C)) { // file_openMap's setup id
            SPDLOG_DEBUG("[GameConfig] WARP_DESTINATIONS entry {} targets map 0x{:02X}, "
                         "which has no setup — dropping override",
                         entry.warp, entry.dest >> 8);
            continue;
        }
        sWarpDests[entry.warp] = entry.dest;
    }

    if (config.hasCustomCode) {
        sCustomCodeRamBase = config.customCodeRamBase;
        sCustomCodeHashHex = HexString(config.customCodeSha1, sizeof(config.customCodeSha1));
    }
    sCustomCodeKind = static_cast<int>(config.customCodeKind);
    if (config.hasRomHash) {
        sRomHashHex = HexString(config.romHash, sizeof(config.romHash));
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
extern "C" int port_getNotesPerLevel(void) {
    const int notesMax = port_getRomhackNotesMax();
    return notesMax >= 0 ? notesMax : 100;
}
extern "C" int port_getRomhackJiggiesPerWorld(void) {
    ROMHACK_GUARD_INT;
    return sJiggiesPerWorld;
}
extern "C" void port_overrideRomhackJiggiesPerWorld(int value) {
    sJiggiesPerWorld = value;
}
extern "C" int port_getRomhackHoneycombsPerWorld(void) {
    ROMHACK_GUARD_INT;
    return sHoneycombsPerWorld;
}
extern "C" void port_overrideRomhackHoneycombsPerWorld(int value) {
    sHoneycombsPerWorld = value;
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
