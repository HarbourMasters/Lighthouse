#pragma once

#include <ship/resource/Resource.h>
#include <ship/resource/ResourceFactoryBinary.h>

#include <cstdint>
#include <vector>

namespace BK64 {
class MapData;
}

namespace Factories {
std::vector<uint8_t> LegacySetupBytes(const BK64::MapData& map);

class ResourceFactoryBinaryBKMapV0 : public Ship::ResourceFactoryBinary {
public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file,
                                                  std::shared_ptr<Ship::ResourceInitData> initData) override;
};
}; // namespace Factories
