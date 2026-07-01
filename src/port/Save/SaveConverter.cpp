#include "SaveConverter.h"
#include "SaveManager.h"
#include <libultraship/libultra/os.h>
#include "save.h"
#include "Types.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "portable-file-dialogs.h"
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

// N64 saves store multi-byte fields big-endian; Lighthouse's native build uses host
// (little-endian) order. Swap those fields; bit/byte-packed regions are neutral.
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

// Convert one raw EEPROM slot to a Lighthouse save and write it to game file 1-3,
// tagging it with where it came from. Returns false if the slot is empty or unwritable.
bool ImportSlotToGame(const uint8_t* slotPtr, int destGame, const std::string& fromTag) {
    if (slotPtr[0] != SAVE_MAGIC) {
        return false;
    }

    SaveData sd;
    std::memset(&sd, 0, sizeof(SaveData));
    std::memcpy(&sd, slotPtr, SAVE_SLOT_SIZE); // magic..checksum (first 120 bytes)
    SwapSlotEndianness(sd.data);               // N64 big-endian -> host byte order
    sd.magic = SAVE_MAGIC;
    sd.shipSaveData.fileType = FILE_TYPE_SAVE_VANILLA; // external saves carry no port data

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

// Compute the EEPROM checksum and store it big-endian (N64 order) at dst.
void StoreChecksumBE(uint8_t* dst, const uint8_t* start, const uint8_t* end) {
    uint32_t cs[2];
    glcrc_calc_checksum(const_cast<uint8_t*>(start), const_cast<uint8_t*>(end), cs);
    uint32_t sum = cs[0] ^ cs[1];
    dst[0] = (uint8_t)(sum >> 24);
    dst[1] = (uint8_t)(sum >> 16);
    dst[2] = (uint8_t)(sum >> 8);
    dst[3] = (uint8_t)sum;
}

// Write a valid empty global block (no Stop 'n' Swop data) so a fresh file is accepted.
void StampEmptyGlobal(uint8_t* buffer) {
    uint8_t* g = buffer + GLOBAL_OFFSET_BLOCK * EEPROM_BLOCK_SIZE; // byte 480
    std::memset(g, 0, GLOBAL_SIZE);                                // snsItems + padding + checksum
    StoreChecksumBE(g + (GLOBAL_SIZE - 4), g, g + (GLOBAL_SIZE - 4));
}

// Convert one Lighthouse save file to an EEPROM slot and place it in the buffer.
// Returns false when that file has no save. fileNum is the decomp 0..2 index.
bool ExportFileToSlot(uint8_t* buffer, int fileNum) {
    SaveData* sd = Convert_JSONToSaveData(fileNum);
    if (sd->slotIndex == 0) {
        delete sd;
        return false;
    }

    uint8_t slot[SAVE_SLOT_SIZE];
    std::memcpy(slot, sd, SAVE_SLOT_SIZE); // magic..checksum (first 120 bytes)
    delete sd;

    slot[0] = SAVE_MAGIC;                                    // magic; slot[1] keeps Convert's slotIndex
    slot[SAVE_SLOT_SIZE - 6] = slot[SAVE_SLOT_SIZE - 5] = 0; // padding before the checksum
    SwapSlotEndianness(slot + 2);                            // host -> N64 big-endian
    StoreChecksumBE(&slot[SAVE_SLOT_SIZE - 4], slot, slot + SAVE_SLOT_SIZE - 4);

    std::memcpy(buffer + (size_t)fileNum * SAVE_SLOT_SIZE, slot, SAVE_SLOT_SIZE);
    return true;
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
    // Banjo-Kazooie uses three of the four slots (Game 1 / 3 / 2 on the file-select).
    if (bytes.size() < static_cast<size_t>(SAVE_SLOT_SIZE) * 3) {
        res.message = "That file doesn't look like a Banjo-Kazooie save.";
        return res;
    }

    std::string fromTag = std::filesystem::path(srcPath).filename().string();
    auto slotAt = [&](int s) { return bytes.data() + static_cast<size_t>(s) * SAVE_SLOT_SIZE; };

    if (slot == kSlotAll) {
        // Each populated slot keeps its own game file.
        for (int s = 0; s < 3; s++) {
            if (ImportSlotToGame(slotAt(s), SlotToFileIndex(s), fromTag)) {
                res.filesImported++;
            }
        }
    } else {
        // Send the first save found into the chosen game file.
        for (int s = 0; s < 3; s++) {
            if (slotAt(s)[0] == SAVE_MAGIC) {
                if (ImportSlotToGame(slotAt(s), slot, fromTag)) {
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
    constexpr size_t kBinSize = 0x800; // Banjo: Recompiled uses a 16k EEPROM image
    std::vector<uint8_t> buffer(kBinSize, 0);

    // Preserve an existing file so untouched slots, global data, and trailing bytes survive.
    std::ifstream existing(dstPath, std::ios::binary);
    if (existing) {
        std::vector<uint8_t> prev((std::istreambuf_iterator<char>(existing)), std::istreambuf_iterator<char>());
        std::memcpy(buffer.data(), prev.data(), std::min(prev.size(), kBinSize));
    } else {
        StampEmptyGlobal(buffer.data()); // a fresh file still needs a valid global block
    }

    if (slot == kSlotAll) {
        for (int f = 0; f < 3; f++) {
            if (ExportFileToSlot(buffer.data(), f)) {
                res.filesImported++;
            }
        }
    } else {
        int f = GameNumberToFileNum(slot);
        if (f >= 0 && ExportFileToSlot(buffer.data(), f)) {
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

Result PickAndImport(int slot) {
    // We can't predict the file extension a save will come in, and we already validate later in the chain
    // Trust the user to pick the correct file
    auto selection = pfd::open_file("Select a save to import", ".", { "All Files", "*" }).result();
    if (selection.empty()) {
        return {};
    }
    return ImportFromRawEeprom(selection[0], slot);
}

Result PickAndExport(int slot) {
    auto dest = pfd::save_file("Export save", "bk.n64.us.1.0.bin", { "Save files (*.bin)", "*.bin", "All Files", "*" })
                    .result();
    if (dest.empty()) {
        return {};
    }
    return ExportToRecompBin(dest, slot);
}

} // namespace SaveConverter
