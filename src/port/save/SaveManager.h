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

    static void RestoreFileEnhancementData(int eepromSlot);

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
    bool mLoaded;
};
