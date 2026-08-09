// Transient Level State
//
// The per-level stores Anchor replicates (huts, breakables, egg tolls, puzzle steps, carried
// collectibles) are written unconditionally by decomp, but only Anchor's sweep ever cleared
// them — offline, a smashed hut survived a level exit that should have restored it.
//
// Per level, not per map, so sub-area hops keep their state like the actor savestate did.

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h"
extern "C" {
#include "enums.h"
s32 port_anchor_isWorldSyncActive(void);
}

void port_breakable_clearForLevel(int32_t levelId);
void port_hutSmash_clearForLevel(int32_t levelId);
void port_eggToll_clearForLevel(int32_t levelId);
void port_puzzleStep_clearForLevel(int32_t levelId);
void port_carriedSync_clearForLevel(int32_t levelId);

void RegisterTransientLevelState_Init() {
    REGISTER_LISTENER(OnLevelReset, EVENT_PRIORITY_HIGH, [](IEvent* event) {
        OnLevelReset* ev = (OnLevelReset*)event;
        int32_t level = ev->levelId;
        if (level == 0 || port_anchor_isWorldSyncActive()) {
            return;
        }
        port_breakable_clearForLevel(level);
        port_hutSmash_clearForLevel(level);
        port_eggToll_clearForLevel(level);
        port_puzzleStep_clearForLevel(level);
        port_carriedSync_clearForLevel(level);
    });
}

static RegisterShipInitFunc initTransientLevelState(RegisterTransientLevelState_Init, {});
