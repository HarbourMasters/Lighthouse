#include <cstring>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#include <libultraship/libultraship.h>
#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/UI/cvar_prefixes.h"

typedef unsigned char u8;

extern "C" {
int ResourceMgr_IsJapanese(void);
int ResourceMgr_GetDialogLanguageCount(void); // 1 = US, 3 = PAL (EN/FR/DE)
int ResourceMgr_GetDialogLanguage(void);      // PAL only: 0=English, 1=French, 2=German
int ResourceMgr_GetLanguageGeneration(void);
int ResourceMgr_IsAssetRepointed(uint32_t assetId);
const char* ResourceMgr_GetLangString(const char* english); // active pack's translation, or the key itself
int ResourceMgr_HasLangStrings(void);                       // active language is a pack carrying UI overrides

bool gameFile_isNotEmpty(int gamenum);
int jiggyscore_total(void);
int itemscore_noteScores_getTotal(void);
int itemscore_timeScores_getTotal(void);
char* gcpausemenu_TimeToA(int time);
void print_dialog(int x, int y, u8* string);
int gczoombox_setStrings(void* zb, int str_cnt, char** str_ptrs);
int func_803183A4(void* zoombox, char* str); // set a single zoombox string (erase-confirm prompt)
void setGameInformationZoombox(int gamenum); // decomp: (re)assemble the file info zoombox
extern void* chGameSelectTopZoombox;         // GcZoombox*
extern u8* D_80365DF4[];                     // top instruction line 0, indexed by language
extern u8* D_80365DF8[];                     // top instruction line 1, indexed by language
extern u8* D_80365DFC[];                     // erase-confirm prompt, indexed by language
int code94620_func_8031B5B0(void);           // current dialog-language index (0=EN,1=FR,2=DE)
int func_8031877C(void* zoombox);            // clear a zoombox's strings before re-setting

// Print font internals
extern void* D_80380AB8[];   // BKSprite*[5]: font alphamask assets (slot 2 = JP font)
extern void* print_sFonts[]; // FontLetter*[4]: decoded glyph tables per slot
void* print_getLettersFromFont(void* alphaMask, void* textureSprite);
int print_getCurrentMapBoldFontTexture(void);
void print_setBoldFontTexture(int textureId);
void* assetcache_get(int assetId);
void assetcache_release(void* ptr);
void bk_free(void* ptr);
void port_refreshDialogFontGlyphCount(void);
}

// Live language change reactions
namespace {

constexpr int kJpDialogFontAssetId = 0x6EA;    // SPRITE_JP_DIALOG_FONT_ALPHAMASK
constexpr int kDialogFontAssetId = 0x6EB;      // SPRITE_DIALOG_FONT_ALPHAMASK (base/latin, slot 0)
constexpr int kBoldLettersFontAssetId = 0x6EC; // SPRITE_BOLD_FONT_LETTERS_ALPHAMASK (world names, headers)
constexpr int kJpFontFreeDelayFrames = 5;      // grace frames before freeing slot 2
int sFontLanguageGen = 0;
int sJpFontFreeDelay = 0;
bool sDialogFontOverridden = false; // slot 0 currently holds a pack's glyph sheet
bool sBoldFontOverridden = false;   // bold-letters font currently holds a pack's glyph sheet

void FreeJpFontSlot() {
    if (print_sFonts[2] != nullptr) {
        bk_free(print_sFonts[2]);
        print_sFonts[2] = nullptr;
    }
    if (D_80380AB8[2] != nullptr) {
        assetcache_release(D_80380AB8[2]);
        D_80380AB8[2] = nullptr;
    }
}

void LoadJpFontSlot() {
    // Bail early if print_init hasn't run to load this slot yet
    if (D_80380AB8[0] == nullptr) {
        return;
    }
    FreeJpFontSlot(); // drop any stale font first (e.g. a different pack)
    void* tex = assetcache_get(print_getCurrentMapBoldFontTexture());
    D_80380AB8[2] = assetcache_get(kJpDialogFontAssetId);
    print_sFonts[2] = print_getLettersFromFont(D_80380AB8[2], tex);
    assetcache_release(tex);
}

// Re-decode the base dialog font (slot 0) from its current asset.
void ReloadDialogFontSlot() {
    if (D_80380AB8[0] == nullptr) {
        return; // print_init hasn't run yet
    }
    void* tex = assetcache_get(print_getCurrentMapBoldFontTexture());
    assetcache_release(D_80380AB8[0]);
    D_80380AB8[0] = assetcache_get(kDialogFontAssetId);
    bk_free(print_sFonts[0]);
    print_sFonts[0] = print_getLettersFromFont(D_80380AB8[0], tex);
    assetcache_release(tex);
    // A pack may ship an extended dialog font; refresh the reachable glyph count to match.
    port_refreshDialogFontGlyphCount();
}

// Per cached-model-slot generation
std::unordered_map<const void*, int> sModelInfoGen;
std::unordered_set<uint32_t> sEverRepointedModels;

} // namespace

// Decomp pause-menu rebuild hooks
extern "C" {
void gcpausemenu_zoomboxes_free(void);
void gcpausemenu_zoomboxes_initMainMenu(void);
void gcPauseMenu_setState(int state); // enum gcpausemenu_state_e
}

extern "C" int port_pauseMenuNeedsRefresh(void) {
    static int sGen = 0;
    static int sLair = 0;
    static bool sInit = false;
    int gen = ResourceMgr_GetLanguageGeneration();
    int lair = CVarGetInteger(CVAR_ENHANCEMENT("Restorations.ReturnToLair"), 0);
    if (!sInit) {
        sInit = true;
        sGen = gen;
        sLair = lair;
        return 0;
    }
    if (gen == sGen && lair == sLair) {
        return 0;
    }
    sGen = gen;
    sLair = lair;
    return 1;
}

// Rebuild the main pause-menu option zoomboxes and replay the open animation, so they
// re-localize / re-evaluate cleanly (re-setting a live box's string would reveal it as a 2nd line).
// The caller re-evaluates the Return to Lair option (a decomp bitfield) first, before the recreate.
extern "C" void port_pauseMenuRebuild(void) {
    gcpausemenu_zoomboxes_free();
    gcpausemenu_zoomboxes_initMainMenu();
    gcPauseMenu_setState(1); // PAUSE_STATE_1_MENU_OPENING
}

// Minimal view of PrintBuffer (src/core2/font/print.c)
struct PrintBufferView {
    int16_t x, y, topVertexAlpha, bottomVertexAlpha;
    uint8_t fmtString[8];
    float scale;
    uint8_t* string;
    uint8_t rgba[4];
};
extern "C" PrintBufferView* print_sCurrentPtr;

// Override the scale of the most-recently pushed print entry (e.g. right after print_dialog),
// so callers can shrink/grow buffered text without a dedicated scaled print variant. No-op if none.
extern "C" void port_setPrintScale(float scale) {
    if (print_sCurrentPtr != nullptr) {
        print_sCurrentPtr->scale = scale;
    }
}

// JP parade subtitles
struct ParadeKana {
    const char* name;
    const u8* kana;
    int kanaX;
};

static const ParadeKana sParadeKana[] = {
    { "GRUNTILDA", (const u8*)"\xfd\x6a\x7e\x76\x4f\x60\x78\x86", 111 },
    { "TOOTY", (const u8*)"\xfd\x6a\x60\x4c\x3b\x62\x46", 124 },
    { "BOTTLES", (const u8*)"\xfd\x6a\x8f\x63\x78\x83", 123 },
    { "JINJO", (const u8*)"\xfd\x6a\x82\x4f\x82\x4d\x3b", 128 },
    { "MUMBO JUMBO", (const u8*)"\xfd\x6a\x6e\x4f\x8f\x05\x82\x4b\x4f\x8f", 100 },
    { "JU-JU", (const u8*)"\xfd\x6a\x82\x4c\x82\x4c", 129 },
    { "CONGA", (const u8*)"\xfd\x6a\x59\x4f\x7c", 134 },
    { "CHIMPY", (const u8*)"\xfd\x6a\x60\x4f\x91\x3b", 132 },
    { "BRENTILDA", (const u8*)"\xfd\x6a\x8d\x79\x4f\x60\x78\x86", 114 },
    { "CAPTAIN BLUBBER", (const u8*)"\xfd\x6a\x56\x4b\x92\x62\x4f\x05\x8d\x76\x8b\x3b", 83 },
    { "NIPPER", (const u8*)"\xfd\x6a\x65\x4a\x90\x3b", 133 },
    { "CLANKER", (const u8*)"\xfd\x6a\x57\x76\x4f\x55\x3b", 125 },
    { "TANKTUP", (const u8*)"\xfd\x6a\x5f\x4f\x57\x5f\x4a\x92", 115 },
    { "MR. VILE", (const u8*)"\xfd\x6a\x6f\x5c\x5f\x3b\x05\x8b\x51\x78", 98 },
    { "TIPTUP", (const u8*)"\xfd\x6a\x62\x46\x4a\x92\x5f\x4a\x92", 114 },
    { "BOGGY", (const u8*)"\xfd\x6a\x8f\x7d\x3b", 136 },
    { "WOZZA", (const u8*)"\xfd\x6a\x52\x49\x3b\x81", 131 },
    { "TRUNKER", (const u8*)"\xfd\x6a\x63\x76\x4f\x55\x3b", 125 },
    { "GOBI", (const u8*)"\xfd\x6a\x80\x8c", 144 },
    { "RUBEE AND TOOTS", (const u8*)"\xfd\x6a\x78\x8c\x51\xcd\x62\x4c\x3b\x61", 100 },
    { "MOTZAND", (const u8*)"\xfd\x6a\x72\x3b\x61\x69\x4f\x8a", 115 },
    { "NAPPER", (const u8*)"\xfd\x6a\x64\x4a\x90\x3b", 131 },
    { "LOGGO", (const u8*)"\xfd\x6a\x7a\x4a\x80\x3b", 127 },
    { "SNORKEL", (const u8*)"\xfd\x6a\x5c\x68\x3b\x58\x78", 125 },
    { "GNAWTY", (const u8*)"\xfd\x6a\x64\x3b\x62\x46", 130 },
    { "EYRIE", (const u8*)"\xfd\x6a\x53\x50\x77\x3b", 131 },
    { "NABNUT", (const u8*)"\xfd\x6a\x64\x8d\x64\x4a\x61", 125 },
    { "BANJO AND KAZOOIE", (const u8*)"\xfd\x6a\x8b\x4f\x82\x4d\x3b\xcd\x55\x83\x3b\x51", 96 },
    { "DINGPOT", (const u8*)"\xfd\x6a\x89\x46\x4f\x7e\x94\x4a\x63", 113 },
    { "KLUNGO", (const u8*)"\xfd\x6a\x57\x76\x4f\x80", 127 },
    { "TOPPER", (const u8*)"\xfd\x6a\x63\x4a\x90\x3b", 129 },
    { "BAWL", (const u8*)"\xfd\x6a\x8f\x52\x78", 136 },
    { "COLLIWOBBLE", (const u8*)"\xfd\x6a\x55\x77\x52\x49\x8d\x78", 114 },
    { "QUARRIE", (const u8*)"\xfd\x6a\x57\x50\x77\x3b", 134 },
    { "GRUNTLING", (const u8*)"\xfd\x6a\x7e\x76\x4f\x63\x77\x4f\x7e", 109 },
    { "TICKER", (const u8*)"\xfd\x6a\x62\x46\x4a\x55\x3b", 120 },
    { "BIGBUTT", (const u8*)"\xfd\x6a\x8c\x4a\x7e\x8b\x4a\x63", 113 },
    { "GRUBLIN", (const u8*)"\xfd\x6a\x7e\x76\x8d\x77\x4f", 121 },
    { "LEAKY", (const u8*)"\xfd\x6a\x77\x3b\x56\x3b", 129 },
    { "LOCKUP", (const u8*)"\xfd\x6a\x7a\x4a\x57\x50\x4a\x92", 113 },
    { "LITTLE LOCKUP", (const u8*)"\xfd\x6a\x77\x63\x78\x05\x7a\x4a\x57\x50\x4a\x92", 88 },
    { "YUM-YUM", (const u8*)"\xfd\x6a\x73\x70\x05\x73\x70", 122 },
    { "SNIPPET", (const u8*)"\xfd\x6a\x5c\x65\x91\x4a\x63", 120 },
    { "SHRAPNEL", (const u8*)"\xfd\x6a\x5b\x4b\x76\x4a\x92\x67\x78", 110 },
    { "SNACKER", (const u8*)"\xfd\x6a\x5c\x64\x4a\x55\x3b", 122 },
    { "GLOOP", (const u8*)"\xfd\x6a\x7e\x78\x52\x92", 120 },
    { "GRILLE CHOMPA", (const u8*)"\xfd\x6a\x7e\x77\x78\x05\x60\x4d\x4f\x90", 100 },
    { "MUTIE-SNIPPET", (const u8*)"\xfd\x6a\x6f\x4c\x3b\x62\x46\x05\x5c\x65\x91\x4a\x63", 84 },
    { "WHIPLASH", (const u8*)"\xfd\x6a\x52\x46\x92\x76\x4a\x5b\x4c", 111 },
    { "CROCTUS", (const u8*)"\xfd\x6a\x57\x7a\x59\x5f\x5c", 122 },
    { "FLIBBIT", (const u8*)"\xfd\x6a\x6b\x77\x8c\x4a\x61", 120 },
    { "BUZZBOMB", (const u8*)"\xfd\x6a\x8b\x83\x8f\x70", 123 },
    { "THE TIPTUP CHOIR", (const u8*)"\xfd\x6a\x62\x46\x4a\x92\x5f\x4a\x92\x05\x59\x3b\x76\x5c", 78 },
    { "GROGGY", (const u8*)"\xfd\x6a\x7e\x7a\x4a\x7d\x3b", 122 },
    { "SOGGY", (const u8*)"\xfd\x6a\x5e\x7d\x3b", 136 },
    { "MOGGY", (const u8*)"\xfd\x6a\x72\x7d\x3b", 136 },
    { "THE TWINKLIES", (const u8*)"\xfd\x6a\x61\x52\x46\x4f\x57\x77\x3b\x83", 100 },
    { "TWINKLY MUNCHER", (const u8*)"\xfd\x6a\x61\x52\x46\x4f\x57\x77\x3b\x05\x6e\x4f\x60\x4b\x3b", 70 },
    { "SIR SLUSH", (const u8*)"\xfd\x6a\x5a\x3b\x05\x5c\x76\x4a\x5b\x4c", 104 },
    { "CHINKER", (const u8*)"\xfd\x6a\x60\x4f\x55\x3b", 129 },
    { "JINXY", (const u8*)"\xfd\x6a\x82\x4f\x57\x5b\x3b", 123 },
    { "THE ANCIENT ONES", (const u8*)"\xfd\x6a\xbb\xcf\xc5\xbd\xd2\x72\x68", 102 },
    { "GRABBA", (const u8*)"\xfd\x6a\x7e\x76\x4a\x8b", 129 },
    { "SCABBY", (const u8*)"\xfd\x6a\x5c\x56\x4b\x8c\x3b", 120 },
    { "SLAPPA", (const u8*)"\xfd\x6a\x5c\x76\x4a\x90", 125 },
    { "MUM-MUM", (const u8*)"\xfd\x6a\x6e\x70\x05\x6e\x70", 121 },
    { "HISTUP", (const u8*)"\xfd\x6a\x6a\x5c\x5f\x4a\x92", 122 },
    { "TUMBLAR", (const u8*)"\xfd\x6a\x5f\x70\x8d\x76\x3b", 119 },
    { "PORTRAIT CHOMPA", (const u8*)"\xfd\x6a\x94\x3b\x63\x79\x51\x63\x05\x60\x4d\x4f\x90", 81 },
    { "TEE-HEE", (const u8*)"\xfd\x6a\x62\x46\x3b\x6a\x3b", 124 },
    { "LIMBO", (const u8*)"\xfd\x6a\x77\x4f\x8f\x52", 128 },
    { "RIPPER", (const u8*)"\xfd\x6a\x77\x4a\x90\x3b", 129 },
    { "NIBBLY", (const u8*)"\xfd\x6a\x65\x8d\x77\x3b", 130 },
    { "BOOM BOX", (const u8*)"\xfd\x6a\x8d\x3b\x70\x05\x8f\x4a\x57\x5c", 100 },
    { "BOSS BOOM BOX", (const u8*)"\xfd\x6a\x8f\x5c\x05\x8d\x3b\x70\x05\x8f\x4a\x57\x5c", 80 },
    { "GRIMLET", (const u8*)"\xfd\x6a\x7e\x77\x70\x79\x4a\x63", 120 },
    { "FLOTSAM", (const u8*)"\xfd\x6a\x6b\x7a\x4a\x5a\x70", 123 },
    { "SEAMAN GRUBLIN", (const u8*)"\xfd\x6a\x5b\x3b\x6e\x4f\x05\x7e\x76\x8d\x77\x4f", 84 },
    { "CHUMP", (const u8*)"\xfd\x6a\x60\x4b\x70\x90", 127 },
    { "SNAREBEAR", (const u8*)"\xfd\x6a\x5c\x67\x50\x3b\x8e\x50", 110 },
    { "BIG CLUCKER", (const u8*)"\xfd\x6a\x8c\x4a\x7e\x05\x57\x76\x4a\x55\x3b", 96 },
    { "THE ZUBBAS", (const u8*)"\xfd\x6a\x81\x8b\x5c", 129 },
    { "GRUBLIN HOOD", (const u8*)"\xfd\x6a\x7e\x76\x8d\x77\x4f\x05\x6b\x3b\x8a", 95 },
    { "WHIPCRACK", (const u8*)"\xfd\x6a\x52\x46\x92\x76\x4a\x57", 115 },
    { "CHEATO", (const u8*)"\xfd\x6a\x60\x51\x63", 139 },
};

// UI Strings (FileSelect, PauseMenu)
struct LocalizedUiString {
    const char* english;
    const u8* jp;
    const u8* fr;
    const u8* de;
};

static const LocalizedUiString sUiStrings[] = {
    // Pause menu
    { "RETURN TO GAME", (const u8*)"\xfd\x6a\x7f\x3b\x70\xcf\xdc\xf4\xe2", (const u8*)"CONTINUER",
      (const u8*)"ZUR]CK ZUM SPIEL" },
    { "EXIT TO WITCH'S LAIR", (const u8*)"\xfd\x6a\x6e\x82\x4d\xd2\xcd\xe1\xf3\xd6\xcf\xdc\xf4\xe2",
      (const u8*)"ANTRE DE LA SORCI\x63RE", (const u8*)"ZUR HEXENH\\HLE" },
    // Spiral Mountain variant of the Return-to-Lair option; reuses the lair translations.
    { "GO TO GRUNTY'S LAIR", (const u8*)"\xfd\x6a\x6e\x82\x4d\xd2\xcd\xe1\xf3\xd6\xcf\xdc\xf4\xe2",
      (const u8*)"ANTRE DE LA SORCI\x63RE", (const u8*)"ZUR HEXENH\\HLE" },
    { "VIEW TOTALS", (const u8*)"\xfd\x6a\x63\x3b\x5f\x78\xb8\xd9\xe2", (const u8*)"STATISTIQUES",
      (const u8*)"STATISTIK" },
    { "SAVE AND QUIT", (const u8*)"\xfd\x6a\x5d\x3b\x8d\xc5\xcc\xbe\xe5\xe2", (const u8*)"SAUVER ET QUITTER",
      (const u8*)"SICHERN UND ENDE" },
    { "ARE YOU SURE?", (const u8*)"\xfd\x6a\xd7\xb9\xcd\xbc\xf3\xc6\xbf\x40", (const u8*)"dTES-VOUS SiR?",
      (const u8*)"SICHER?" },
    { "A - YES, B - NO", (const u8*)"\xfd\x6a\x1a\x3e\xd3\xbb\x0f\x1b\x3e\xbb\xbb\xbd", (const u8*)"A - OUI, B - NON",
      (const u8*)"A - JA, B - NEIN" },
};

static const u8* getLocalizedUiString(const char* english) {
    if (english == nullptr) {
        return nullptr;
    }
    for (unsigned i = 0; i < sizeof(sUiStrings) / sizeof(sUiStrings[0]); i++) {
        if (std::strcmp(sUiStrings[i].english, english) != 0) {
            continue;
        }
        if (ResourceMgr_IsJapanese()) {
            return sUiStrings[i].jp;
        }
        switch (ResourceMgr_GetDialogLanguage()) {
            case 1:
                return sUiStrings[i].fr;
            case 2:
                return sUiStrings[i].de;
            default:
                return nullptr; // English
        }
    }
    return nullptr;
}

static const u8* getParadeKana(const char* name, int* outKanaX) {
    if (name == nullptr) {
        return nullptr;
    }
    for (unsigned i = 0; i < sizeof(sParadeKana) / sizeof(sParadeKana[0]); i++) {
        if (std::strcmp(sParadeKana[i].name, name) == 0) {
            if (outKanaX != nullptr) {
                *outKanaX = sParadeKana[i].kanaX;
            }
            return sParadeKana[i].kana;
        }
    }
    return nullptr;
}

// FileSelect Game Info
#define JP_FW_DIGIT 0x10
#define JP_FW_COLON "\x3e"
static void appendFwInt(char* dst, int v) {
    char rev[16];
    int r = 0;
    if (v <= 0) {
        rev[r++] = (char)JP_FW_DIGIT;
    } else {
        while (v > 0) {
            rev[r++] = (char)(JP_FW_DIGIT + v % 10);
            v /= 10;
        }
    }
    size_t n = std::strlen(dst);
    while (r > 0) {
        dst[n++] = rev[--r];
    }
    dst[n] = '\0';
}

// Appends a 2-digit, zero-padded fullwidth number (used for the MM and SS time fields).
static void appendFwInt2(char* dst, int v) {
    size_t n = std::strlen(dst);
    dst[n++] = (char)(JP_FW_DIGIT + (v / 10) % 10);
    dst[n++] = (char)(JP_FW_DIGIT + v % 10);
    dst[n] = '\0';
}

static void buildJpFileSelectInfo(char* upper, char* lower, int gamenum, int timeSeconds, int jiggy, int note,
                                  int isEmpty) {
    const int num = (gamenum == 0) ? 1 : ((gamenum == 1) ? 3 : 2);
    std::strcpy(upper, "\xfd\x6a"); // switch to font 2; no "GAME" word on the JP cart
    appendFwInt(upper, num);
    if (isEmpty) {
        std::strcat(upper, "\x3e\x0f\x27\x1e\x30\x6b\x45\x51\x78"); // "：ＮＥＷファイル"
        lower[0] = '\0';
        return;
    }
    std::strcat(upper, "\x3e\x0f\x5f\x51\x70\x0f"); // "：タイム"
    // Time as H:MM:SS in fullwidth digits/colo
    appendFwInt(upper, timeSeconds / 3600);
    std::strcat(upper, JP_FW_COLON);
    appendFwInt2(upper, (timeSeconds / 60) % 60);
    std::strcat(upper, JP_FW_COLON);
    appendFwInt2(upper, timeSeconds % 60);
    std::strcpy(lower, "\xfd\x6a\x82\x7e\x5e\x3b\x3e"); // "ジグソー："
    appendFwInt(lower, jiggy);
    std::strcat(lower, "\x0f\x54\x4f\x92\x3e"); // " オンプ："
    appendFwInt(lower, note);
}

// Append a base-10 integer in ASCII digits.
static void appendAsciiInt(char* dst, int v) {
    char rev[16];
    int r = 0;
    if (v <= 0) {
        rev[r++] = '0';
    } else {
        while (v > 0) {
            rev[r++] = (char)('0' + v % 10);
            v /= 10;
        }
    }
    size_t n = std::strlen(dst);
    while (r > 0) {
        dst[n++] = rev[--r];
    }
    dst[n] = '\0';
}

// Append a static UI fragment, swapped for the active pack's translation when it has one.
static void appendLocFragment(char* dst, const char* english) {
    std::strcat(dst, ResourceMgr_GetLangString(english));
}

// Rebuild the file-select info box for a language pack (Latin script). Mirrors the decomp's
// English layout (prefix + game number, then time / jiggy + note totals) but pulls each static
// label from the pack's `strings:` map, with independent singular/plural noun forms so a
// translation can give jiggy and note distinct plurals. Untranslated fragments stay English.
static void buildPackFileSelectInfo(char* upper, char* lower, int gamenum, int timeSeconds, int jiggy, int note,
                                    int isEmpty) {
    const int num = (gamenum == 0) ? 1 : ((gamenum == 1) ? 3 : 2);
    upper[0] = '\0';
    appendLocFragment(upper, "GAME ");
    appendAsciiInt(upper, num);
    if (isEmpty) {
        appendLocFragment(upper, ": EMPTY");
        lower[0] = '\0';
        return;
    }
    appendLocFragment(upper, ": TIME ");
    std::strcat(upper, gcpausemenu_TimeToA(timeSeconds));
    std::strcat(upper, ",");

    lower[0] = '\0';
    appendAsciiInt(lower, jiggy);
    appendLocFragment(lower, (jiggy == 1) ? " JIGSAW" : " JIGSAWS");
    std::strcat(lower, ", ");
    appendAsciiInt(lower, note);
    appendLocFragment(lower, (note == 1) ? " NOTE" : " NOTES");
    std::strcat(lower, ".");
}

// File Select (JP)
static const u8 sJpFileSelectInstr0[] = "\xfd\x6a\x13\x1d\x5c\x62\x46\x4a\x57\xf3"; // "コントロールスティックで"
static const u8 sJpFileSelectInstr1[] =
    "\xfd\x6a\x6b\x45\x51\x78\xb8\xbd\xe0\xb9\xf3\xc1\xf0\xc4\xbb\x42"; // "ファイルをえらんでください。"
static const u8 sJpFileSelectInstr2[] =
    "\xfd\x6a\x1a\x8f\x5f\x4f\x3e\x0f\x7f\x3b\x70\xbf\xbb\xc5"; // "Aボタン：ゲームかいし"
static const u8 sJpFileSelectInstr3[] =
    "\xfd\x6a\x33\x8f\x5f\x4f\x3e\x0f\x6b\x45\x51\x78\xb8\xc2\xc6";                 // "Zボタン：ファイルけす"
static const u8 sJpFileSelectErase0[] = "\xfd\x6a\xd7\xb9\xcd\xbc\xf3\xc6\xbf\x40"; // "ほんとうにけしますか？"
static const u8 sJpFileSelectErase1[] =
    "\xfd\x6a\x1a\x3e\xec\xb4\xc3\xbc\x0f\x1b\x3e\x56\x4b\x4f\x5d\x78"; // "Aボタン：けす Bボタン：やめる"

static bool SetJpFileSelectInstructions(void* zoombox) {
    if (!ResourceMgr_IsJapanese()) {
        return false;
    }
    static char* lines[4] = { (char*)sJpFileSelectInstr0, (char*)sJpFileSelectInstr1, (char*)sJpFileSelectInstr2,
                              (char*)sJpFileSelectInstr3 };
    gczoombox_setStrings(zoombox, 4, lines);
    return true;
}

static bool SetJpFileSelectEraseConfirm(void* zoombox) {
    if (!ResourceMgr_IsJapanese()) {
        return false;
    }
    static char* lines[2] = { (char*)sJpFileSelectErase0, (char*)sJpFileSelectErase1 };
    gczoombox_setStrings(zoombox, 2, lines);
    return true;
}

// File Select (language pack): swap the two top-instruction lines and the erase-confirm
// prompt for the active pack's translations, keyed on the English originals (D_80365DF4/
// DF8/DFC slot 0). Each line independently falls back to English when the pack omits it.
// Inert unless a pack with `strings:` overrides is active, so base US/PAL is untouched.
static bool SetPackFileSelectInstructions(void* zoombox) {
    if (!ResourceMgr_HasLangStrings()) {
        return false;
    }
    static char* lines[2];
    lines[0] = (char*)ResourceMgr_GetLangString((const char*)D_80365DF4[0]);
    lines[1] = (char*)ResourceMgr_GetLangString((const char*)D_80365DF8[0]);
    gczoombox_setStrings(zoombox, 2, lines);
    return true;
}

static bool SetPackFileSelectEraseConfirm(void* zoombox) {
    if (!ResourceMgr_HasLangStrings()) {
        return false;
    }
    func_803183A4(zoombox, (char*)ResourceMgr_GetLangString((const char*)D_80365DFC[0]));
    return true;
}

// Character Parade
struct PortParadeInfo {
    uint8_t map;
    int8_t exit;
    int16_t x;
    const char* str;
    int8_t unk8;
};
extern "C" {
extern PortParadeInfo D_8036D9A0[]; // US Furnace-Fun parade (27 entries)
extern PortParadeInfo D_8036DAE4[]; // US final parade (58 entries)
}
static PortParadeInfo sPalJpParade0[28];
static PortParadeInfo sPalJpParade1[57];
static bool sPalJpParadesBuilt = false;

// JP and PAL both move Motzand into the Furnace-Fun parade (parade 0, after RUBEE
// AND TOOTS) and drop it from the post-Grunty parade (parade 1).
static void buildPalJpParades() {
    for (int i = 0; i < 20; i++) {
        sPalJpParade0[i] = D_8036D9A0[i];
    }
    sPalJpParade0[20] = { 0x1C, 5, 90, "MOTZAND", 0 }; // MAP_1C_MMM_CHURCH, after RUBEE AND TOOTS
    for (int i = 20; i < 27; i++) {
        sPalJpParade0[i + 1] = D_8036D9A0[i];
    }
    int j = 0;
    for (int i = 0; i < 58; i++) {
        if (i == 39) { // US final-parade MOTZAND, moved to the Furnace-Fun parade above
            continue;
        }
        sPalJpParade1[j++] = D_8036DAE4[i];
    }
}

// The Furnace-Fun parade moves Motzand into the credit roll. JP and PAL carry the
// Motzand credit (canonical id 0x11CA) natively; US v1.0 does not. A US-based pack can
// still supply 0x11CA as an additive asset, so key the alt parade on that credit being
// present in the active language (sDialogOverride) rather than on the base version.
static constexpr uint32_t kMotzandParadeCreditId = 0x11CA;
static bool port_useMotzandParade() {
    return ResourceMgr_IsJapanese() || ResourceMgr_GetDialogLanguageCount() > 1 ||
           ResourceMgr_IsAssetRepointed(kMotzandParadeCreditId) != 0;
}

// Swap the active parade table for the Motzand variant.
static void LocalizeParadeTable(int paradeId, void** table, uint8_t* count) {
    if (!port_useMotzandParade()) {
        return;
    }
    if (!sPalJpParadesBuilt) {
        buildPalJpParades();
        sPalJpParadesBuilt = true;
    }
    if (paradeId == 0) {
        *table = sPalJpParade0;
        *count = 28;
    } else {
        *table = sPalJpParade1;
        *count = 57;
    }
}

// Event listeners
static void RegisterLocalizedText() {
    // File select, translations taken direct from PAL
    D_80365DF4[1] = (u8*)"S"
                         "\x62"
                         "LECTIONNEZ UN FICHIER _ L'AIDE DU STICK.";
    D_80365DF4[2] = (u8*)"W[HLE MIT DEM 3D-STICK";
    D_80365DF8[1] = (u8*)"APPUYEZ SUR A POUR JOUER OU SUR Z POUR EFFACER!";
    D_80365DF8[2] = (u8*)"EIN SPIEL AUS. DR]CKE A, UM ZU SPIELEN, ODER DEN Z-TRIGGER, UM DEN SPIELSTAND ZU L\\SCHEN!";
    D_80365DFC[1] = (u8*)"dTES-VOUS SiR? APPUYEZ SUR A POUR CONFIRMER OU SUR B POUR ANNULER.";
    D_80365DFC[2] = (u8*)"SICHER? DR]CKE A, UM ZU BEST[TIGEN, ODER B, UM ZU WIDERRUFEN.";

    // Drive the JP dialog-font slot off the language generation, at a safe
    // between-frames point: load slot 2 when Japanese becomes active, and on
    // switch away defer the free a few frames so in-flight JP strings finish.
    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        int gen = ResourceMgr_GetLanguageGeneration();
        if (gen != sFontLanguageGen) {
            sFontLanguageGen = gen;
            if (ResourceMgr_IsJapanese()) {
                sJpFontFreeDelay = 0; // cancel a pending free; ensure slot 2 is current
                LoadJpFontSlot();
            } else if (D_80380AB8[2] != nullptr) {
                sJpFontFreeDelay = kJpFontFreeDelayFrames;
            }
            bool fontOverridden = ResourceMgr_IsAssetRepointed(kDialogFontAssetId) != 0;
            if (fontOverridden || sDialogFontOverridden) {
                ReloadDialogFontSlot();
                sDialogFontOverridden = fontOverridden;
            }
            bool boldOverridden = ResourceMgr_IsAssetRepointed(kBoldLettersFontAssetId) != 0;
            if (boldOverridden || sBoldFontOverridden) {
                print_setBoldFontTexture(print_getCurrentMapBoldFontTexture());
                sBoldFontOverridden = boldOverridden;
            }
        }
        if (sJpFontFreeDelay > 0 && --sJpFontFreeDelay == 0) {
            FreeJpFontSlot();
        }
    });

    // Font slot fallback: the JP font (slot 2) can be momentarily unloaded for a
    // frame across a language swap. Redirect to the base font (glyph 0) for that
    // frame instead of letting the draw dereference a NULL slot.
    REGISTER_LISTENER(ResolveBoldFontSlot, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (ResolveBoldFontSlot*)event;
        if (D_80380AB8[*ev->slot] == nullptr) {
            *ev->slot = 0;
            *ev->letterId = 0;
        }
    });

    // Invalidate a cached model that the active language re-points, so it
    // re-fetches the localized version on its next draw (live model swap). A model
    // any language has re-pointed keeps reloading on every change so it can also
    // revert to the base model; others early-out immediately.
    REGISTER_LISTENER(OnModelLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (OnModelLoad*)event;
        if (ev->reload == nullptr) {
            return;
        }
        uint32_t modelId = (uint32_t)ev->modelId;
        if (ResourceMgr_IsAssetRepointed(modelId)) {
            sEverRepointedModels.insert(modelId);
        } else if (sEverRepointedModels.find(modelId) == sEverRepointedModels.end()) {
            return; // never re-pointed by any language
        }
        int& slotGen = sModelInfoGen[ev->modelInfo];
        int gen = ResourceMgr_GetLanguageGeneration();
        if (slotGen == gen) {
            return; // already synced to the active language
        }
        slotGen = gen;
        *ev->reload = 1;
    });

    // Swap an English pause-menu / file-select string for its translation. A language
    // pack's `strings:` override (loaded into the langinfo string map) wins; otherwise
    // fall back to the built-in retail JP / PAL FR-DE table.
    REGISTER_LISTENER(LocalizeUiString, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (LocalizeUiString*)event;
        if (ev->str == nullptr || *ev->str == nullptr) {
            return;
        }
        const char* packStr = ResourceMgr_GetLangString(*ev->str);
        if (packStr != *ev->str) {
            *ev->str = packStr;
            return;
        }
        const u8* loc = getLocalizedUiString(*ev->str);
        if (loc != nullptr) {
            *ev->str = (const char*)loc;
        }
    });

    // JP: draw the subtitle 0x18px below the bold English character-parade name.
    REGISTER_LISTENER(OnParadeNameDraw, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        if (!ResourceMgr_IsJapanese()) {
            return;
        }
        auto* ev = (OnParadeNameDraw*)event;
        int kanaX = 0;
        const u8* kana = getParadeKana(ev->name, &kanaX);
        if (kana != nullptr) {
            print_dialog(kanaX, ev->yPosition + 0x18, (u8*)kana);
        }
    });

    // Rebuild the game-info line over the US/PAL one the decomp just assembled: JP uses its
    // own kana layout; a language pack carrying UI overrides rebuilds it from `strings:`
    // fragments (independent jiggy/note plurals). Base US/PAL keeps the decomp's assembly.
    REGISTER_LISTENER(OnFileSelectInfoBuild, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (OnFileSelectInfoBuild*)event;
        const bool isEmpty = !gameFile_isNotEmpty(ev->gamenum);
        const int timeSeconds = isEmpty ? 0 : itemscore_timeScores_getTotal();
        if (ResourceMgr_IsJapanese()) {
            buildJpFileSelectInfo(ev->upper, ev->lower, ev->gamenum, timeSeconds, jiggyscore_total(),
                                  itemscore_noteScores_getTotal(), isEmpty);
        } else if (ResourceMgr_HasLangStrings()) {
            buildPackFileSelectInfo(ev->upper, ev->lower, ev->gamenum, timeSeconds, jiggyscore_total(),
                                    itemscore_noteScores_getTotal(), isEmpty);
        }
    });

    // JP: write the kana file-select prompts and cancel the event so the decomp
    // skips its English ones (promptId 0 = controls, 1 = erase confirm).
    REGISTER_LISTENER(LocalizeFileSelectPrompt, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (LocalizeFileSelectPrompt*)event;
        bool done = (ev->promptId == 1)
                        ? (SetJpFileSelectEraseConfirm(ev->zoombox) || SetPackFileSelectEraseConfirm(ev->zoombox))
                        : (SetJpFileSelectInstructions(ev->zoombox) || SetPackFileSelectInstructions(ev->zoombox));
        if (done) {
            ev->Event.Cancelled = true;
        }
    });

    // Rebuild the file-select info zoombox once per language change.
    REGISTER_LISTENER(OnFileSelectLanguageRefresh, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (OnFileSelectLanguageRefresh*)event;
        if (!ev->isSelected) {
            return;
        }
        static int sLastGen = -1;
        int gen = ResourceMgr_GetLanguageGeneration();
        if (sLastGen == gen) {
            return;
        }
        sLastGen = gen;
        // Bottom info line.
        setGameInformationZoombox(ev->gamenum);
        // Top controls prompt: clear it, then re-push for the new language.
        if (chGameSelectTopZoombox != nullptr) {
            func_8031877C(chGameSelectTopZoombox);
            if (!SetJpFileSelectInstructions(chGameSelectTopZoombox) &&
                !SetPackFileSelectInstructions(chGameSelectTopZoombox)) {
                int lang = code94620_func_8031B5B0();
                char* lines[2];
                lines[0] = (char*)D_80365DF4[lang];
                lines[1] = (char*)D_80365DF8[lang];
                gczoombox_setStrings(chGameSelectTopZoombox, 2, lines);
            }
        }
    });

    // JP: swap the character-parade table for its kana variant.
    REGISTER_LISTENER(LocalizeParade, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (LocalizeParade*)event;
        LocalizeParadeTable(ev->paradeId, ev->table, ev->count);
    });

    // The Motzand parade inserts Motzand at index 20, which shifts the later
    // credit-dialog ids by one. 0x11CA is the Motzand credit: on a PAL/JP base it
    // re-points to that version's native dialog (PAL 3065 / JP 3073) via the override;
    // on a US base it resolves to the pack's additive 0x11CA asset. The other shifted
    // ids (0x11AF + index - 1) are v1.0 canonical and resolve directly from the base.
    REGISTER_LISTENER(ParadeCreditDialogId, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (ParadeCreditDialogId*)event;
        if (!port_useMotzandParade() || ev->index <= 19 || ev->dialogId == nullptr) {
            return;
        }
        *ev->dialogId = (ev->index == 20) ? kMotzandParadeCreditId : (0x11AF + ev->index - 1);
    });

    // The port owns the dialog-language index (driven by the language picker). The
    // decomp's game-init reset (func_8031B62C) would zero it on parade/mode warps and
    // revert the user's selection to the base language, so skip it.
    COND_VB_SHOULD(VB_RESET_DIALOG_LANGUAGE, EVENT_PRIORITY_NORMAL, true, { *should = false; });

    // JP: nudge the pause-menu zoombox text a few px right so the wider kana clear the portrait.
    COND_VB_SHOULD(VB_ZOOMBOX_TEXT_ADJUST, EVENT_PRIORITY_NORMAL, true, {
        double boxScale = va_arg(args, double);
        va_arg(args, float*);
        int* xOfs = va_arg(args, int*);
        if (xOfs != nullptr && boxScale < 1.0 && ResourceMgr_IsJapanese()) {
            *xOfs += 4;
        }
    });
}

static RegisterShipInitFunc localizedTextInitFunc(RegisterLocalizedText);
