#include "Language.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "libultraship/libultraship.h"
#include "ship/Context.h"
#include "ship/resource/ResourceManager.h"
#include "ship/resource/archive/Archive.h"
#include "ship/resource/archive/ArchiveManager.h"
#include "ship/resource/File.h"

#include "port/ResourceHelpers.h"
#include "port/GameVersion/AssetVersionRemap.h"
#include "port/GameVersion/BaseGameVersion.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/UI/cvar_prefixes.h"

namespace Lighthouse {
namespace {

enum LanguageScript { SCRIPT_LATIN = 0, SCRIPT_JP = 1 };

struct LanguageEntry {
    std::string name;      // display name (from region defaults or pack langinfo)
    Ship::Archive* source; // base game's own dialog if nullptr; else the pack
    int index;             // dialog index within the source's multi-language blob
    int count;             // number of languages packed in the source's blobs
    int script;            // LanguageScript: drives the JP font/sprite path
};

std::vector<LanguageEntry> sLanguages;
std::string sActiveLanguage;

// Localized UI-string tables (key = English, value = translation), keyed by language
// name; one table per pack, shared by its languages. sActiveLangStrings points at the
// selected language's table.
using StringMap = std::unordered_map<std::string, std::string>;
std::unordered_map<std::string, std::shared_ptr<StringMap>> sLangStringsByName;
std::shared_ptr<StringMap> sActiveLangStrings;

// One parsed langinfo entry: display name, dialog index, script.
struct LangInfoEntry {
    std::string name;
    int index;
    int script;
};

// Parse a pack's binary langinfo. The optional out-map receives the localized-string
// table appended after the languages.
std::vector<LangInfoEntry> ParseLangInfo(const char* data, size_t size,
                                         std::unordered_map<std::string, std::string>* strings = nullptr) {
    std::vector<LangInfoEntry> out;
    size_t pos = 0;
    auto getU32 = [&](uint32_t& v) -> bool {
        if (pos + 4 > size) {
            return false;
        }
        std::memcpy(&v, data + pos, 4);
        pos += 4;
        return true;
    };
    auto getStr = [&](std::string& s) -> bool {
        uint32_t len = 0;
        if (!getU32(len) || pos + len > size) {
            return false;
        }
        s.assign(data + pos, len);
        pos += len;
        return true;
    };
    uint32_t version = 0, count = 0;
    if (!getU32(version) || version != 1 || !getU32(count)) {
        return out; // unknown/old format — ignore (pack must be re-extracted)
    }
    for (uint32_t i = 0; i < count; i++) {
        uint32_t index = 0, script = 0;
        std::string name;
        if (!getU32(index) || !getU32(script) || !getStr(name)) {
            break;
        }
        out.push_back({ std::move(name), static_cast<int>(index), static_cast<int>(script) });
    }
    // Appended localized-string table (key = English, value = translation). EOF here
    // just means the pack carries no overrides.
    if (strings != nullptr) {
        uint32_t scount = 0;
        if (getU32(scount)) {
            for (uint32_t i = 0; i < scount; i++) {
                std::string key, val;
                if (!getStr(key) || !getStr(val)) {
                    break;
                }
                (*strings)[std::move(key)] = std::move(val);
            }
        }
    }
    return out;
}

bool AssetHexFromPath(const std::string& path, uint32_t& out) {
    auto a = path.rfind("ASSET_");
    if (a == std::string::npos) {
        return false;
    }
    a += 6;
    auto e = path.find('_', a);
    if (e == std::string::npos) {
        return false;
    }
    try {
        out = static_cast<uint32_t>(std::stoul(path.substr(a, e - a), nullptr, 16));
        return true;
    } catch (...) { return false; }
}

// True if the archive contains an asset whose id hex matches `hex`.
bool ArchiveHasAssetHex(Ship::Archive* archive, uint32_t hex) {
    if (archive == nullptr) {
        return false;
    }
    auto files = archive->ListFiles();
    if (!files) {
        return false;
    }
    for (const auto& [hash, path] : *files) {
        uint32_t x = 0;
        if (AssetHexFromPath(path, x) && x == hex) {
            return true;
        }
    }
    return false;
}

const LanguageEntry* FindLanguage(const std::string& name) {
    for (const auto& e : sLanguages) {
        if (e.name == name) {
            return &e;
        }
    }
    return nullptr;
}

} // namespace

int32_t LanguageKey(const std::string& name) {
    // Stable FNV-1a hash so the combobox key / CVar survives reboots and changes
    // to the pack list (unlike a list index).
    uint32_t h = 2166136261u;
    for (unsigned char c : name) {
        h ^= c;
        h *= 16777619u;
    }
    return static_cast<int32_t>(h & 0x7FFFFFFF);
}

std::vector<std::string> GetAvailableLanguageNames() {
    std::vector<std::string> names;
    names.reserve(sLanguages.size());
    for (const auto& e : sLanguages) {
        names.push_back(e.name);
    }
    return names;
}

std::string GetActiveLanguage() {
    return sActiveLanguage;
}

// Localize a hardcoded UI string: returns the active pack's translation of `english`,
// or `english` itself when the pack doesn't override it (or no pack is active). The
// returned pointer is valid until the language changes; callers pass string literals.
extern "C" const char* ResourceMgr_GetLangString(const char* english) {
    if (english != nullptr && sActiveLangStrings != nullptr) {
        auto it = sActiveLangStrings->find(english);
        if (it != sActiveLangStrings->end() && !it->second.empty()) {
            return it->second.c_str();
        }
    }
    return english;
}

// True when the active language is a pack that carries UI-string overrides. Lets the port
// rebuild variable-formatted lines (the file-select info box) only for such packs, leaving
// the base game's own (English / PAL FR-DE) assembly untouched.
extern "C" int ResourceMgr_HasLangStrings(void) {
    return (sActiveLangStrings != nullptr && !sActiveLangStrings->empty()) ? 1 : 0;
}

int GetAvailableLanguageCount() {
    return static_cast<int>(sLanguages.size());
}

std::vector<std::pair<int32_t, const char*>> GetLanguageComboEntries() {
    std::vector<std::pair<int32_t, const char*>> entries;
    entries.reserve(sLanguages.size());
    for (const auto& e : sLanguages) {
        entries.emplace_back(LanguageKey(e.name), e.name.c_str());
    }
    return entries;
}

void SetActiveLanguage(const std::string& name) {
    const LanguageEntry* entry = FindLanguage(name);
    if (entry == nullptr) {
        return;
    }
    sActiveLanguage = name;
    CVarSetInteger(CVAR_SETTING("DialogLanguage"), LanguageKey(name));
    if (auto it = sLangStringsByName.find(name); it != sLangStringsByName.end()) {
        sActiveLangStrings = it->second;
    } else {
        sActiveLangStrings = nullptr;
    }

    std::unordered_map<uint32_t, std::string> dialogOverride;
    if (entry->source != nullptr) {
        const std::unordered_map<uint32_t, uint32_t>* remap = nullptr;
        switch (static_cast<BKVersion>(entry->source->GetGameVersion())) {
            case BK_VER_US_11:
                remap = &sV10toV11Remap;
                break;
            case BK_VER_PAL:
                remap = &sV10toPALRemap;
                break;
            case BK_VER_JP:
                remap = &sV10toJPRemap;
                break;
            default:
                break; // pack ids already match v1.0
        }
        std::unordered_map<uint32_t, uint32_t> inverse;
        if (remap != nullptr) {
            for (const auto& [v10Id, targetId] : *remap) {
                inverse[targetId] = v10Id;
            }
        }

        std::unordered_map<uint32_t, std::string> packMain;
        if (auto files = entry->source->ListFiles()) {
            for (const auto& [hash, path] : *files) {
                uint32_t id = 0;
                if (!AssetHexFromPath(path, id)) {
                    continue;
                }
                auto it = packMain.find(id);
                if (it == packMain.end() || path.size() < it->second.size()) {
                    packMain[id] = path;
                }
            }
        }

        for (const auto& [packId, path] : packMain) {
            uint32_t v10Id;
            if (entry->script == SCRIPT_JP && packId >= 0xE2C && packId <= 0xE38) {
                v10Id = 0x1600 + (packId - 0xE2C); // JP kanji world-name banners
            } else if (auto it = inverse.find(packId); it != inverse.end()) {
                v10Id = it->second;
            } else {
                v10Id = packId;
            }
            dialogOverride[v10Id] = path;
        }
    }

    // Hand the computed language to the resource layer
    const size_t repointed = dialogOverride.size();
    ResourceHelpers_ApplyLanguage(std::move(dialogOverride), entry->script == SCRIPT_JP, entry->count, entry->index);
    SPDLOG_INFO("[Lang] Active language '{}' ({} ids re-pointed)", name, repointed);
}

void RescanLanguages() {
    sLanguages.clear();
    sLangStringsByName.clear();

    // Base language(s) inferred from the base game's region.
    switch (GetBaseVersion()) {
        case BK_VER_PAL:
            sLanguages.push_back({ "English (UK)", nullptr, 0, 3, SCRIPT_LATIN });
            sLanguages.push_back({ "French", nullptr, 1, 3, SCRIPT_LATIN });
            sLanguages.push_back({ "German", nullptr, 2, 3, SCRIPT_LATIN });
            break;
        case BK_VER_JP:
            sLanguages.push_back({ "Japanese", nullptr, 0, 1, SCRIPT_JP });
            break;
        default: // US v1.0 / v1.1
            sLanguages.push_back({ "English (US)", nullptr, 0, 1, SCRIPT_LATIN });
            break;
    }

    // Don't allow languages in romhacks
    if (port_isRomhack()) {
    } else if (auto archives =
                   Ship::Context::GetRawInstance()->GetResourceManager()->GetArchiveManager()->GetArchives()) {
        for (const auto& archive : *archives) {
            if (!archive) {
                continue;
            }
            auto file = archive->LoadFile("langinfo");
            if (!file || !file->IsLoaded || !file->Buffer) {
                continue;
            }
            const char* data = file->Buffer->data() + file->BufferOffset;
            const size_t size = file->Buffer->size() - file->BufferOffset;
            auto packStrings = std::make_shared<StringMap>();
            auto langs = ParseLangInfo(data, size, packStrings.get());
            const int cnt = static_cast<int>(langs.size());
            for (const auto& le : langs) {
                if (FindLanguage(le.name) != nullptr) {
                    continue;
                }
                // A JP-script pack must carry the JP dialog font (0x6EA).
                if (le.script == SCRIPT_JP && !ArchiveHasAssetHex(archive.get(), 0x6EA)) {
                    SPDLOG_ERROR("[Lang] Pack '{}' declares Japanese ('{}') but is missing the JP font sprite "
                                 "(0x6EA); excluding it. Re-extract the pack from a Japanese ROM.",
                                 archive->GetPath(), le.name);
                    continue;
                }
                sLanguages.push_back({ le.name, archive.get(), le.index, cnt, le.script });
                if (!packStrings->empty()) {
                    sLangStringsByName[le.name] = packStrings;
                }
            }
        }
    }

    // Re-apply the persisted selection against the refreshed list; fall back to
    // the base game's native (first) language.
    const int32_t savedKey = CVarGetInteger(CVAR_SETTING("DialogLanguage"), 0);
    std::string target;
    for (const auto& e : sLanguages) {
        if (LanguageKey(e.name) == savedKey) {
            target = e.name;
            break;
        }
    }
    if (target.empty() && !sLanguages.empty()) {
        target = sLanguages.front().name;
    }
    sActiveLanguage.clear();
    if (!target.empty()) {
        SetActiveLanguage(target);
    }
}

} // namespace Lighthouse
