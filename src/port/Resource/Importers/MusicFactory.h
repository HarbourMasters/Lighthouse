#pragma once

#include <ship/resource/Resource.h>
#include <ship/resource/ResourceFactoryBinary.h>

#include <cstdint>
#include <vector>

namespace Factories {
constexpr uint32_t kBKMusicResourceType = 0x424B4D55; // BKMU
class BKMusic final : public Ship::Resource<void> {
  public:
    using Resource::Resource;
    void* GetPointer() override {
        return Data.empty() ? nullptr : Data.data();
    }
    size_t GetPointerSize() override {
        return Data.size();
    }
    std::vector<uint8_t> Data;
    uint32_t Volume = 0;
};
class ResourceFactoryBinaryBKMusicV0 : public Ship::ResourceFactoryBinary {
  public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file,
                                                  std::shared_ptr<Ship::ResourceInitData> initData) override;
};
}; // namespace Factories
