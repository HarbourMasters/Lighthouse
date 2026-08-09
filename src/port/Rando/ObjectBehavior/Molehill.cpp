#include <libultraship/bridge.h>
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomCollectible/CustomCollectible.h"

#define BRIDGE_REQUIREMENT 6

extern "C" {
s32 mapSpecificFlags_get(s32 i);
void mapSpecificFlags_set(s32 i, s32 val);
}

// clang-format off
std::vector<RandoCheckId> spiralMountainBridge = {
    RC_SM_MOLEHILL_JUMP,
    RC_SM_MOLEHILL_CAMERA_CONTROL,
    RC_SM_MOLEHILL_ATTACK,
    RC_SM_MOLEHILL_DIVE,
    RC_SM_MOLEHILL_CLIMB,
    RC_SM_MOLEHILL_BEAK_BARGE,
};
// clang-format on

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_MOLEHILLS].optionValue

bool CheckBridgeState() {
    int32_t smBridgeCheck = 0;
    if (!mapSpecificFlags_get(SM_SPECIFIC_FLAG_3_ALL_SM_ABILITIES_LEARNED)) {
        for (auto& check : spiralMountainBridge) {
            if (RANDO_SAVE_CHECKS[check].eligible) {
                smBridgeCheck++;
            }
        }
    }

    return smBridgeCheck == BRIDGE_REQUIREMENT ? true : false;
}

bool OverrideMoleSpawn(OnActorSpawn* ev) {
    if (ev->actorId != ACTOR_12B_TUTORIAL_BOTTLES && ev->actorId != ACTOR_37A_BOTTLES) {
        return false;
    }

    int32_t spawnPosition[3] = { ev->posX, ev->posY, ev->posZ };
    RandoCheckId randoCheckId = Rando::StaticData::GetCheckByPosition(ev->posX, ev->posY, ev->posZ);

    if (randoCheckId == RC_UNKNOWN || !Rando::Logic::IsCheckShuffled(randoCheckId)) {
        return false;
    }

    if (Rando::Logic::IsCheckObtained(randoCheckId)) {
        return true;
    }

    CustomCollectible::Spawn(spawnPosition, randoCheckId);
    return true;
}

void RegisterRandoMolehills() {
    COND_HOOK(OnCheckSpiralMountainAbilities, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnCheckSpiralMountainAbilities* ev = (OnCheckSpiralMountainAbilities*)event;
        if (mapSpecificFlags_get(SM_SPECIFIC_FLAG_3_ALL_SM_ABILITIES_LEARNED)) {
            mapSpecificFlags_set(SM_SPECIFIC_FLAG_3_ALL_SM_ABILITIES_LEARNED, false);
        }

        if (CheckBridgeState()) {
            mapSpecificFlags_set(SM_SPECIFIC_FLAG_3_ALL_SM_ABILITIES_LEARNED, true);
            mapSpecificFlags_set(SM_SPECIFIC_FLAG_1_TALKED_TO_BOTTLES, true);
            event->Cancelled = true;
            ev->result = true;
        }
    });

    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;

        bool replaceMole = OverrideMoleSpawn(ev);
        if (replaceMole) {
            event->Cancelled = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterRandoMolehills, { "IS_RANDO" });
