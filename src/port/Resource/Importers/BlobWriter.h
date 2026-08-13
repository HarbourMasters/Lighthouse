#pragma once

#include <cstring>
#include <memory>
#include <vector>

#include <libultraship/libultraship.h>
#include <ship/resource/type/Blob.h>

namespace Factories {

template <typename T> void AppendValue(std::vector<uint8_t>& dst, const T& value) {
    const auto base = dst.size();
    dst.resize(base + sizeof(T));
    std::memcpy(dst.data() + base, &value, sizeof(T));
}

inline void AppendBytes(std::vector<uint8_t>& dst, const void* data, size_t size) {
    const auto base = dst.size();
    dst.resize(base + size);
    std::memcpy(dst.data() + base, data, size);
}

inline std::shared_ptr<Ship::Blob> MakeBlob(const std::shared_ptr<Ship::ResourceInitData>& initData,
                                            std::vector<uint8_t> data) {
    auto blob = std::make_shared<Ship::Blob>(initData);
    blob->Data = std::move(data);
    return blob;
}

} // namespace Factories
