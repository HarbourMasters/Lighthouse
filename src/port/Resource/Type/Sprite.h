#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <libultraship/libultraship.h>
#include <ship/resource/Resource.h>

extern "C" {
#include "structs.h"
}

namespace Factories {

// Helper to align offsets to 8-byte boundaries (N64 requirement)
inline size_t Align8(size_t offset) {
    return (offset + 7) & ~7;
}

// Store frame data with palette and texture chunks
struct SpriteFrameData {
    BKSpriteFrame frameHeader;
    std::vector<uint8_t> paletteData;

    struct ChunkData {
        BKSpriteTextureBlock header;
        std::vector<uint8_t> textureData;
        std::string resPath;
    };
    std::vector<ChunkData> chunks;
};

class Sprite : public Ship::Resource<BKSprite> {
public:
    using Resource::Resource;

    Sprite() : Resource(std::shared_ptr<Ship::ResourceInitData>()) {
    }

    ~Sprite() override;

    BKSprite* GetPointer();
    size_t GetPointerSize();

    // Build the sprite structure with direct pointers
    void BuildSpriteStructure();

    // Modern storage
    int16_t frameCount;
    int16_t formatType;
    int16_t headerUnk4;
    int16_t headerUnk6;
    int16_t headerUnk8; // Display width (billboard vertex positioning)
    int16_t headerUnkA; // Display height (billboard vertex positioning)
    // Animation parameters from ROM unkC bitfield
    uint8_t animSpeed = 0;     // bit31: 4 bits
    uint8_t animType = 0;      // bit27: 3 bits
    uint8_t animDirection = 0; // bit24: 2 bits
    uint8_t animFlip = 0;      // bit22: 2 bits
    std::vector<SpriteFrameData> frames;

private:
    std::unique_ptr<uint8_t[]> mSpriteHeader;
    std::vector<const void*> mRegisteredChunks;
};
} // namespace Factories
