#pragma once

#include <ship/resource/Resource.h>
#include <ship/resource/ResourceFactoryBinary.h>

namespace Factories {
class ResourceFactoryBinaryBKDialogV0 : public Ship::ResourceFactoryBinary {
public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file,
                                                  std::shared_ptr<Ship::ResourceInitData> initData) override;
};

class ResourceFactoryBinaryBKQuizQuestionV0 : public Ship::ResourceFactoryBinary {
public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file,
                                                  std::shared_ptr<Ship::ResourceInitData> initData) override;
};

class ResourceFactoryBinaryBKGruntyQuestionV0 : public Ship::ResourceFactoryBinary {
public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file,
                                                  std::shared_ptr<Ship::ResourceInitData> initData) override;
};
}; // namespace Factories
