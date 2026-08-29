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
#include "include/functions.h"
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

void TableCellCenteredSetCursorPosY(float size) {
    float textHeight = ImGui::GetTextLineHeight();
    float offsetY = (size - textHeight + 5.0f) * 0.5f;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
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
void BK_LOG_DEBUG(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bk_log_vfmt(spdlog::level::debug, fmt, args);
    va_end(args);
}

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
    if ((enum map_e)map_id == MAP_91_FILE_SELECT) {
        return "File Select";
    }
    return worldNameList[map_getLevel((enum map_e)map_id)];
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

typedef struct {
    std::string name;
    std::string texturePath;
    std::string palettePath;
} PaletteTextureLoad;

std::vector<PaletteTextureLoad> paletteTextureLoad = {
    { "Music Note", "assets/sprite/ASSET_81B_LIVE_MUSIC_NOTE_1_0", "assets/sprite/ASSET_81B_LIVE_MUSIC_NOTE_1_TLUT" },
    { "Jiggy", "assets/sprite/ASSET_80D_LIVE_JIGGY_1_0", "assets/sprite/ASSET_80D_LIVE_JIGGY_1_TLUT" },
    { "Empty Honeycomb", "assets/sprite/ASSET_81D_LIVE_EXTRA_HEALTH_MAX_1_0",
      "assets/sprite/ASSET_81D_LIVE_EXTRA_HEALTH_MAX_1_TLUT" },
    { "Mumbo Token", "assets/sprite/ASSET_41A_MUMBO_TOKEN_1_0", "assets/sprite/ASSET_41A_MUMBO_TOKEN_1_TLUT" },
    { "Blue Jinjo", "assets/sprite/ASSET_804_JINJO_BLUE_0_0", "assets/sprite/ASSET_804_JINJO_BLUE_0_TLUT" },
    { "Green Jinjo", "assets/sprite/ASSET_803_JINJO_GREEN_0_0", "assets/sprite/ASSET_803_JINJO_GREEN_0_TLUT" },
    { "Orange Jinjo", "assets/sprite/ASSET_806_JINJO_ORANGE_0_0", "assets/sprite/ASSET_806_JINJO_ORANGE_0_TLUT" },
    { "Pink Jinjo", "assets/sprite/ASSET_805_JINJO_PINK_0_0", "assets/sprite/ASSET_805_JINJO_PINK_0_TLUT" },
    { "Yellow Jinjo", "assets/sprite/ASSET_802_JINJO_YELLOW_0_0", "assets/sprite/ASSET_802_JINJO_YELLOW_0_TLUT" },
};

void LoadGuiTextures() {
    auto gui = std::dynamic_pointer_cast<Fast::Fast3dGui>(Ship::Context::GetRawInstance()->GetWindow()->GetGui());
    for (const auto entry : paletteTextureLoad) {
        gui->LoadGuiTexture(entry.name, entry.texturePath, entry.palettePath, ImVec4(1, 1, 1, 1));
    }
}
