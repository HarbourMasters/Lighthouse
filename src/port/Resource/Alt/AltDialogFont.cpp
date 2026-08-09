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
#include "port/GameVersion/BaseGameVersion.h"
#include "port/Patches/Patches.h"
#include "port/ResourceHelpers.h"
#include "port/ShipInit.hpp"

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

constexpr uint32_t kDialogFontAssetId = 0x6EB;
constexpr const char* kFontBaseFallback = "assets/sprite/ASSET_6EB_DIALOG_FONT_ALPHAMASK";
std::unordered_set<std::string> sBuiltSheets;
std::string fontSheetPath(std::string resolved) {
    return resolved.empty() ? kFontBaseFallback : resolved;
}

// The same sheet under the base cartridge's language folder. HD packs file the glyphs a region
// adds (the PAL accents, the JP kana) under the language that introduces them, matching how a
// language pack lays its own assets out. Empty when the sheet isn't under "assets/".
std::string langSheetPath(const std::string& sheet) {
    const std::string prefix = "assets/";
    if (sheet.rfind(prefix, 0) != 0) {
        return std::string();
    }
    return prefix + "lang/" + Lighthouse::BaseRegionSlug() + "/" + sheet.substr(prefix.size());
}

std::shared_ptr<Fast::Texture> loadTex(const std::string& path, bool loadExact) {
    return std::static_pointer_cast<Fast::Texture>(
        Ship::Context::GetRawInstance()->GetResourceManager()->LoadResourceProcess(path, loadExact));
}

// HD art for one chunk: the active sheet's own alt when the pack ships one, else the base game's
// alt for the same slot. Null when neither has art for it.
std::shared_ptr<Fast::Texture> loadHdChunk(const std::string& activePath, const std::string& basePath,
                                           const Fast::Texture* nativeCell, bool& fromOtherSheet) {
    fromOtherSheet = false;
    auto alt = loadTex(activePath, false);
    if (alt != nullptr && alt.get() != nativeCell) {
        return alt;
    }
    if (basePath != activePath) {
        fromOtherSheet = true;
        return loadTex(Ship::IResource::gAltAssetPrefix + basePath, true);
    }
    const std::string langPath = langSheetPath(activePath);
    if (langPath.empty() || langPath == activePath) {
        return nullptr;
    }
    fromOtherSheet = true;
    return loadTex(Ship::IResource::gAltAssetPrefix + langPath, true);
}

// Build a corrected alt for one chunk and cache it under "alt/<activePath>". Returns false when
// there is nothing to do: no alt shipped, the dims already reconcile, or it isn't an HD raw texture.
bool correctChunk(const std::string& activePath, const std::string& basePath) {
    auto base = loadTex(activePath, true); // live native cell
    if (base == nullptr || base->Width <= 0 || base->Height <= 0) {
        return false;
    }
    bool fromOtherSheet = false;
    auto alt = loadHdChunk(activePath, basePath, base.get(), fromOtherSheet);
    if (alt == nullptr || alt->ImageData == nullptr) {
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
    // Matching heights mean no drift to correct. Art borrowed from another sheet still has to be
    // re-cached, though: nothing is reachable at the active path until we put it there.
    if (dstH == (int)alt->Height && !fromOtherSheet) {
        return false;
    }

    // The extra row v1.1/PAL/JP added sits at the top of the cell
    const int altH = (int)alt->Height;
    const int dstRow0 = dstH > altH ? dstH - altH : 0;
    const int srcRow0 = altH > dstH ? altH - dstH : 0;
    const int copyRows = altH < dstH ? altH : dstH;
    const int rowBytes = dstW * 4;

    auto pixels = std::make_shared<std::vector<char>>((size_t)dstW * dstH * 4, 0);
    for (int r = 0; r < copyRows; r++) {
        std::memcpy(pixels->data() + (size_t)(dstRow0 + r) * rowBytes,
                    alt->ImageData + (size_t)(srcRow0 + r) * rowBytes, (size_t)rowBytes);
    }

    // Same HD pixels, but VPixelScale re-derived against the live native rows (native_rows * k == dstH).
    // Cache under the alt path so the resource layer returns this in place of the raw archive alt.
    const std::string altPath = Ship::IResource::gAltAssetPrefix + activePath;
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

// Called when the dialog font is (re)loaded, boot and language switch alike. Scans every chunk of
// the sheet now in play and corrects each.
extern "C" void port_dialogFontHd_rebuild(void) {
    if (!Ship::Context::GetRawInstance()->GetResourceManager()->IsAltAssetsEnabled()) {
        return;
    }

    const std::string activeFont = fontSheetPath(ResourceHelpers_GetActiveAssetPath(kDialogFontAssetId));
    if (sBuiltSheets.count(activeFont) != 0) {
        return;
    }
    const std::string baseFont = fontSheetPath(ResourceHelpers_GetBaseAssetPath(kDialogFontAssetId));

    bool built = false;
    for (int frame = 0; frame < 64; frame++) {
        bool anyInFrame = false;
        for (int chunk = 0; chunk < 256; chunk++) {
            const std::string suffix = "_" + std::to_string(frame) + "_" + std::to_string(chunk);
            const std::string activePath = activeFont + suffix;
            if (loadTex(activePath, true) == nullptr) {
                break;
            }
            anyInFrame = true;
            built = true;
            correctChunk(activePath, baseFont + suffix);
        }
        if (!anyInFrame) {
            break;
        }
    }

    if (built) {
        sBuiltSheets.insert(activeFont);
    }
}
