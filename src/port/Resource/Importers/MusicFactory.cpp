#include "MusicFactory.h"

#include <libultraship/libultraship.h>

namespace Factories {

std::shared_ptr<Ship::IResource>
ResourceFactoryBinaryBKMusicV0::ReadResource(std::shared_ptr<Ship::File> file,
                                             std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }
    if (file->Buffer == nullptr) {
        return nullptr;
    }
    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);
    auto music = std::make_shared<BKMusic>(initData);
    music->Volume = reader->ReadUInt32();
    const uint32_t size = reader->ReadUInt32();
    const size_t begin = file->BufferOffset + 8;
    if (begin > file->Buffer->size() || size > file->Buffer->size() - begin) {
        SPDLOG_ERROR("[BKMusic] '{}' claims {} bytes of sequence but the file holds {}", initData->Path, size,
                     file->Buffer->size() - std::min(begin, file->Buffer->size()));
        return nullptr;
    }
    const auto at = file->Buffer->begin() + begin;
    music->Data.assign(at, at + size);
    return music;
}

} // namespace Factories
