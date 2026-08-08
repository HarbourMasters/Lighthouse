#include "SaveConverter.h"
#include "SaveManager.h"
#include <libultraship/libultra/os.h>
#include "save.h"
#include "Types.h"
#include "enums.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "port/FilePicker.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

nlohmann::ordered_json Convert_SaveDataToJSON(SaveData* saveData, int32_t fileNum);
SaveData* Convert_JSONToSaveData(int32_t fileNum);
std::string CollapsedJSONArray(const nlohmann::ordered_json& jsonFile);

extern "C" void glcrc_calc_checksum(void* start, void* end, uint32_t checksum[2]);

namespace {

constexpr size_t kRecompBinSize = 0x800;
constexpr size_t kExtBlockOffset = EEPROM_TOTAL_SIZE;
constexpr size_t kExtBlockSize = 320;
constexpr int kExtNoteLevels = 9;
constexpr size_t kExtNoteBytesPerLevel = 32;
constexpr size_t kExtJinjoOffset = kExtNoteLevels * kExtNoteBytesPerLevel; // 288
constexpr int kJinjoBitsPerLevel = 6;                                      // five plus an "all collected" flag
constexpr uint8_t kAllJinjos = 0x1F;

const int kExtNoteLevelOrder[kExtNoteLevels] = {
    LEVEL_1_MUMBOS_MOUNTAIN,     LEVEL_2_TREASURE_TROVE_COVE, LEVEL_3_CLANKERS_CAVERN,
    LEVEL_4_BUBBLEGLOOP_SWAMP,   LEVEL_5_FREEZEEZY_PEAK,      LEVEL_7_GOBIS_VALLEY,
    LEVEL_A_MAD_MONSTER_MANSION, LEVEL_9_RUSTY_BUCKET_BAY,    LEVEL_8_CLICK_CLOCK_WOOD,
};

struct NoteMapDef {
    int mapId;
    int levelId;
    int noteCount;
};

const NoteMapDef kNoteMaps[] = {
    { MAP_2_MM_MUMBOS_MOUNTAIN, LEVEL_1_MUMBOS_MOUNTAIN, 90 },
    { MAP_5_TTC_BLUBBERS_SHIP, LEVEL_2_TREASURE_TROVE_COVE, 8 },
    { MAP_6_TTC_NIPPERS_SHELL, LEVEL_2_TREASURE_TROVE_COVE, 6 },
    { MAP_7_TTC_TREASURE_TROVE_COVE, LEVEL_2_TREASURE_TROVE_COVE, 82 },
    { MAP_A_TTC_SANDCASTLE, LEVEL_2_TREASURE_TROVE_COVE, 4 },
    { MAP_B_CC_CLANKERS_CAVERN, LEVEL_3_CLANKERS_CAVERN, 72 },
    { MAP_C_MM_TICKERS_TOWER, LEVEL_1_MUMBOS_MOUNTAIN, 6 },
    { MAP_D_BGS_BUBBLEGLOOP_SWAMP, LEVEL_4_BUBBLEGLOOP_SWAMP, 88 },
    { MAP_E_MM_MUMBOS_SKULL, LEVEL_1_MUMBOS_MOUNTAIN, 4 },
    { MAP_10_BGS_MR_VILE, LEVEL_4_BUBBLEGLOOP_SWAMP, 6 },
    { MAP_11_BGS_TIPTUP, LEVEL_4_BUBBLEGLOOP_SWAMP, 6 },
    { MAP_12_GV_GOBIS_VALLEY, LEVEL_7_GOBIS_VALLEY, 70 },
    { MAP_13_GV_MEMORY_GAME, LEVEL_7_GOBIS_VALLEY, 4 },
    { MAP_14_GV_SANDYBUTTS_MAZE, LEVEL_7_GOBIS_VALLEY, 7 },
    { MAP_15_GV_WATER_PYRAMID, LEVEL_7_GOBIS_VALLEY, 4 },
    { MAP_16_GV_RUBEES_CHAMBER, LEVEL_7_GOBIS_VALLEY, 8 },
    { MAP_1A_GV_INSIDE_JINXY, LEVEL_7_GOBIS_VALLEY, 7 },
    { MAP_1B_MMM_MAD_MONSTER_MANSION, LEVEL_A_MAD_MONSTER_MANSION, 47 },
    { MAP_1C_MMM_CHURCH, LEVEL_A_MAD_MONSTER_MANSION, 10 },
    { MAP_1D_MMM_CELLAR, LEVEL_A_MAD_MONSTER_MANSION, 4 },
    { MAP_21_CC_WITCH_SWITCH_ROOM, LEVEL_3_CLANKERS_CAVERN, 6 },
    { MAP_22_CC_INSIDE_CLANKER, LEVEL_3_CLANKERS_CAVERN, 16 },
    { MAP_23_CC_GOLDFEATHER_ROOM, LEVEL_3_CLANKERS_CAVERN, 6 },
    { MAP_24_MMM_TUMBLARS_SHED, LEVEL_A_MAD_MONSTER_MANSION, 4 },
    { MAP_25_MMM_WELL, LEVEL_A_MAD_MONSTER_MANSION, 7 },
    { MAP_26_MMM_NAPPERS_ROOM, LEVEL_A_MAD_MONSTER_MANSION, 8 },
    { MAP_27_FP_FREEZEEZY_PEAK, LEVEL_5_FREEZEEZY_PEAK, 82 },
    { MAP_29_MMM_NOTE_ROOM, LEVEL_A_MAD_MONSTER_MANSION, 9 },
    { MAP_2D_MMM_BEDROOM, LEVEL_A_MAD_MONSTER_MANSION, 4 },
    { MAP_2F_MMM_WATERDRAIN_BARREL, LEVEL_A_MAD_MONSTER_MANSION, 5 },
    { MAP_30_MMM_MUMBOS_SKULL, LEVEL_A_MAD_MONSTER_MANSION, 2 },
    { MAP_31_RBB_RUSTY_BUCKET_BAY, LEVEL_9_RUSTY_BUCKET_BAY, 43 },
    { MAP_34_RBB_ENGINE_ROOM, LEVEL_9_RUSTY_BUCKET_BAY, 16 },
    { MAP_35_RBB_WAREHOUSE, LEVEL_9_RUSTY_BUCKET_BAY, 4 },
    { MAP_37_RBB_CONTAINER_1, LEVEL_9_RUSTY_BUCKET_BAY, 8 },
    { MAP_38_RBB_CONTAINER_3, LEVEL_9_RUSTY_BUCKET_BAY, 4 },
    { MAP_39_RBB_CREW_CABIN, LEVEL_9_RUSTY_BUCKET_BAY, 4 },
    { MAP_3B_RBB_STORAGE_ROOM, LEVEL_9_RUSTY_BUCKET_BAY, 5 },
    { MAP_3C_RBB_KITCHEN, LEVEL_9_RUSTY_BUCKET_BAY, 5 },
    { MAP_3D_RBB_NAVIGATION_ROOM, LEVEL_9_RUSTY_BUCKET_BAY, 4 },
    { MAP_3F_RBB_CAPTAINS_CABIN, LEVEL_9_RUSTY_BUCKET_BAY, 3 },
    { MAP_40_CCW_HUB, LEVEL_8_CLICK_CLOCK_WOOD, 4 },
    { MAP_43_CCW_SPRING, LEVEL_8_CLICK_CLOCK_WOOD, 16 },
    { MAP_44_CCW_SUMMER, LEVEL_8_CLICK_CLOCK_WOOD, 16 },
    { MAP_45_CCW_AUTUMN, LEVEL_8_CLICK_CLOCK_WOOD, 37 },
    { MAP_46_CCW_WINTER, LEVEL_8_CLICK_CLOCK_WOOD, 16 },
    { MAP_48_FP_MUMBOS_SKULL, LEVEL_5_FREEZEEZY_PEAK, 6 },
    { MAP_4C_CCW_AUTUMN_MUMBOS_SKULL, LEVEL_8_CLICK_CLOCK_WOOD, 4 },
    { MAP_53_FP_CHRISTMAS_TREE, LEVEL_5_FREEZEEZY_PEAK, 12 },
    { MAP_5C_CCW_AUTUMN_ZUBBA_HIVE, LEVEL_8_CLICK_CLOCK_WOOD, 4 },
    { MAP_60_CCW_AUTUMN_NABNUTS_HOUSE, LEVEL_8_CLICK_CLOCK_WOOD, 3 },
    { MAP_8B_RBB_ANCHOR_ROOM, LEVEL_9_RUSTY_BUCKET_BAY, 4 },
};
constexpr int kNoteMapCount = (int)(sizeof(kNoteMaps) / sizeof(kNoteMaps[0]));

void SwapU16(uint8_t* p) {
    std::swap(p[0], p[1]);
}
void SwapU32(uint8_t* p) {
    std::swap(p[0], p[3]);
    std::swap(p[1], p[2]);
}
void SwapU64(uint8_t* p) {
    for (int i = 0; i < 4; i++) {
        std::swap(p[i], p[7 - i]);
    }
}

// Swap a slot's multi-byte data[] fields between N64 and host order (its own inverse).
// Offsets are relative to data[0], two bytes into the slot (after magic + slotIndex).
void SwapSlotEndianness(uint8_t* data) {
    SwapU64(&data[NOTE_OFFSET]);   // packed note scores (u64)
    for (int i = 0; i < 11; i++) { // 11 world time scores (u16)
        SwapU16(&data[TIME_OFFSET + i * 2]);
    }
    SwapU32(&data[ABILITY_OFFSET]);     // learned abilities (u32)
    SwapU32(&data[ABILITY_OFFSET + 4]); // used abilities (u32)
}

void StoreChecksumBE(uint8_t* dst, const uint8_t* start, const uint8_t* end) {
    uint32_t cs[2];
    glcrc_calc_checksum(const_cast<uint8_t*>(start), const_cast<uint8_t*>(end), cs);
    uint32_t sum = cs[0] ^ cs[1];
    dst[0] = (uint8_t)(sum >> 24);
    dst[1] = (uint8_t)(sum >> 16);
    dst[2] = (uint8_t)(sum >> 8);
    dst[3] = (uint8_t)sum;
}

bool SlotChecksumOk(const uint8_t* slot) {
    uint32_t cs[2];
    glcrc_calc_checksum(const_cast<uint8_t*>(slot), const_cast<uint8_t*>(slot + SAVE_SLOT_SIZE - 4), cs);
    uint32_t sum = cs[0] ^ cs[1];
    uint32_t stored = ((uint32_t)slot[SAVE_SLOT_SIZE - 4] << 24) | ((uint32_t)slot[SAVE_SLOT_SIZE - 3] << 16) |
                      ((uint32_t)slot[SAVE_SLOT_SIZE - 2] << 8) | (uint32_t)slot[SAVE_SLOT_SIZE - 1];
    return sum == stored;
}

int SlotClaim(const uint8_t* slot) {
    if (slot[1] < 1 || slot[1] > 3 || !SlotChecksumOk(slot)) {
        return -1;
    }
    return slot[1] - 1;
}

int SlotDisplayedGame(const uint8_t* slot) {
    int claim = SlotClaim(slot);
    if (claim < 0 || slot[0] != SAVE_MAGIC) {
        return 0;
    }
    return SlotToFileIndex(claim);
}

int ExtNoteLevelSlot(int levelId) {
    for (int i = 0; i < kExtNoteLevels; i++) {
        if (kExtNoteLevelOrder[i] == levelId) {
            return i;
        }
    }
    return -1;
}

int ExtNoteStartIndex(int entry) {
    int start = 0;
    for (int i = 0; i < entry; i++) {
        if (kNoteMaps[i].levelId == kNoteMaps[entry].levelId) {
            start += kNoteMaps[i].noteCount;
        }
    }
    return start;
}

int ExtJinjoBitBase(int levelId) {
    if (levelId < LEVEL_1_MUMBOS_MOUNTAIN || levelId > LEVEL_A_MAD_MONSTER_MANSION) {
        return -1;
    }
    return (levelId - LEVEL_1_MUMBOS_MOUNTAIN) * kJinjoBitsPerLevel;
}

bool ExtBitGet(const uint8_t* bits, int index) {
    return (bits[index >> 3] >> (index & 7)) & 1;
}

void ExtBitSet(uint8_t* bits, int index) {
    bits[index >> 3] |= (uint8_t)(1 << (index & 7));
}

void ImportExtensionBlock(const uint8_t* ext, SaveData& sd) {
    for (int e = 0; e < kNoteMapCount; e++) {
        const int levelSlot = ExtNoteLevelSlot(kNoteMaps[e].levelId);
        if (levelSlot < 0) {
            continue;
        }
        const uint8_t* levelBits = ext + levelSlot * kExtNoteBytesPerLevel;
        const int start = ExtNoteStartIndex(e);
        for (int i = 0; i < kNoteMaps[e].noteCount && i < NOTE_RETENTION_NOTES_PER_MAP; i++) {
            if (ExtBitGet(levelBits, start + i)) {
                ExtBitSet(sd.shipSaveData.noteRetention.collected[kNoteMaps[e].mapId], i);
            }
        }
    }

    const uint8_t* jinjoBits = ext + kExtJinjoOffset;
    for (int level = LEVEL_1_MUMBOS_MOUNTAIN; level <= LEVEL_A_MAD_MONSTER_MANSION; level++) {
        const int base = ExtJinjoBitBase(level);
        uint8_t collected = 0;
        for (int colour = 0; colour < 5; colour++) {
            if (ExtBitGet(jinjoBits, base + colour)) {
                collected |= (uint8_t)(1 << colour);
            }
        }
        if (ExtBitGet(jinjoBits, base + 5)) {
            collected = kAllJinjos;
        }
        sd.shipSaveData.jinjoRetention.collected[level] = collected;
    }
}

void ExportExtensionBlock(const SaveData& sd, uint8_t* ext) {
    std::memset(ext, 0, kExtJinjoOffset + 8); // level_notes[] + jinjo_data[]

    for (int e = 0; e < kNoteMapCount; e++) {
        const int levelSlot = ExtNoteLevelSlot(kNoteMaps[e].levelId);
        if (levelSlot < 0) {
            continue;
        }
        uint8_t* levelBits = ext + levelSlot * kExtNoteBytesPerLevel;
        const int start = ExtNoteStartIndex(e);
        for (int i = 0; i < kNoteMaps[e].noteCount && i < NOTE_RETENTION_NOTES_PER_MAP; i++) {
            if (ExtBitGet(sd.shipSaveData.noteRetention.collected[kNoteMaps[e].mapId], i)) {
                ExtBitSet(levelBits, start + i);
            }
        }
    }

    uint8_t* jinjoBits = ext + kExtJinjoOffset;
    for (int level = LEVEL_1_MUMBOS_MOUNTAIN; level <= LEVEL_A_MAD_MONSTER_MANSION; level++) {
        const int base = ExtJinjoBitBase(level);
        const uint8_t collected = sd.shipSaveData.jinjoRetention.collected[level];
        if (collected == kAllJinjos) {
            for (int colour = 0; colour < 4; colour++) {
                ExtBitSet(jinjoBits, base + colour);
            }
            ExtBitSet(jinjoBits, base + 5);
            continue;
        }
        for (int colour = 0; colour < 5; colour++) {
            if (collected & (1 << colour)) {
                ExtBitSet(jinjoBits, base + colour);
            }
        }
    }
}

bool ImportSlotToGame(const uint8_t* slotPtr, const uint8_t* ext, int destGame, const std::string& fromTag) {
    if (slotPtr[0] != SAVE_MAGIC) {
        return false;
    }

    SaveData sd;
    std::memset(&sd, 0, sizeof(SaveData));
    std::memcpy(&sd, slotPtr, SAVE_SLOT_SIZE); // magic..checksum (first 120 bytes)
    SwapSlotEndianness(sd.data);               // N64 big-endian -> host byte order
    sd.magic = SAVE_MAGIC;
    sd.shipSaveData.fileType = FILE_TYPE_SAVE_VANILLA; // external saves carry no port data
    if (ext != nullptr) {
        ImportExtensionBlock(ext, sd);
    }

    nlohmann::ordered_json j = Convert_SaveDataToJSON(&sd, destGame - 1);
    if (j.empty()) {
        return false;
    }
    j["ship"]["importedFrom"] = fromTag; // record the source for reference

    std::string filePath = SaveManager_GetSavePath("file" + std::to_string(destGame) + ".json");
    std::ofstream out(filePath);
    if (!out.is_open()) {
        SPDLOG_ERROR("SaveConverter: failed to write \"{}\"", filePath);
        return false;
    }
    out << CollapsedJSONArray(j);
    out.close();
    return true;
}

// Map a displayed game number (1/2/3) to the decomp file index that holds it.
int GameNumberToFileNum(int gameNumber) {
    for (int f = 0; f < 3; f++) {
        if (SlotToFileIndex(f) == gameNumber) {
            return f;
        }
    }
    return -1;
}

// Write a valid empty global block (no Stop 'n' Swop data) so a fresh file is accepted.
void StampEmptyGlobal(uint8_t* buffer) {
    uint8_t* g = buffer + GLOBAL_OFFSET_BLOCK * EEPROM_BLOCK_SIZE; // byte 480
    std::memset(g, 0, GLOBAL_SIZE);                                // snsItems + padding + checksum
    StoreChecksumBE(g + (GLOBAL_SIZE - 4), g, g + (GLOBAL_SIZE - 4));
}

void PlanSlots(const uint8_t* buffer, const bool wanted[3], int dest[3]) {
    bool taken[SAVE_SLOT_COUNT] = {};
    int claim[SAVE_SLOT_COUNT];
    for (int s = 0; s < SAVE_SLOT_COUNT; s++) {
        claim[s] = SlotClaim(buffer + (size_t)s * SAVE_SLOT_SIZE);
    }
    for (int f = 0; f < 3; f++) {
        dest[f] = -1;
        if (!wanted[f]) {
            continue;
        }
        for (int s = 0; s < SAVE_SLOT_COUNT; s++) {
            if (!taken[s] && claim[s] == f) {
                dest[f] = s;
                taken[s] = true;
                break;
            }
        }
    }
    for (int pass = 0; pass < 2; pass++) {
        for (int f = 0; f < 3; f++) {
            if (!wanted[f] || dest[f] >= 0) {
                continue;
            }
            for (int s = 0; s < SAVE_SLOT_COUNT; s++) {
                if (taken[s] || (pass == 0 && claim[s] >= 0)) {
                    continue;
                }
                dest[f] = s;
                taken[s] = true;
                break;
            }
        }
    }
}

// Convert one Lighthouse save file to an EEPROM slot and place it in the buffer.
// Returns false when that file has no save. fileNum is the decomp 0..2 index.
bool ExportFileToSlot(uint8_t* buffer, int fileNum, int physSlot) {
    SaveData* sd = Convert_JSONToSaveData(fileNum);
    if (sd->slotIndex == 0) {
        delete sd;
        return false;
    }

    uint8_t slot[SAVE_SLOT_SIZE];
    std::memcpy(slot, sd, SAVE_SLOT_SIZE); // magic..checksum (first 120 bytes)

    slot[0] = SAVE_MAGIC;                                    // magic; slot[1] keeps Convert's slotIndex
    slot[SAVE_SLOT_SIZE - 6] = slot[SAVE_SLOT_SIZE - 5] = 0; // padding before the checksum
    SwapSlotEndianness(slot + 2);                            // host -> N64 big-endian
    StoreChecksumBE(&slot[SAVE_SLOT_SIZE - 4], slot, slot + SAVE_SLOT_SIZE - 4);

    std::memcpy(buffer + (size_t)physSlot * SAVE_SLOT_SIZE, slot, SAVE_SLOT_SIZE);
    ExportExtensionBlock(*sd, buffer + kExtBlockOffset + (size_t)physSlot * kExtBlockSize);
    delete sd;
    return true;
}

void ClearStaleClaims(uint8_t* buffer, int fileNum, int keepSlot) {
    for (int s = 0; s < SAVE_SLOT_COUNT; s++) {
        if (s == keepSlot || SlotClaim(buffer + (size_t)s * SAVE_SLOT_SIZE) != fileNum) {
            continue;
        }
        std::memset(buffer + (size_t)s * SAVE_SLOT_SIZE, 0, SAVE_SLOT_SIZE);
        std::memset(buffer + kExtBlockOffset + (size_t)s * kExtBlockSize, 0, kExtBlockSize);
    }
}

} // namespace

namespace SaveConverter {

Result ImportFromRawEeprom(const std::string& srcPath, int slot) {
    Result res;

    std::ifstream in(srcPath, std::ios::binary);
    if (!in) {
        res.message = "Could not open the selected file.";
        return res;
    }
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    // Save data sits at offset 0 for both .srm files and raw dumps.
    if (bytes.size() < static_cast<size_t>(SAVE_SLOT_SIZE) * SAVE_SLOT_COUNT) {
        res.message = "That file doesn't look like a Banjo-Kazooie save.";
        return res;
    }

    std::string fromTag = std::filesystem::path(srcPath).filename().string();
    auto slotAt = [&](int s) { return bytes.data() + static_cast<size_t>(s) * SAVE_SLOT_SIZE; };
    const bool hasExtension = bytes.size() >= kRecompBinSize;
    auto extAt = [&](int s) {
        return hasExtension ? bytes.data() + kExtBlockOffset + static_cast<size_t>(s) * kExtBlockSize : nullptr;
    };

    if (slot == kSlotAll) {
        bool imported[4] = {};
        for (int s = 0; s < SAVE_SLOT_COUNT; s++) {
            int game = SlotDisplayedGame(slotAt(s));
            if (game == 0 || imported[game]) {
                continue;
            }
            if (ImportSlotToGame(slotAt(s), extAt(s), game, fromTag)) {
                imported[game] = true;
                res.filesImported++;
            }
        }
    } else {
        // Send the first save found into the chosen game file.
        for (int s = 0; s < SAVE_SLOT_COUNT; s++) {
            if (SlotDisplayedGame(slotAt(s)) != 0) {
                if (ImportSlotToGame(slotAt(s), extAt(s), slot, fromTag)) {
                    res.filesImported++;
                }
                break;
            }
        }
    }

    res.ok = true;
    if (res.filesImported == 0) {
        res.message = "No save files were found in that file.";
    } else {
        res.message = "Imported " + std::to_string(res.filesImported) +
                      " save file(s).\n\n"
                      "Restart Lighthouse to see them.";
    }
    return res;
}

Result ExportToRecompBin(const std::string& dstPath, int slot) {
    Result res;
    std::vector<uint8_t> buffer(kRecompBinSize, 0);

    // Preserve an existing file so untouched slots, global data, and trailing bytes survive.
    std::ifstream existing(dstPath, std::ios::binary);
    if (existing) {
        std::vector<uint8_t> prev((std::istreambuf_iterator<char>(existing)), std::istreambuf_iterator<char>());
        std::memcpy(buffer.data(), prev.data(), std::min(prev.size(), kRecompBinSize));
    } else {
        StampEmptyGlobal(buffer.data()); // a fresh file still needs a valid global block
    }

    bool wanted[3] = {};
    if (slot == kSlotAll) {
        wanted[0] = wanted[1] = wanted[2] = true;
    } else {
        int f = GameNumberToFileNum(slot);
        if (f >= 0) {
            wanted[f] = true;
        }
    }

    int dest[3];
    PlanSlots(buffer.data(), wanted, dest);

    for (int f = 0; f < 3; f++) {
        if (!wanted[f] || dest[f] < 0) {
            continue;
        }
        if (ExportFileToSlot(buffer.data(), f, dest[f])) {
            ClearStaleClaims(buffer.data(), f, dest[f]);
            res.filesImported++;
        }
    }

    if (res.filesImported == 0) {
        res.message = "No saves were found to export.";
        return res;
    }

    std::ofstream out(dstPath, std::ios::binary);
    if (!out) {
        res.message = "Couldn't write to that location.";
        return res;
    }
    out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    res.ok = true;
    res.message = "Exported " + std::to_string(res.filesImported) + " save file(s).";
    return res;
}

void PickAndImport(int slot, std::function<void(Result)> onComplete) {
    // We can't predict the file extension a save will come in, and we already validate later in the
    // chain, so trust the user to pick the correct file.
    Ship::FileBrowserRequest req;
    req.Title = "Select a save to import";
    req.Filters = { { "All Files", { "*" } } };
    Lighthouse::PickFile(std::move(req), [slot, onComplete](std::optional<std::filesystem::path> path) {
        if (!path) {
            if (onComplete) {
                onComplete({}); // cancelled
            }
            return;
        }
        Result r = ImportFromRawEeprom(path->string(), slot);
        if (onComplete) {
            onComplete(r);
        }
    });
}

void PickAndExport(int slot, std::function<void(Result)> onComplete) {
    Ship::FileBrowserRequest req;
    req.Title = "Export save";
    req.Save = true;
    req.DefaultName = "bk.n64.us.1.0.bin";
    req.Filters = { { "Save files (*.bin)", { "*.bin" } }, { "All Files", { "*" } } };
    Lighthouse::PickFile(std::move(req), [slot, onComplete](std::optional<std::filesystem::path> path) {
        if (!path) {
            if (onComplete) {
                onComplete({}); // cancelled
            }
            return;
        }
        Result r = ExportToRecompBin(path->string(), slot);
        if (onComplete) {
            onComplete(r);
        }
    });
}

} // namespace SaveConverter
