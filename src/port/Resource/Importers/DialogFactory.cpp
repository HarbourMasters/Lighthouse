#include "DialogFactory.h"

#include <libultraship/libultraship.h>
#include <ship/resource/type/Blob.h>

namespace Factories {
namespace {
void AppendBytes(std::vector<uint8_t>& dst, const char* data, size_t size) {
    const auto base = dst.size();
    dst.resize(base + size);
    std::memcpy(dst.data() + base, data, size);
}

std::string ReadSizedString(const std::shared_ptr<Ship::BinaryReader>& reader, uint32_t len) {
    std::string out;
    out.resize(len);
    if (len > 0) {
        reader->Read(out.data(), static_cast<int32_t>(len));
    }
    return out;
}

std::shared_ptr<Ship::Blob> MakeBlob(const std::shared_ptr<Ship::ResourceInitData>& initData,
                                     std::vector<uint8_t>&& data) {
    auto blob = std::make_shared<Ship::Blob>(initData);
    blob->Data = std::move(data);
    return blob;
}

// Read one language block from Torch's o2r format (u32 counts, u8 cmd, u32 strlen, chars)
// and reconstruct the ROM format (u8 count, u8 cmd, u8 strlen, chars)
std::vector<uint8_t> ReadLangBlock(const std::shared_ptr<Ship::BinaryReader>& reader) {
    std::vector<uint8_t> block;

    // Bottom entries
    const uint32_t bottomCount = reader->ReadUInt32();
    block.push_back(static_cast<uint8_t>(bottomCount));
    for (uint32_t i = 0; i < bottomCount; i++) {
        const uint8_t cmd = reader->ReadUByte();
        const uint32_t len = reader->ReadUInt32();
        const auto str = ReadSizedString(reader, len);
        block.push_back(cmd);
        block.push_back(static_cast<uint8_t>(len));
        AppendBytes(block, str.data(), str.size());
    }

    // Top entries
    const uint32_t topCount = reader->ReadUInt32();
    block.push_back(static_cast<uint8_t>(topCount));
    for (uint32_t i = 0; i < topCount; i++) {
        const uint8_t cmd = reader->ReadUByte();
        const uint32_t len = reader->ReadUInt32();
        const auto str = ReadSizedString(reader, len);
        block.push_back(cmd);
        block.push_back(static_cast<uint8_t>(len));
        AppendBytes(block, str.data(), str.size());
    }

    return block;
}

uint32_t ReadEntryRun(const std::shared_ptr<Ship::BinaryReader>& reader, std::vector<uint8_t>& block) {
    const uint32_t count = reader->ReadUInt32();
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t cmd = reader->ReadUByte();
        const uint32_t len = reader->ReadUInt32();
        const auto str = ReadSizedString(reader, len);
        block.push_back(cmd);
        block.push_back(static_cast<uint8_t>(len));
        AppendBytes(block, str.data(), str.size());
    }
    return count;
}

std::vector<uint8_t> ReadQuizLangBlock(const std::shared_ptr<Ship::BinaryReader>& reader) {
    std::vector<uint8_t> entries;
    const uint32_t textCount = ReadEntryRun(reader, entries);
    const uint32_t optionCount = ReadEntryRun(reader, entries);

    std::vector<uint8_t> block;
    block.push_back(static_cast<uint8_t>(textCount + optionCount));
    block.insert(block.end(), entries.begin(), entries.end());
    return block;
}

std::vector<uint8_t> ReadGruntyLangBlock(const std::shared_ptr<Ship::BinaryReader>& reader) {
    std::vector<uint8_t> entries;
    const uint32_t textCount = ReadEntryRun(reader, entries);

    const uint32_t optionCount = reader->ReadUInt32();
    for (uint32_t i = 0; i < optionCount; i++) {
        const uint8_t cmd = reader->ReadUByte();
        const uint8_t unk0 = reader->ReadUByte();
        const uint8_t unk1 = reader->ReadUByte();
        const uint32_t len = reader->ReadUInt32();
        const auto str = ReadSizedString(reader, len);

        entries.push_back(cmd);
        entries.push_back(static_cast<uint8_t>(len + 2));
        entries.push_back(unk0);
        entries.push_back(unk1);
        AppendBytes(entries, str.data(), str.size());
    }

    std::vector<uint8_t> block;
    block.push_back(static_cast<uint8_t>(textCount + optionCount));
    block.insert(block.end(), entries.begin(), entries.end());
    return block;
}

std::vector<uint8_t> BuildQuestionBlob(uint8_t header1, uint8_t header2,
                                       const std::vector<std::vector<uint8_t>>& blocks) {
    const uint32_t langCount = static_cast<uint32_t>(blocks.size());
    const uint32_t headerSize = 3 + langCount * 2;

    std::vector<uint8_t> out;
    out.push_back(static_cast<uint8_t>(langCount));
    out.push_back(header1);
    out.push_back(header2);

    uint32_t pos = headerSize;
    for (const auto& block : blocks) {
        out.push_back(static_cast<uint8_t>(pos & 0xFF));
        out.push_back(static_cast<uint8_t>((pos >> 8) & 0xFF));
        pos += static_cast<uint32_t>(block.size());
    }

    for (const auto& block : blocks) {
        out.insert(out.end(), block.begin(), block.end());
    }

    return out;
}
} // namespace

std::shared_ptr<Ship::IResource>
ResourceFactoryBinaryBKDialogV0::ReadResource(std::shared_ptr<Ship::File> file,
                                              std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }

    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);

    const uint32_t langCount = reader->ReadUInt32();

    if (langCount == 1) {
        // US/JP: reconstruct as 01 03 00 [data]
        // dialogBin_get reads bytes 1-2 as LE u16 offset (0x03, 0x00 = 3), data at byte 3
        auto block = ReadLangBlock(reader);
        std::vector<uint8_t> out;
        out.push_back(0x01); // header byte 0
        out.push_back(0x03); // byte 1: offset lo = 3
        out.push_back(0x00); // byte 2: offset hi = 0
        out.insert(out.end(), block.begin(), block.end());
        return MakeBlob(initData, std::move(out));
    }

    // PAL (3 languages): reconstruct as [header] [fr_off LE u16] [de_off LE u16] [EN] [FR] [DE]
    // dialogBin_get: ptr+1, skip lang*2, read LE u16 offset, jump to ptr+offset
    //   lang=0: bytes 1-2 = EN offset (= 7, right after header+offsets)
    //   lang=1: bytes 3-4 = FR offset
    //   lang=2: bytes 5-6 = DE offset

    // Read all language blocks from o2r
    std::vector<std::vector<uint8_t>> blocks;
    for (uint32_t i = 0; i < langCount; i++) {
        blocks.push_back(ReadLangBlock(reader));
    }

    // Header is 1 byte + (langCount * 2) bytes for offset table
    const uint32_t headerSize = 1 + langCount * 2;

    // Calculate offsets for each language block
    std::vector<uint16_t> offsets;
    uint32_t pos = headerSize;
    for (uint32_t i = 0; i < langCount; i++) {
        offsets.push_back(static_cast<uint16_t>(pos));
        pos += static_cast<uint32_t>(blocks[i].size());
    }

    // Build the blob
    std::vector<uint8_t> out;
    out.push_back(0x03); // PAL header byte 0

    // Write offset table (LE u16 for each language)
    for (uint32_t i = 0; i < langCount; i++) {
        out.push_back(static_cast<uint8_t>(offsets[i] & 0xFF));
        out.push_back(static_cast<uint8_t>((offsets[i] >> 8) & 0xFF));
    }

    // Append all language blocks
    for (const auto& block : blocks) {
        out.insert(out.end(), block.begin(), block.end());
    }

    return MakeBlob(initData, std::move(out));
}

std::shared_ptr<Ship::IResource>
ResourceFactoryBinaryBKQuizQuestionV0::ReadResource(std::shared_ptr<Ship::File> file,
                                                    std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }

    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);

    const uint32_t langCount = reader->ReadUInt32();

    std::vector<std::vector<uint8_t>> blocks;
    for (uint32_t i = 0; i < langCount; i++) {
        blocks.push_back(ReadQuizLangBlock(reader));
    }

    return MakeBlob(initData, BuildQuestionBlob(0x01, 0x02, blocks));
}

std::shared_ptr<Ship::IResource>
ResourceFactoryBinaryBKGruntyQuestionV0::ReadResource(std::shared_ptr<Ship::File> file,
                                                      std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }

    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);

    const uint32_t langCount = reader->ReadUInt32();

    std::vector<std::vector<uint8_t>> blocks;
    for (uint32_t i = 0; i < langCount; i++) {
        blocks.push_back(ReadGruntyLangBlock(reader));
    }

    return MakeBlob(initData, BuildQuestionBlob(0x03, 0x00, blocks));
}
} // namespace Factories
