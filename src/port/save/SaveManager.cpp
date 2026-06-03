#include "SaveManager.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipUtils.h"
#include "port/GameConfig.h"
#include <fstream>
#include <filesystem>
#include <regex>

#include "save.h"
#include "Types.h"

extern "C" {
#include "core1/sns.h"

extern SaveData gameFile_saveData[4];
void savedata_update_crc(void* buffer, s32 size);
s32 item_getCount(enum item_e item);
extern u8 gCompletedBottlesBonusGames[7];
}

using nlohmann::json;
using nlohmann::ordered_json;
namespace fs = std::filesystem;
static bool mLoaded = false;

std::string SaveManager_GetSavePath(const std::string& filename) {
    const char* romName = port_getRomhackName();
    std::string dir = Ship_IsCStringEmpty(romName)
                          ? Ship::Context::GetPathRelativeToAppDirectory("saves")
                          : Ship::Context::GetPathRelativeToAppDirectory("saves/" + std::string(romName));
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

std::string CollapsedJSONArray(ordered_json jsonFile) {
    std::string jsonString = jsonFile.dump(4);
    jsonString = std::regex_replace(jsonString, std::regex(R"(\[\s+([01,\s]+?)\s+\])"), "[$1]");
    jsonString = std::regex_replace(jsonString, std::regex(R"(\s+([01]))"), " $1");
    jsonString = std::regex_replace(jsonString, std::regex(R"(\s+\])"), "]");

    return jsonString;
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
        if (f.bitWidth == 1) {
            general[f.name] = BitfieldGetBit(progressFlags, f.bitIndex);
        } else {
            general[f.name] = BitfieldGetNBits(progressFlags, f.bitIndex, f.bitWidth);
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
        if (f.bitWidth == 1) {
            cheats[f.name] = BitfieldGetBit(progressFlags, f.bitIndex);
        } else {
            cheats[f.name] = BitfieldGetNBits(progressFlags, f.bitIndex, f.bitWidth);
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
            if (f.bitWidth == 1) {
                worldProgress[f.name] = BitfieldGetBit(progressFlags, f.bitIndex);
            } else {
                worldProgress[f.name] = BitfieldGetNBits(progressFlags, f.bitIndex, f.bitWidth);
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

    ship["fileType"] = FILE_TYPE_SAVE_VANILLA;
    ship["randoSaveData"] = shipRando;

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

            if (f.bitWidth == 1) {
                BitfieldSetBit(progressFlags, f.bitIndex, value != 0);
            } else {
                BitfieldSetNBits(progressFlags, f.bitIndex, f.bitWidth, value);
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

            if (f.bitWidth == 1) {
                // Set single bit (0 or 1)
                BitfieldSetBit(progressFlags, f.bitIndex, value != 0);
            } else {
                // Set multiple bits for specific cheat values
                BitfieldSetNBits(progressFlags, f.bitIndex, f.bitWidth, value);
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
                    if (f.bitWidth == 1)
                        BitfieldSetBit(progressFlags, f.bitIndex, val != 0);
                    else
                        BitfieldSetNBits(progressFlags, f.bitIndex, f.bitWidth, val);
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

    // Bottles Bonus
    if (j.contains("bottlesBonusCompleted")) {
        const auto& bb = j["bottlesBonusCompleted"];
        for (int k = 0; k < 7 && k < (int)bb.size(); k++) {
            gCompletedBottlesBonusGames[k] = bb[k].get<int>() ? 1 : 0;
        }
    }
}

void SaveManager_LoadAll() {
    for (int i = 0; i < 3; i++) {
        SaveData* loadSave = Convert_JSONToSaveData(i);
        if (loadSave->slotIndex != 0) {
            loadSave->magic = SAVE_MAGIC;
        }
        gameFile_saveData[i] = *(loadSave);
        delete loadSave;
    }

    LoadGlobalData();
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
    std::ofstream ofs(globalPath);
    if (ofs.is_open()) {
        ofs << CollapsedJSONArray(j);
        ofs.close();
    } else {
        SPDLOG_ERROR("SaveManager: failed to open global save file \"{}\" for writing", globalPath);
    }
}

void SaveManager_Init() {
    SaveManager_LoadAll();

    // Ensure global.json exists
    std::string globalPath = SaveManager_GetSavePath("global.json");
    if (!fs::exists(globalPath)) {
        SaveGlobalData();
    }

    REGISTER_LISTENER(OnSaveFileLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSaveFileLoad* ev = (OnSaveFileLoad*)event;
        SaveData* loaded = Convert_JSONToSaveData(ev->fileNum);
        if (loaded && ev->saveBuffer && loaded->slotIndex != 0) {
            loaded->magic = SAVE_MAGIC;
            memcpy(ev->saveBuffer, loaded, sizeof(SaveData));
            ev->result = 0; // success
        } else {
            ev->result = 2; // empty/missing — let decomp treat as scratch slot
        }
        delete loaded;
        event->Cancelled = true;
    });

    REGISTER_LISTENER(OnSaveFileSave, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSaveFileSave* ev = (OnSaveFileSave*)event;

        ordered_json saveFile = Convert_SaveDataToJSON((SaveData*)ev->saveBuffer, ev->fileNum);
        if (!saveFile.empty()) {
            std::string collapsedString = CollapsedJSONArray(saveFile);

            std::string fileName = "file" + std::to_string(SlotToFileIndex(ev->fileNum)) + ".json";
            std::string filePath = SaveManager_GetSavePath(fileName);

            std::ofstream outputFile(filePath);
            if (outputFile.is_open()) {
                outputFile << collapsedString;
                outputFile.close();
            } else {
                SPDLOG_ERROR("SaveManager: failed to open save file \"{}\" for writing", filePath);
            }
        }

        SaveGlobalData();
        event->Cancelled = true;
    });

    // Decomp clears global arrays (e.g. gCompletedBottlesBonusGames) just before
    // gameFile_load fires OnGameLoad. Restore them from global.json here before
    // other OnGameLoad listeners read them.
    REGISTER_LISTENER(OnGameLoad, EVENT_PRIORITY_HIGH, [](IEvent* event) { LoadGlobalData(); });
}
