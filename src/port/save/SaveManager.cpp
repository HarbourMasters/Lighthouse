#include "SaveManager.h"
#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"
#include "port/GameConfig.h"

#include <nlohmann/json.hpp>
#include <ship/Context.h>
#include <spdlog/spdlog.h>

#include <cstring>
#include <fstream>
#include <filesystem>

using json = nlohmann::ordered_json;
namespace fs = std::filesystem;

extern "C" void savedata_update_crc(void* buffer, int size);
extern "C" int item_getCount(int item);
extern "C" void item_set(int, int);

// Bottles Bonus variables
extern "C" uint8_t gCompletedBottlesBonusGames[7];
extern "C" uint8_t D_8037DCC7; // "has seen instructions" flag
extern "C" uint8_t D_8037DCC8; // "has seen quit lose text" flag
extern "C" uint8_t D_8037DCC9; // "has seen timeout lose text" flag
extern "C" uint8_t D_8037DCCA; // "has seen secret game discovered" flag
extern "C" int32_t D_80385F30[];

// ─── Compact-array JSON formatter ───────────────────────────────────────────
// Pretty-prints objects but collapses primitive arrays onto single lines.
static std::string JsonDumpCompactArrays(const json& j, int indent = 2) {
    std::string raw = j.dump(indent);
    std::string out;
    out.reserve(raw.size());

    for (size_t i = 0; i < raw.size(); i++) {
        if (raw[i] == '[') {
            // Peek ahead: if this array contains only primitives (no { or [),
            // collapse everything up to the matching ] onto one line.
            size_t depth = 1;
            size_t end = i + 1;
            bool simple = true;
            while (end < raw.size() && depth > 0) {
                if (raw[end] == '[' || raw[end] == '{') {
                    simple = false;
                    break;
                }
                if (raw[end] == ']') {
                    depth--;
                }
                end++;
            }
            if (simple && depth == 0) {
                // Collect content between [ and ], strip newlines/excess whitespace
                out += "[ ";
                std::string inner;
                for (size_t k = i + 1; k < end - 1; k++) {
                    char c = raw[k];
                    if (c == '\n' || c == '\r') {
                        continue;
                    }
                    if (c == ' ' && !inner.empty() && inner.back() == ' ') {
                        continue;
                    }
                    inner += c;
                }
                // Trim leading/trailing whitespace
                size_t s = inner.find_first_not_of(' ');
                size_t e = inner.find_last_not_of(' ');
                if (s != std::string::npos)
                    out += inner.substr(s, e - s + 1);
                out += " ]";
                i = end - 1; // skip past ]
                continue;
            }
        }
        out += raw[i];
    }
    return out;
}

// ─── Slot ↔ Visual Mapping ─────────────────────────────────────────────────
// File select visual order (left-to-right) is gamenum 0, 2, 1.
// slotIndex = gamenum + 1, so internal slots 1,2,3 map to visual games 1,3,2.
// We name files by visual game number so file2.json = the 2nd game on screen.
static int SlotToVisualGame(int slotIndex) {
    static const int kMap[4] = { 0, 1, 3, 2 };
    return (slotIndex >= 1 && slotIndex <= 3) ? kMap[slotIndex] : slotIndex;
}
static int VisualGameToSlot(int visual) {
    static const int kMap[4] = { 0, 1, 3, 2 }; // symmetric: same remap
    return (visual >= 1 && visual <= 3) ? kMap[visual] : visual;
}

// ─── Binary Layout Constants ────────────────────────────────────────────────
// These match the offsets computed by savedata_init() in savedata.c.
// SaveData is 120 bytes: magic(1) + slotIndex(1) + data(112) + padding(2) + crc(4)

static constexpr int JIGGY_OFFSET = 2;
static constexpr int JIGGY_SIZE = 13; // bit array for 100 jiggies
static constexpr int HONEYCOMB_OFFSET = 15;
static constexpr int HONEYCOMB_SIZE = 3; // bit array for 24 honeycombs
static constexpr int MUMBO_OFFSET = 18;
static constexpr int MUMBO_SIZE = 16; // bit array for 125 mumbo tokens
static constexpr int NOTE_OFFSET = 34;
static constexpr int NOTE_SIZE = 8; // packed u64: 9 worlds × 7 bits
static constexpr int TIME_OFFSET = 42;
static constexpr int TIME_SIZE = 22; // 11 × u16
static constexpr int PROGRESS_OFFSET = 64;
static constexpr int PROGRESS_SIZE = 37; // 296 bits for file_progress_e
static constexpr int ITEMS_OFFSET = 101;
static constexpr int ITEMS_SIZE = 5; // mumboTokens, eggs, redFeathers, goldFeathers, jiggyTotal
static constexpr int ABILITY_OFFSET = 106;
static constexpr int ABILITY_SIZE = 8; // learnedAbilities(4) + usedAbilities(4)

// ─── World Collectible Definitions ──────────────────────────────────────────
// Each world's collectible ID ranges, derived from enums.h.
// Jiggy IDs: 10 per level_e value. Honeycomb/Mumbo ranges are non-uniform.

struct WorldDef {
    const char* name;
    int levelId;
    int jiggyStart, jiggyCount;         // 1-based global IDs; 0,0 = none
    int honeycombStart, honeycombCount; // 1-based global IDs; 0,0 = none
    int mumboStart, mumboCount;         // 1-based global IDs; 0,0 = none
    bool hasNoteScore;
    bool hasTimeScore;
};

static const WorldDef kWorlds[] = {
    //      lvl  jig_s jig_c  hc_s hc_c  mt_s mt_c  note  time
    { "SM", 11, 0, 0, 19, 6, 0, 0, false, true },     { "MM", 1, 1, 10, 1, 2, 1, 5, true, true },
    { "TTC", 2, 11, 10, 3, 2, 6, 10, true, true },    { "CC", 3, 21, 10, 5, 2, 16, 5, true, true },
    { "BGS", 4, 31, 10, 7, 2, 21, 10, true, true },   { "FP", 5, 41, 10, 9, 2, 31, 10, true, true },
    { "GV", 7, 61, 10, 11, 2, 41, 10, true, true },   { "MMM", 10, 91, 10, 17, 2, 51, 15, true, true },
    { "RBB", 9, 81, 10, 15, 2, 66, 15, true, true },  { "CCW", 8, 71, 10, 13, 2, 91, 25, true, true },
    { "LAIR", 6, 51, 10, 0, 0, 81, 10, false, true }, { "FINALE", 12, 0, 0, 0, 0, 0, 0, false, false },
};
static constexpr int kWorldCount = sizeof(kWorlds) / sizeof(kWorlds[0]);

// Note score world order: packed MSB-first, 9 worlds × 7 bits in u64
static const int kNoteScoreWorlds[] = { 1, 2, 3, 4, 5, 7, 8, 9, 10 };

// ─── Progress Flag Table ────────────────────────────────────────────────────
// Maps FILEPROG enum values to JSON names, bit widths, and owning world.
// world == NULL means the flag goes into the top-level "progress" section.
// Assignment rule: flags are categorized by WHERE they get SET in-game.

struct FlagDef {
    int bitIndex;
    int bitWidth;
    const char* name;
    const char* world; // NULL = general
};

static const FlagDef kProgressFlags[] = {
    // ── BGS ──
    { 0x00, 1, "TIPTUP_MINIGAME_PROGRESS", "BGS" },
    // ── General: first-time text popups (can trigger in any world) ──
    { 0x03, 1, "MUSIC_NOTE_TEXT", nullptr },
    { 0x04, 1, "MUMBO_TOKEN_TEXT", nullptr },
    { 0x05, 1, "BLUE_EGG_TEXT", nullptr },
    { 0x06, 1, "RED_FEATHER_TEXT", nullptr },
    { 0x07, 1, "GOLD_FEATHER_TEXT", nullptr },
    { 0x08, 1, "ORANGE_TEXT", nullptr },
    { 0x09, 1, "GOLD_BULLION_TEXT", nullptr },
    { 0x0A, 1, "HONEYCOMB_TEXT", nullptr },
    { 0x0B, 1, "EMPTY_HONEYCOMB_TEXT", nullptr },
    { 0x0C, 1, "EXTRA_LIFE_TEXT", nullptr },
    { 0x0D, 1, "BEEHIVE_TEXT", nullptr },
    { 0x0E, 1, "JINJO_TEXT", nullptr },
    // ── World-specific environmental ──
    { 0x0F, 1, "HAS_TOUCHED_PIRAHANA_WATER", "BGS" },
    { 0x10, 1, "HAS_TOUCHED_SAND_EEL_SAND", "GV" },
    { 0x11, 1, "HAS_MET_MUMBO", "MM" },
    { 0x12, 1, "HAS_TRANSFORMED_BEFORE", nullptr },
    { 0x13, 1, "COMPLETED_TWINKLIES_MINIGAME", "FP" },
    { 0x14, 1, "HAS_TOUCHED_FP_ICY_WATER", "FP" },
    { 0x15, 1, "ENTER_MMM_TEXT", "MMM" },
    // ── LAIR: puzzle podiums ──
    { 0x16, 1, "STOOD_ON_JIGSAW_PODIUM", "LAIR" },
    { 0x17, 1, "HAS_HAD_ENOUGH_JIGSAW_PIECES", "LAIR" },
    // ── Witch switches (set in respective worlds) ──
    { 0x18, 1, "MM_WITCH_SWITCH_JIGGY_PRESSED", "MM" },
    { 0x19, 1, "MMM_WITCH_SWITCH_JIGGY_PRESSED", "MMM" },
    { 0x1A, 1, "TTC_WITCH_SWITCH_JIGGY_PRESSED", "TTC" },
    { 0x1B, 1, "MET_YELLOW_FLIBBITS", "BGS" },
    { 0x1C, 1, "RBB_WITCH_SWITCH_JIGGY_PRESSED", "RBB" },
    { 0x1D, 1, "MMM_DINNING_ROOM_CUTSCENE", "MMM" },
    // ── LAIR: structural ──
    { 0x1E, 1, "LAIR_GRATE_TO_BGS_PUZZLE_OPEN", "LAIR" },
    { 0x1F, 1, "CC_LOBBY_PIPE_1_RAISED", "LAIR" },
    { 0x20, 1, "CC_LOBBY_PIPE_2_RAISED", "LAIR" },
    { 0x21, 1, "CC_LOBBY_PIPE_3_RAISED", "LAIR" },
    { 0x22, 1, "WATER_SWITCH_1_PRESSED", "LAIR" },
    { 0x23, 1, "LAIR_WATER_LEVEL_1", "LAIR" },
    { 0x24, 1, "WATER_SWITCH_2_PRESSED", "LAIR" },
    { 0x25, 1, "LAIR_WATER_LEVEL_2", "LAIR" },
    { 0x26, 1, "WATER_SWITCH_3_PRESSED", "LAIR" },
    { 0x27, 1, "LAIR_WATER_LEVEL_3", "LAIR" },
    // ── LAIR: world _OPEN flags (puzzle completion unlocks entry) ──
    { 0x31, 1, "MM_OPEN", "LAIR" },
    { 0x32, 1, "TTC_OPEN", "LAIR" },
    { 0x33, 1, "CC_OPEN", "LAIR" },
    { 0x34, 1, "BGS_OPEN", "LAIR" },
    { 0x35, 1, "FP_OPEN", "LAIR" },
    { 0x36, 1, "GV_OPEN", "LAIR" },
    { 0x37, 1, "MMM_OPEN", "LAIR" },
    { 0x38, 1, "RBB_OPEN", "LAIR" },
    { 0x39, 1, "CCW_OPEN", "LAIR" },
    // ── LAIR: note doors ──
    { 0x3A, 1, "NOTE_DOOR_50_OPEN", "LAIR" },
    { 0x3B, 1, "NOTE_DOOR_180_OPEN", "LAIR" },
    { 0x3C, 1, "NOTE_DOOR_260_OPEN", "LAIR" },
    { 0x3D, 1, "NOTE_DOOR_350_OPEN", "LAIR" },
    { 0x3E, 1, "NOTE_DOOR_450_OPEN", "LAIR" },
    { 0x3F, 1, "NOTE_DOOR_640_OPEN", "LAIR" },
    { 0x40, 1, "NOTE_DOOR_765_OPEN", "LAIR" },
    { 0x41, 1, "NOTE_DOOR_810_OPEN", "LAIR" },
    { 0x42, 1, "NOTE_DOOR_828_OPEN", "LAIR" },
    { 0x43, 1, "NOTE_DOOR_846_OPEN", "LAIR" },
    { 0x44, 1, "NOTE_DOOR_864_OPEN", "LAIR" },
    { 0x45, 1, "NOTE_DOOR_882_OPEN", "LAIR" },
    // ── Witch switches (continued) ──
    { 0x46, 1, "CCW_WITCH_SWITCH_JIGGY_PRESSED", "CCW" },
    { 0x47, 1, "FP_WITCH_SWITCH_JIGGY_PRESSED", "FP" },
    { 0x48, 1, "FP_WITCH_SWITCH_ADVENT_DOOR_OPEN", "FP" },
    // ── LAIR: warp cauldrons ──
    { 0x49, 1, "PINK_CAULDRON_1_ACTIVE", "LAIR" },
    { 0x4A, 1, "PINK_CAULDRON_2_ACTIVE", "LAIR" },
    { 0x4B, 1, "GREEN_CAULDRON_1_ACTIVE", "LAIR" },
    { 0x4C, 1, "GREEN_CAULDRON_2_ACTIVE", "LAIR" },
    { 0x4D, 1, "RED_CAULDRON_1_ACTIVE", "LAIR" },
    { 0x4E, 1, "RED_CAULDRON_2_ACTIVE", "LAIR" },
    { 0x4F, 1, "UNUSED_CAULDRON_1_ACTIVE", "LAIR" },
    { 0x50, 1, "UNUSED_CAULDRON_2_ACTIVE", "LAIR" },
    { 0x51, 1, "YELLOW_CAULDRON_1_ACTIVE", "LAIR" },
    { 0x52, 1, "YELLOW_CAULDRON_2_ACTIVE", "LAIR" },
    // ── LAIR: CCW puzzle podium ──
    { 0x53, 1, "CCW_PUZZLE_PODIUM_SWITCH_PRESSED", "LAIR" },
    { 0x54, 1, "CCW_PUZZLE_PODIUM_ACTIVE", "LAIR" },
    // ── FINALE: Furnace Fun ──
    { 0x55, 1, "FF_BK_SQUARE_INSTRUCTIONS", "FINALE" },
    { 0x56, 1, "FF_PICTURE_SQUARE_INSTRUCTIONS", "FINALE" },
    { 0x57, 1, "FF_MUSIC_SQUARE_INSTRUCTIONS", "FINALE" },
    { 0x58, 1, "FF_MINIGAME_SQUARE_INSTRUCTIONS", "FINALE" },
    { 0x59, 1, "FF_GRUNTY_SQUARE_INSTRUCTIONS", "FINALE" },
    { 0x5A, 1, "FF_DEATH_SQUARE_INSTRUCTIONS", "FINALE" },
    { 0x5B, 1, "FF_JOKER_SQUARE_INSTRUCTIONS", "FINALE" },
    { 0x5C, 1, "FF_PATTERN_SET", "FINALE" },
    // ── LAIR: puzzle pieces placed (set at puzzle podiums) ──
    { 0x5D, 1, "MM_PUZZLE_PIECES_PLACED", "LAIR" },
    { 0x5E, 2, "TTC_PUZZLE_PIECES_PLACED", "LAIR" },
    { 0x60, 3, "CC_PUZZLE_PIECES_PLACED", "LAIR" },
    { 0x63, 3, "BGS_PUZZLE_PIECES_PLACED", "LAIR" },
    { 0x66, 4, "FP_PUZZLE_PIECES_PLACED", "LAIR" },
    { 0x6A, 4, "GV_PUZZLE_PIECES_PLACED", "LAIR" },
    { 0x6E, 4, "MMM_PUZZLE_PIECES_PLACED", "LAIR" },
    { 0x72, 4, "RBB_PUZZLE_PIECES_PLACED", "LAIR" },
    { 0x76, 4, "CCW_PUZZLE_PIECES_PLACED", "LAIR" },
    { 0x7A, 5, "DOG_PUZZLE_PIECES_PLACED", "LAIR" },
    { 0x7F, 3, "DOUBLE_HEALTH_PUZZLE_PIECES_PLACED", "LAIR" },
    // ── FP ──
    { 0x82, 1, "MET_TWINKLIES", "FP" },
    // ── General ──
    { 0x83, 1, "MAGIC_GET_WEAK_TEXT", nullptr },
    { 0x84, 1, "MAGIC_ALL_GONE_TEXT", nullptr },
    // ── MMM ──
    { 0x86, 1, "HAS_TOUCHED_MMM_THORN_HEDGE", "MMM" },
    { 0x88, 1, "TRIED_LOGGO_AS_BEAR", "MMM" },
    { 0x89, 1, "ENTERED_LOGGO_AS_PUMPKIN", "MMM" },
    { 0x8A, 1, "EXITED_LOGGO", "MMM" },
    // ── CCW ──
    { 0x8B, 1, "CCW_SPRING_OPEN", "CCW" },
    { 0x8C, 1, "CCW_SUMMER_OPEN", "CCW" },
    { 0x8D, 1, "CCW_AUTUMN_OPEN", "CCW" },
    { 0x8E, 1, "CCW_WINTER_OPEN", "CCW" },
    { 0x8F, 1, "MET_BEE_INFESTED_BEEHIVE", "CCW" },
    // ── Mumbo transform costs (set in respective worlds) ──
    { 0x90, 1, "PAID_TERMITE_COST", "MM" },
    { 0x91, 1, "PAID_PUMPKIN_COST", "MMM" },
    { 0x92, 1, "PAID_WALRUS_COST", "FP" },
    { 0x93, 1, "PAID_CROC_COST", "BGS" },
    { 0x94, 1, "PAID_BEE_COST", "CCW" },
    { 0x95, 1, "UNKNOWN_95", nullptr },
    // ── LAIR: Brentilda, text ──
    { 0x96, 1, "MET_BRENTILDA", "LAIR" },
    { 0x97, 1, "ENTERED_LAIR_TEXT", "LAIR" },
    { 0x98, 1, "EXITED_LEVEL_TEXT", "LAIR" },
    { 0x99, 1, "PAST_50_NOTE_DOOR_TEXT", "LAIR" },
    // ── CC ──
    { 0x9A, 1, "CC_WITCH_SWITCH_PRESSED", "CC" },
    // ── LAIR: CC witch switch eyes ──
    { 0x9B, 1, "LAIR_CC_WITCH_SWITCH_EYES_ACTIVE", "LAIR" },
    { 0x9C, 1, "LAIR_CC_WITCH_SWITCH_LEFT_EYE_PRESSED", "LAIR" },
    { 0x9D, 1, "LAIR_CC_WITCH_SWITCH_RIGHT_EYE_PRESSED", "LAIR" },
    { 0x9E, 1, "CRYPT_COFFIN_LID_OPEN", "LAIR" },
    // ── BGS ──
    { 0x9F, 1, "BGS_WITCH_SWITCH_JIGGY_PRESSED", "BGS" },
    // ── GV ──
    { 0xA0, 1, "GV_WITCH_SWITCH_JIGGY_PRESSED", "GV" },
    // ── LAIR ──
    { 0xA1, 1, "STATUE_HAT_OPEN", "LAIR" },
    { 0xA2, 1, "GV_LOBBY_COFFIN_OPEN", "LAIR" },
    // ── GV ──
    { 0xA3, 1, "GV_SNS_SWITCH_PRESSED", "GV" },
    { 0xA4, 1, "GV_SNS_SARCOPHAGUS_OPEN", "GV" },
    // ── LAIR ──
    { 0xA5, 1, "LAIR_CRYPT_GATE_OPEN", "LAIR" },
    // ── FINALE ──
    { 0xA6, 1, "FURNACE_FUN_COMPLETE", "FINALE" },
    // ── LAIR ──
    { 0xA7, 1, "NEAR_PUZZLE_PODIUM_TEXT", "LAIR" },
    // ── General ──
    { 0xA8, 1, "HAS_DIED", nullptr },
    // ── RBB ──
    { 0xA9, 1, "HAS_TOUCHED_RBB_OVEN", "RBB" },
    // ── CCW ──
    { 0xAA, 1, "HAS_TOUCHED_CCW_BRAMBLE_FIELD", "CCW" },
    // ── RBB ──
    { 0xAB, 1, "SWIM_OILY_WATER", "RBB" },
    { 0xAC, 1, "DIVE_OILY_WATER", "RBB" },
    // ── LAIR: Cheato ──
    { 0xAD, 1, "CHEATO_BLUEEGGS_UNLOCKED", "LAIR" },
    { 0xAE, 1, "CHEATO_REDFEATHERS_UNLOCKED", "LAIR" },
    { 0xAF, 1, "CHEATO_GOLDFEATHERS_UNLOCKED", "LAIR" },
    // ── HAS_ENTERED (set in respective worlds) ──
    { 0xB0, 1, "HAS_ENTERED_MM", "MM" },
    { 0xB1, 1, "HAS_ENTERED_BGS", "BGS" },
    { 0xB2, 1, "HAS_ENTERED_TTC", "TTC" },
    { 0xB3, 1, "HAS_ENTERED_GV", "GV" },
    { 0xB4, 1, "HAS_ENTERED_RBB", "RBB" },
    { 0xB5, 1, "HAS_ENTERED_CCW", "CCW" },
    { 0xB6, 1, "HAS_ENTERED_FP", "FP" },
    { 0xB7, 1, "HAS_ENTERED_MMM", "MMM" },
    { 0xB8, 1, "HAS_ENTERED_CC", "CC" },
    // ── LAIR ──
    { 0xB9, 1, "DOUBLE_HEALTH", "LAIR" },
    // ── General ──
    { 0xBA, 1, "HAS_SEEN_TREX_TEXT", nullptr },
    { 0xBB, 2, "MUMBO_MISTAKE_INDEX", nullptr },
    // ── LAIR ──
    { 0xBD, 1, "ENTER_LAIR_CUTSCENE", "LAIR" },
    { 0xBE, 1, "CHEATO_BLUEEGGS", "LAIR" },
    { 0xBF, 1, "CHEATO_REDFEATHERS", "LAIR" },
    { 0xC0, 1, "CHEATO_GOLDFEATHERS", "LAIR" },
    // ── General ──
    { 0xC1, 1, "BADDIES_ESCAPE_TEXT", nullptr },
    // ── LAIR ──
    { 0xC2, 1, "GRATE_TO_RBB_PUZZLE_OPEN", "LAIR" },
    { 0xC3, 1, "ICE_BALL_TO_CHEATO_BROKEN", "LAIR" },
    { 0xC4, 1, "STATUE_EYE_BROKEN", "LAIR" },
    { 0xC5, 1, "RAREWARE_BOX_BROKEN", "LAIR" },
    { 0xC6, 1, "LAIR_JUMP_PAD_SWITCH_PRESSED", "LAIR" },
    { 0xC7, 1, "LAIR_JUMP_PAD_ACTIVE", "LAIR" },
    { 0xC8, 1, "LAIR_BRICKWALL_TO_WADINGBOOTS_BROKEN", "LAIR" },
    { 0xC9, 1, "LAIR_BRICKWALL_TO_SHOCKJUMP_PAD_BROKEN", "LAIR" },
    { 0xCA, 1, "COBWEB_BLOCKING_PURPLE_CAULDRON_BROKEN", "LAIR" },
    { 0xCB, 1, "LAIR_COBWEB_OVER_FLIGHTPAD_BROKEN", "LAIR" },
    { 0xCC, 1, "LAIR_COBWEB_OVER_GREEN_CAULDRON_BROKEN", "LAIR" },
    { 0xCD, 1, "GRATE_TO_WATER_SWITCH_3_OPEN", "LAIR" },
    { 0xCE, 1, "GRATE_TO_MMM_PUZZLE_OPEN", "LAIR" },
    // ── FINALE ──
    { 0xCF, 1, "HAS_ENTERED_FINAL_FIGHT", "FINALE" },
    { 0xD1, 1, "HAS_ACTIVATED_A_JINJO_STATUE_IN_FINAL_FIGHT", "FINALE" },
    { 0xD2, 1, "HAS_SPAWNED_A_JINJO_STATUE_IN_FINAL_FIGHT", "FINALE" },
    { 0xD3, 8, "FF_PATTERN", "FINALE" },
    // ── SM ──
    { 0xDB, 1, "SKIPPED_TUTORIAL", "SM" },
    // ── General ──
    { 0xDC, 1, "HAS_HAD_ENOUGH_TOKENS_BEFORE", nullptr },
    // ── CCW ──
    { 0xDD, 1, "HAS_TOUCHED_CCW_ICY_WATER", "CCW" },
    // ── LAIR ──
    { 0xDE, 1, "USED_ALL_YOUR_PUZZLE_PIECES", "LAIR" },
    { 0xDF, 1, "CAN_REMOVE_ALL_PUZZLE_PIECES", "LAIR" },
    { 0xE0, 1, "CAN_PLACE_ALL_PUZZLE_PIECES", "LAIR" },
    // ── General ──
    { 0xE1, 1, "UNKNOWN_E1", nullptr },
    // ── LAIR ──
    { 0xE2, 1, "DOOR_OF_GRUNTY_OPEN", "LAIR" },
    // ── CCW ──
    { 0xE3, 1, "CCW_FLOWER_SPRING", "CCW" },
    { 0xE4, 1, "CCW_FLOWER_SUMMER", "CCW" },
    { 0xE5, 1, "CCW_FLOWER_AUTUMN", "CCW" },
    { 0xE6, 1, "SPRING_EYRIE_HATCHED", "CCW" },
    { 0xE7, 1, "SUMMER_EYRIE_FED", "CCW" },
    { 0xE8, 1, "AUTUMN_EYRIE_FED", "CCW" },
    // ── LAIR: Brentilda heals ──
    { 0xE9, 1, "HEALED_BY_BRENTILDA_1", "LAIR" },
    { 0xEA, 1, "HEALED_BY_BRENTILDA_2", "LAIR" },
    { 0xEB, 1, "HEALED_BY_BRENTILDA_3", "LAIR" },
    { 0xEC, 1, "HEALED_BY_BRENTILDA_4", "LAIR" },
    { 0xED, 1, "HEALED_BY_BRENTILDA_5", "LAIR" },
    { 0xEE, 1, "HEALED_BY_BRENTILDA_6", "LAIR" },
    { 0xEF, 1, "HEALED_BY_BRENTILDA_7", "LAIR" },
    { 0xF0, 1, "HEALED_BY_BRENTILDA_8", "LAIR" },
    { 0xF1, 1, "HEALED_BY_BRENTILDA_9", "LAIR" },
    { 0xF2, 1, "HEALED_BY_BRENTILDA_10", "LAIR" },
    // ── LAIR ──
    { 0xF3, 1, "MET_DINGPOT", "LAIR" },
    // ── FINALE ──
    { 0xF4, 1, "ENTER_FF_CUTSCENE", "FINALE" },
    // ── LAIR ──
    { 0xF5, 1, "COMPLETED_A_WARP_CAULDRON_SET", "LAIR" },
    { 0xF6, 1, "SEEN_DOOR_OF_GRUNTY_PUZZLE_PODIUM", "LAIR" },
    { 0xF7, 1, "HAS_TRANSFORMED_IN_CRYPT", "LAIR" },
    // ── GV ──
    { 0xF8, 2, "KING_SANDYBUTT_PYRAMID_STATE", "GV" },
    // ── General ──
    { 0xFA, 1, "UNKNOWN_FA", nullptr },
    // ── FINALE ──
    { 0xFC, 1, "DEFEAT_GRUNTY", "FINALE" },
    // ── Cheats: Sandcastle cheat codes (entered in TTC's sandcastle) ──
    { 0xFD, 2, "BANNED_CHEATCODES_ENTERED", "CHEATS" },
    { 0xFF, 1, "SANDCASTLE_OPEN_DOOR_TWO", "CHEATS" },
    { 0x100, 1, "SANDCASTLE_OPEN_DOOR_THREE", "CHEATS" },
    { 0x101, 1, "SANDCASTLE_OPEN_DOOR_FOUR", "CHEATS" },
    { 0x102, 1, "SANDCASTLE_OPEN_DOOR_FIVE", "CHEATS" },
    { 0x103, 1, "SANDCASTLE_OPEN_DOOR_SIX", "CHEATS" },
    { 0x104, 1, "SANDCASTLE_OPEN_DOOR_SEVEN", "CHEATS" },
    { 0x105, 1, "SANDCASTLE_PUZZLE_COMPLETE_CC", "CHEATS" },
    { 0x106, 1, "SANDCASTLE_PUZZLE_COMPLETE_BGS", "CHEATS" },
    { 0x107, 1, "SANDCASTLE_PUZZLE_COMPLETE_FP", "CHEATS" },
    { 0x108, 1, "SANDCASTLE_PUZZLE_COMPLETE_GV", "CHEATS" },
    { 0x109, 1, "SANDCASTLE_PUZZLE_COMPLETE_MMM", "CHEATS" },
    { 0x10A, 1, "SANDCASTLE_PUZZLE_COMPLETE_RBB", "CHEATS" },
    { 0x10B, 1, "SANDCASTLE_PUZZLE_COMPLETE_CCC", "CHEATS" },
    { 0x10C, 1, "SANDCASTLE_RAISE_PIPES_TO_CC", "CHEATS" },
    { 0x10D, 1, "SANDCASTLE_RAISE_PIPE_TO_BRENTILDA", "CHEATS" },
    { 0x10E, 1, "SANDCASTLE_OPEN_CC", "CHEATS" },
    { 0x10F, 1, "SANDCASTLE_REMOVE_GRILL_NEAR_BGS_JIGGY", "CHEATS" },
    { 0x110, 1, "SANDCASTLE_CCC_JIGGY_PODIUM", "CHEATS" },
    { 0x111, 1, "SANDCASTLE_REMOVE_GRILL_AND_HAT_FROM_STATUE", "CHEATS" },
    { 0x112, 1, "SANDCASTLE_REMOVE_ICE", "CHEATS" },
    { 0x113, 1, "SANDCASTLE_OPEN_BGS", "CHEATS" },
    { 0x114, 1, "SANDCASTLE_REMOVE_BREAKABLE_WALLS", "CHEATS" },
    { 0x115, 1, "SANDCASTLE_SHOCKSPRING_JUMP_UNLOCKED", "CHEATS" },
    { 0x116, 1, "SANDCASTLE_OPEN_GV", "CHEATS" },
    { 0x117, 1, "SANDCASTLE_REMOVE_WEBS", "CHEATS" },
    { 0x118, 1, "SANDCASTLE_REMOVE_GLASS_EYE", "CHEATS" },
    { 0x119, 1, "SANDCASTLE_FLIGHT_UNLOCKED", "CHEATS" },
    { 0x11A, 1, "SANDCASTLE_OPEN_FP", "CHEATS" },
    { 0x11B, 1, "SANDCASTLE_OPEN_MMM", "CHEATS" },
    { 0x11C, 1, "SANDCASTLE_REMOVE_CRYPT_GATE", "CHEATS" },
    { 0x11D, 1, "SANDCASTLE_REMOVE_CRYPT_COFFIN_LID", "CHEATS" },
    { 0x11E, 1, "SANDCASTLE_REMOVE_GRATE_NEAR_WATER_SWITCH", "CHEATS" },
    { 0x11F, 1, "SANDCASTLE_OPEN_RBB", "CHEATS" },
    { 0x120, 1, "SANDCASTLE_REMOVE_GRILL_NEAR_RBB_JIGGY", "CHEATS" },
    { 0x121, 1, "SANDCASTLE_REMOVE_TUNNEL_GRILL_NEAR_RBB_JIGGY", "CHEATS" },
    { 0x122, 1, "SANDCASTLE_OPEN_CCW", "CHEATS" },
    { 0x123, 1, "CHEAT_ENTERED", "CHEATS" },
};
static constexpr int kProgressFlagCount = sizeof(kProgressFlags) / sizeof(kProgressFlags[0]);

// ─── Ability Names ──────────────────────────────────────────────────────────
static const char* kAbilityNames[] = {
    "BARGE",              // 0
    "BEAK_BOMB",          // 1
    "BEAK_BUSTER",        // 2
    "CAMERA_CONTROL",     // 3
    "CLAW_SWIPE",         // 4
    "CLIMB",              // 5
    "EGGS",               // 6
    "FEATHERY_FLAP",      // 7
    "FLAP_FLIP",          // 8
    "FLIGHT",             // 9
    "HOLD_A_JUMP_HIGHER", // 10
    "RATATAT_RAP",        // 11
    "ROLL",               // 12
    "SHOCK_JUMP",         // 13
    "WADING_BOOTS",       // 14
    "DIVE",               // 15
    "TALON_TROT",         // 16
    "TURBO_TALON",        // 17
    "WONDERWING",         // 18
    "FIRST_NOTEDOOR",     // 19
};
static constexpr int kAbilityCount = sizeof(kAbilityNames) / sizeof(kAbilityNames[0]);

// ─── Stop 'N' Swop Bit Definitions ─────────────────────────────────────────
// Bit positions match the StopNSwop_Bitfield struct layout (LSB-first on LE).
// Organized as unlocked/collected subtrees with 7 items each.

struct SnsBitDef {
    int bit;
    const char* name;
};

// Unlocked flags (bits 0-6): item is visible and collectible
static const SnsBitDef kSnsUnlocked[] = {
    { 0, "eggYellow" }, { 1, "eggRed" },  { 2, "eggGreen" }, { 3, "eggBlue" },
    { 4, "eggPink" },   { 5, "eggCyan" }, { 6, "iceKey" },
};

// Collected flags (bits 7-13): item was picked up
static const SnsBitDef kSnsCollected[] = {
    { 7, "eggYellow" }, { 8, "eggRed" },   { 9, "eggGreen" }, { 10, "eggBlue" },
    { 11, "eggPink" },  { 12, "eggCyan" }, { 13, "iceKey" },
};
static constexpr int kSnsItemCount = sizeof(kSnsUnlocked) / sizeof(kSnsUnlocked[0]);

// ─── Bitfield Helpers ───────────────────────────────────────────────────────
// Match the decomp's bitfield_get_bit / bitfield_set_bit layout exactly.

static int BitfieldGetBit(const uint8_t* array, int index) {
    return (array[index / 8] & (1 << (index & 7))) ? 1 : 0;
}

static int BitfieldGetNBits(const uint8_t* array, int offset, int numBits) {
    int ret = 0;
    for (int i = 0; i < numBits; i++) {
        ret |= (BitfieldGetBit(array, offset + i) << i);
    }
    return ret;
}

static void BitfieldSetBit(uint8_t* array, int index, int set) {
    if (set) {
        array[index / 8] |= (1 << (index & 7));
    } else {
        array[index / 8] &= ~(1 << (index & 7));
    }
}

static void BitfieldSetNBits(uint8_t* array, int startIndex, int set, int length) {
    for (int i = 0; i < length; i++) {
        BitfieldSetBit(array, startIndex + i, (1 << i) & set);
    }
}

// ─── Singleton ──────────────────────────────────────────────────────────────

SaveManager& SaveManager::Instance() {
    static SaveManager instance;
    return instance;
}

SaveManager::SaveManager() : mLoaded(false) {
    memset(mEeprom, 0, sizeof(mEeprom));
}

// ─── EEPROM Interface ───────────────────────────────────────────────────────

int SaveManager::ReadBlocks(int file, int offset, void* buffer, int count) {
    if (!mLoaded) {
        LoadFromDisk();
        mLoaded = true;
    }

    int absoluteBlock = file * SAVE_SLOT_BLOCKS + offset;
    int byteOffset = absoluteBlock * EEPROM_BLOCK_SIZE;
    int byteCount = count * EEPROM_BLOCK_SIZE;

    if (byteOffset + byteCount > EEPROM_TOTAL_SIZE) {
        SPDLOG_ERROR("[SaveManager] ReadBlocks out of bounds: file={} offset={} count={}", file, offset, count);
        return 1;
    }

    memcpy(buffer, mEeprom + byteOffset, byteCount);
    return 0;
}

int SaveManager::WriteBlocks(int file, int offset, void* buffer, int count) {
    if (!mLoaded) {
        LoadFromDisk();
        mLoaded = true;
    }

    int absoluteBlock = file * SAVE_SLOT_BLOCKS + offset;
    int byteOffset = absoluteBlock * EEPROM_BLOCK_SIZE;
    int byteCount = count * EEPROM_BLOCK_SIZE;

    if (byteOffset + byteCount > EEPROM_TOTAL_SIZE) {
        SPDLOG_ERROR("[SaveManager] WriteBlocks out of bounds: file={} offset={} count={}", file, offset, count);
        return 1;
    }

    memcpy(mEeprom + byteOffset, buffer, byteCount);

    // Flush the appropriate JSON file
    if (offset == GLOBAL_OFFSET_BLOCK) {
        FlushGlobalToDisk();
    } else {
        // Check slotIndex (byte 1) — if 1-3, the slot is live; flush it.
        int slotBase = file * SAVE_SLOT_SIZE;
        uint8_t slotIndex = mEeprom[slotBase + 1];

        if (slotIndex >= 1 && slotIndex <= 3) {
            FlushSlotToDisk(slotIndex);
        } else {
            // Slot was cleared (slotIndex 0) — remove JSON for any game slot no longer in EEPROM
            bool gameFilePresent[4] = { false, false, false, false };
            for (int i = 0; i < SAVE_SLOT_COUNT; i++) {
                int base = i * SAVE_SLOT_SIZE;
                uint8_t si = mEeprom[base + 1];
                if (si >= 1 && si <= 3) {
                    gameFilePresent[si] = true;
                }
            }
            for (int si = 1; si <= 3; si++) {
                if (!gameFilePresent[si]) {
                    std::string path = GetSavePath("file" + std::to_string(SlotToVisualGame(si)) + ".json");
                    if (fs::exists(path)) {
                        fs::remove(path);
                        SPDLOG_INFO("[SaveManager] Deleted {}", path);
                    }
                }
            }
        }
    }

    return 0;
}

// ─── Disk I/O ───────────────────────────────────────────────────────────────

std::string SaveManager::GetSavePath(const std::string& filename) {
    // Romhack saves go into saves/<romName>/ (e.g. saves/bk-jot/file1.json)
    // Vanilla saves stay in saves/ (e.g. saves/file1.json)
    const char* romName = port_getRomhackName();
    if (romName && romName[0] != '\0') {
        return Ship::Context::GetPathRelativeToAppDirectory("saves/" + std::string(romName) + "/" + filename);
    }
    return Ship::Context::GetPathRelativeToAppDirectory("saves/" + filename);
}

void SaveManager::EnsureDirectory() {
    std::string dir = GetSavePath("");
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }
}

// ─── Slot → JSON (per-world named fields) ───────────────────────────────────

json SaveManager::SlotToJson(const uint8_t* slotData) {
    json j;
    j["slotIndex"] = static_cast<int>(slotData[1]);
    j["version"] = SAVE_VERSION;

    // ── File data (everything the game serializes) ──
    json file = json::object();

    // Unpack note scores into temporary array
    int noteScores[9] = {};
    {
        uint64_t packed = 0;
        memcpy(&packed, slotData + NOTE_OFFSET, sizeof(uint64_t));
        for (int i = 8; i >= 0; i--) {
            noteScores[i] = static_cast<int>(packed & 0x7F);
            packed >>= 7;
        }
    }

    const uint8_t* progressFlags = slotData + PROGRESS_OFFSET;

    // ── Abilities ──
    {
        const uint8_t* data = slotData + ABILITY_OFFSET;
        uint32_t learned, used;
        memcpy(&learned, data, sizeof(uint32_t));
        memcpy(&used, data + 4, sizeof(uint32_t));

        json learnedObj = json::object();
        json usedObj = json::object();
        for (int i = 0; i < kAbilityCount; i++) {
            learnedObj[kAbilityNames[i]] = (learned & (1u << i)) ? 1 : 0;
            usedObj[kAbilityNames[i]] = (used & (1u << i)) ? 1 : 0;
        }
        json abilities = json::object();
        abilities["learned"] = learnedObj;
        abilities["used"] = usedObj;
        file["abilities"] = abilities;
    }

    // ── General progress flags (world == nullptr) ──
    {
        json general = json::object();
        for (int i = 0; i < kProgressFlagCount; i++) {
            const auto& f = kProgressFlags[i];
            if (f.world != nullptr) {
                continue;
            }
            if (f.bitWidth == 1) {
                general[f.name] = BitfieldGetBit(progressFlags, f.bitIndex);
            } else {
                general[f.name] = BitfieldGetNBits(progressFlags, f.bitIndex, f.bitWidth);
            }
        }
        file["progress"] = general;
    }

    // ── Sandcastle cheat flags (world == "CHEATS") ──
    {
        json cheats = json::object();
        for (int i = 0; i < kProgressFlagCount; i++) {
            const auto& f = kProgressFlags[i];
            if (f.world == nullptr || strcmp(f.world, "CHEATS") != 0) {
                continue;
            }
            if (f.bitWidth == 1) {
                cheats[f.name] = BitfieldGetBit(progressFlags, f.bitIndex);
            } else {
                cheats[f.name] = BitfieldGetNBits(progressFlags, f.bitIndex, f.bitWidth);
            }
        }
        file["cheats"] = cheats;
    }

    // ── Saved items ──
    {
        const uint8_t* data = slotData + ITEMS_OFFSET;
        json obj = json::object();
        obj["mumboTokens"] = static_cast<int>(data[0]);
        obj["eggs"] = static_cast<int>(data[1]);
        obj["redFeathers"] = static_cast<int>(data[2]);
        obj["goldFeathers"] = static_cast<int>(data[3]);
        obj["jiggyTotal"] = static_cast<int>(data[4]);
        file["savedItems"] = obj;
    }

    // ── Build per-world sections ──
    json worlds = json::object();
    for (int w = 0; w < kWorldCount; w++) {
        const auto& wd = kWorlds[w];
        json world = json::object();

        // Honeycombs (array of 0/1)
        if (wd.honeycombCount > 0) {
            json arr = json::array();
            const uint8_t* data = slotData + HONEYCOMB_OFFSET;
            for (int i = 0; i < wd.honeycombCount; i++) {
                int id = wd.honeycombStart + i;
                arr.push_back((data[(id - 1) / 8] & (1 << (id & 7))) ? 1 : 0);
            }
            world["honeycombs"] = arr;
        }

        // Jiggies (array of 0/1)
        if (wd.jiggyCount > 0) {
            json arr = json::array();
            const uint8_t* data = slotData + JIGGY_OFFSET;
            for (int i = 0; i < wd.jiggyCount; i++) {
                int id = wd.jiggyStart + i;
                arr.push_back((data[(id - 1) / 8] & (1 << (id & 7))) ? 1 : 0);
            }
            world["jiggies"] = arr;
        }

        // Mumbo tokens (array of 0/1)
        if (wd.mumboCount > 0) {
            json arr = json::array();
            const uint8_t* data = slotData + MUMBO_OFFSET;
            for (int i = 0; i < wd.mumboCount; i++) {
                int id = wd.mumboStart + i;
                arr.push_back((data[(id - 1) / 8] & (1 << (id & 7))) ? 1 : 0);
            }
            world["mumboTokens"] = arr;
        }

        // Note high score
        if (wd.hasNoteScore) {
            int score = 0;
            for (int i = 0; i < 9; i++) {
                if (kNoteScoreWorlds[i] == wd.levelId) {
                    score = noteScores[i];
                    break;
                }
            }
            world["noteScore"] = score;
        }

        // Progress flags belonging to this world
        json progress = json::object();
        for (int i = 0; i < kProgressFlagCount; i++) {
            const auto& f = kProgressFlags[i];
            if (f.world == nullptr || strcmp(f.world, wd.name) != 0) {
                continue;
            }
            if (f.bitWidth == 1) {
                progress[f.name] = BitfieldGetBit(progressFlags, f.bitIndex);
            } else {
                progress[f.name] = BitfieldGetNBits(progressFlags, f.bitIndex, f.bitWidth);
            }
        }
        if (!progress.empty()) {
            world["progress"] = progress;
        }

        // Time score
        if (wd.hasTimeScore) {
            const uint8_t* data = slotData + TIME_OFFSET;
            int idx = wd.levelId - 1;
            uint16_t score = 0;
            memcpy(&score, data + idx * 2, sizeof(uint16_t));
            world["timeScore"] = static_cast<int>(score);
        }

        worlds[wd.name] = world;
    }
    file["worlds"] = worlds;

    j["file"] = file;
    return j;
}

// ─── JSON → Slot (reconstruct binary from per-world JSON) ───────────────────

void SaveManager::JsonToSlot(const json& j, uint8_t* slotData) {
    memset(slotData, 0, SAVE_SLOT_SIZE);

    slotData[0] = SAVE_MAGIC;
    slotData[1] = static_cast<uint8_t>(j["slotIndex"].get<int>());

    // Support both wrapped ("file":{...}) and legacy flat format
    const auto& f = j.contains("file") ? j["file"] : j;

    // ── Per-world sections ──
    int noteScores[9] = {};

    if (f.contains("worlds")) {
        const auto& worlds = f["worlds"];

        for (int w = 0; w < kWorldCount; w++) {
            const auto& wd = kWorlds[w];
            if (!worlds.contains(wd.name)) {
                continue;
            }
            const auto& world = worlds[wd.name];

            // Jiggies
            if (wd.jiggyCount > 0 && world.contains("jiggies")) {
                uint8_t* data = slotData + JIGGY_OFFSET;
                const auto& arr = world["jiggies"];
                for (int i = 0; i < wd.jiggyCount && i < static_cast<int>(arr.size()); i++) {
                    if (arr[i].get<int>()) {
                        int id = wd.jiggyStart + i;
                        data[(id - 1) / 8] |= (1 << (id & 7));
                    }
                }
            }

            // Honeycombs
            if (wd.honeycombCount > 0 && world.contains("honeycombs")) {
                uint8_t* data = slotData + HONEYCOMB_OFFSET;
                const auto& arr = world["honeycombs"];
                for (int i = 0; i < wd.honeycombCount && i < static_cast<int>(arr.size()); i++) {
                    if (arr[i].get<int>()) {
                        int id = wd.honeycombStart + i;
                        data[(id - 1) / 8] |= (1 << (id & 7));
                    }
                }
            }

            // Mumbo tokens
            if (wd.mumboCount > 0 && world.contains("mumboTokens")) {
                uint8_t* data = slotData + MUMBO_OFFSET;
                const auto& arr = world["mumboTokens"];
                for (int i = 0; i < wd.mumboCount && i < static_cast<int>(arr.size()); i++) {
                    if (arr[i].get<int>()) {
                        int id = wd.mumboStart + i;
                        data[(id - 1) / 8] |= (1 << (id & 7));
                    }
                }
            }

            // Note score (collect into array, pack after loop)
            if (wd.hasNoteScore && world.contains("noteScore")) {
                int score = world["noteScore"].get<int>();
                for (int i = 0; i < 9; i++) {
                    if (kNoteScoreWorlds[i] == wd.levelId) {
                        noteScores[i] = score;
                        break;
                    }
                }
            }

            // Time score
            if (wd.hasTimeScore && world.contains("timeScore")) {
                uint8_t* data = slotData + TIME_OFFSET;
                int idx = wd.levelId - 1;
                uint16_t score = static_cast<uint16_t>(world["timeScore"].get<int>());
                memcpy(data + idx * 2, &score, sizeof(uint16_t));
            }

            // Progress flags for this world
            if (world.contains("progress")) {
                uint8_t* flags = slotData + PROGRESS_OFFSET;
                const auto& progress = world["progress"];
                for (int i = 0; i < kProgressFlagCount; i++) {
                    const auto& pf = kProgressFlags[i];
                    if (pf.world == nullptr || strcmp(pf.world, wd.name) != 0) {
                        continue;
                    }
                    auto it = progress.find(pf.name);
                    if (it == progress.end()) {
                        continue;
                    }
                    if (pf.bitWidth == 1) {
                        BitfieldSetBit(flags, pf.bitIndex, it->get<int>());
                    } else {
                        BitfieldSetNBits(flags, pf.bitIndex, it->get<int>(), pf.bitWidth);
                    }
                }
            }
        }
    }

    // Pack note scores into u64
    {
        uint64_t packed = 0;
        for (int i = 0; i < 9; i++) {
            packed <<= 7;
            packed |= (noteScores[i] & 0x7F);
        }
        memcpy(slotData + NOTE_OFFSET, &packed, sizeof(uint64_t));
    }

    // ── General progress flags (world == nullptr) ──
    if (f.contains("progress")) {
        uint8_t* flags = slotData + PROGRESS_OFFSET;
        const auto& progress = f["progress"];
        for (int i = 0; i < kProgressFlagCount; i++) {
            const auto& pf = kProgressFlags[i];
            if (pf.world != nullptr) {
                continue;
            }
            auto it = progress.find(pf.name);
            if (it == progress.end()) {
                continue;
            }
            if (pf.bitWidth == 1) {
                BitfieldSetBit(flags, pf.bitIndex, it->get<int>());
            } else {
                BitfieldSetNBits(flags, pf.bitIndex, it->get<int>(), pf.bitWidth);
            }
        }
    }

    // ── Sandcastle cheat flags (world == "CHEATS") ──
    if (f.contains("cheats")) {
        uint8_t* flags = slotData + PROGRESS_OFFSET;
        const auto& cheats = f["cheats"];
        for (int i = 0; i < kProgressFlagCount; i++) {
            const auto& pf = kProgressFlags[i];
            if (pf.world == nullptr || strcmp(pf.world, "CHEATS") != 0) {
                continue;
            }
            auto it = cheats.find(pf.name);
            if (it == cheats.end()) {
                continue;
            }
            if (pf.bitWidth == 1) {
                BitfieldSetBit(flags, pf.bitIndex, it->get<int>());
            } else {
                BitfieldSetNBits(flags, pf.bitIndex, it->get<int>(), pf.bitWidth);
            }
        }
    }

    // ── Saved items ──
    if (f.contains("savedItems")) {
        uint8_t* data = slotData + ITEMS_OFFSET;
        const auto& si = f["savedItems"];
        if (si.contains("mumboTokens")) {
            data[0] = static_cast<uint8_t>(si["mumboTokens"].get<int>());
        }
        if (si.contains("eggs")) {
            data[1] = static_cast<uint8_t>(si["eggs"].get<int>());
        }
        if (si.contains("redFeathers")) {
            data[2] = static_cast<uint8_t>(si["redFeathers"].get<int>());
        }
        if (si.contains("goldFeathers")) {
            data[3] = static_cast<uint8_t>(si["goldFeathers"].get<int>());
        }
        if (si.contains("jiggyTotal")) {
            data[4] = static_cast<uint8_t>(si["jiggyTotal"].get<int>());
        }
    }

    // ── Abilities ──
    {
        uint32_t learned = 0, used = 0;
        if (f.contains("abilities")) {
            const auto& ab = f["abilities"];
            if (ab.contains("learned")) {
                const auto& al = ab["learned"];
                for (int i = 0; i < kAbilityCount; i++) {
                    auto it = al.find(kAbilityNames[i]);
                    if (it != al.end() && it->get<int>()) {
                        learned |= (1u << i);
                    }
                }
            }
            if (ab.contains("used")) {
                const auto& au = ab["used"];
                for (int i = 0; i < kAbilityCount; i++) {
                    auto it = au.find(kAbilityNames[i]);
                    if (it != au.end() && it->get<int>()) {
                        used |= (1u << i);
                    }
                }
            }
        }
        memcpy(slotData + ABILITY_OFFSET, &learned, sizeof(uint32_t));
        memcpy(slotData + ABILITY_OFFSET + 4, &used, sizeof(uint32_t));
    }

    // Recompute checksum
    savedata_update_crc(slotData, SAVE_SLOT_SIZE);
}

// ─── Load/Flush ─────────────────────────────────────────────────────────────

void SaveManager::LoadFromDisk() {
    memset(mEeprom, 0, sizeof(mEeprom));

    // Load game files (file1.json, file2.json, file3.json)
    for (int i = 1; i <= 3; i++) {
        std::string path = GetSavePath("file" + std::to_string(i) + ".json");
        if (!fs::exists(path)) {
            continue;
        }

        try {
            std::ifstream ifs(path);
            json j = json::parse(ifs);

            if (!j.contains("version") || !j.contains("slotIndex")) {
                SPDLOG_WARN("[SaveManager] Malformed save file: {}", path);
                continue;
            }

            int slotIndex = j["slotIndex"].get<int>();
            int eepromSlot = i - 1;
            int base = eepromSlot * SAVE_SLOT_SIZE;

            JsonToSlot(j, mEeprom + base);
        } catch (const std::exception& e) { SPDLOG_ERROR("[save] Failed to load {}: {}", path, e.what()); }
    }

    // Load global data
    std::string globalPath = GetSavePath("global.json");
    if (fs::exists(globalPath)) {
        try {
            std::ifstream ifs(globalPath);
            json j = json::parse(ifs);

            uint32_t snsRaw = 0;
            if (j.contains("snsItems")) {
                const auto& sns = j["snsItems"];
                if (sns.is_object() && sns.contains("unlocked")) {
                    // Subtree format (v3+): unlocked/collected
                    if (sns.contains("unlocked")) {
                        const auto& u = sns["unlocked"];
                        for (int i = 0; i < kSnsItemCount; i++) {
                            auto it = u.find(kSnsUnlocked[i].name);
                            if (it != u.end() && it->get<int>()) {
                                snsRaw |= (1u << kSnsUnlocked[i].bit);
                            }
                        }
                    }
                    if (sns.contains("collected")) {
                        const auto& c = sns["collected"];
                        for (int i = 0; i < kSnsItemCount; i++) {
                            auto it = c.find(kSnsCollected[i].name);
                            if (it != c.end() && it->get<int>()) {
                                snsRaw |= (1u << kSnsCollected[i].bit);
                            }
                        }
                    }
                } else if (sns.is_number()) {
                    // Legacy raw u32 format (v2)
                    snsRaw = sns.get<uint32_t>();
                }
            }

            int globalBase = GLOBAL_OFFSET_BLOCK * EEPROM_BLOCK_SIZE;
            memset(mEeprom + globalBase, 0, GLOBAL_SIZE);
            memcpy(mEeprom + globalBase, &snsRaw, sizeof(uint32_t));

            savedata_update_crc(mEeprom + globalBase, GLOBAL_SIZE);
        } catch (const std::exception& e) { SPDLOG_ERROR("[SaveManager] Failed to load global.json: {}", e.what()); }
    }
}

void SaveManager::FlushSlotToDisk(int slotIndex) {
    if (slotIndex < 1 || slotIndex > 3) {
        return;
    }

    EnsureDirectory();

    // Find the eeprom slot that holds this slotIndex
    int eepromSlot = -1;
    for (int i = 0; i < SAVE_SLOT_COUNT; i++) {
        int base = i * SAVE_SLOT_SIZE;
        if (mEeprom[base + 1] == slotIndex) {
            eepromSlot = i;
            break;
        }
    }

    if (eepromSlot < 0) {
        SPDLOG_WARN("[SaveManager] FlushSlotToDisk: slotIndex {} not found in eeprom", slotIndex);
        return;
    }

    int base = eepromSlot * SAVE_SLOT_SIZE;

    // Don't write JSON for empty/uninitialized slots (magic 0x11 = valid save)
    if (mEeprom[base] != 0x11) {
        return;
    }

    json j = SlotToJson(mEeprom + base);

    j["file"]["enhancements"]["lives"] = item_getCount(0x16); // ITEM_16_LIFE
    {
        // Bottles bonus: merge with existing JSON — completions are permanent,
        // a zeroed live array (from init) must not downgrade saved 1s to 0s.
        std::string existingFilename = "file" + std::to_string(SlotToVisualGame(slotIndex)) + ".json";
        std::string existingPath = GetSavePath(existingFilename);
        json bbArr = json::array();
        json oldBb;
        if (fs::exists(existingPath)) {
            try {
                std::ifstream ifs(existingPath);
                auto ej = nlohmann::ordered_json::parse(ifs);
                if (ej.contains("file") && ej["file"].contains("enhancements") &&
                    ej["file"]["enhancements"].contains("bottlesBonusCompleted")) {
                    oldBb = ej["file"]["enhancements"]["bottlesBonusCompleted"];
                }
            } catch (...) {}
        }
        for (int k = 0; k < 7; k++) {
            int val = gCompletedBottlesBonusGames[k] ? 1 : 0;
            if (!val && k < (int)oldBb.size() && oldBb[k].get<int>()) {
                val = 1;
            }
            bbArr.push_back(val);
        }
        j["file"]["enhancements"]["bottlesBonusCompleted"] = bbArr;
    }

    std::string filename = "file" + std::to_string(SlotToVisualGame(slotIndex)) + ".json";
    std::string path = GetSavePath(filename);
    std::string tmpPath = path + ".tmp";

    try {
        std::ofstream ofs(tmpPath);
        ofs << JsonDumpCompactArrays(j) << std::endl;
        ofs.close();

        if (fs::exists(path)) {
            fs::remove(path);
        }
        fs::rename(tmpPath, path);
    } catch (const std::exception& e) { SPDLOG_ERROR("[SaveManager] Failed to write {}: {}", filename, e.what()); }
}

void SaveManager::FlushGlobalToDisk() {
    EnsureDirectory();

    int globalBase = GLOBAL_OFFSET_BLOCK * EEPROM_BLOCK_SIZE;

    uint32_t snsRaw = 0;
    memcpy(&snsRaw, mEeprom + globalBase, sizeof(uint32_t));

    json j;
    j["version"] = SAVE_VERSION;

    // Named SNS fields: unlocked/collected subtrees
    json sns = json::object();
    json unlocked = json::object();
    json collected = json::object();
    for (int i = 0; i < kSnsItemCount; i++) {
        unlocked[kSnsUnlocked[i].name] = (snsRaw & (1u << kSnsUnlocked[i].bit)) ? 1 : 0;
        collected[kSnsCollected[i].name] = (snsRaw & (1u << kSnsCollected[i].bit)) ? 1 : 0;
    }
    sns["unlocked"] = unlocked;
    sns["collected"] = collected;
    j["snsItems"] = sns;

    std::string path = GetSavePath("global.json");
    std::string tmpPath = path + ".tmp";

    try {
        std::ofstream ofs(tmpPath);
        ofs << JsonDumpCompactArrays(j) << std::endl;
        ofs.close();

        if (fs::exists(path)) {
            fs::remove(path);
        }
        fs::rename(tmpPath, path);
    } catch (const std::exception& e) { SPDLOG_ERROR("[SaveManager] Failed to write global.json: {}", e.what()); }
}

// ─── C Bridge ───────────────────────────────────────────────────────────────

extern "C" {

int32_t eeprom_readBlocks(int32_t file, int32_t offset, void* buffer, int32_t count) {
    return SaveManager::Instance().ReadBlocks(file, offset, buffer, count);
}

int32_t eeprom_writeBlocks(int32_t file, int32_t offset, void* buffer, int32_t count) {
    return SaveManager::Instance().WriteBlocks(file, offset, buffer, count);
}

void port_restoreFileEnhancementData(int eepromSlot) {
    SaveManager::RestoreFileEnhancementData(eepromSlot);
}

} // extern "C"

void SaveManager::RestoreFileEnhancementData(int eepromSlot) {
    if (eepromSlot < 0 || eepromSlot >= SAVE_SLOT_COUNT) {
        return;
    }

    int base = eepromSlot * SAVE_SLOT_SIZE;
    int slotIndex = Instance().mEeprom[base + 1];
    if (slotIndex < 1 || slotIndex > 3) {
        return;
    }

    std::string filename = "file" + std::to_string(SlotToVisualGame(slotIndex)) + ".json";
    std::string path = Instance().GetSavePath(filename);
    if (!fs::exists(path)) {
        return;
    }

    try {
        std::ifstream ifs(path);
        nlohmann::ordered_json j = nlohmann::ordered_json::parse(ifs);

        if (j.contains("file") && j["file"].contains("enhancements")) {
            const auto& enh = j["file"]["enhancements"];

            if (CVarGetInteger(CVAR_ENHANCEMENT("Saving.PersistExtraLives"), 0)) {
                if (enh.contains("lives")) {
                    D_80385F30[0x16] = enh["lives"].get<int>(); // ITEM_16_LIFE — direct write, safe before HUD init
                }
            }

            if (CVarGetInteger(CVAR_ENHANCEMENT("Saving.PersistBottlesBonus"), 0)) {
                if (enh.contains("bottlesBonusCompleted")) {
                    const auto& bb = enh["bottlesBonusCompleted"];
                    int anyCompleted = 0;
                    for (int k = 0; k < 7 && k < (int)bb.size(); k++) {
                        gCompletedBottlesBonusGames[k] |= bb[k].get<int>() ? 1 : 0;
                        anyCompleted |= gCompletedBottlesBonusGames[k];
                    }
                    if (anyCompleted) {
                        D_8037DCC7 = 1; // skip instructions text
                        D_8037DCC8 = 1; // skip quit lose text
                        D_8037DCC9 = 1; // skip timeout lose text
                        D_8037DCCA = 1; // skip "secret game discovered" text
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[save] Failed to restore file enhancement data from {}: {}", path, e.what());
    }
}
