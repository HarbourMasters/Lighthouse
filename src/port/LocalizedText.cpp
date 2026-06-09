#include <cstring>

#include <libultraship/libultraship.h>
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipInit.hpp"

typedef unsigned char u8;

extern "C" {
int ResourceMgr_IsJapanese(void);
int ResourceMgr_GetDialogLanguage(void); // PAL only: 0=English, 1=French, 2=German

bool gameFile_isNotEmpty(int gamenum);
int jiggyscore_total(void);
int itemscore_noteScores_getTotal(void);
int itemscore_timeScores_getTotal(void);
char* gcpausemenu_TimeToA(int time);
void print_dialog(int x, int y, u8* string);
int gczoombox_setStrings(void* zb, int str_cnt, char** str_ptrs);
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

extern "C" int port_setJpFileSelectInstructions(void* zoombox) {
    if (!ResourceMgr_IsJapanese()) {
        return 0;
    }
    static char* lines[4] = { (char*)sJpFileSelectInstr0, (char*)sJpFileSelectInstr1, (char*)sJpFileSelectInstr2,
                              (char*)sJpFileSelectInstr3 };
    gczoombox_setStrings(zoombox, 4, lines);
    return 1;
}

extern "C" int port_setJpFileSelectEraseConfirm(void* zoombox) {
    if (!ResourceMgr_IsJapanese()) {
        return 0;
    }
    static char* lines[2] = { (char*)sJpFileSelectErase0, (char*)sJpFileSelectErase1 };
    gczoombox_setStrings(zoombox, 2, lines);
    return 1;
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
static PortParadeInfo sJpParade0[28];
static PortParadeInfo sJpParade1[57];
static bool sJpParadesBuilt = false;

static void buildJpParades() {
    for (int i = 0; i < 20; i++) {
        sJpParade0[i] = D_8036D9A0[i];
    }
    sJpParade0[20] = { 0x1C, 5, 90, "MOTZAND", 0 }; // MAP_1C_MMM_CHURCH, after RUBEE AND TOOTS
    for (int i = 20; i < 27; i++) {
        sJpParade0[i + 1] = D_8036D9A0[i];
    }
    int j = 0;
    for (int i = 0; i < 58; i++) {
        if (i == 39) { // US final-parade MOTZAND, removed in JP
            continue;
        }
        sJpParade1[j++] = D_8036DAE4[i];
    }
}

// Swap the active parade table for its JP variant
extern "C" void port_localizeParade(int paradeId, void** table, uint8_t* count) {
    if (!ResourceMgr_IsJapanese()) {
        return;
    }
    if (!sJpParadesBuilt) {
        buildJpParades();
        sJpParadesBuilt = true;
    }
    if (paradeId == 0) {
        *table = sJpParade0;
        *count = 28;
    } else {
        *table = sJpParade1;
        *count = 57;
    }
}

// Furnace-Fun parade credit-dialog id
extern "C" int port_paradeDialogId(int indx) {
    if (!ResourceMgr_IsJapanese() || indx <= 19) {
        return 0x11AF + indx;
    }
    if (indx == 20) {
        return 0x11CA; // MOTZAND credit -> JP native 3073
    }
    return 0x11AF + indx - 1;
}

// Event listeners
static void RegisterLocalizedText() {
    // Swap an English pause-menu / file-select string for its JP / PAL FR-DE translation.
    REGISTER_LISTENER(LocalizeUiString, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = (LocalizeUiString*)event;
        if (ev->str == nullptr || *ev->str == nullptr) {
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

    // JP: rebuild the game-info line over the US/PAL one the decomp just assembled.
    REGISTER_LISTENER(OnFileSelectInfoBuild, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        if (!ResourceMgr_IsJapanese()) {
            return;
        }
        auto* ev = (OnFileSelectInfoBuild*)event;
        const bool isEmpty = !gameFile_isNotEmpty(ev->gamenum);
        buildJpFileSelectInfo(ev->upper, ev->lower, ev->gamenum, isEmpty ? 0 : itemscore_timeScores_getTotal(),
                              jiggyscore_total(), itemscore_noteScores_getTotal(), isEmpty);
    });
}

static RegisterShipInitFunc localizedTextInitFunc(RegisterLocalizedText);
