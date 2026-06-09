#include "BaseGameVersion.h"

#include <filesystem>
#include <string>
#include <zip.h>

#include "ship/Context.h"
#include <spdlog/spdlog.h>

namespace Lighthouse {

BaseGameVersion ClassifyAssetCount(uint32_t assetCount) {
    if (assetCount >= 3030 && assetCount <= 3050) {
        return BaseGameVersion::USV11;
    }
    if (assetCount >= 3051 && assetCount <= 3080) {
        return BaseGameVersion::Localized; // PAL (3059) or JP (3065)
    }
    if (assetCount >= 3300) {
        return BaseGameVersion::USV10; // ~3314
    }
    return BaseGameVersion::Unknown;
}

BaseGameVersion GetBaseGameVersion() {
    static BaseGameVersion sVersion = BaseGameVersion::Unknown;
    static bool sResolved = false;
    if (sResolved) {
        return sVersion;
    }

    std::string basePath = Ship::Context::LocateFileAcrossAppDirs("bk.o2r", "bk");
    if (basePath.empty() || !std::filesystem::exists(basePath)) {
        return sVersion; // can't determine yet — retry on the next call
    }
    int err = 0;
    zip_t* z = zip_open(basePath.c_str(), ZIP_RDONLY, &err);
    if (z == nullptr) {
        return sVersion;
    }
    zip_int64_t idx = zip_name_locate(z, "assets/aBKAssetTable", 0);
    if (idx >= 0) {
        if (zip_file_t* f = zip_fopen_index(z, idx, 0)) {
            // aBKAssetTable is a LUS Blob: a 0x40-byte resource header, then a
            // u32 blob size, then the table payload whose first u32 is the entry
            // count. So the count lives at 0x44 in the raw archive entry.
            constexpr size_t kCountOffset = 0x40 + 4;
            uint8_t buf[kCountOffset + 4];
            size_t got = 0;
            while (got < sizeof(buf)) {
                zip_int64_t n = zip_fread(f, buf + got, sizeof(buf) - got);
                if (n <= 0) {
                    break;
                }
                got += static_cast<size_t>(n);
            }
            if (got == sizeof(buf)) {
                const uint8_t* p = buf + kCountOffset;
                uint32_t count = static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
                                 (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
                sVersion = ClassifyAssetCount(count);
                sResolved = true;
                SPDLOG_INFO("[BaseGameVersion] Base bk.o2r has {} assets (version class {})", count,
                            static_cast<int>(sVersion));
            }
            zip_fclose(f);
        }
    }
    zip_close(z);
    return sVersion;
}

bool BaseGameSupportsRomhacks() {
    BaseGameVersion v = GetBaseGameVersion();
    return v != BaseGameVersion::USV11 && v != BaseGameVersion::Localized;
}

} // namespace Lighthouse
