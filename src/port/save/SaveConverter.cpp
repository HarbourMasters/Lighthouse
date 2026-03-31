#include <nlohmann/json.hpp>

#include <libultraship/libultra/gbi.h>
#include "save.h"
#include "Types.h"

extern "C" {
extern SaveData gameFile_saveData[4];
}

#define SAVE_VERSION 1

using nlohmann::json;

/*
typedef struct {
    bool isRando;
} RandoSaveData;

typedef struct {
    RandoSaveData randoSaveData;
} ShipSaveData;

typedef struct{
    u8 magic;
    u8 slotIndex; [X]
    u8 data[0x70];
    u8 padding[0x2];
    u32 checksum;
    ShipSaveData shipSaveData;
}SaveData;
*/

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

json Convert_SaveDataToJSON(SaveData* saveData) {
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
    const uint8_t* data = &saveData->data[ITEMS_OFFSET];
    json savedItems = json::object();
    savedItems["mumboTokens"] = static_cast<int>(data[0]);
    savedItems["eggs"] = static_cast<int>(data[1]);
    savedItems["redFeathers"] = static_cast<int>(data[2]);
    savedItems["goldFeathers"] = static_cast<int>(data[3]);
    savedItems["jiggyTotal"] = static_cast<int>(data[4]);

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

    // Ship Save Data
    json ship = json::object();
    json shipRando = json::object();

    ship["saveType"] = saveData->shipSaveData.saveType;

    shipRando["isRando"] = static_cast<int>(saveData->shipSaveData.randoSaveData.isRando);
    ship["randoSaveData"] = shipRando;

    j["ship"] = ship;

    return j;
}
