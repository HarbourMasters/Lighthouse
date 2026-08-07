#include "SaveManager.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"
#include "port/UI/cvar_prefixes.h"
#include "port/UI/LighthouseModMenuWindow.h"
#include "port/UI/Notification.h"
#include <fstream>
#include <filesystem>
#include <regex>

#include "save.h"
#include "Types.h"

#include "port/Rando/Logic/Logic.h"
#include "port/Romhack/RomhackConfig.h"

extern "C" {
#include "core1/sns.h"

extern SaveData gameFile_saveData[4];
extern s8 gameFile_GameIdToFileIdMap[4];
extern u8 gCompletedBottlesBonusGames[7];
}

using nlohmann::json;
using nlohmann::ordered_json;
namespace fs = std::filesystem;
static bool mLoaded = false;
const std::string savesFolderPathString(Ship::Context::GetPathRelativeToAppDirectory("saves"));
const std::filesystem::path savesFolderPath(savesFolderPathString);

#define CVAR_NAME_BOTTLES_BONUS CVAR_ENHANCEMENT("Saving.PersistBottlesBonus")

std::string SaveManager_GetSavePath(const std::string& filename) {
    std::string romName = GetActiveRomhackBasename();
    std::string dir = romName.empty() ? savesFolderPathString
                                      : Ship::Context::GetPathRelativeToAppDirectory("saves/~romhacks/" + romName);
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) {
        SPDLOG_ERROR("SaveManager: failed to create save directory \"{}\": {}", dir, ec.message());
    }
    return dir + "/" + filename;
}

static int BitfieldGetBit(const uint8_t* array, int index) {
    return (array[index / 8] & (1 << (index & 7))) ? 1 : 0;
}

static int BitfieldGetNBits(const uint8_t* array, int offset, int numBits) {
    int ret = 0;
    for (int i = 0; i < numBits; i++) {
        ret |= (BitfieldGetBit(array, offset + i) << i);
    }
    return ret;
}

static void BitfieldSetBit(uint8_t* array, int index, int set) {
    if (set) {
        array[index / 8] |= (1 << (index & 7));
    } else {
        array[index / 8] &= ~(1 << (index & 7));
    }
}

static void BitfieldSetNBits(uint8_t* array, int startIndex, int numBits, int value) {
    for (int i = 0; i < numBits; i++) {
        BitfieldSetBit(array, startIndex + i, (1 << i) & value);
    }
}

struct ResolvedFlag {
    int index;
    int width;
    int puzzle;
};

static int PuzzleIndexForFlag(const FlagDef& f) {
    static const char* const kPuzzleFlagNames[] = {
        "MM_PUZZLE_PIECES_PLACED",
        "TTC_PUZZLE_PIECES_PLACED",
        "CC_PUZZLE_PIECES_PLACED",
        "BGS_PUZZLE_PIECES_PLACED",
        "FP_PUZZLE_PIECES_PLACED",
        "GV_PUZZLE_PIECES_PLACED",
        "MMM_PUZZLE_PIECES_PLACED",
        "RBB_PUZZLE_PIECES_PLACED",
        "CCW_PUZZLE_PIECES_PLACED",
        "DOG_PUZZLE_PIECES_PLACED",
        "DOUBLE_HEALTH_PUZZLE_PIECES_PLACED",
    };
    if (f.name != nullptr) {
        for (int i = 0; i < 11; i++) {
            if (strcmp(f.name, kPuzzleFlagNames[i]) == 0) {
                return i;
            }
        }
    }
    return -1;
}

static ResolvedFlag ResolveFlag(const FlagDef& f) {
    const int puzzle = PuzzleIndexForFlag(f);
    if (puzzle < 0) {
        return { f.bitIndex, f.bitWidth, -1 };
    }

    const int index = port_getRomhackJiggyPuzzleFlag(puzzle);
    const int width = port_getRomhackJiggyPuzzleSize(puzzle);
    if (index < 0 || width < 0) {
        return { f.bitIndex, f.bitWidth, puzzle }; // no override, vanilla placement
    }
    // A malformed override would put the counter on top of unrelated progress flags.
    // The vanilla counters tile 0x5D-0x81 exactly, and 0x82 is the next flag along.
    if (width == 0 || index < 0x5D || index + width - 1 > 0x81) {
        SPDLOG_WARN("[SaveManager] {} declares an out-of-range puzzle layout (offset {:#x}, {} bits); "
                    "falling back to the vanilla placement",
                    f.name, index, width);
        return { f.bitIndex, f.bitWidth, puzzle };
    }
    return { index, width, puzzle };
}

// A picture's cost can shrink when a romhack ships a revised aGameConfig, which leaves an
// existing save reporting more pieces placed than the picture now has slots. The podium
// picks its occupied slots by rejection sampling, which never terminates once every slot
// is taken, so pin the counter to what the current table can represent on the way in.
static uint32_t ClampPuzzleCount(int puzzle, uint32_t value, const char* name) {
    const int cost = _puzzleCost(puzzle);
    if (cost >= 0 && value > static_cast<uint32_t>(cost)) {
        SPDLOG_WARN("[SaveManager] {} is {} but the picture only has {} slots; clamping", name, value, cost);
        return static_cast<uint32_t>(cost);
    }
    return value;
}

void RandoSaveCheck_to_json(nlohmann::json& j, const RandoSaveCheck& randoSaveCheck) {
    j = nlohmann::json::array({ randoSaveCheck.randoCheckId, randoSaveCheck.randoItemId,
                                randoSaveCheck.randoCollectionId, randoSaveCheck.isShuffled, randoSaveCheck.eligible,
                                randoSaveCheck.received, randoSaveCheck.skipped });
}

RandoSaveCheck RandoSaveCheck_from_json(const nlohmann::json& j, RandoSaveCheck& randoSaveCheck) {
    j.at(0).get_to(randoSaveCheck.randoCheckId);
    j.at(1).get_to(randoSaveCheck.randoItemId);
    j.at(2).get_to(randoSaveCheck.randoCollectionId);
    j.at(3).get_to(randoSaveCheck.isShuffled);
    j.at(4).get_to(randoSaveCheck.eligible);
    j.at(5).get_to(randoSaveCheck.received);
    j.at(6).get_to(randoSaveCheck.skipped);

    return randoSaveCheck;
}

std::string CollapsedJSONArray(const nlohmann::ordered_json& jsonFile) {
    std::string source = jsonFile.dump(4);
    std::string result;
    result.reserve(source.length());

    bool isCollapsed = false;

    for (size_t i = 0; i < source.length(); ++i) {
        char c = source[i];

        if (c == '[') {
            size_t next = source.find_first_not_of(" \n\r\t", i + 1);
            if (next != std::string::npos && (isdigit(source[next]) || source[next] == ']')) {
                isCollapsed = true;
                result += c;
                continue;
            }
        }

        if (isCollapsed) {
            if (isspace(c)) {
                continue;
            }

            if (c == ']') {
                isCollapsed = false;
                result += c;
            } else if (c == ',') {
                result += ", ";
            } else {
                result += c;
            }
        } else {
            result += c;
        }
    }

    return result;
}

ordered_json Convert_SaveDataToJSON(SaveData* saveData, int32_t fileNum) {
    ordered_json j;
    j = ordered_json::object();

    j["version"] = SAVE_VERSION;

    // Abilities
    const uint8_t* abilityData = &saveData->data[ABILITY_OFFSET];
    uint32_t learned, used;
    memcpy(&learned, abilityData, sizeof(uint32_t));
    memcpy(&used, abilityData + 4, sizeof(uint32_t));

    ordered_json learnedAbilities = ordered_json::object();
    ordered_json usedAbilities = ordered_json::object();
    for (int i = 0; i < kAbilityCount; i++) {
        learnedAbilities[kAbilityNames[i]] = (learned & (1u << i)) ? 1 : 0;
        usedAbilities[kAbilityNames[i]] = (used & (1u << i)) ? 1 : 0;
    }
    ordered_json abilities = ordered_json::object();
    abilities["learned"] = learnedAbilities;
    abilities["used"] = usedAbilities;
    j["abilities"] = abilities;

    // General Progress Flags
    const uint8_t* progressFlags = &saveData->data[PROGRESS_OFFSET];
    ordered_json general = ordered_json::object();
    for (int i = 0; i < kProgressFlagCount; i++) {
        const auto& f = kProgressFlags[i];
        if (f.world != nullptr) {
            continue;
        }
        const ResolvedFlag r = ResolveFlag(f);
        if (r.width == 1) {
            general[f.name] = BitfieldGetBit(progressFlags, r.index);
        } else {
            general[f.name] = BitfieldGetNBits(progressFlags, r.index, r.width);
        }
    }
    j["progress"] = general;

    // Sandcastle Cheat Flags
    ordered_json cheats = ordered_json::object();
    for (int i = 0; i < kProgressFlagCount; i++) {
        const auto& f = kProgressFlags[i];
        if (f.world == nullptr || strcmp(f.world, "CHEATS") != 0) {
            continue;
        }
        const ResolvedFlag r = ResolveFlag(f);
        if (r.width == 1) {
            cheats[f.name] = BitfieldGetBit(progressFlags, r.index);
        } else {
            cheats[f.name] = BitfieldGetNBits(progressFlags, r.index, r.width);
        }
    }
    j["cheats"] = cheats;

    // Saved Items
    const uint8_t* offsetData = &saveData->data[ITEMS_OFFSET];
    ordered_json savedItems = ordered_json::object();
    savedItems["mumboTokens"] = static_cast<int>(offsetData[0]);
    savedItems["eggs"] = static_cast<int>(offsetData[1]);
    savedItems["redFeathers"] = static_cast<int>(offsetData[2]);
    savedItems["goldFeathers"] = static_cast<int>(offsetData[3]);
    savedItems["jiggyTotal"] = static_cast<int>(offsetData[4]);

    j["savedItems"] = savedItems;

    // World Progress
    ordered_json worlds = ordered_json::object();
    for (int w = 0; w < kWorldCount; w++) {
        const auto& wd = kWorlds[w];
        ordered_json world = ordered_json::object();

        // Honeycombs (array of 0/1)
        if (wd.honeycombCount > 0) {
            json honeycombArray = json::array();
            const uint8_t* honeycombData = &saveData->data[HONEYCOMB_OFFSET];
            for (int i = 0; i < wd.honeycombCount; i++) {
                int id = wd.honeycombStart + i;
                honeycombArray.push_back((honeycombData[(id - 1) / 8] & (1 << (id & 7))) ? 1 : 0);
            }
            world["honeycombs"] = honeycombArray;
        }

        // Jiggies (array of 0/1)
        if (wd.jiggyCount > 0) {
            json jiggyArray = json::array();
            const uint8_t* jiggyData = &saveData->data[JIGGY_OFFSET];
            for (int i = 0; i < wd.jiggyCount; i++) {
                int id = wd.jiggyStart + i;
                jiggyArray.push_back((jiggyData[(id - 1) / 8] & (1 << (id & 7))) ? 1 : 0);
            }
            world["jiggies"] = jiggyArray;
        }

        // Mumbo tokens (array of 0/1)
        if (wd.mumboCount > 0) {
            json tokenArray = json::array();
            const uint8_t* tokenData = &saveData->data[MUMBO_OFFSET];
            for (int i = 0; i < wd.mumboCount; i++) {
                int id = wd.mumboStart + i;
                tokenArray.push_back((tokenData[(id - 1) / 8] & (1 << (id & 7))) ? 1 : 0);
            }
            world["mumboTokens"] = tokenArray;
        }

        // Note high score
        // Unpack note scores into temporary array
        int noteScores[9] = {};
        {
            uint64_t notesPacked = 0;
            memcpy(&notesPacked, &saveData->data[NOTE_OFFSET], sizeof(uint64_t));
            for (int i = 8; i >= 0; i--) {
                noteScores[i] = static_cast<int>(notesPacked & 0x7F);
                notesPacked >>= 7;
            }
        }

        if (wd.hasNoteScore) {
            int score = 0;
            for (int i = 0; i < 9; i++) {
                if (kNoteScoreWorlds[i] == wd.levelId) {
                    score = noteScores[i];
                    break;
                }
            }
            world["noteScore"] = score;
        }

        // Progress flags belonging to this world
        ordered_json worldProgress = ordered_json::object();
        for (int i = 0; i < kProgressFlagCount; i++) {
            const auto& f = kProgressFlags[i];
            if (f.world == nullptr || strcmp(f.world, wd.name) != 0) {
                continue;
            }
            const ResolvedFlag r = ResolveFlag(f);
            if (r.width == 1) {
                worldProgress[f.name] = BitfieldGetBit(progressFlags, r.index);
            } else {
                worldProgress[f.name] = BitfieldGetNBits(progressFlags, r.index, r.width);
            }
        }
        if (!worldProgress.empty()) {
            if (strcmp(wd.name, "BOSS") == 0) {
                for (auto& [key, val] : worldProgress.items()) {
                    world[key] = val;
                }
            } else {
                world["progress"] = worldProgress;
            }
        }

        // Time score
        if (wd.hasTimeScore) {
            const uint8_t* timeData = &saveData->data[TIME_OFFSET];
            int idx = wd.levelId - 1;
            uint16_t score = 0;
            memcpy(&score, timeData + idx * 2, sizeof(uint16_t));
            world["timeScore"] = static_cast<int>(score);
        }

        worlds[wd.name] = world;
    }
    j["worlds"] = worlds;

    // Enhancements
    int lives = item_getCount(ITEM_16_LIFE);
    j["enhancements"]["life"] = lives;

    // Ship Save Data
    ordered_json ship = ordered_json::object();
    ordered_json shipRando = ordered_json::object();

    ship["fileType"] = static_cast<int>(saveData->shipSaveData.fileType);
    ship["fileCreatedAt"] = static_cast<int>(saveData->shipSaveData.fileCreatedAt);

    // Note retention (all files): sparse per-map collected bitfields.
    ordered_json noteRetention = ordered_json::object();
    for (int m = 0; m < NOTE_RETENTION_MAP_SLOTS; m++) {
        bool any = false;
        for (int b = 0; b < NOTE_RETENTION_BYTES_PER_MAP; b++) {
            if (saveData->shipSaveData.noteRetention.collected[m][b]) {
                any = true;
                break;
            }
        }
        if (!any) {
            continue;
        }
        ordered_json bytes = ordered_json::array();
        for (int b = 0; b < NOTE_RETENTION_BYTES_PER_MAP; b++) {
            bytes.push_back(saveData->shipSaveData.noteRetention.collected[m][b]);
        }
        noteRetention[std::to_string(m)] = bytes;
    }
    ship["noteRetention"] = noteRetention;

    // Jinjo retention (all files): sparse per-level collected colour bitmasks.
    ordered_json jinjoRetention = ordered_json::object();
    for (int l = 0; l < JINJO_RETENTION_LEVEL_SLOTS; l++) {
        if (!saveData->shipSaveData.jinjoRetention.collected[l]) {
            continue;
        }
        jinjoRetention[std::to_string(l)] = saveData->shipSaveData.jinjoRetention.collected[l];
    }
    ship["jinjoRetention"] = jinjoRetention;

    if (saveData->shipSaveData.fileType == FILE_TYPE_SAVE_RANDO) {
        Rando::Logic::GenerateSaveData(saveData);
        shipRando["seedId"] = saveData->shipSaveData.randoSaveData.seedId;

        for (int i = RC_UNKNOWN; i < RC_MAX; i++) {
            json jsonSaveChecks = nlohmann::json::object();
            RandoSaveCheck randoSaveCheck = saveData->shipSaveData.randoSaveData.randoSaveCheck[i];
            RandoSaveCheck_to_json(jsonSaveChecks, randoSaveCheck);

            shipRando["randoSaveCheck"][Rando::StaticData::Checks[(RandoCheckId)i].name] = jsonSaveChecks;
        }

        for (int o = RO_LOGIC; o < RO_MAX; o++) {
            RandoSaveOption randoSaveOption = saveData->shipSaveData.randoSaveData.randoSaveOption[o];

            shipRando["randoSaveOption"][Rando::StaticData::Options[(RandoOptionId)o].name] =
                randoSaveOption.optionValue;
        }

        nlohmann::json randoInfArray = nlohmann::json::array();
        for (int f = RANDO_INF_UNKNOWN; f < RANDO_INF_MAX; f++) {
            randoInfArray.push_back(saveData->shipSaveData.randoSaveData.randoSaveFlag[f].flagState);
        }
        shipRando["randoSaveFlag"] = randoInfArray;

        ship["rando"] = shipRando;
    }

    j["ship"] = ship;

    return j;
}

SaveData* Convert_JSONToSaveData(int32_t fileNum) {
    json j = Ship_RetrieveSaveFile(fileNum);

    if (j.empty()) {
        SaveData* emptySave = new SaveData();
        memset(emptySave, 0, sizeof(SaveData));
        return emptySave;
    }

    SaveData* saveData = new SaveData();
    memset(saveData, 0, sizeof(SaveData));

    // fileNum is the decomp 0..2 file index; slotIndex is 1-based.
    saveData->slotIndex = fileNum + 1;

    // Abilities
    uint32_t learnedIndex = 0;
    uint32_t usedIndex = 0;

    auto& abilities = j["abilities"];
    auto& learnedJson = abilities["learned"];
    auto& usedJson = abilities["used"];

    for (int i = 0; i < kAbilityCount; i++) {
        const std::string& abilityName = kAbilityNames[i];
        if (learnedJson.contains(abilityName) && learnedJson[abilityName] == 1) {
            learnedIndex |= (1u << i);
        }
        if (usedJson.contains(abilityName) && usedJson[abilityName] == 1) {
            usedIndex |= (1u << i);
        }
    }

    uint8_t* abilityData = &saveData->data[ABILITY_OFFSET];
    memcpy(abilityData, &learnedIndex, sizeof(uint32_t));
    memcpy(abilityData + 4, &usedIndex, sizeof(uint32_t));

    // General Progress Flags
    uint8_t* progressFlags = &saveData->data[PROGRESS_OFFSET];
    auto& generalProgress = j["progress"];

    for (int i = 0; i < kProgressFlagCount; i++) {
        const auto& f = kProgressFlags[i];

        if (f.world != nullptr) {
            continue;
        }

        if (generalProgress.contains(f.name)) {
            uint32_t value = generalProgress[f.name].get<uint32_t>();
            const ResolvedFlag r = ResolveFlag(f);
            if (r.puzzle >= 0) {
                value = ClampPuzzleCount(r.puzzle, value, f.name);
            }

            if (r.width == 1) {
                BitfieldSetBit(progressFlags, r.index, value != 0);
            } else {
                BitfieldSetNBits(progressFlags, r.index, r.width, value);
            }
        }
    }

    // Sandcastle Cheat Flags
    auto& cheats = j["cheats"];

    for (int i = 0; i < kProgressFlagCount; i++) {
        const auto& f = kProgressFlags[i];

        if (f.world == nullptr || strcmp(f.world, "CHEATS") != 0) {
            continue;
        }

        if (cheats.contains(f.name)) {
            uint32_t value = cheats[f.name].get<uint32_t>();
            const ResolvedFlag r = ResolveFlag(f);

            if (r.width == 1) {
                // Set single bit (0 or 1)
                BitfieldSetBit(progressFlags, r.index, value != 0);
            } else {
                // Set multiple bits for specific cheat values
                BitfieldSetNBits(progressFlags, r.index, r.width, value);
            }
        }
    }

    // Saved Items
    uint8_t* savedItems = &saveData->data[ITEMS_OFFSET];

    savedItems[0] = j["savedItems"]["mumboTokens"];
    savedItems[1] = j["savedItems"]["eggs"];
    savedItems[2] = j["savedItems"]["redFeathers"];
    savedItems[3] = j["savedItems"]["goldFeathers"];
    savedItems[4] = j["savedItems"]["jiggyTotal"];

    // World Progress
    uint8_t* honeycombData = &saveData->data[HONEYCOMB_OFFSET];
    uint8_t* jiggyData = &saveData->data[JIGGY_OFFSET];
    uint8_t* tokenData = &saveData->data[MUMBO_OFFSET];
    uint8_t* timeData = &saveData->data[TIME_OFFSET];

    uint64_t notesPacked = 0;
    memcpy(&notesPacked, &saveData->data[NOTE_OFFSET], sizeof(uint64_t));
    int noteScores[9] = {};
    uint64_t tempPacked = notesPacked;
    for (int i = 8; i >= 0; i--) {
        noteScores[i] = static_cast<int>(tempPacked & 0x7F);
        tempPacked >>= 7;
    }

    auto& worldsProgress = j["worlds"];

    for (int w = 0; w < kWorldCount; w++) {
        const auto& wd = kWorlds[w];
        if (!worldsProgress.contains(wd.name))
            continue;
        auto& world = worldsProgress[wd.name];

        // Honeycombs
        if (wd.honeycombCount > 0 && world.contains("honeycombs")) {
            for (int i = 0; i < wd.honeycombCount; i++) {
                int id = wd.honeycombStart + i;
                if (world["honeycombs"][i] == 1)
                    honeycombData[(id - 1) / 8] |= (1 << (id & 7));
                else
                    honeycombData[(id - 1) / 8] &= ~(1 << (id & 7));
            }
        }

        // Jiggies
        if (wd.jiggyCount > 0 && world.contains("jiggies")) {
            for (int i = 0; i < wd.jiggyCount; i++) {
                int id = wd.jiggyStart + i;
                if (world["jiggies"][i] == 1)
                    jiggyData[(id - 1) / 8] |= (1 << (id & 7));
                else
                    jiggyData[(id - 1) / 8] &= ~(1 << (id & 7));
            }
        }

        // Mumbo Tokens
        if (wd.mumboCount > 0 && world.contains("mumboTokens")) {
            for (int i = 0; i < wd.mumboCount; i++) {
                int id = wd.mumboStart + i;
                if (world["mumboTokens"][i] == 1)
                    tokenData[(id - 1) / 8] |= (1 << (id & 7));
                else
                    tokenData[(id - 1) / 8] &= ~(1 << (id & 7));
            }
        }

        // Note Score
        if (wd.hasNoteScore && world.contains("noteScore")) {
            for (int i = 0; i < 9; i++) {
                if (kNoteScoreWorlds[i] == wd.levelId) {
                    noteScores[i] = world["noteScore"].get<int>();
                    break;
                }
            }
        }

        // World Progress Flags (BOSS stores flags directly, others use "progress")
        const auto& src =
            (strcmp(wd.name, "BOSS") == 0) ? world : (world.contains("progress") ? world["progress"] : world);
        for (int i = 0; i < kProgressFlagCount; i++) {
            const auto& f = kProgressFlags[i];
            if (f.world != nullptr && strcmp(f.world, wd.name) == 0) {
                if (src.contains(f.name)) {
                    uint32_t val = src[f.name].get<uint32_t>();
                    const ResolvedFlag r = ResolveFlag(f);
                    if (r.puzzle >= 0) {
                        val = ClampPuzzleCount(r.puzzle, val, f.name);
                    }
                    if (r.width == 1)
                        BitfieldSetBit(progressFlags, r.index, val != 0);
                    else
                        BitfieldSetNBits(progressFlags, r.index, r.width, val);
                }
            }
        }

        // Time Score
        if (wd.hasTimeScore && world.contains("timeScore")) {
            uint16_t score = static_cast<uint16_t>(world["timeScore"].get<int>());
            memcpy(timeData + (wd.levelId - 1) * 2, &score, sizeof(uint16_t));
        }
    }

    notesPacked = 0;
    for (int i = 0; i < 9; i++) {
        notesPacked = (notesPacked << 7) | (noteScores[i] & 0x7F);
    }
    memcpy(&saveData->data[NOTE_OFFSET], &notesPacked, sizeof(uint64_t));

    // Ship Save Data
    saveData->shipSaveData.fileType = j["ship"]["fileType"];
    // Saves from before the gameplay timer lack fileCreatedAt; treat them as created now.
    if (j["ship"].contains("fileCreatedAt") && j["ship"]["fileCreatedAt"].is_number()) {
        saveData->shipSaveData.fileCreatedAt = j["ship"]["fileCreatedAt"];
    } else {
        saveData->shipSaveData.fileCreatedAt = GetUnixTimestamp();
    }

    // Note retention (all files): clear then load sparse per-map bitfields.
    for (int m = 0; m < NOTE_RETENTION_MAP_SLOTS; m++) {
        for (int b = 0; b < NOTE_RETENTION_BYTES_PER_MAP; b++) {
            saveData->shipSaveData.noteRetention.collected[m][b] = 0;
        }
    }
    if (j.contains("ship") && j["ship"].contains("noteRetention")) {
        for (auto& [key, bytes] : j["ship"]["noteRetention"].items()) {
            int m = std::stoi(key);
            if (m < 0 || m >= NOTE_RETENTION_MAP_SLOTS) {
                continue;
            }
            for (int b = 0; b < NOTE_RETENTION_BYTES_PER_MAP && b < (int)bytes.size(); b++) {
                saveData->shipSaveData.noteRetention.collected[m][b] = bytes[b].get<uint8_t>();
            }
        }
    }

    // Jinjo retention (all files): clear then load sparse per-level bitmasks.
    for (int l = 0; l < JINJO_RETENTION_LEVEL_SLOTS; l++) {
        saveData->shipSaveData.jinjoRetention.collected[l] = 0;
    }
    if (j.contains("ship") && j["ship"].contains("jinjoRetention")) {
        for (auto& [key, bits] : j["ship"]["jinjoRetention"].items()) {
            int l = std::stoi(key);
            if (l < 0 || l >= JINJO_RETENTION_LEVEL_SLOTS) {
                continue;
            }
            saveData->shipSaveData.jinjoRetention.collected[l] = bits.get<uint8_t>();
        }
    }

    if (j["ship"]["fileType"].get<int>() == FILE_TYPE_SAVE_RANDO) {
        json rando = j["ship"]["rando"];
        saveData->shipSaveData.randoSaveData.seedId = rando["seedId"];

        for (int i = RC_UNKNOWN; i < RC_MAX; i++) {
            json jsonSaveChecks = rando["randoSaveCheck"][Rando::StaticData::Checks[(RandoCheckId)i].name];
            RandoSaveCheck randoSaveCheck = RandoSaveCheck_from_json(jsonSaveChecks, randoSaveCheck);

            saveData->shipSaveData.randoSaveData.randoSaveCheck[i] = randoSaveCheck;
        }

        for (int o = RO_LOGIC; o < RO_MAX; o++) {
            RandoSaveOption randoSaveOption = {
                .name = Rando::StaticData::Options[(RandoOptionId)o].name,
                .optionValue = rando["randoSaveOption"][Rando::StaticData::Options[(RandoOptionId)o].name],
            };

            saveData->shipSaveData.randoSaveData.randoSaveOption[o] = randoSaveOption;
        }

        for (int f = RANDO_INF_UNKNOWN; f < RANDO_INF_MAX; f++) {
            saveData->shipSaveData.randoSaveData.randoSaveFlag[f].flagState = rando["randoSaveFlag"][f];
        }
    }

    return saveData;
}

static void LoadGlobalData() {
    std::string globalPath = SaveManager_GetSavePath("global.json");
    if (!fs::exists(globalPath)) {
        return;
    }
    std::ifstream ifs(globalPath);
    json j = json::parse(ifs);

    gSaveData.snsw = 0;
    if (j.contains("snsItems")) {
        const auto& sns = j["snsItems"];
        if (sns.is_object() && sns.contains("unlocked")) {
            const auto& u = sns["unlocked"];
            for (int i = 0; i < kSnsItemCount; i++) {
                auto it = u.find(kSnsUnlocked[i].name);
                if (it != u.end() && it->get<int>()) {
                    gSaveData.snsw |= (1u << kSnsUnlocked[i].bit);
                }
            }
            if (sns.contains("collected")) {
                const auto& c = sns["collected"];
                for (int i = 0; i < kSnsItemCount; i++) {
                    auto it = c.find(kSnsCollected[i].name);
                    if (it != c.end() && it->get<int>()) {
                        gSaveData.snsw |= (1u << kSnsCollected[i].bit);
                    }
                }
            }
        } else if (sns.is_number()) {
            gSaveData.snsw = sns.get<uint32_t>();
        }
    }

    // Bottles Bonus: only restore from the global save when persistence is enabled.
    // With it off, completion stays session-only (vanilla), reset at file select.
    if (CVarGetInteger(CVAR_NAME_BOTTLES_BONUS, 0) && j.contains("bottlesBonusCompleted")) {
        const auto& bb = j["bottlesBonusCompleted"];
        for (int k = 0; k < 7 && k < (int)bb.size(); k++) {
            gCompletedBottlesBonusGames[k] = bb[k].get<int>() ? 1 : 0;
        }
    }
}

static bool WriteFileAtomically(const std::filesystem::path& path, const std::string& contents) {
    const std::filesystem::path tempPath = path.parent_path() / (path.filename().string() + ".tmp");
    std::error_code ec;
    {
        std::ofstream ofs(tempPath, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open()) {
            SPDLOG_ERROR("SaveManager: failed to open \"{}\" for writing", tempPath.string());
            return false;
        }
        ofs << contents;
        ofs.flush();
        if (!ofs.good()) {
            SPDLOG_ERROR("SaveManager: failed writing \"{}\"; leaving the existing file alone", tempPath.string());
            ofs.close();
            std::filesystem::remove(tempPath, ec);
            return false;
        }
    }
    std::filesystem::rename(tempPath, path, ec);
    if (ec) {
        SPDLOG_ERROR("SaveManager: failed to replace \"{}\": {}", path.string(), ec.message());
        std::error_code removeEc;
        std::filesystem::remove(tempPath, removeEc);
        return false;
    }
    return true;
}

static void SaveGlobalData() {
    ordered_json j = ordered_json::object();

    // SNS items
    ordered_json unlocked = ordered_json::object();
    ordered_json collected = ordered_json::object();
    for (int i = 0; i < kSnsItemCount; i++) {
        unlocked[kSnsUnlocked[i].name] = (gSaveData.snsw & (1u << kSnsUnlocked[i].bit)) ? 1 : 0;
        collected[kSnsCollected[i].name] = (gSaveData.snsw & (1u << kSnsCollected[i].bit)) ? 1 : 0;
    }
    j["snsItems"]["unlocked"] = unlocked;
    j["snsItems"]["collected"] = collected;

    // Bottles Bonus
    ordered_json bb = ordered_json::array();
    for (int i = 0; i < 7; i++) {
        bb.push_back(gCompletedBottlesBonusGames[i] ? 1 : 0);
    }
    j["bottlesBonusCompleted"] = bb;

    std::string globalPath = SaveManager_GetSavePath("global.json");
    WriteFileAtomically(globalPath, CollapsedJSONArray(j));
}

void SaveManager_MoveInvalidSaveFile(const std::filesystem::path& fileName, const std::string& message) {
    const std::filesystem::path filePath = savesFolderPath / fileName;
    const std::filesystem::path backupFilePath =
        savesFolderPath / (fileName.stem().string() + "_invalid_" + std::to_string(std::time(nullptr)) + ".json");

    try {
        if (std::filesystem::exists(filePath)) {
            std::filesystem::rename(filePath, backupFilePath);
        }

        SPDLOG_INFO("{}", message.c_str());
        Notification::Emit({ .message = message });
    } catch (...) { SPDLOG_ERROR("Failed to move invalid save file"); }
}

std::string createFileName(int fileNum) {
    return "file" + std::to_string(SlotToFileIndex(fileNum)) + ".json";
}

void SaveManager_Init() {
    LoadGlobalData();

    // Ensure global.json exists
    std::string globalPath = SaveManager_GetSavePath("global.json");
    if (!fs::exists(globalPath)) {
        SaveGlobalData();
    }

    REGISTER_LISTENER(OnSaveFileLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSaveFileLoad* ev = (OnSaveFileLoad*)event;
        SaveData* loaded = nullptr;
        std::string fileName = createFileName(ev->fileNum);
        try {
            loaded = Convert_JSONToSaveData(ev->fileNum);
            if (loaded && ev->saveBuffer && loaded->slotIndex != 0) {
                loaded->magic = SAVE_MAGIC;
                memcpy(ev->saveBuffer, loaded, sizeof(SaveData));
                ev->result = 0; // success
            } else {
                ev->result = 2; // empty/missing — let decomp treat as scratch slot
            }
        } catch (...) {
            SaveManager_MoveInvalidSaveFile(fileName, "Something went wrong trying to load " + fileName +
                                                          ". Original save file has been backed up.");
        }
        delete loaded;
        event->Cancelled = true;
    });

    REGISTER_LISTENER(OnSaveFileSave, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSaveFileSave* ev = (OnSaveFileSave*)event;

        std::string fileName = createFileName(ev->fileNum);
        std::string filePath = SaveManager_GetSavePath(fileName);

        if (((SaveData*)ev->saveBuffer)->magic == 0) {
            std::error_code ec;
            fs::remove(filePath, ec);
            if (ec) {
                SPDLOG_ERROR("SaveManager: failed to remove erased save file \"{}\": {}", filePath, ec.message());
            }
            SaveGlobalData();
            event->Cancelled = true;
            return;
        }

        ordered_json saveFile = Convert_SaveDataToJSON((SaveData*)ev->saveBuffer, ev->fileNum);
        if (!saveFile.empty()) {
            WriteFileAtomically(filePath, CollapsedJSONArray(saveFile));
        }

        SaveGlobalData();
        event->Cancelled = true;
    });

    REGISTER_LISTENER(OnGameErase, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnGameErase* ev = (OnGameErase*)event;
        std::string fileName = createFileName(ev->gameNum);
        std::error_code ec;
        if (fs::remove(SaveManager_GetSavePath(fileName), ec)) {
            SPDLOG_INFO("SaveManager: deleted erased save file \"{}\"", fileName);
        } else if (ec) {
            SPDLOG_ERROR("SaveManager: failed to delete erased save file \"{}\": {}", fileName, ec.message());
        }
        // Drop retention data too.
        if (ev->gameNum >= 0 && ev->gameNum < 4) {
            gameFile_saveData[gameFile_GameIdToFileIdMap[ev->gameNum]].shipSaveData = {};
        }
    });

    REGISTER_LISTENER(OnSaveClear, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSaveClear* ev = (OnSaveClear*)event;
        SaveData* saveData = (SaveData*)ev->result;

        ShipSaveData ship = saveData->shipSaveData; // Retain ShipSaveData during Save Process

        u8* savedata = (u8*)saveData;
        int i;
        for (i = 0; i < sizeof(SaveData); i++) {
            savedata[i] = 0;
        }

        saveData = (SaveData*)savedata;
        saveData->shipSaveData = ship;

        event->Cancelled = true;
    });

    REGISTER_LISTENER(OnGameFileErase, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnGameFileErase* ev = (OnGameFileErase*)event;
        gameFile_8033CFD4(ev->gamenum);
    });

    REGISTER_LISTENER(OnGameLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnGameLoad* ev = (OnGameLoad*)event;
        gSelectedFileNum = ev->fileNum;
    });

    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSaveLoad* ev = (OnSaveLoad*)event;
        SaveData* saveData = (SaveData*)ev->saveData;

        if (saveData->magic == 0) {
            saveData->shipSaveData.fileCreatedAt = GetUnixTimestamp();
        }
    });

    // Decomp clears global arrays (e.g. gCompletedBottlesBonusGames) just before
    // gameFile_load fires OnGameLoad. Restore them from global.json here before
    // other OnGameLoad listeners read them.
    REGISTER_LISTENER(OnGameLoad, EVENT_PRIORITY_HIGH, [](IEvent* event) { LoadGlobalData(); });
}

static void RegisterPersistBottlesBonus_Init() {
    COND_HOOK(OnBottlesBonusComplete, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NAME_BOTTLES_BONUS, 0),
              [](IEvent* event) { SaveGlobalData(); });
}

static RegisterShipInitFunc initPersistBottlesBonus(RegisterPersistBottlesBonus_Init, { CVAR_NAME_BOTTLES_BONUS });
