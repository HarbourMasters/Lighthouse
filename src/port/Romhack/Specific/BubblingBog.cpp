#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/Shared/HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
}

// The Tooie jiggy animation exists in this hack
void TooieJiggyDance_ForceEnable();

void RegisterBubblingBogPatches() {
    TooieJiggyDance_ForceEnable();
    HackShared_EnableForceAbilitiesUsed(kAllUsedAbilities);

    // Bubbling Bog Brewery's swapy water is hazardous
    REGISTER_VB_SHOULD(VB_GROUND_HAZARD_ACTIVE, EVENT_PRIORITY_NORMAL, {
        s32 map = va_arg(args, s32);
        if (map == MAP_D_BGS_BUBBLEGLOOP_SWAMP && func_80294554()) {
            *should = true;
        }
    });
}
