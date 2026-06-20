#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"

extern "C" int port_scalePlayerDamage(int damage) {
    if (damage <= 0) {
        return damage;
    }
    int mode = CVarGetInteger(CVAR_ENHANCEMENT("Gameplay.Difficulty"), 1);
    if (mode >= 4) {
        return 9999;
    }
    return damage * (mode < 1 ? 1 : mode);
}