// HD bold-font compositing.
//
// The big colored map/level names use a "bold font" that is not a stored texture: each letter is
// computed at runtime as (sphere * alpha-mask): print_applyTextureToBoldFontLetter multiplies a
// per-map sphere texture by an alpha-mask glyph into a fresh native-size RGBA32 buffer, and that
// product is what gets drawn. Because the product is synthesized, it never flows through the normal
// alt-asset path, so even with an HD pack mounted the bold font stays native-res.
//
// The sphere and the alpha mask, however, are ordinary sprite assets with HD alts. When the decomp
// finishes a base letter it fires OnBoldFontLetterBuilt; we re-run the exact same composite using the
// HD sphere + HD mask (loaded from the alt archive) into a higher-res buffer, wrap it in a runtime
// Fast::Texture (LOAD_AS_RAW + HD scale) cached under a synthetic "__OTR__" path, and key it by the
// base output chunk. At draw time the decomp fires ResolveBoldFontHd and we hand back that path; the
// glyph then loads through the exact same code path the (working) HD dialog font uses — base-size
// gDPLoadTextureTile + the interpreter's scale-aware LOAD_AS_RAW upload — so the letter keeps its
// on-screen size with more detail. With alt assets off, or when the pack ships no HD sphere/mask,
// nothing is cached and the draw uses the native composite exactly as before.

#include <libultraship.h>
#include <ship/Context.h>
#include <ship/resource/ResourceManager.h>
#include <ship/resource/archive/ArchiveManager.h>
#include <ship/resource/File.h>
#include <fast/resource/type/Texture.h>

#include "AltPathPool.h"
#include "AltSprites.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

struct ChunkHeader {
    int16_t x, y, w, h;
};

// resolveHd hands these strings' addresses into display lists, and texture
// binds may be memoized on that address downstream: the strings must be
// immutable and immortal, so they live in the shared alt-path pool.
// reset() clears the output mapping, never the pool.
std::unordered_map<const void*, const char*> sByOutput;
std::unordered_set<std::string> sBuilt;
std::mutex sMutex;

const char* dropSignature(const char* p) {
    return std::string(p).rfind("__OTR__", 0) == 0 ? p + 7 : p;
}

// Build a cache path unique to (mask glyph, sphere) from their resource paths. To read like the other
// alt assets, it lives under assets/boldfont/ and keeps the mask's full asset basename (which carries
// the glyph index, e.g. ..._0_0) tagged with the sphere's hex asset id:
// assets/boldfont/ASSET_6EC_BOLD_FONT_LETTERS_ALPHAMASK_0_0_6E4
// The offline pack tooling must emit alt/ entries at byte-identical paths or the lookup misses.
std::string variantPath(const char* maskOtr, const char* sphereOtr) {
    // Mask: the asset basename
    std::string mask = dropSignature(maskOtr);
    const size_t slash = mask.find_last_of("/\\");
    if (slash != std::string::npos) {
        mask = mask.substr(slash + 1);
    }
    // Sphere: just the hex id
    const std::string sphere = dropSignature(sphereOtr);
    std::string hex;
    const size_t a = sphere.find("ASSET_");
    if (a != std::string::npos) {
        const size_t start = a + 6;
        const size_t end = sphere.find('_', start);
        hex = sphere.substr(start, end == std::string::npos ? std::string::npos : end - start);
    }
    return "assets/boldfont/" + mask + "_" + hex;
}

std::shared_ptr<Fast::Texture> loadTexture(const char* otrPath) {
    if (otrPath == nullptr) {
        return nullptr;
    }
    // loadExact = false so the alt version is returned when alternate assets are enabled and the
    // pack ships one; otherwise this resolves to the base asset (scale 1), which we then reject below.
    return std::static_pointer_cast<Fast::Texture>(
        Ship::Context::GetRawInstance()->GetResourceManager()->LoadResourceProcess(otrPath, false));
}

int integerScale(const std::shared_ptr<Fast::Texture>& tex, int baseH) {
    if (tex == nullptr || tex->ImageData == nullptr || baseH <= 0) {
        return 0;
    }
    if (tex->Height % baseH != 0) {
        return 0;
    }
    const int s = tex->Height / baseH;
    return s > 1 ? s : 0;
}

void readSphere5(const Fast::Texture& sphere, int x, int y, int& r5, int& g5, int& b5) {
    if (x < 0) {
        x = 0;
    } else if (x >= sphere.Width) {
        x = sphere.Width - 1;
    }
    if (y < 0) {
        y = 0;
    } else if (y >= sphere.Height) {
        y = sphere.Height - 1;
    }
    const size_t idx = (size_t)y * sphere.Width + x;
    if (sphere.Type == Fast::TextureType::RGBA16bpp) {
        const uint8_t* p = sphere.ImageData + idx * 2;
        const uint16_t pixel = (uint16_t)((p[0] << 8) | p[1]);
        r5 = (pixel >> 11) & 0x1F;
        g5 = (pixel >> 6) & 0x1F;
        b5 = (pixel >> 1) & 0x1F;
    } else {
        const uint8_t* p = sphere.ImageData + idx * 4;
        r5 = p[0] >> 3;
        g5 = p[1] >> 3;
        b5 = p[2] >> 3;
    }
}

void buildHd(const void* output, const void* maskChunk, const void* sphereChunk) {
    if (output == nullptr || maskChunk == nullptr || sphereChunk == nullptr ||
        !Ship::Context::GetRawInstance()->GetResourceManager()->IsAltAssetsEnabled()) {
        return;
    }

    const char* maskOtr = BkSpriteAlt::RegisteredChunkPath(maskChunk);
    const char* sphereOtr = BkSpriteAlt::RegisteredChunkPath(sphereChunk);
    if (maskOtr == nullptr || sphereOtr == nullptr) {
        return;
    }
    const std::string cachePath = variantPath(maskOtr, sphereOtr);

    std::unique_lock<std::mutex> lock(sMutex);
    if (sBuilt.count(cachePath)) {
        sByOutput[output] = InternAltPath("__OTR__" + cachePath);
        return;
    }

    // The pack ships a pre-textured HD output for this exact (glyph, sphere) at
    // the identity path. Serve it directly and skip compositing.
    if (Ship::Context::GetRawInstance()->GetResourceManager()->GetArchiveManager()->HasFile("alt/" + cachePath)) {
        sBuilt.insert(cachePath);
        sByOutput[output] = InternAltPath("__OTR__" + cachePath);
        return;
    }
    lock.unlock();

    // No authored output, so composite from HD sphere + HD mask if the pack
    // provides them.
    const auto* maskHdr = (const ChunkHeader*)maskChunk;
    const auto* sphereHdr = (const ChunkHeader*)sphereChunk;

    auto maskTex = loadTexture(maskOtr);
    auto sphereTex = loadTexture(sphereOtr);

    const int maskScale = integerScale(maskTex, maskHdr->h);
    const int sphereScale = integerScale(sphereTex, sphereHdr->h);

    if (maskScale == 0 || maskScale != sphereScale) {
        return;
    }

    const int dstW = maskHdr->w * maskScale;
    const int dstH = maskHdr->h * maskScale;
    const int srcW = maskTex->Width, srcH = maskTex->Height;
    const int sphereW = sphereTex->Width, sphereH = sphereTex->Height;
    const int xMin = (sphereW - dstW) >> 1;
    const int yMin = (sphereH - dstH) >> 1;

    auto pixels = std::make_shared<std::vector<char>>((size_t)dstW * dstH * 4);
    auto* out = (uint8_t*)pixels->data();

    for (int my = 0; my < dstH; my++) {
        for (int mx = 0; mx < dstW; mx++) {
            const size_t oi = (size_t)my * dstW + mx;
            int intensity = 0, alpha = 0;
            if (mx < srcW && my < srcH) {
                const uint8_t* maskPx = maskTex->ImageData + ((size_t)my * srcW + mx) * 4;
                intensity = maskPx[2];
                alpha = maskPx[3];
            }

            int r5, g5, b5;
            readSphere5(*sphereTex, xMin + mx, yMin + my, r5, g5, b5);

            const int s = intensity / 0x1F;
            out[oi * 4 + 0] = (uint8_t)(r5 * s);
            out[oi * 4 + 1] = (uint8_t)(g5 * s);
            out[oi * 4 + 2] = (uint8_t)(b5 * s);
            out[oi * 4 + 3] = (uint8_t)alpha;
        }
    }

    auto initData = std::make_shared<Ship::ResourceInitData>();
    initData->Path = cachePath;
    auto tex = std::make_shared<Fast::Texture>(initData);
    tex->Type = Fast::TextureType::RGBA32bpp;
    tex->Width = (uint16_t)dstW;
    tex->Height = (uint16_t)dstH;
    tex->Flags = TEX_FLAG_LOAD_AS_RAW;
    tex->HByteScale = (float)maskScale;
    tex->VPixelScale = (float)maskScale;
    tex->ImageDataSize = (uint32_t)pixels->size();
    tex->mImageBuffer = pixels;
    tex->ImageData = (uint8_t*)pixels->data();

    Ship::Context::GetRawInstance()->GetResourceManager()->CacheExternalResource(cachePath, tex);

    lock.lock();
    sBuilt.insert(cachePath);
    sByOutput[output] = InternAltPath("__OTR__" + cachePath);
}

void resolveHd(const void* output, const char** path) {
    if (!Ship::Context::GetRawInstance()->GetResourceManager()->IsAltAssetsEnabled()) {
        return;
    }
    std::lock_guard<std::mutex> lock(sMutex);
    auto it = sByOutput.find(output);
    if (it != sByOutput.end()) {
        *path = it->second;
    }
}

void reset() {
    std::lock_guard<std::mutex> lock(sMutex);
    sByOutput.clear();
}

} // namespace

static void RegisterBoldFontHd() {
    REGISTER_LISTENER(OnBoldFontLetterBuilt, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (OnBoldFontLetterBuilt*)event;
        buildHd(ev->output, ev->maskChunk, ev->sphereChunk);
    });

    REGISTER_LISTENER(ResolveBoldFontHd, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (ResolveBoldFontHd*)event;
        resolveHd(ev->output, ev->path);
    });

    REGISTER_LISTENER(OnBoldFontReset, EVENT_PRIORITY_NORMAL, [](IEvent*) { reset(); });
}

static RegisterShipInitFunc boldFontHdInit(RegisterBoldFontHd, {});
