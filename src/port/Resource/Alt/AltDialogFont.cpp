// Cross-revision HD dialog font.
//
// The dialog font changed between ROM revisions: US v1.0 has 62 glyphs at native 8x12; v1.1/PAL/JP
// extended it to 75 glyphs at 8x13 (the extra 13 are the PAL accents; every glyph also grew one row).
// A LOAD_AS_RAW alt bakes its h/v scale factors against the native dims of the ROM the pack was built
// from, so a v1.0-built pack's glyph carries a scale computed for 12 rows.
//
// The pixels are fine; only the baked scale is wrong, and it is wrong purely because native dims
// differ between revisions. Calculate and re-scale port-side so hd packs don't need per-revision art.

#include <libultraship.h>
#include <ship/Context.h>
#include <ship/resource/ResourceManager.h>
#include <fast/resource/type/Texture.h>

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

extern "C" void port_dialogFontHd_rebuild(void);

namespace {

constexpr const char* kFontBase = "assets/sprite/ASSET_6EB_DIALOG_FONT_ALPHAMASK";

bool sBuilt = false;

std::shared_ptr<Fast::Texture> loadTex(const std::string& path, bool loadExact) {
    return std::static_pointer_cast<Fast::Texture>(
        Ship::Context::GetRawInstance()->GetResourceManager()->LoadResourceProcess(path, loadExact));
}

// Build a corrected alt for one chunk and cache it under "alt/<basePath>". Returns false when there
// is nothing to do: no alt shipped, the dims already reconcile, or the alt isn't an HD raw texture.
bool correctChunk(const std::string& basePath) {
    auto base = loadTex(basePath, true); // live native cell
    if (base == nullptr || base->Width <= 0 || base->Height <= 0) {
        return false;
    }
    auto alt = loadTex(basePath, false); // HD art, or the base itself if the pack has no alt
    if (alt == nullptr || alt->ImageData == nullptr || alt.get() == base.get()) {
        return false;
    }
    if (alt->Type != Fast::TextureType::RGBA32bpp || (alt->Flags & TEX_FLAG_LOAD_AS_RAW) == 0) {
        return false;
    }

    const int nativeW = base->Width;
    const int nativeH = base->Height;

    // Width is revision-stable (drift is height-only), so the integer scale comes cleanly off it.
    // Bail if the alt isn't an integer multiple of the native width, or is degenerate.
    if (nativeW <= 0 || alt->Width % nativeW != 0) {
        return false;
    }
    const int k = alt->Width / nativeW;
    if (k < 1) {
        return false;
    }

    const int dstW = nativeW * k;
    const int dstH = nativeH * k;
    if (dstH == (int)alt->Height) {
        return false; // alt height already matches the live native cell -> no drift, nothing to fix
    }

    // Top-align the HD rows into the k-scaled native cell; the zero-filled remainder stays transparent.
    const int copyRows = (int)alt->Height < dstH ? (int)alt->Height : dstH;
    const int rowBytes = dstW * 4;

    auto pixels = std::make_shared<std::vector<char>>((size_t)dstW * dstH * 4, 0);
    for (int r = 0; r < copyRows; r++) {
        std::memcpy(pixels->data() + (size_t)r * rowBytes, alt->ImageData + (size_t)r * rowBytes, (size_t)rowBytes);
    }

    // Same HD pixels, but VPixelScale re-derived against the live native rows (native_rows * k == dstH).
    // Cache under the alt path so the resource layer returns this in place of the raw archive alt.
    const std::string altPath = std::string("alt/") + basePath;
    auto initData = std::make_shared<Ship::ResourceInitData>();
    initData->Path = altPath;
    auto tex = std::make_shared<Fast::Texture>(initData);
    tex->Type = Fast::TextureType::RGBA32bpp;
    tex->Width = (uint16_t)dstW;
    tex->Height = (uint16_t)dstH;
    tex->Flags = TEX_FLAG_LOAD_AS_RAW;
    tex->HByteScale = alt->HByteScale;
    tex->VPixelScale = (float)k;
    tex->ImageDataSize = (uint32_t)pixels->size();
    tex->mImageBuffer = pixels;
    tex->ImageData = (uint8_t*)pixels->data();

    Ship::Context::GetRawInstance()->GetResourceManager()->CacheExternalResource(altPath, tex);
    return true;
}

} // namespace

// Called when the dialog font is (re)loaded. Scans every font chunk once per boot and corrects each;
// idempotent, and a no-op when no alt pack is mounted (a later call may still succeed).
extern "C" void port_dialogFontHd_rebuild(void) {
    if (sBuilt) {
        return;
    }
    if (!Ship::Context::GetRawInstance()->GetResourceManager()->IsAltAssetsEnabled()) {
        return;
    }
    sBuilt = true;

    for (int frame = 0; frame < 64; frame++) {
        bool anyInFrame = false;
        for (int chunk = 0; chunk < 256; chunk++) {
            const std::string basePath =
                std::string(kFontBase) + "_" + std::to_string(frame) + "_" + std::to_string(chunk);
            if (loadTex(basePath, true) == nullptr) {
                break;
            }
            anyInFrame = true;
            correctChunk(basePath);
        }
        if (!anyInFrame) {
            break;
        }
    }
}
