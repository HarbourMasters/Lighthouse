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

const char* resolvePath(const void* chunkAddr) {
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
    return it->second;
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
        auto* ev = (ResolveSpriteHdPath*) event;
        *ev->path = resolvePath(ev->chunkAddr);
    });
}

static RegisterShipInitFunc spriteAltAssetsInit(RegisterSpriteAltAssets, {});
