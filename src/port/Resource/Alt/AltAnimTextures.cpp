#include <libultraship.h>
#include <ship/Context.h>
#include <ship/resource/ResourceManager.h>
#include <ship/resource/archive/ArchiveManager.h>

#include "AltAnimTextures.h"
#include "AltPathPool.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#include <mutex>
#include <string>
#include <unordered_map>

namespace {

struct Model {
    std::string path;
    BkAnimTextureAlt::Slot slots[4];
};

std::unordered_map<const void*, Model> sModels;
std::unordered_map<std::string, bool> sAltPresent;
std::mutex sMutex;

bool altPresent(const std::string& path) {
    auto it = sAltPresent.find(path);
    if (it != sAltPresent.end()) {
        return it->second;
    }
    const bool present =
        Ship::Context::GetRawInstance()->GetResourceManager()->GetArchiveManager()->HasFile("alt/" + path);
    sAltPresent.emplace(path, present);
    return present;
}

void resolve(const void* textures, int32_t slot, int32_t frameOffset, uintptr_t* segment) {
    if (slot < 0 || slot >= 4 || !Ship::Context::GetRawInstance()->GetResourceManager()->IsAltAssetsEnabled()) {
        return;
    }
    std::lock_guard<std::mutex> lock(sMutex);
    auto it = sModels.find(textures);
    if (it == sModels.end()) {
        return;
    }
    const auto& s = it->second.slots[slot];
    if (s.texIndex < 0 || s.frameSize == 0) {
        return;
    }
    const uint32_t frame = (uint32_t)frameOffset / s.frameSize;
    std::string path = it->second.path + "_tex_" + std::to_string(s.texIndex);
    if (frame > 0) {
        path += "_frame_" + std::to_string(frame);
    }
    if (!altPresent(path)) {
        return;
    }
    *segment = (uintptr_t)InternAltPath("__OTR__" + path) - s.pixelOffset;
}

} // namespace

namespace BkAnimTextureAlt {

void Register(const void* textures, const std::string& modelPath, const Slot (&slots)[4]) {
    std::lock_guard<std::mutex> lock(sMutex);
    auto& model = sModels[textures];
    model.path = modelPath;
    for (int i = 0; i < 4; i++) {
        model.slots[i] = slots[i];
    }
}

} // namespace BkAnimTextureAlt

static void RegisterAnimTextureAltAssets() {
    REGISTER_LISTENER(ResolveAnimTextureHd, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (ResolveAnimTextureHd*)event;
        resolve(ev->textures, ev->slot, ev->frameOffset, ev->segment);
    });
}

static RegisterShipInitFunc animTextureAltAssetsInit(RegisterAnimTextureAltAssets, {});
