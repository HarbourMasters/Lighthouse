#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"

extern "C" {
#include "enums.h"

int getGameMode(void);

int port_getDrawDistanceLevel(void) {
    int level = CVarGetInteger(CVAR_ENHANCEMENT("Graphics.DrawDistance"), 0);
    if (getGameMode() == GAME_MODE_7_ATTRACT_DEMO) {
        level = 0;
    }
    return level;
}

int port_shouldForceHighPolyBanjo(void) {
    return CVarGetInteger(CVAR_ENHANCEMENT("Graphics.AlwaysHighPolyBanjo"), 0);
}
}
