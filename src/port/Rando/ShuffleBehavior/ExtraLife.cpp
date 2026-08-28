#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomCollectible/CustomCollectible.h"

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_MUMBO_TOKENS].optionValue

bool OverrideExtraLifeSpawn(OnActorSpawn* ev) {
    if (ev->actorId != ACTOR_49_EXTRA_LIFE) {
        return false;
    }

    int32_t spawnPosition[3] = { ev->posX, ev->posY, ev->posZ };
    RandoCheckId randoCheckId = Rando::StaticData::GetCheckByPosition(ev->posX, ev->posY, ev->posZ);

    if (randoCheckId == RC_UNKNOWN || !Rando::Logic::IsCheckShuffled(randoCheckId)) {
        return false;
    }

    if (!Rando::Logic::IsCheckObtained(randoCheckId)) {
        CustomCollectible::Spawn(spawnPosition, randoCheckId);
    }

    return true;
}

void RegisterRandoExtraLifes() {
    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;

        bool replaceExtraLife = OverrideExtraLifeSpawn(ev);
        if (replaceExtraLife) {
            event->Cancelled = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterRandoExtraLifes, { "IS_RANDO" });
