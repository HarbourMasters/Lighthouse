
#include "ship/resource/ResourceManager.h"
#include "ship/resource/type/Blob.h"
#include "fast/resource/ResourceType.h"
#include "fast/resource/type/DisplayList.h"
#include "libultraship/bridge/resourcebridge.h"
#include "libultraship/libultraship.h"
#include "ship/Context.h"
#include "ui/cvar_prefixes.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>

#if defined(_DEBUG) && defined(_MSC_VER)
#include <crtdbg.h>
#endif

#include "AssetVersionRemap.h"

extern "C" {
#include "enums.h"
}

extern "C" uint16_t ResourceMgr_LoadTexWidthByName(char* texPath);
extern "C" uint16_t ResourceMgr_LoadTexHeightByName(char* texPath);
extern "C" void func_8031B5C4(int32_t lang); // decomp: set dialog language index

// [port] Dialog language state — detected at boot from o2r version
static int sDialogLanguageCount = 1; // 1 for US/JP, 3 for PAL (EN/FR/DE)
static int sDialogLanguage = 0;      // 0=English, 1=French, 2=German

namespace {
const std::unordered_map<uint32_t, std::string>& GetAssetSymbolMap() {
    static std::once_flag mapOnce;
    static std::unordered_map<uint32_t, std::string> symbolMap;

    std::call_once(mapOnce, [] {
        // Torch writes this as a Blob at "assets/aBKAssetTable".
        // Format: u32 count, then for each entry: u32 assetId, s32 pathLen, char path[pathLen]
        auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResource("assets/aBKAssetTable");
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

            symbolMap[assetId] = std::move(path);
        }

        SPDLOG_INFO("Loaded asset manifest from o2r with {} entries", symbolMap.size());

        // [port] If this o2r was built from a non-v1.0 ROM, inject v1.0 ID aliases
        // so the decomp's hardcoded IDs resolve transparently.
        // Detection: v1.0 has 3314 assets, v1.1/PAL/JP have 3044-3065.
        // This is done once at boot — no per-lookup cost after this point.
        const std::unordered_map<uint32_t, uint32_t>* remapTable = nullptr;
        const char* versionName = nullptr;

        if (symbolMap.size() >= 3030 && symbolMap.size() <= 3050) {
            remapTable = &sV10toV11Remap;
            versionName = "v1.1";
        } else if (symbolMap.size() >= 3051 && symbolMap.size() <= 3080) {
            // PAL (3059) and JP (3065) both fall here.
            // Try JP first — if JP-specific IDs exist in manifest, use JP table.
            // JP has mode 7 entries at IDs 3628+; PAL does not.
            if (symbolMap.find(3628) != symbolMap.end()) {
                remapTable = &sV10toJPRemap;
                versionName = "JP";
            } else {
                remapTable = &sV10toPALRemap;
                versionName = "PAL";
                sDialogLanguageCount = 3; // EN, FR, DE
                sDialogLanguage = CVarGetInteger(CVAR_SETTING("DialogLanguage"), 0);
                func_8031B5C4(sDialogLanguage); // Initialize decomp language index
                SPDLOG_INFO("[ResourceHelpers] PAL detected, dialog language CVar = {}", sDialogLanguage);
            }
        }

        if (remapTable) {
            // [port] Snapshot the manifest before injection
            const auto snapshot = symbolMap;
            uint32_t remapCount = 0;
            for (const auto& [v10Id, targetId] : *remapTable) {
                if (auto targetEntry = snapshot.find(targetId); targetEntry != snapshot.end()) {
                    symbolMap[v10Id] = targetEntry->second;
                    remapCount++;
                }
            }
            SPDLOG_INFO("[ResourceHelpers] Detected {} o2r — injected {} v1.0 ID aliases into symbol map", versionName,
                        remapCount);
        }
    });

    return symbolMap;
}
} // namespace

extern "C" int ResourceMgr_GetDialogLanguageCount(void) {
    return sDialogLanguageCount;
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
        return Ship::Context::GetInstance()->GetResourceManager()->LoadResource(path);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[ResourceHelpers] GetResourceByName('{}') exception: {}", path, e.what());
        return nullptr;
    }
}

// [port] Keep shared_ptr references alive so raw pointers from GetResourceRawPointer
// don't become dangling when the resource manager evicts entries from its cache.
static std::unordered_map<uint32_t, std::shared_ptr<Ship::IResource>> sResourceRefCache;

static char* LoadAndRetainResource(const std::string& path, uint32_t assetId) {
    auto res = GetResourceByName(path.c_str());
    if (res && res->GetRawPointer() != nullptr) {
        sResourceRefCache[assetId] = res;
        return reinterpret_cast<char*>(res->GetRawPointer());
    }
    return nullptr;
}

// [port] Reload an asset, evicting any cached version first.
// Used for map models whose vertex data gets modified at runtime.
extern "C" char* ResourceMgr_ReloadByAssetId(uint32_t assetId) {
    std::shared_ptr<Ship::IResource> oldRef;
    if (auto it = sResourceRefCache.find(assetId); it != sResourceRefCache.end()) {
        oldRef = std::move(it->second);
        sResourceRefCache.erase(it);
    }

    const auto& symbolMap = GetAssetSymbolMap();
    if (const auto entry = symbolMap.find(assetId); entry != symbolMap.end()) {
        auto mappedPath = entry->second;
        std::replace(mappedPath.begin(), mappedPath.end(), '\\', '/');

        // Unload from LUS cache so it re-reads from the o2r
        Ship::Context::GetInstance()->GetResourceManager()->UnloadResource(mappedPath);

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

    const auto& symbolMap = GetAssetSymbolMap();
    if (const auto entry = symbolMap.find(assetId); entry != symbolMap.end()) {
        auto mappedPath = entry->second;
        std::replace(mappedPath.begin(), mappedPath.end(), '\\', '/');

        if (auto result = LoadAndRetainResource(mappedPath, assetId); result != nullptr) {
            return result;
        } else {
            SPDLOG_WARN("[ResourceManager({})] symbol '{}' found but resource data is NULL", assetId, mappedPath);
            return nullptr;
        }
    } else {
        SPDLOG_WARN("[ResourceManager({})] not found in symbol map", assetId);
        return nullptr;
    }

    return nullptr;
}

// [port] Returns the data size of a previously loaded resource (from the ref cache).
// Used by decomp code that depends on assetCacheCurrentSize (e.g. demo_load).
extern "C" size_t ResourceMgr_GetResourceSize(uint32_t assetId) {
    if (auto it = sResourceRefCache.find(assetId); it != sResourceRefCache.end()) {
        return it->second->GetPointerSize();
    }
    return 0;
}

// [port] On N64, sprites and models were raw binary blobs that could be type-punned.
// On PC, they're separate resource types from different importers. Actors with sprite
// assets can be spawned as "model" props (unk8_1=1), causing collision code to call
// marker_loadModelBin which reinterprets sprite data as BKModelBin. This helper lets
// decomp code detect and skip the model path for sprite assets.
extern "C" int ResourceMgr_IsModelAsset(uint32_t assetId) {
    if (auto it = sResourceRefCache.find(assetId); it != sResourceRefCache.end()) {
        return it->second->GetInitData()->Type == 0x424B4D4F; // Torch::ResourceType::BKModel
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
