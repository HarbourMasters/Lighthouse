#include "AnimFactory.h"

#include <libultraship/libultraship.h>
#include <ship/resource/type/Blob.h>

namespace Factories {
namespace {
template <typename T> void AppendValue(std::vector<uint8_t>& dst, const T& value) {
    const auto base = dst.size();
    dst.resize(base + sizeof(T));
    std::memcpy(dst.data() + base, &value, sizeof(T));
}

std::shared_ptr<Ship::Blob> MakeBlob(const std::shared_ptr<Ship::ResourceInitData>& initData,
                                     std::vector<uint8_t>&& data) {
    auto blob = std::make_shared<Ship::Blob>(initData);
    blob->Data = std::move(data);
    return blob;
}
} // namespace

std::shared_ptr<Ship::IResource>
ResourceFactoryBinaryBKAnimationV0::ReadResource(std::shared_ptr<Ship::File> file,
                                                 std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }

    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);
    auto out = std::vector<uint8_t>();

    const int16_t startFrame = reader->ReadInt16();
    const int16_t endFrame = reader->ReadInt16();
    const uint32_t fileCount = reader->ReadUInt32();

    AppendValue<int16_t>(out, startFrame);
    AppendValue<int16_t>(out, endFrame);
    AppendValue<int16_t>(out, static_cast<int16_t>(fileCount));
    AppendValue<int16_t>(out, 0);

    for (uint32_t i = 0; i < fileCount; i++) {
        const int16_t boneIndex = reader->ReadInt16();
        const int16_t transformType = reader->ReadInt16();
        const uint32_t dataCount = reader->ReadUInt32();

        // Struct declares unk0_15:12 first, unk0_3:4 second.
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        // BE: first field at MSB → unk0_15 at bits 15:4, unk0_3 at bits 3:0
        const uint16_t fileBitField =
            (static_cast<uint16_t>(boneIndex & 0x0FFF) << 4) | static_cast<uint16_t>(transformType & 0x000F);
#else
        // LE: first field at LSB → unk0_15 at bits 11:0, unk0_3 at bits 15:12
        const uint16_t fileBitField =
            (static_cast<uint16_t>(transformType & 0x000F) << 12) | static_cast<uint16_t>(boneIndex & 0x0FFF);
#endif
        AppendValue<uint16_t>(out, fileBitField);
        AppendValue<int16_t>(out, static_cast<int16_t>(dataCount));

        for (uint32_t j = 0; j < dataCount; j++) {
            const uint8_t bit15 = reader->ReadUByte();
            const uint8_t bit14 = reader->ReadUByte();
            const uint16_t unk13 = reader->ReadUInt16();
            const int16_t unk2 = reader->ReadInt16();

            // Struct declares unk0_15:1 first, unk0_14:1 second, unk0_13:14 third.
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            // BE: first field at MSB → unk0_15 at bit 15, unk0_14 at bit 14, unk0_13 at bits 13:0
            const uint16_t dataBitField =
                (static_cast<uint16_t>(bit15 & 1) << 15) | (static_cast<uint16_t>(bit14 & 1) << 14) | (unk13 & 0x3FFF);
#else
            // LE: first field at LSB → unk0_15 at bit 0, unk0_14 at bit 1, unk0_13 at bits 15:2
            const uint16_t dataBitField =
                ((unk13 & 0x3FFF) << 2) | (static_cast<uint16_t>(bit14 & 1) << 1) | static_cast<uint16_t>(bit15 & 1);
#endif
            AppendValue<uint16_t>(out, dataBitField);
            AppendValue<int16_t>(out, unk2);
        }
    }

    return MakeBlob(initData, std::move(out));
}
} // namespace Factories
