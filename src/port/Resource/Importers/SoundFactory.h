#pragma once

#include <ship/resource/Resource.h>
#include <ship/resource/ResourceFactoryBinary.h>

namespace Factories {
constexpr uint32_t kBKSoundResourceType = 0x424B534E;     // BKSN
constexpr uint32_t kBKSoundBankResourceType = 0x424B5342; // BKSB
class ResourceFactoryBinaryBKSoundV0 : public Ship::ResourceFactoryBinary {
public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file,
                                                  std::shared_ptr<Ship::ResourceInitData> initData) override;
};
class ResourceFactoryBinaryBKSoundBankV0 : public Ship::ResourceFactoryBinary {
public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file,
                                                  std::shared_ptr<Ship::ResourceInitData> initData) override;
};
}; // namespace Factories
