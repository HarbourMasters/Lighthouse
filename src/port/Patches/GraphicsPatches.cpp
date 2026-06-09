#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"

extern "C" {
#include "enums.h"

int getGameMode(void);

int port_getDrawDistanceLevel(void) {
    int level = CVarGetInteger(CVAR_ENHANCEMENT("Graphics.DrawDistance"), 0);
    if (getGameMode() == GAME_MODE_7_ATTRACT_DEMO || GAME_MODE_9_BANJO_AND_KAZOOIE) {
        level = 0;
    }
    return level;
}

int port_shouldDisableLOD(void) {
    return CVarGetInteger(CVAR_ENHANCEMENT("Graphics.DisableLOD"), 0);
}
}
