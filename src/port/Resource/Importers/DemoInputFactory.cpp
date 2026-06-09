#include "DemoInputFactory.h"

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
ResourceFactoryBinaryBKDemoInputV0::ReadResource(std::shared_ptr<Ship::File> file,
                                                 std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }

    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);
    auto out = std::vector<uint8_t>();

    const uint32_t inputCount = reader->ReadUInt32();
    const uint32_t packedSize = inputCount * 6;
    AppendValue<uint32_t>(out, packedSize);

    for (uint32_t i = 0; i < inputCount; i++) {
        const int8_t stickX = reader->ReadInt8();
        const int8_t stickY = reader->ReadInt8();
        const uint16_t buttons = reader->ReadUInt16();
        const uint8_t frames = reader->ReadUByte();
        const uint8_t unkFlag = reader->ReadUByte();

        AppendValue<int8_t>(out, stickX);
        AppendValue<int8_t>(out, stickY);
        AppendValue<uint16_t>(out, buttons);
        AppendValue<uint8_t>(out, frames);
        AppendValue<uint8_t>(out, unkFlag);
    }

    return MakeBlob(initData, std::move(out));
}
} // namespace Factories
