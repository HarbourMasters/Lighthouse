#include "CustomCollectible.h"

#include <libultraship/bridge.h>
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"

std::map<RandoCheckId, std::tuple<int32_t, int32_t, int32_t>> collectibleSaveState;

// Vanilla uses something called a SaveState to persist collectibles like bundles spawned from
// enemies, MM huts etc. Because our collectibles use more data than just the actor ID, we
// separately save these with their positions and check ID to identify them later, and spawn
// them back manually later in OnLoadActorSaveState using this data.
void RegisterCustomCollectibleSaveState() {
    COND_HOOK(OnSaveActorSaveState, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnSaveActorSaveState* ev = (OnSaveActorSaveState*)event;
        Actor* actor = ev->actor;
        ActorLocal_CustomCollectible* customLocal = (ActorLocal_CustomCollectible*)&actor->local;
        if ((actor_e)actor->modelCacheIndex != ACTOR_3CD_CUSTOM_COLLECTIBLE) {
            return;
        }

        collectibleSaveState.insert(
            { customLocal->randoCheckId,
              { ev->actor->marker->propPtr->x, ev->actor->marker->propPtr->y, ev->actor->marker->propPtr->z } });
    });

    COND_HOOK(OnLoadActorSaveState, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnLoadActorSaveState* ev = (OnLoadActorSaveState*)event;
        if ((actor_e)ev->actor->modelCacheIndex != ACTOR_3CD_CUSTOM_COLLECTIBLE) {
            return;
        }

        event->Cancelled = true;

        if (collectibleSaveState.empty()) {
            return;
        }

        int32_t position[3];
        position[0] = ev->posX;
        position[1] = ev->posY;
        position[2] = ev->posZ;

        RandoCheckId randoCheckId = RC_UNKNOWN;
        for (auto& [checkId, location] : collectibleSaveState) {
            if (std::get<0>(location) == ev->posX && std::get<1>(location) == ev->posY &&
                std::get<2>(location) == ev->posZ) {
                randoCheckId = checkId;
                break;
            }
        }

        if (randoCheckId == RC_UNKNOWN || CustomCollectible::GetActorByRC(randoCheckId) != NULL) {
            return;
        }

        CustomCollectible::Spawn(position, randoCheckId);
        collectibleSaveState.erase(randoCheckId);
    });
}

static RegisterShipInitFunc initFunc(RegisterCustomCollectibleSaveState, { "IS_RANDO" });
