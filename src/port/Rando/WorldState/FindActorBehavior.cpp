#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Rando/Rando.h"
#include "port/Rando/CustomCollectible/CustomCollectible.h"

extern "C" {
#include "functions.h"
}

void RegisterFindActorBehavior() {
    COND_HOOK(OnFindActorFromActorId, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnFindActorFromActorId* ev = (OnFindActorFromActorId*)event;
        RandoCheckId randoCheckId = RC_UNKNOWN;
        map_e currentMap = gsworld_getMap();
        ev->result = NULL;

        switch (ev->actorId) {
            case ACTOR_46_JIGGY:
                if (currentMap == MAP_26_MMM_NAPPERS_ROOM) {
                    randoCheckId = RC_MMM_JIGGY_MANSION_TABLE;
                } else if (currentMap == MAP_5A_CCW_SUMMER_ZUBBA_HIVE || currentMap == MAP_5B_CCW_SPRING_ZUBBA_HIVE) {
                    randoCheckId = RC_CCW_JIGGY_ZUBBAS;
                }
                break;
            default:
                if (currentMap == MAP_D_BGS_BUBBLEGLOOP_SWAMP) {
                    if (CustomCollectible::GetActorByRC(RC_BGS_JIGGY_ELEVATED_WALKWAY) != NULL) {
                        randoCheckId = RC_BGS_JIGGY_ELEVATED_WALKWAY;
                    } else if (CustomCollectible::GetActorByRC(RC_BGS_JIGGY_MAZE) != NULL) {
                        randoCheckId = RC_BGS_JIGGY_MAZE;
                    }
                }
                break;
        }

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        ev->result = CustomCollectible::GetActorByRC(randoCheckId);

        if (ev->result != NULL) {
            event->Cancelled = true;
        }
    })

    COND_HOOK(OnFindActorMarkerFromJiggyId, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnFindActorMarkerFromJiggyId* ev = (OnFindActorMarkerFromJiggyId*)event;
        RandoCheckId randoCheckId = RC_UNKNOWN;

        switch (ev->jiggyId) {
            case JIGGY_3E_GV_GRABBA:
                randoCheckId = RC_GV_JIGGY_GRABBA;
                break;
            default:
                return;
        }

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        if (CustomCollectible::GetActorByRC(randoCheckId) != NULL) {
            event->Cancelled = true;
            ev->result = CustomCollectible::GetActorByRC(randoCheckId)->marker;
        }
    })

    COND_HOOK(OnFindClosestActorFromActorId, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnFindClosestActorFromActorId* ev = (OnFindClosestActorFromActorId*)event;
        RandoCheckId randoCheckId = RC_UNKNOWN;
        map_e currentMap = gsworld_getMap();
        ev->result = NULL;

        switch (ev->actorId) {
            case ACTOR_46_JIGGY:
                if (currentMap == MAP_24_MMM_TUMBLARS_SHED) {
                    randoCheckId = RC_MMM_JIGGY_TUMBLARS_PUZZLE;
                }
                break;
            default:
                break;
        }

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        ev->result = CustomCollectible::GetActorByRC(randoCheckId);

        if (ev->result != NULL) {
            event->Cancelled = true;
        }
    })
}

static RegisterShipInitFunc initFunc(RegisterFindActorBehavior, { "IS_RANDO" });
