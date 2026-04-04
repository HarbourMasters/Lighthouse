#include "SaveManager.h"
#include <nlohmann/json.hpp>
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipUtils.h"
#include <fstream>
#include <filesystem>

#include "save.h"
#include "Types.h"

extern "C" {
extern SaveData gameFile_saveData[4];
void savedata_update_crc(void* buffer, s32 size);
s32 item_getCount(enum item_e item);
u8 gCompletedBottlesBonusGames[7];
}

using nlohmann::json;
namespace fs = std::filesystem;
static bool mLoaded = false;

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

static void BitfieldSetNBits(uint8_t* array, int startIndex, int set, int length) {
    for (int i = 0; i < length; i++) {
        BitfieldSetBit(array, startIndex + i, (1 << i) & set);
    }
}

json Convert_SaveDataToJSON(SaveData* saveData, int32_t fileNum) {
    json j;
    j = json::object();

    j["slotIndex"] = saveData->slotIndex;
    j["version"] = SAVE_VERSION;

    // Abilities
    const uint8_t* abilityData = &saveData->data[ABILITY_OFFSET];
    uint32_t learned, used;
    memcpy(&learned, abilityData, sizeof(uint32_t));
    memcpy(&used, abilityData + 4, sizeof(uint32_t));

    json learnedAbilities = json::object();
    json usedAbilities = json::object();
    for (int i = 0; i < kAbilityCount; i++) {
        learnedAbilities[kAbilityNames[i]] = (learned & (1u << i)) ? 1 : 0;
        usedAbilities[kAbilityNames[i]] = (used & (1u << i)) ? 1 : 0;
    }
    json abilities = json::object();
    abilities["learned"] = learnedAbilities;
    abilities["used"] = usedAbilities;
    j["abilities"] = abilities;

    // General Progress Flags
    const uint8_t* progressFlags = &saveData->data[PROGRESS_OFFSET];
    json general = json::object();
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
    json cheats = json::object();
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
    json savedItems = json::object();
    savedItems["mumboTokens"] = static_cast<int>(offsetData[0]);
    savedItems["eggs"] = static_cast<int>(offsetData[1]);
    savedItems["redFeathers"] = static_cast<int>(offsetData[2]);
    savedItems["goldFeathers"] = static_cast<int>(offsetData[3]);
    savedItems["jiggyTotal"] = static_cast<int>(offsetData[4]);

    j["savedItems"] = savedItems;

    // World Progress
    json worlds = json::object();
    for (int w = 0; w < kWorldCount; w++) {
        const auto& wd = kWorlds[w];
        json world = json::object();

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
        json worldProgress = json::object();
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
            world["progress"] = worldProgress;
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

    json bonusArray = json::array();
    for (auto& state : gCompletedBottlesBonusGames) {
        bonusArray.push_back(state);
    }
    j["enhancements"]["bottlesBonusCompleted"] = bonusArray;

    // Ship Save Data
    json ship = json::object();
    json shipRando = json::object();

    ship["fileType"] = FILE_TYPE_SAVE_VANILLA;
    ship["randoSaveData"] = shipRando;

    j["ship"] = ship;

    return j;
}

SaveData* Convert_JSONToSaveData(int32_t fileNum) {
    json j = Ship_RetrieveSaveFile(fileNum);

    if (j.empty() || !j.contains("slotIndex")) {
        SaveData* emptySave = new SaveData();
        emptySave->slotIndex = fileNum;
        memset(emptySave, 0, sizeof(SaveData));
        return emptySave;
    }

    SaveData* saveData = new SaveData();
    memset(saveData, 0, sizeof(SaveData));

    saveData->slotIndex = j["slotIndex"];

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

        // World Progress Flags
        if (world.contains("progress")) {
            auto& worldProgress = world["progress"];
            for (int i = 0; i < kProgressFlagCount; i++) {
                const auto& f = kProgressFlags[i];
                if (f.world != nullptr && strcmp(f.world, wd.name) == 0) {
                    if (worldProgress.contains(f.name)) {
                        uint32_t val = worldProgress[f.name].get<uint32_t>();
                        if (f.bitWidth == 1)
                            BitfieldSetBit(progressFlags, f.bitIndex, val != 0);
                        else
                            BitfieldSetNBits(progressFlags, f.bitIndex, f.bitWidth, val);
                    }
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

void SaveManager_LoadAll() {
    uint8_t mEeprom[EEPROM_TOTAL_SIZE];
    for (int i = 1; i <= 3; i++) {
        SaveData* loadSave = Convert_JSONToSaveData(i);
        if (loadSave->slotIndex != 0) {
            loadSave->magic = SAVE_MAGIC;
        }
        gameFile_saveData[i - 1] = *(loadSave);
    }

    // Load global data
    std::string globalName = "global.json";
    std::string globalPath = Ship::Context::GetPathRelativeToAppDirectory("saves/" + globalName);
    if (fs::exists(globalPath)) {
        std::ifstream ifs(globalPath);
        json j = json::parse(ifs);

        uint32_t snsRaw = 0;
        if (j.contains("snsItems")) {
            const auto& sns = j["snsItems"];
            if (sns.is_object() && sns.contains("unlocked")) {
                // Subtree format (v3+): unlocked/collected
                if (sns.contains("unlocked")) {
                    const auto& u = sns["unlocked"];
                    for (int i = 0; i < kSnsItemCount; i++) {
                        auto it = u.find(kSnsUnlocked[i].name);
                        if (it != u.end() && it->get<int>()) {
                            snsRaw |= (1u << kSnsUnlocked[i].bit);
                        }
                    }
                }
                if (sns.contains("collected")) {
                    const auto& c = sns["collected"];
                    for (int i = 0; i < kSnsItemCount; i++) {
                        auto it = c.find(kSnsCollected[i].name);
                        if (it != c.end() && it->get<int>()) {
                            snsRaw |= (1u << kSnsCollected[i].bit);
                        }
                    }
                }
            } else if (sns.is_number()) {
                // Legacy raw u32 format (v2)
                snsRaw = sns.get<uint32_t>();
            }
        }

        int globalBase = GLOBAL_OFFSET_BLOCK * EEPROM_BLOCK_SIZE;
        memset(mEeprom + globalBase, 0, GLOBAL_SIZE);
        memcpy(mEeprom + globalBase, &snsRaw, sizeof(uint32_t));

        savedata_update_crc(mEeprom + globalBase, GLOBAL_SIZE);
    }
}

void SaveManager_Init() {
    SaveManager_LoadAll();
    REGISTER_LISTENER(OnSaveFileLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSaveFileLoad* ev = (OnSaveFileLoad*)event;
        SaveData* loaded = Convert_JSONToSaveData(ev->fileNum);
        if (loaded && ev->saveBuffer) {
            if (loaded->slotIndex != 0) {
                loaded->magic = SAVE_MAGIC;
            }
            memcpy(ev->saveBuffer, loaded, sizeof(SaveData));
            ev->result = 0; // success
        } else {
            ev->result = 2; // error
        }
        delete loaded;
        event->cancelled = true;
    });

    REGISTER_LISTENER(OnSaveFileSave, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSaveFileSave* ev = (OnSaveFileSave*)event;

        json saveFile = Convert_SaveDataToJSON((SaveData*)ev->saveBuffer, ev->fileNum);
        if (!saveFile.empty()) {
            std::string fileName = "file" + std::to_string(SlotToVisualGame(ev->fileNum) + 1) + ".json";
            std::string filePath = Ship::Context::GetPathRelativeToAppDirectory("saves/" + fileName);

            std::ofstream outputFile(filePath);
            if (outputFile.is_open()) {
                outputFile << saveFile.dump(4);
                outputFile.close();
            }
        }

        event->cancelled = true;
    });
}
