#include "BaseGameVersion.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <zip.h>

#include "ship/Context.h"
#include <spdlog/spdlog.h>

namespace Lighthouse {

namespace {

bool ReadStampedCrc(const std::string& archivePath, uint32_t& outCrc) {
    int err = 0;
    zip_t* z = zip_open(archivePath.c_str(), ZIP_RDONLY, &err);
    if (z == nullptr) {
        return false;
    }

    bool ok = false;
    if (zip_int64_t idx = zip_name_locate(z, "version", 0); idx >= 0) {
        if (zip_file_t* f = zip_fopen_index(z, idx, 0)) {
            uint8_t buf[5] = {};
            if (zip_fread(f, buf, sizeof(buf)) == static_cast<zip_int64_t>(sizeof(buf))) {
                const bool big = buf[0] != 0;
                outCrc = big ? (static_cast<uint32_t>(buf[1]) << 24 | static_cast<uint32_t>(buf[2]) << 16 |
                                static_cast<uint32_t>(buf[3]) << 8 | static_cast<uint32_t>(buf[4]))
                             : (static_cast<uint32_t>(buf[4]) << 24 | static_cast<uint32_t>(buf[3]) << 16 |
                                static_cast<uint32_t>(buf[2]) << 8 | static_cast<uint32_t>(buf[1]));
                ok = true;
            }
            zip_fclose(f);
        }
    }

    zip_close(z);
    return ok;
}

BKVersion ClassifyCrc(uint32_t crc) {
    switch (crc) {
        case BK_VER_US_11:
            return BK_VER_US_11;
        case BK_VER_PAL:
            return BK_VER_PAL;
        case BK_VER_JP:
            return BK_VER_JP;
        case BK_VER_US_10:
        default:
            // Vanilla v1.0, a v1.0-based romhack, or an unknown dump.
            return BK_VER_US_10;
    }
}

} // namespace

BKVersion GetBaseVersion() {
    static BKVersion sVersion = BK_VER_US_10;
    static bool sResolved = false;
    if (sResolved) {
        return sVersion;
    }

    std::string basePath = Ship::Context::LocateFileAcrossAppDirs("bk.o2r", "bk");
    if (basePath.empty() || !std::filesystem::exists(basePath)) {
        return sVersion; // not extracted yet — retry on the next call
    }

    uint32_t crc = 0;
    if (ReadStampedCrc(basePath, crc)) {
        sVersion = ClassifyCrc(crc);
        sResolved = true;
        SPDLOG_INFO("[BaseGameVersion] base bk.o2r CRC 0x{:08X} -> BKVersion 0x{:08X}", crc,
                    static_cast<uint32_t>(sVersion));
    }
    return sVersion;
}

bool BaseGameSupportsRomhacks() {
    return GetBaseVersion() == BK_VER_US_10;
}

std::string BaseRegionSlug() {
    switch (GetBaseVersion()) {
        case BK_VER_PAL:
            return "pal";
        case BK_VER_JP:
            return "jp";
        case BK_VER_US_10:
        case BK_VER_US_11:
        default:
            return "us";
    }
}

} // namespace Lighthouse
