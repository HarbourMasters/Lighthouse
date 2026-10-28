// Transient Level State
//
// The per-level stores Anchor replicates (huts, breakables, egg tolls, puzzle steps, carried
// collectibles) are only written and read with world sync on. Without it, whatever an earlier
// synced session left behind is dropped as each level resets, so it can't resurface in the next.

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h"
extern "C" {
#include "enums.h"
s32 Anchor_IsWorldSyncActive(void);
}

void BreakObject_ClearForLevel(int32_t levelId);
void HutSmash_ClearForLevel(int32_t levelId);
void EggToll_ClearForLevel(int32_t levelId);
void PuzzleStep_ClearForLevel(int32_t levelId);
void CarriedSync_ClearForLevel(int32_t levelId);

void RegisterTransientLevelState_Init() {
    REGISTER_LISTENER(OnLevelReset, EVENT_PRIORITY_HIGH, [](IEvent* event) {
        OnLevelReset* ev = (OnLevelReset*)event;
        int32_t level = ev->levelId;
        if (level == 0 || Anchor_IsWorldSyncActive()) {
            return;
        }
        BreakObject_ClearForLevel(level);
        HutSmash_ClearForLevel(level);
        EggToll_ClearForLevel(level);
        PuzzleStep_ClearForLevel(level);
        CarriedSync_ClearForLevel(level);
    });
}

static RegisterShipInitFunc initTransientLevelState(RegisterTransientLevelState_Init, {});
