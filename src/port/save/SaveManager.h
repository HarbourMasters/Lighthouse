#pragma once

#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>

// BK EEPROM layout:
//   4 save slots × 15 blocks (120 bytes each) = blocks 0-59
//   GlobalData = 4 blocks (32 bytes) at block offset 0x3C (60)
// eeprom_readBlocks(file, offset, buffer, count)
//   absoluteBlock = file * 15 + offset
//   reads count * 8 bytes

#define EEPROM_TOTAL_SIZE 512
#define EEPROM_BLOCK_SIZE 8
#define SAVE_SLOT_BLOCKS 15
#define SAVE_SLOT_SIZE (SAVE_SLOT_BLOCKS * EEPROM_BLOCK_SIZE) // 120
#define SAVE_SLOT_COUNT 4
#define GLOBAL_OFFSET_BLOCK 0x3C // block 60
#define GLOBAL_BLOCK_COUNT 4
#define GLOBAL_SIZE (GLOBAL_BLOCK_COUNT * EEPROM_BLOCK_SIZE) // 32

#define SAVE_MAGIC 0x11
#define SAVE_VERSION 1

class SaveManager {
public:
    static SaveManager& Instance();

    // Called from eeprom stubs
    int ReadBlocks(int file, int offset, void* buffer, int count);
    int WriteBlocks(int file, int offset, void* buffer, int count);

    // [port] Lives persistence — N64 resets to 3 each session, PC port saves them
    static int GetSavedLives(int eepromSlot);

    // [port] Bottles Bonus completion persistence
    static void GetSavedBottleBonusGames(int eepromSlot, uint8_t out[7]);
    static void SetSavedBottleBonusGames(int eepromSlot, const uint8_t in[7]);

private:
    SaveManager();

    // Named-field JSON serialization
    nlohmann::ordered_json SlotToJson(const uint8_t* slotData);
    void JsonToSlot(const nlohmann::ordered_json& j, uint8_t* slotData);

    void LoadFromDisk();
    void FlushSlotToDisk(int slotIndex);
    void FlushGlobalToDisk();
    void EnsureDirectory();
    std::string GetSavePath(const std::string& filename);

    uint8_t mEeprom[EEPROM_TOTAL_SIZE];
    int mSavedLives[SAVE_SLOT_COUNT];              // [port] Per-slot lives count
    uint8_t mSavedBottleBonus[SAVE_SLOT_COUNT][7]; // [port] Per-slot Bottles Bonus completions
    bool mLoaded;
};
