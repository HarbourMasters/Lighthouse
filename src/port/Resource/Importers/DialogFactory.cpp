#include "DialogFactory.h"

#include <libultraship/libultraship.h>
#include <ship/resource/type/Blob.h>

#include <algorithm>
#include <vector>

namespace Factories {
namespace {

constexpr uint32_t kMaxDialogLanguages = 3;

// Little-endian u16 offset per slot, measured from the start of the blob.
void AppendOffsetTable(std::vector<uint8_t>& out, const std::vector<std::vector<uint8_t>>& blocks, uint32_t headerSize,
                       uint32_t slots) {
    std::vector<uint16_t> offsets;
    offsets.reserve(blocks.size());
    uint32_t pos = headerSize;
    for (const auto& block : blocks) {
        offsets.push_back(static_cast<uint16_t>(pos));
        pos += static_cast<uint32_t>(block.size());
    }
    for (uint32_t i = 0; i < slots; i++) {
        const uint16_t off = offsets[std::min<size_t>(i, offsets.size() - 1)];
        out.push_back(static_cast<uint8_t>(off & 0xFF));
        out.push_back(static_cast<uint8_t>((off >> 8) & 0xFF));
    }
}
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
    if (langCount == 0) {
        return {};
    }
    const uint32_t slots = std::max(langCount, kMaxDialogLanguages);
    const uint32_t headerSize = 3 + slots * 2;

    std::vector<uint8_t> out;
    out.push_back(static_cast<uint8_t>(langCount));
    out.push_back(header1);
    out.push_back(header2);
    AppendOffsetTable(out, blocks, headerSize, slots);

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
    if (langCount == 0) {
        return nullptr;
    }

    std::vector<std::vector<uint8_t>> blocks;
    blocks.reserve(langCount);
    for (uint32_t i = 0; i < langCount; i++) {
        blocks.push_back(ReadLangBlock(reader));
    }

    // Header is 1 byte + (langCount * 2) bytes for offset table
    const uint32_t slots = std::max(langCount, kMaxDialogLanguages);
    const uint32_t headerSize = 1 + slots * 2;

    std::vector<uint8_t> out;
    out.push_back(static_cast<uint8_t>(langCount == 1 ? 0x01 : 0x03));
    AppendOffsetTable(out, blocks, headerSize, slots);

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
