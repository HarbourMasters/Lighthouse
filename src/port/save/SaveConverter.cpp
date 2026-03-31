#include <nlohmann/json.hpp>

#include <libultraship/libultra/gbi.h>
#include "save.h"

extern "C" {
extern SaveData gameFile_saveData[4];
}

#define SAVE_VERSION 1

// ??? Binary Layout Constants ????????????????????????????????????????????????
// These match the offsets computed by savedata_init() in savedata.c.
// SaveData is 120 bytes: magic(1) + slotIndex(1) + data(112) + padding(2) + crc(4)

static constexpr int JIGGY_OFFSET = 2;
static constexpr int JIGGY_SIZE = 13; // bit array for 100 jiggies
static constexpr int HONEYCOMB_OFFSET = 15;
static constexpr int HONEYCOMB_SIZE = 3; // bit array for 24 honeycombs
static constexpr int MUMBO_OFFSET = 18;
static constexpr int MUMBO_SIZE = 16; // bit array for 125 mumbo tokens
static constexpr int NOTE_OFFSET = 34;
static constexpr int NOTE_SIZE = 8; // packed u64: 9 worlds × 7 bits
static constexpr int TIME_OFFSET = 42;
static constexpr int TIME_SIZE = 22; // 11 × u16
static constexpr int PROGRESS_OFFSET = 64;
static constexpr int PROGRESS_SIZE = 37; // 296 bits for file_progress_e
static constexpr int ITEMS_OFFSET = 101;
static constexpr int ITEMS_SIZE = 5; // mumboTokens, eggs, redFeathers, goldFeathers, jiggyTotal
static constexpr int ABILITY_OFFSET = 106;
static constexpr int ABILITY_SIZE = 8; // learnedAbilities(4) + usedAbilities(4)

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

void Convert_SaveDataToJSON(SaveData* saveData) {
    json j;
    j = json::object();

    j["slotIndex"] = saveData->slotIndex;
    j["version"] = SAVE_VERSION;

    // ?? Abilities ??
    const uint8_t* abilityData = &saveData->data[ABILITY_OFFSET];
    uint32_t learned, used;
    memcpy(&learned, abilityData, sizeof(uint32_t));
    memcpy(&used, abilityData + 4, sizeof(uint32_t));

    json learnedObj = json::object();
    json usedObj = json::object();
    for (int i = 0; i < kAbilityCount; i++) {
        learnedObj[kAbilityNames[i]] = (learned & (1u << i)) ? 1 : 0;
        usedObj[kAbilityNames[i]] = (used & (1u << i)) ? 1 : 0;
    }
    json abilities = json::object();
    abilities["learned"] = learnedObj;
    abilities["used"] = usedObj;
    file["abilities"] = abilities;

    //  ?? Ship Save Data ??
    json ship = json::object();
    json shipRando = json::object();

    ship["saveType"] = saveData->shipSaveData.saveType;

    shipRando["isRando"] = static_cast<int>(saveData->shipSaveData.randoSaveData.isRando);
    ship["randoSaveData"] = shipRando;

    j["ship"] = ship;
}
