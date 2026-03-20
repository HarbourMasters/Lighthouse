#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"
#include "port/enhancements/events/PortEnhancements.h"
#include "port/enhancements/events/hooks/Events.h"

#include "structs.h"

extern "C" struct1Bs D_8036C560[];
extern "C" enum level_e level_get(void);

void RegisterReturnToLair_Init() {
    REGISTER_LISTENER(VanillaBehavior, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        VanillaBehavior* ev = (VanillaBehavior*)event;
    
        if (ev->id != VB_INIT_RETURN_TO_LAIR) {
            return;
        }
    
        if (CVarGetInteger(CVAR_ENHANCEMENT("Restorations.ReturnToLair"), 0)) {
            s32 level = level_get();
            *ev->should = !(level > 0 && level < LEVEL_C_BOSS && D_8036C560[level - 1].map != -1);
        }
        return;
    });
}