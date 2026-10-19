#pragma once

#include <cstdint>
#include <string>

namespace BkAnimTextureAlt {
constexpr uint32_t kHdSegmentBase = 0x07;
struct Slot {
    int32_t texIndex = -1;
    uint32_t pixelOffset = 0;
    uint32_t frameSize = 0;
};
void Register(const void* textures, const std::string& modelPath, const Slot (&slots)[4]);
} // namespace BkAnimTextureAlt
