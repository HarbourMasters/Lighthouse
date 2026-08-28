#include "WorldState.h"

#include "port/ShipInit.hpp"
#include "port/Rando/CustomCollectible/CustomCollectible.h"
#include "port/Rando/StaticData/StaticData.h"

#include "enums.h"
#include "functions.h"

#define JIGGY_OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_JIGGIES].optionValue

void Rando::WorldState::SetRandoInfFlag(RandoInf flagId, bool flagState) {
    if (flagId < RANDO_INF_UNKNOWN || flagId >= RANDO_INF_MAX) {
        return;
    }

    RANDO_SAVE_FLAGS[(RandoInf)flagId].flagState = flagState;
}

void SetFlagsOnCheckObtained(RandoCheckId randoCheckId) {
    // Set flags when receiving checks that are normally triggered by collecting the vanilla counterpart.
    switch (randoCheckId) {
        case RC_CC_JIGGY_TOOTH:
            mapSpecificFlags_set(LEVEL_FLAG_1_CC_JIGGY_TOOTH_OPEN, 1);
            break;
        case RC_CC_MUMBO_TOKEN_CLANKERS_LEFT_TOOTH:
            mapSpecificFlags_set(LEVEL_FLAG_0_CC_TOKEN_TOOTH_OPEN, 1);
            break;
        case RC_GV_MUMBO_TOKEN_INSIDE_WATER_PYRAMID:
            if (RANDO_SAVE_CHECKS[RC_GV_JIGGY_WATER_PYRAMID].eligible &&
                RANDO_SAVE_CHECKS[RC_GV_MUMBO_TOKEN_INSIDE_WATER_PYRAMID].eligible) {
                Rando::WorldState::SetRandoInfFlag(RANDO_INF_WATER_PYRAMID_DRAINED, true);
            }
            break;
        default:
            break;
    }
}

void RegisterWorldState() {

    COND_HOOK(OnRandoCheckObtained, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        auto ev = reinterpret_cast<OnRandoCheckObtained*>(event);
        SetFlagsOnCheckObtained((RandoCheckId)ev->randoCheckId);
    });

    COND_HOOK(OnIsJiggyScoreSpawned, EVENT_PRIORITY_NORMAL, IS_RANDO && JIGGY_OPTION_ENABLED, [](IEvent* event) {
        // Map spawned vanilla jiggy ID to spawned custom collectible.
        OnIsJiggyScoreSpawned* ev = (OnIsJiggyScoreSpawned*)event;

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByJiggyId(ev->jiggyId);

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        event->Cancelled = true;

        Actor* actor = CustomCollectible::GetActorByRC(randoCheckId);
        ev->result = actor != NULL || RANDO_SAVE_CHECKS[randoCheckId].eligible;

        /*
        if (randoCheckId == RC_MMM_JIGGY_TUMBLARS_PUZZLE) {
            ev->result = mapSpecificFlags_get(MMM_SPECIFIC_FLAG_TUMBLAR_BROKEN);
        }
        */
    });

    COND_HOOK(OnIsJiggyScoreCollected, EVENT_PRIORITY_NORMAL, IS_RANDO && JIGGY_OPTION_ENABLED, [](IEvent* event) {
        OnIsJiggyScoreCollected* ev = (OnIsJiggyScoreCollected*)event;

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByJiggyId(ev->jiggyId);

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        event->Cancelled = true;

        // Exception for draining the pyramid as it's otherwise possible to permanently miss a Mumbo Token.
        if (ev->jiggyId == JIGGY_42_GV_WATER_PYRAMID) {
            ev->result = RANDO_SAVE_FLAGS[RANDO_INF_WATER_PYRAMID_DRAINED].flagState;
        } else {
            ev->result = RANDO_SAVE_CHECKS[randoCheckId].eligible;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterWorldState, { "IS_RANDO" });
