#include "GameStatus.h"
#include "ShipUtils.h"
#include <cstdio>
#include <cstring>
#include <spdlog/spdlog.h>
#ifdef _WIN32
#include <windows.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#endif

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Patches/Patches.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/ShipInit.hpp"

#ifdef _WIN32
namespace {
HWND ResolveGameWindow() {
    static HWND sCached = nullptr;
    if (sCached != nullptr && IsWindow(sCached)) {
        return sCached;
    }
    for (Uint32 id = 1; id <= 16 && sCached == nullptr; id++) {
        SDL_Window* window = SDL_GetWindowFromID(id);
        if (window == nullptr) {
            continue;
        }
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        if (SDL_GetWindowWMInfo(window, &info) == SDL_TRUE && info.subsystem == SDL_SYSWM_WINDOWS) {
            sCached = info.info.win.window;
        }
    }
    if (sCached == nullptr) {
        struct Search {
            DWORD pid;
            HWND gameWindow;
            HWND fallback;
        };
        Search search{ GetCurrentProcessId(), nullptr, nullptr };
        EnumWindows(
            [](HWND hwnd, LPARAM param) -> BOOL {
                Search* s = reinterpret_cast<Search*>(param);
                DWORD pid = 0;
                GetWindowThreadProcessId(hwnd, &pid);
                if (pid != s->pid || GetWindow(hwnd, GW_OWNER) != nullptr || !IsWindowVisible(hwnd)) {
                    return TRUE;
                }
                char cls[64] = { 0 };
                GetClassNameA(hwnd, cls, (int)sizeof(cls));
                if (strcmp(cls, "N64GAME") == 0) {
                    s->gameWindow = hwnd;
                    return FALSE;
                }
                if (s->fallback == nullptr && strcmp(cls, "ConsoleWindowClass") != 0) {
                    s->fallback = hwnd;
                }
                return TRUE;
            },
            reinterpret_cast<LPARAM>(&search));
        sCached = (search.gameWindow != nullptr) ? search.gameWindow : search.fallback;
    }
    return sCached;
}
} // namespace
#endif

#include "functions.h"
extern "C" {
#include "enums.h"

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
    HWND hwnd = ResolveGameWindow();
    if (hwnd) {
        SetWindowTextA(hwnd, title);
    }
#endif
}

static int sPendingTitleMap = 0;

void RegisterGameStatus_Init() {
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_LOW, true, [](IEvent* event) {
        OnMapLoad* ev = (OnMapLoad*)event;
        sPendingTitleMap = ev->nextMap;
        port_runOnRenderThread([](void*) { port_setWindowTitle(sPendingTitleMap); }, nullptr);
    });
}

static RegisterShipInitFunc initFunc(RegisterGameStatus_Init);
