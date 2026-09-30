#include "SoundFactory.h"

#include <libultraship/libultraship.h>
#include <ship/resource/type/Blob.h>

namespace Factories {
namespace {

std::shared_ptr<Ship::IResource> PassThrough(const std::shared_ptr<Ship::File>& file,
                                             const std::shared_ptr<Ship::ResourceInitData>& initData) {
    if (file->Buffer == nullptr || file->BufferOffset > file->Buffer->size()) {
        return nullptr;
    }

    auto blob = std::make_shared<Ship::Blob>(initData);
    const auto begin = file->Buffer->begin() + file->BufferOffset;
    blob->Data.assign(begin, file->Buffer->end());
    return blob;
}

} // namespace

std::shared_ptr<Ship::IResource>
ResourceFactoryBinaryBKSoundV0::ReadResource(std::shared_ptr<Ship::File> file,
                                             std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }
    return PassThrough(file, initData);
}

std::shared_ptr<Ship::IResource>
ResourceFactoryBinaryBKSoundBankV0::ReadResource(std::shared_ptr<Ship::File> file,
                                                 std::shared_ptr<Ship::ResourceInitData> initData) {
    if (!FileHasValidFormatAndReader(file, initData)) {
        return nullptr;
    }
    return PassThrough(file, initData);
}

} // namespace Factories
