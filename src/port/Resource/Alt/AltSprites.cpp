// HD sprite support.
//
// BK sprites bake their texture pixels inline (the CPU software rasterizers in
// framebufferdraw.c, the font system, and the JP banner fill all read those bytes
// directly), so they never flow through the gfx interpreter's resource path that
// gives models/levels their HD replacements. SpriteFactory always inlines the
// BASE texture (loadExact) so those consumers stay correct, and records each
// non-split chunk's "__OTR__<path>" resource path here, keyed by the chunk block's
// address in the built BKSprite buffer.
//
// Every GPU sprite render path resolves a chunk via a BKSpriteTextureBlock* and
// computes the inline pixel pointer as align(chunk + 1). Just before the load those
// paths fire ResolveSpriteHdPath with the chunk block address; the listener below
// returns the chunk's "__OTR__<path>" (when alternate assets are enabled) and the
// decomp hands it to the texture-load command instead of the inline pointer. The
// interpreter alt-redirects to alt/<path> and applies HD dims/scale exactly like a
// model texture. With alt assets off, the path stays null.

#include <libultraship.h>
#include <ship/Context.h>
#include <ship/resource/ResourceManager.h>
#include <ship/resource/archive/ArchiveManager.h>

#include "AltSprites.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Resource/Alt/AltPathPool.h"
#include "port/ShipInit.hpp"

#include <mutex>
#include <string>
#include <unordered_map>

namespace {
std::unordered_map<const void*, const char*> sChunkPaths;
std::mutex sMutex;

thread_local int sColorVariant = -1;
const char* const kJinjoColors[] = { "BLUE", "GREEN", "ORANGE", "PURPLE", "YELLOW" };
std::unordered_map<std::string, const char*> sVariantCache;

const char* resolveColorVariant(const char* base, int variant) {
    if (variant < 0 || variant >= (int)(sizeof(kJinjoColors) / sizeof(kJinjoColors[0]))) {
        return nullptr;
    }
    std::string candidate = std::string(base) + "_" + kJinjoColors[variant];
    auto it = sVariantCache.find(candidate);
    if (it != sVariantCache.end()) {
        return it->second;
    }

    const char* result = nullptr;
    if (candidate.rfind("__OTR__", 0) == 0) {
        std::string altFile = "alt/" + candidate.substr(7);
        if (Ship::Context::GetRawInstance()->GetResourceManager()->GetArchiveManager()->HasFile(altFile)) {
            result = InternAltPath(candidate);
        }
    }
    sVariantCache.emplace(std::move(candidate), result);
    return result;
}

std::unordered_map<const char*, bool> sAltPresentCache;

bool altFilePresent(const char* path) {
    auto it = sAltPresentCache.find(path);
    if (it != sAltPresentCache.end()) {
        return it->second;
    }
    bool present = false;
    if (std::string(path).rfind("__OTR__", 0) == 0) {
        present = Ship::Context::GetRawInstance()->GetResourceManager()->GetArchiveManager()->HasFile(
            "alt/" + std::string(path + 7));
    }
    sAltPresentCache.emplace(path, present);
    return present;
}

const char* resolvePath(const void* chunkAddr) {
    const int variant = sColorVariant;
    sColorVariant = -1;

    if (chunkAddr == nullptr) {
        return nullptr;
    }
    if (!Ship::Context::GetRawInstance()->GetResourceManager()->IsAltAssetsEnabled()) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(sMutex);
    auto it = sChunkPaths.find(chunkAddr);
    if (it == sChunkPaths.end()) {
        return nullptr;
    }
    if (variant >= 0) {
        const char* colored = resolveColorVariant(it->second, variant);
        if (colored != nullptr) {
            return colored;
        }
    }
    return altFilePresent(it->second) ? it->second : nullptr;
}
} // namespace

extern "C" {

void port_spriteAltRegisterChunk(const void* chunkAddr, const char* path) {
    if (chunkAddr == nullptr || path == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(sMutex);
    sChunkPaths[chunkAddr] = InternAltPath(path);
}

void port_spriteAltUnregisterChunk(const void* chunkAddr) {
    if (chunkAddr == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(sMutex);
    sChunkPaths.erase(chunkAddr);
}

} // extern "C"

namespace BkSpriteAlt {
const char* RegisteredChunkPath(const void* chunkAddr) {
    if (chunkAddr == nullptr) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(sMutex);
    auto it = sChunkPaths.find(chunkAddr);
    return it != sChunkPaths.end() ? it->second : nullptr;
}
} // namespace BkSpriteAlt

static void RegisterSpriteAltAssets() {
    REGISTER_LISTENER(ResolveSpriteHdPath, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (ResolveSpriteHdPath*)event;
        *ev->path = resolvePath(ev->chunkAddr);
    });

    REGISTER_LISTENER(OnJinjoHeadDraw, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (OnJinjoHeadDraw*)event;
        sColorVariant = ev->jinjoId;
    });
}

static RegisterShipInitFunc spriteAltAssetsInit(RegisterSpriteAltAssets, {});
