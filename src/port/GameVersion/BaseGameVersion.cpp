#include "BaseGameVersion.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <zip.h>

#include "ship/Context.h"
#include <spdlog/spdlog.h>

namespace Lighthouse {

namespace {

bool IsKnownVersion(uint32_t crc) {
    switch (crc) {
        case BK_VER_US_10:
        case BK_VER_US_11:
        case BK_VER_PAL:
        case BK_VER_JP:
            return true;
        default:
            return false;
    }
}

uint32_t ByteSwap32(uint32_t v) {
    return (v >> 24) | ((v >> 8) & 0x0000FF00u) | ((v << 8) & 0x00FF0000u) | (v << 24);
}

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
            if (zip_fread(f, buf, sizeof(buf)) == sizeof(buf)) {
                outCrc = (static_cast<uint32_t>(buf[1]) << 24) | (static_cast<uint32_t>(buf[2]) << 16) |
                         (static_cast<uint32_t>(buf[3]) << 8) | static_cast<uint32_t>(buf[4]);
                ok = true;
            }
            zip_fclose(f);
        }
    }

    zip_close(z);
    return ok;
}

} // namespace

std::string BaseArchivePath() {
    std::string basePath = Ship::Context::LocateFileAcrossAppDirs("bk.o2r", "bk");
    if (basePath.empty() || !std::filesystem::exists(basePath)) {
        return std::string(); // not extracted yet
    }
    return basePath;
}

BKVersion GetBaseVersion() {
    static BKVersion sVersion = BK_VER_US_10;
    static bool sResolved = false;
    if (sResolved) {
        return sVersion;
    }

    const std::string basePath = BaseArchivePath();
    if (basePath.empty()) {
        return sVersion; // not extracted yet — retry on the next call
    }

    uint32_t crc = 0;
    BKVersion version = BK_VER_US_10;
    if (ReadStampedCrc(basePath, crc) && ClassifyArchiveVersion(crc, version)) {
        sVersion = version;
        sResolved = true;
        SPDLOG_INFO("[BaseGameVersion] base bk.o2r CRC 0x{:08X} -> BKVersion 0x{:08X}", crc,
                    static_cast<uint32_t>(sVersion));
    }
    return sVersion; // unstamped or unrecognised: stay on the v1.0 default and retry
}

bool ClassifyArchiveVersion(uint32_t rawVersion, BKVersion& out) {
    if (IsKnownVersion(rawVersion)) {
        out = static_cast<BKVersion>(rawVersion);
        return true;
    }
    const uint32_t swapped = ByteSwap32(rawVersion);
    if (IsKnownVersion(swapped)) {
        out = static_cast<BKVersion>(swapped);
        return true;
    }
    return false;
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
