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

namespace {

int32_t sCurrentLevel = -1;

// 0 for maps outside the section table; map_getLevel would deref null on those.
int32_t levelOfMap(int32_t map) {
    if (map <= 0 || map >= MAP_NUM_MAPS) {
        return 0;
    }
    return (int32_t)map_getLevel((enum map_e)map);
}

} // namespace

void RegisterTransientLevelState_Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent*) { sCurrentLevel = -1; });

    // HIGH so the stores are empty before this event's NORMAL listeners read them for the new
    // map (port_breakable_despawnBrokenRestores).
    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_HIGH, [](IEvent* event) {
        OnMapLoad* ev = (OnMapLoad*)event;
        int32_t nextLevel = levelOfMap((int32_t)ev->nextMap);
        if (nextLevel == 0 || nextLevel == sCurrentLevel) {
            return;
        }
        sCurrentLevel = nextLevel;
        if (port_anchor_isWorldSyncActive()) {
            return;
        }
        port_breakable_clearForLevel(nextLevel);
        port_hutSmash_clearForLevel(nextLevel);
        port_eggToll_clearForLevel(nextLevel);
        port_puzzleStep_clearForLevel(nextLevel);
        port_carriedSync_clearForLevel(nextLevel);
    });
}

static RegisterShipInitFunc initTransientLevelState(RegisterTransientLevelState_Init, {});
