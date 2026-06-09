#include "GameStatus.h"
#include "ShipUtils.h"
#include <cstdio>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#endif

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
enum level_e map_getLevel(enum map_e map);
s32 itemscore_noteScores_get(enum level_e lvl_id);
s32 jiggyscore_leveltotal(s32 lvl);
s32 honeycombscore_get_level_total(enum level_e level_id);
u16 itemscore_timeScores_get(enum level_e level_id);
int port_getRomhackNotesMax(void);
int port_getRomhackJiggiesPerWorld(void);
int port_getRomhackHoneycombsPerWorld(void);
int port_getRomhackSpecialLevel(void);
int port_getRomhackExtraHcStart(void);
int port_getRomhackHideCollectiblesLevel(void);
int port_getRomhackHideJiggiesLevel(void);
const char* port_getRomhackLevelName(int level_index);

// Pause menu level name table (supports romhack string patches via Torch config)
typedef struct {
    s16 level_id;
    s16 x;
    u8* string;
} PauseLevelEntry;
extern PauseLevelEntry D_8036C58C[0xD];
}

extern "C" const char* port_getLevelName(int map_id) {
    enum level_e level = map_getLevel((enum map_e)map_id);
    for (int i = 0; i < 0xD; i++) {
        if (D_8036C58C[i].level_id == level) {
            // Check romhack override first
            const char* rhName = port_getRomhackLevelName(i);
            return rhName ? rhName : (const char*)D_8036C58C[i].string;
        }
    }
    return port_mapName(map_id);
}

extern "C" void port_getLevelStats(int map_id, s32* noteVal, s32* noteMax, s32* jiggyVal, s32* jiggyMax, s32* hcVal,
                                   s32* hcMax) {
    enum level_e level = map_getLevel((enum map_e)map_id);

    *noteVal = itemscore_noteScores_get(level);
    *jiggyVal = jiggyscore_leveltotal(level);
    *hcVal = honeycombscore_get_level_total(level);

    int n = port_getRomhackNotesMax();
    *noteMax = (n >= 0) ? n : 100;
    int j = port_getRomhackJiggiesPerWorld();
    *jiggyMax = (j >= 0) ? j : 10;

    int hMax = port_getRomhackHoneycombsPerWorld();
    if (hMax < 0)
        hMax = 2;
    int specialLevel = port_getRomhackSpecialLevel();
    if (specialLevel < 0)
        specialLevel = 0xB; // LEVEL_B_SPIRAL_MOUNTAIN
    if ((int)level == specialLevel) {
        int hcSpecial = port_getRomhackExtraHcStart();
        if (hcSpecial < 0)
            hcSpecial = 6;
        hMax = hcSpecial;
    }
    *hcMax = hMax;
}

extern "C" u16 port_getLevelTime(int map_id) {
    return itemscore_timeScores_get(map_getLevel((enum map_e)map_id));
}

// Trim leading/trailing whitespace from a string into a static buffer.
static const char* trimName(const char* name) {
    static char buf[128];
    while (*name == ' ')
        name++;
    int len = (int)strlen(name);
    while (len > 0 && name[len - 1] == ' ')
        len--;
    if (len >= (int)sizeof(buf))
        len = (int)sizeof(buf) - 1;
    memcpy(buf, name, len);
    buf[len] = '\0';
    return buf;
}

extern "C" void port_setWindowTitle(int map_id) {
    enum level_e level = map_getLevel((enum map_e)map_id);
    const char* levelName;
    // Override the level name for file select
    if (map_id == MAP_91_FILE_SELECT)
        levelName = "FILE SELECT";
    else
        levelName = trimName(port_getLevelName(map_id));

    // Determine which stats to hide (mirrors pause menu totals screen logic)
    int hideCollLvl = port_getRomhackHideCollectiblesLevel();
    int hideJigLvl = port_getRomhackHideJiggiesLevel();
    if (hideCollLvl < 0)
        hideCollLvl = 0x6; // LEVEL_6_LAIR
    if (hideJigLvl < 0)
        hideJigLvl = 0xB; // LEVEL_B_SPIRAL_MOUNTAIN

    // File select and cutscenes have no meaningful stats
    bool isNonGameplay = (map_id == MAP_91_FILE_SELECT || (int)level == LEVEL_D_CUTSCENE);

    // hideCollLvl hides notes + honeycombs, hideJigLvl hides notes + jiggies
    bool showNotes = !isNonGameplay && ((int)level != hideCollLvl && (int)level != hideJigLvl);
    bool showJiggies = !isNonGameplay && ((int)level != hideJigLvl);
    bool showHoneycombs = !isNonGameplay && ((int)level != hideCollLvl);

    s32 noteVal, noteMax, jiggyVal, jiggyMax, hcVal, hcMax;
    port_getLevelStats(map_id, &noteVal, &noteMax, &jiggyVal, &jiggyMax, &hcVal, &hcMax);

    char noteStr[16], jiggyStr[16], hcStr[16];
    if (showNotes)
        snprintf(noteStr, sizeof(noteStr), "%d/%d", noteVal, noteMax);
    else
        snprintf(noteStr, sizeof(noteStr), "--");
    if (showJiggies)
        snprintf(jiggyStr, sizeof(jiggyStr), "%d/%d", jiggyVal, jiggyMax);
    else
        snprintf(jiggyStr, sizeof(jiggyStr), "--");
    if (showHoneycombs)
        snprintf(hcStr, sizeof(hcStr), "%d/%d", hcVal, hcMax);
    else
        snprintf(hcStr, sizeof(hcStr), "--");

    char timeStr[16];
    if (!isNonGameplay && showNotes) {
        u16 timeSec = port_getLevelTime(map_id);
        int hours = timeSec / 3600;
        int minutes = (timeSec / 60) % 60;
        int seconds = timeSec % 60;
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, seconds);
    } else {
        snprintf(timeStr, sizeof(timeStr), "--");
    }

    char title[256];
    snprintf(title, sizeof(title), "Lighthouse - %s | Notes: %s | Jiggies: %s | Honeycombs: %s | Time: %s", levelName,
             noteStr, jiggyStr, hcStr, timeStr);

#ifdef _WIN32
    HWND hwnd = GetActiveWindow();
    if (hwnd) {
        SetWindowTextA(hwnd, title);
    }
#endif
}

void RegisterGameStatus_Init() {
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_LOW, true, [](IEvent* event) {
        OnMapLoad* ev = (OnMapLoad*)event;
        port_setWindowTitle(ev->nextMap);
    });
}

static RegisterShipInitFunc initFunc(RegisterGameStatus_Init);
