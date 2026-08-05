
#include "ship/resource/ResourceManager.h"
#include "ship/resource/archive/Archive.h"
#include "ship/resource/archive/ArchiveManager.h"
#include "ship/resource/File.h"
#include "ship/resource/type/Blob.h"
#include "fast/resource/ResourceType.h"
#include "fast/resource/type/DisplayList.h"
#include "libultraship/bridge/resourcebridge.h"
#include "libultraship/libultraship.h"
#include "ship/Context.h"
#include "UI/cvar_prefixes.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#if defined(_DEBUG) && defined(_MSC_VER)
#include <crtdbg.h>
#endif

#include "Resource/AssetTrace.h"

#include "GameVersion/AssetVersionRemap.h"
#include "GameVersion/BaseGameVersion.h"

#include "enums.h"

extern "C" void func_8031B5C4(int32_t lang); // decomp: set dialog language index

extern "C" {
// SNS picture demo table (core2/map/exit.c, Struct_core2_C4320_0). unk2 is both
// the demo file slot and the map exit/entry-point id used for the warp.
typedef struct {
    uint8_t map, unk1, exit, unk3, unk4, unk5;
} BKSnsDemoEntry;
extern BKSnsDemoEntry D_80371F78[3];
}

// Dialog language state — detected at boot from o2r version
static int sDialogLanguageCount = 1; // 1 for US/JP, 3 for PAL (EN/FR/DE)
static int sDialogLanguage = 0;      // 0=English, 1=French, 2=German
static bool sIsJapanese = false;     // true when a JP o2r is loaded

namespace {
std::unordered_set<uint32_t> sEmptyAssetSlots;

const std::unordered_map<uint32_t, std::string>& GetAssetSymbolMap() {
    static std::once_flag mapOnce;
    static std::unordered_map<uint32_t, std::string> symbolMap;

    std::call_once(mapOnce, [] {
        // Torch writes this as a Blob at "assets/aBKAssetTable".
        // Format: u32 count, then for each entry: u32 assetId, s32 pathLen, char path[pathLen]
        auto res = Ship::Context::GetRawInstance()->GetResourceManager()->LoadResource("assets/aBKAssetTable");
        if (!res) {
            SPDLOG_ERROR("Failed to load asset manifest from o2r");
            return;
        }

        auto blob = std::dynamic_pointer_cast<Ship::Blob>(res);
        if (!blob || blob->Data.empty()) {
            SPDLOG_ERROR("Asset manifest blob is empty or wrong type");
            return;
        }

        const uint8_t* data = blob->Data.data();
        const size_t dataSize = blob->Data.size();
        size_t pos = 0;

        if (dataSize < 4) {
            SPDLOG_ERROR("Asset manifest too small");
            return;
        }

        uint32_t count;
        std::memcpy(&count, data + pos, 4);
        pos += 4;

        for (uint32_t i = 0; i < count && pos + 8 <= dataSize; i++) {
            uint32_t assetId;
            std::memcpy(&assetId, data + pos, 4);
            pos += 4;

            int32_t pathLen;
            std::memcpy(&pathLen, data + pos, 4);
            pos += 4;

            if (pathLen < 0 || pos + static_cast<size_t>(pathLen) > dataSize) {
                SPDLOG_ERROR("Asset manifest corrupt at entry {}", i);
                break;
            }

            std::string path(reinterpret_cast<const char*>(data + pos), static_cast<size_t>(pathLen));
            pos += static_cast<size_t>(pathLen);

            // pathLen == 0 marks an intentionally-empty ROM slot.
            if (path.empty()) {
                sEmptyAssetSlots.insert(assetId);
            } else {
                symbolMap[assetId] = std::move(path);
            }
        }

        // If this o2r was built from a non-v1.0 ROM, inject v1.0 ID aliases so
        // the decomp's hardcoded IDs resolve transparently.
        const std::unordered_map<uint32_t, uint32_t>* remapTable = nullptr;

        switch (Lighthouse::GetBaseVersion()) {
            case BK_VER_US_11:
                remapTable = &sV10toV11Remap;
                SPDLOG_INFO("Loaded US v1.1 o2r with {} entries", symbolMap.size());
                break;
            case BK_VER_PAL:
                remapTable = &sV10toPALRemap;
                SPDLOG_INFO("Loaded PAL o2r with {} entries", symbolMap.size());
                break;
            case BK_VER_JP:
                remapTable = &sV10toJPRemap;
                SPDLOG_INFO("Loaded JP o2r with {} entries", symbolMap.size());
                break;
            case BK_VER_US_10:
            default:
                SPDLOG_INFO("Loaded US v1.0 o2r");
                // v1.0 or a v1.0-based romhack: decomp IDs are already correct.
                break;
        }

        // Non-v1.0 ROMs keep the SNS picture demos one slot lower (0x5E, matching
        // the PAL VER_SELECT in the decomp) and their map setups place the demo
        // entry points at that exit id too. The compiled v1.0 value (0x5F) would
        // load the right demo file via the alias but warp to a nonexistent exit,
        // so retarget the table; the 2508/2511/2513 aliases cover the asset side.
        if (remapTable) {
            for (auto& entry : D_80371F78) {
                entry.exit = 0x5E;
            }
        }

        // Relocate the JP world-name banners to their 0x1600 aliases for a JP
        // base (a JP pack does this re-point at language-select time instead).
        if (Lighthouse::GetBaseVersion() == BK_VER_JP) {
            constexpr uint32_t kJpBannerNativeBase = 0xE2C;
            constexpr uint32_t kJpBannerAliasBase = 0x1600; // SPRITE_WORLD_NAME_TOTAL
            constexpr uint32_t kJpBannerCount = 13;         // 0xE2C..0xE38
            for (uint32_t i = 0; i < kJpBannerCount; i++) {
                auto it = symbolMap.find(kJpBannerNativeBase + i);
                if (it != symbolMap.end()) {
                    std::string path = std::move(it->second);
                    symbolMap.erase(kJpBannerNativeBase + i);
                    symbolMap[kJpBannerAliasBase + i] = std::move(path);
                }
            }
        }

        if (remapTable) {
            // Snapshot the manifest before injection
            const auto snapshot = symbolMap;
            for (const auto& [v10Id, targetId] : *remapTable) {
                if (auto targetEntry = snapshot.find(targetId); targetEntry != snapshot.end()) {
                    symbolMap[v10Id] = targetEntry->second;
                }
                // An empty slot on the native id stays empty under its v1.0 alias,
                // so the decomp's hardcoded v1.0 id resolves to a silent NULL too.
                if (sEmptyAssetSlots.count(targetId)) {
                    sEmptyAssetSlots.insert(v10Id);
                }
            }
        }
    });

    return symbolMap;
}

// True when assetId names a ROM slot that exists in the asset table but was never
// backed with data. Such a lookup is expected to return NULL (the game tolerates
// it, exactly as on N64) and must not be logged as a bad asset.
bool IsKnownEmptyAssetSlot(uint32_t assetId) {
    GetAssetSymbolMap(); // ensure the manifest (and empty-slot set) is parsed
    return sEmptyAssetSlots.count(assetId) != 0;
}
} // namespace

extern "C" int ResourceMgr_GetDialogLanguageCount(void) {
    return sDialogLanguageCount;
}

// PAL is the only base that carries more than one dialog language (EN/FR/DE).
extern "C" int ResourceMgr_IsPal(void) {
    return sDialogLanguageCount > 1 ? 1 : 0;
}

extern "C" int ResourceMgr_IsJapanese(void) {
    return sIsJapanese ? 1 : 0;
}

extern "C" int ResourceMgr_GetDialogLanguage(void) {
    return sDialogLanguage;
}

extern "C" void ResourceMgr_SetDialogLanguage(int lang) {
    if (lang >= 0 && lang < sDialogLanguageCount) {
        sDialogLanguage = lang;
        func_8031B5C4(lang); // Set decomp's internal language index
        SPDLOG_INFO("[ResourceHelpers] Dialog language set to {}", lang);
    }
}

std::shared_ptr<Ship::IResource> GetResourceByName(const char* path) {
    try {
        return Ship::Context::GetRawInstance()->GetResourceManager()->LoadResource(path);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[ResourceHelpers] GetResourceByName('{}') exception: {}", path, e.what());
        return nullptr;
    }
}

// Keep shared_ptr references alive so raw pointers from GetResourceRawPointer
// don't become dangling when the resource manager evicts entries from its cache.
static std::unordered_map<uint32_t, std::shared_ptr<Ship::IResource>> sResourceRefCache;

// Active dialog re-point: maps base (v1.0) dialog asset IDs to an alternate
// source archive's paths (a loaded language pack).
static std::unordered_map<uint32_t, std::string> sDialogOverride;

// Bumped on every language change. This is the single "language changed" signal:
// the localization layer reads it to drive every live reaction: font slot, model
// re-fetch, file-select rebuild, etc
static int sLanguageGeneration = 0;

extern "C" int ResourceMgr_GetLanguageGeneration(void) {
    return sLanguageGeneration;
}

// 1 if the active language re-points this asset id (i.e. it resolves to a language
// pack rather than the base game). Used by the model reload to know which models
// must track the language.
extern "C" int ResourceMgr_IsAssetRepointed(uint32_t assetId) {
    return sDialogOverride.find(assetId) != sDialogOverride.end() ? 1 : 0;
}

// The base game's own o2r path for an asset id, ignoring any active language re-point.
std::string ResourceHelpers_GetBaseAssetPath(uint32_t assetId) {
    const auto& symbolMap = GetAssetSymbolMap();
    if (auto entry = symbolMap.find(assetId); entry != symbolMap.end()) {
        return entry->second;
    }
    return std::string();
}

// The o2r path an asset id resolves to right now.
std::string ResourceHelpers_GetActiveAssetPath(uint32_t assetId) {
    if (auto ov = sDialogOverride.find(assetId); ov != sDialogOverride.end()) {
        return ov->second;
    }
    return ResourceHelpers_GetBaseAssetPath(assetId);
}

static char* LoadAndRetainResource(const std::string& path, uint32_t assetId) {
    auto res = GetResourceByName(path.c_str());
    if (res && res->GetRawPointer() != nullptr) {
        sResourceRefCache[assetId] = res;
        return reinterpret_cast<char*>(res->GetRawPointer());
    }
    return nullptr;
}

// Reload an asset, evicting any cached version first.
// Used for map models whose vertex data gets modified at runtime.
extern "C" char* ResourceMgr_ReloadByAssetId(uint32_t assetId) {
    std::shared_ptr<Ship::IResource> oldRef;
    if (auto it = sResourceRefCache.find(assetId); it != sResourceRefCache.end()) {
        oldRef = std::move(it->second);
        sResourceRefCache.erase(it);
    }

    std::string mappedPath = ResourceHelpers_GetActiveAssetPath(assetId);
    if (!mappedPath.empty()) {
        std::replace(mappedPath.begin(), mappedPath.end(), '\\', '/');

        // Unload from LUS cache so it re-reads from the o2r
        Ship::Context::GetRawInstance()->GetResourceManager()->UnloadResource(mappedPath);

        if (auto result = LoadAndRetainResource(mappedPath, assetId); result != nullptr) {
            return result;
        }
    }

    return nullptr;
}

extern "C" char* ResourceMgr_LoadByAssetId(uint32_t assetId) {
    // Return cached resource if already loaded
    if (auto it = sResourceRefCache.find(assetId); it != sResourceRefCache.end()) {
        auto ptr = it->second->GetRawPointer();
        if (ptr != nullptr) {
            return reinterpret_cast<char*>(ptr);
        }
        sResourceRefCache.erase(it);
    }

    std::string mappedPath = ResourceHelpers_GetActiveAssetPath(assetId);
    if (mappedPath.empty()) {
        // A reserved-but-empty ROM slot resolves to nothing by design (the game
        // tolerates it, as on N64); only a genuinely-missing asset is worth tracing.
        if (!IsKnownEmptyAssetSlot(assetId)) {
            port_traceBadAssetId(assetId);
        }
        return nullptr;
    }
    std::replace(mappedPath.begin(), mappedPath.end(), '\\', '/');

    if (auto result = LoadAndRetainResource(mappedPath, assetId); result != nullptr) {
        return result;
    }
    SPDLOG_WARN("[ResourceManager({})] symbol '{}' found but resource data is NULL", assetId, mappedPath);
    return nullptr;
}

// Returns the data size of a previously loaded resource (from the ref cache).
// Used by decomp code that depends on assetCacheCurrentSize (e.g. demo_load).
extern "C" size_t ResourceMgr_GetResourceSize(uint32_t assetId) {
    if (auto it = sResourceRefCache.find(assetId); it != sResourceRefCache.end()) {
        return it->second->GetPointerSize();
    }
    return 0;
}

// Force an asset id to resolve to a custom resource shipped at `customPath`.
extern "C" void ResourceMgr_RegisterAssetOverride(uint32_t assetId, const char* customPath) {
    auto res = GetResourceByName(customPath);
    if (res != nullptr && res->GetRawPointer() != nullptr) {
        sResourceRefCache[assetId] = res;
    }
}

// Actors with sprite assets can be spawned as "model" props (unk8_1=1), causing collision
// code to call marker_loadModelBin which reinterprets sprite data as BKModelBin. This helper
// lets decomp code detect and skip the model path for sprite assets.
extern "C" int ResourceMgr_IsModelAsset(uint32_t assetId) {
    if (auto it = sResourceRefCache.find(assetId); it != sResourceRefCache.end()) {
        return it->second->GetInitData()->Type == 0x424B4D4F;
    }
    return 0;
}

extern "C" char* ResourceMgr_LoadTexOrDListByName(const char* filePath) {
    auto res = GetResourceByName(filePath);

    if (res->GetInitData()->Type == static_cast<uint32_t>(Fast::ResourceType::DisplayList))
        return (char*)&((std::static_pointer_cast<Fast::DisplayList>(res))->Instructions[0]);
    else {
        return (char*)ResourceGetDataByName(filePath);
    }
}

extern "C" char* ResourceMgr_LoadIfDListByName(const char* filePath) {
    auto res = GetResourceByName(filePath);

    if (res->GetInitData()->Type == static_cast<uint32_t>(Fast::ResourceType::DisplayList))
        return (char*)&((std::static_pointer_cast<Fast::DisplayList>(res))->Instructions[0]);

    return nullptr;
}

extern "C" Gfx* ResourceMgr_LoadGfxByName(const char* path) {
    auto res = std::static_pointer_cast<Fast::DisplayList>(GetResourceByName(path));
    return (Gfx*)&res->Instructions[0];
}

extern "C" Vtx* ResourceMgr_LoadVtxByName(char* path) {
    return (Vtx*)ResourceGetDataByName(path);
}

extern "C" Mtx* ResourceMgr_LoadMtxByName(char* path) {
    return (Mtx*)ResourceGetDataByName(path);
}

// Release all retained resource refs so destructors fire before spdlog shutdown
void ResourceHelpers_ClearRefCache() {
    sResourceRefCache.clear();
}

void ResourceHelpers_ApplyLanguage(std::unordered_map<uint32_t, std::string> dialogOverride, bool isJapanese,
                                   int dialogCount, int dialogIndex) {
    for (const auto& [id, path] : sDialogOverride) {
        sResourceRefCache.erase(id);
    }
    sDialogOverride = std::move(dialogOverride);
    for (const auto& [id, path] : sDialogOverride) {
        sResourceRefCache.erase(id);
    }
    ++sLanguageGeneration; // invalidates re-pointed models on their next draw
    sIsJapanese = isJapanese;
    sDialogLanguageCount = (dialogCount > 0) ? dialogCount : 1;
    if (dialogIndex < 0 || dialogIndex >= sDialogLanguageCount) {
        SPDLOG_WARN("[ResourceHelpers] Dialog language index {} out of range for a {}-language source; using 0",
                    dialogIndex, sDialogLanguageCount);
        dialogIndex = 0;
    }
    sDialogLanguage = dialogIndex; // keep ResourceMgr_GetDialogLanguage() in sync
    func_8031B5C4(dialogIndex);
}
