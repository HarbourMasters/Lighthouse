#include "ShipUtils.h"
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <spdlog/spdlog.h>
#include <libultraship/libultraship.h>
#include <fast/Fast3dWindow.h>
#include "ui/cvar_prefixes.h"
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _DEBUG
#include <crtdbg.h>
#endif

extern "C" uint64_t GetUnixTimestamp() {
    auto time = std::chrono::system_clock::now();
    auto since_epoch = time.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
    long now = millis.count();
    return now;
}

extern "C" bool Ship_IsCStringEmpty(const char* str) {
    return str == NULL || str[0] == '\0';
}

extern "C" int port_checkHeap(const char* label) {
#ifdef _DEBUG
    if (!_CrtCheckMemory()) {
        SPDLOG_ERROR("[port] HEAP CORRUPT at: {}", label);
        return 0;
    }
#endif
    return 1;
}

static void bk_log_vfmt(spdlog::level::level_enum level, const char* fmt, va_list args) {
    char buf[512];
    vsnprintf(buf, sizeof(buf), fmt, args);
    spdlog::default_logger_raw()->log(spdlog::source_loc{}, level, buf);
}

extern "C" void BK_LOG_INFO(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bk_log_vfmt(spdlog::level::info, fmt, args);
    va_end(args);
}

extern "C" void BK_LOG_WARN(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bk_log_vfmt(spdlog::level::warn, fmt, args);
    va_end(args);
}

extern "C" void BK_LOG_ERROR(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bk_log_vfmt(spdlog::level::err, fmt, args);
    va_end(args);
}

extern "C" const char* port_mapName(int map_id) {
    switch (map_id) {
        // Spiral Mountain
        case 0x01:
        case 0x8C:
            return "Spiral Mountain";
        // Mumbo's Mountain
        case 0x02:
        case 0x0C:
        case 0x0E:
            return "Mumbo's Mountain";
        // Treasure Trove Cove
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x0A:
        case 0x8F:
            return "Treasure Trove Cove";
        // Clanker's Cavern
        case 0x0B:
        case 0x21:
        case 0x22:
        case 0x23:
            return "Clanker's Cavern";
        // Bubblegloop Swamp
        case 0x0D:
        case 0x10:
        case 0x11:
        case 0x47:
            return "Bubblegloop Swamp";
        // Gobi's Valley
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x1A:
        case 0x92:
            return "Gobi's Valley";
        // Mad Monster Mansion
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        case 0x2F:
        case 0x30:
        case 0x8D:
            return "Mad Monster Mansion";
        // Freezeezy Peak
        case 0x27:
        case 0x41:
        case 0x48:
        case 0x53:
        case 0x7F:
            return "Freezeezy Peak";
        // Rusty Bucket Bay
        case 0x31:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
        case 0x8B:
            return "Rusty Bucket Bay";
        // Click Clock Wood
        case 0x40:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5E:
        case 0x5F:
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
            return "Click Clock Wood";
        // Gruntilda's Lair
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7A:
        case 0x80:
        case 0x8E:
        case 0x90:
        case 0x93:
            return "Gruntilda's Lair";
        // Cutscenes
        case 0x1E:
        case 0x1F:
        case 0x20:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
            return "Cutscene";
        // File Select
        case 0x91:
            return "File Select";
        default:
            return "Unknown";
    }
}

extern "C" int port_getBootSequence(void) {
    return CVarGetInteger(CVAR_SETTING("BootSequence"), 0);
}

extern "C" int port_getViewportWidth(void) {
    // Returns the logical viewport width based on the actual game viewport aspect ratio.
    // Uses the interpreter's current dimensions (respects imgui aspect ratio settings).
    // At 4:3, returns 320. At 16:9, returns ~427.
    auto ctx = Ship::Context::GetInstance();
    if (!ctx)
        return 320;
    auto fastWnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(ctx->GetWindow());
    if (!fastWnd)
        return 320;
    auto interp = fastWnd->GetInterpreterWeak().lock();
    if (!interp)
        return 320;
    uint32_t w = 0, h = 0;
    interp->GetCurDimensions(&w, &h);
    if (h == 0)
        return 320;
    float vpAspect = (float)w / (float)h;
    float gameAspect = 320.0f / 240.0f;
    if (vpAspect > gameAspect + 0.01f) {
        return (int)(320.0f * vpAspect / gameAspect);
    }
    return 320;
}

extern "C" void port_setMapDebugTitle(int map_id) {
    char title[128];
    snprintf(title, sizeof(title), "Lighthouse - %s (0x%02X)", port_mapName(map_id), map_id);
    SPDLOG_INFO("[port] {}", title);
#ifdef _WIN32
    // [port] Update the DXGI/SDL window title for quick visual debugging
    HWND hwnd = GetActiveWindow();
    if (hwnd) {
        SetWindowTextA(hwnd, title);
    }
#endif
}

// [port] Returns 0.0–1.0 rumble intensity scale from the ImGui controller config.
// Uses the average of low/high frequency percentages for the given controller port.
extern "C" float port_getRumbleScale(void) {
    auto ctx = Ship::Context::GetInstance();
    if (!ctx) {
        return 0.5f;
    }

    auto controller = ctx->GetControlDeck()->GetControllerByPort(0);
    if (!controller) {
        return 0.5f;
    }

    auto rumble = controller->GetRumble();
    for (auto& [id, mapping] : rumble->GetAllRumbleMappings()) {
        float low = mapping->GetLowFrequencyIntensityPercentage() / 100.0f;
        float high = mapping->GetHighFrequencyIntensityPercentage() / 100.0f;
        return (low + high) * 0.5f;
    }
    return 1.0f;
}
