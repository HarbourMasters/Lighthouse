#include "ShipUtils.h"
#include "Save/SaveManager.h"
#include "Save/Types.h"
#include "Engine.h"
#include "port/Romhack/RomhackConfig.h"
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <spdlog/spdlog.h>
#include <libultraship/libultraship.h>
#include <fast/Fast3dWindow.h>
#include "UI/cvar_prefixes.h"
#include "UI/enhancementTypes.h"
#include "fast/Fast3dGui.h"
#ifdef _WIN32
#include <windows.h>
#endif

#if defined(_DEBUG) && defined(_MSC_VER)
#include <crtdbg.h>
#endif

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

int32_t gSelectedFileNum = 0;

std::vector<std::string> worldNameList = {
    "Mumbo's Mountain", "Treasure Trove Cove", "Clanker's Cavern", "Bubblegloop Swamp",
    "Freezeezy Peak",   "Gruntilda's Lair",    "Gobi's Valley",    "Click Clock Wood",
    "Rusty Bucket Bay", "Mad Monster Mansion", "Spiral Mountain",
};

std::vector<std::string> abilityNameList = { "Beak Barge", "Beak Bomb",   "Beak Buster",  "Camera Control",
                                             "Claw Swipe", "Climb",       "Eggs",         "Feathery Flap",
                                             "Flap Flip",  "Flight",      "Jump Higher",  "Ratatat Rap",
                                             "Roll",       "Shock Jump",  "Wading Boots", "Dive",
                                             "Talon Trot", "Turbo Talon", "Wonderwing",   "Note Door" };

// Helper for C-style variadic log functions
static void bk_log_vfmt(spdlog::level::level_enum level, const char* fmt, va_list args) {
    char buf[512];
    vsnprintf(buf, sizeof(buf), fmt, args);
    spdlog::default_logger_raw()->log(spdlog::source_loc{}, level, buf);
}

void TableCellCenteredText(const char* text) {
    float textHeight = ImGui::GetTextLineHeight();
    float offsetY = (32.0f - textHeight + 5.0f) * 0.5f;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
    ImGui::Text("%s", text);
}

extern uint32_t Ship_Hash(std::string str) {
    const size_t len = str.size();
    uint32_t hval = 0x811c9dc5;
    for (size_t pos = 0; pos < len; pos++) {
        hval ^= (uint32_t)str[pos];
        hval *= 0x01000193;
    }
    return hval;
}

extern std::string port_FormatTimeDisplay(uint64_t value) {
    uint32_t sec = value / 10;
    uint32_t hh = sec / 3600;
    uint32_t mm = (sec - hh * 3600) / 60;
    uint32_t ss = sec - hh * 3600 - mm * 60;
    return fmt::format("{}:{:0>2}:{:0>2}", hh, mm, ss);
}

extern "C" {

#include "enums.h" // game_mode_e

// Furnace Fun active flag
s32 getGameMode(void);

int gPortResetPending = 0;

bool IsDemoMode(void) {
    return (getGameMode() != GAME_MODE_3_NORMAL);
}

uint64_t GetUnixTimestamp() {
    auto time = std::chrono::system_clock::now();
    auto since_epoch = time.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
    long now = static_cast<long>(millis.count());
    return now;
}

bool Ship_IsCStringEmpty(const char* str) {
    return str == NULL || str[0] == '\0';
}

int port_checkHeap(const char* label) {
#if defined(_DEBUG) && defined(_MSC_VER)
    if (!_CrtCheckMemory()) {
        SPDLOG_ERROR("[port] HEAP CORRUPT at: {}", label);
        return 0;
    }
#endif
    return 1;
}

// Wrappers to use SPDLOG from C code
void BK_LOG_INFO(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bk_log_vfmt(spdlog::level::info, fmt, args);
    va_end(args);
}

void BK_LOG_WARN(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bk_log_vfmt(spdlog::level::warn, fmt, args);
    va_end(args);
}

void BK_LOG_ERROR(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bk_log_vfmt(spdlog::level::err, fmt, args);
    va_end(args);
}

const char* port_mapName(int map_id) {
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
        // Freezeezy Peak
        case 0x27:
        case 0x41:
        case 0x48:
        case 0x53:
        case 0x7F:
            return "Freezeezy Peak";
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

int port_getBootSequence(void) {
    // Romhacks always boot to file select; their intros aren't compatible with the
    // vanilla cutscene/demo path.
    if (port_isRomhack())
        return BOOTSEQUENCE_FILESELECT;
    return CVarGetInteger(CVAR_SETTING("BootSequence"), BOOTSEQUENCE_DEFAULT);
}

float port_getRumbleScale(void) {
    auto ctx = Ship::Context::GetRawInstance();
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

} // extern "C"

std::vector<file_progress_e> worldOpenFlags = {
    FILEPROG_31_MM_OPEN, FILEPROG_32_TTC_OPEN, FILEPROG_33_CC_OPEN,  FILEPROG_34_BGS_OPEN, FILEPROG_35_FP_OPEN,
    FILEPROG_36_GV_OPEN, FILEPROG_37_MMM_OPEN, FILEPROG_38_RBB_OPEN, FILEPROG_39_CCW_OPEN,
};

std::unordered_map<std::string, std::string> levelAbbreviations = {
    { "MM", "Mumbos Mountain" },      { "TTC", "Treasure Trove Cove" }, { "CC", "Clankers Cavern" },
    { "BGS", "Bubblegloop Swamp" },   { "FP", "Freezeezy Peak" },       { "GL", "Gruntildas Lair" },
    { "GV", "Gobis Valley" },         { "CCW", "Click Clock Wood" },    { "RBB", "Rusty Bucket Bay" },
    { "MMM", "Mad Monster Mansion" }, { "SM", "Spiral Mountain" },
};

json Ship_RetrieveSaveFile(int32_t filenum) {
    if (filenum < 0 || filenum > 2) {
        return json::object();
    }
    std::string fileName = "file" + std::to_string(SlotToFileIndex(filenum)) + ".json";
    std::string filePath = SaveManager_GetSavePath(fileName);

    if (!std::filesystem::exists(filePath)) {
        return json::object();
    }

    std::ifstream file(filePath);
    json jsonSave;

    file >> jsonSave;

    return jsonSave;
}

std::string Ship_ConvertEnumToReadableName(const std::string& input, bool addPrefix) {
    std::string result;
    std::string content = input;
    std::string abbreviation = "";

    // Step 1: Remove "RC_" prefix if present
    const std::string prefix = "RC_";
    if (content.rfind(prefix, 0) == 0) {
        content = content.substr(prefix.size());
    }

    // Step 2: Remove level abbreviation if present
    for (auto& abbr : levelAbbreviations) {
        std::string prefix = abbr.first + "_";
        if (content.rfind(prefix, 0) == 0) {
            content = content.substr(prefix.size());
            abbreviation = abbr.first;
            break;
        }
    }

    // Step 3: Split the string by '_'
    std::vector<std::string> words;
    std::string word;
    std::istringstream stream(content);
    while (std::getline(stream, word, '_')) {
        words.push_back(word);
    }

    // Step 4: Capitalize the first letter of each word
    for (auto& w : words) {
        std::transform(w.begin(), w.end(), w.begin(), [](unsigned char c) { return std::tolower(c); });
        if (!w.empty()) {
            if (w == "rbb") {
                w = "RBB";
            } else if (w == "mmm") {
                w = "MMM";
            } else {
                w[0] = std::toupper(w[0]);
            }
        }
    }

    // Step 5: Join the words with spaces
    for (size_t i = 0; i < words.size(); ++i) {
        result += words[i];
        if (i < words.size() - 1) {
            result += " ";
        }
    }

    // Step 6: Add back full prefix if enabled
    if (addPrefix && abbreviation != "") {
        auto it = levelAbbreviations.find(abbreviation);
        if (it != levelAbbreviations.end()) {
            result = it->second + " " + result;
        }
    }

    return result;
}

// std::vector <std::pair<std::string, std::string>> miscellaneousTextures = {
//     { "Music Note", "assets/sprite/ASSET_7D9_NOTE_3_0" },
// };
//
// void LoadGuiTextures() {
//     auto gui = std::dynamic_pointer_cast<Fast::Fast3dGui>(Ship::Context::GetRawInstance()->GetWindow()->GetGui());
//     for (const auto entry : miscellaneousTextures) {
//         gui->LoadGuiTexture(entry.first, entry.second, ImVec4(1, 0, 0, 1));
//     }
// }
