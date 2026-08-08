#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomCollectible/CustomCollectible.h"

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_MUMBO_TOKENS].optionValue

bool OverrideMumboTokenSpawn(OnActorSpawn* ev) {
    if (ev->actorId != ACTOR_2D_MUMBO_TOKEN) {
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

void RegisterRandoMumboTokens() {
    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;

        bool replaceMumboToken = OverrideMumboTokenSpawn(ev);
        if (replaceMumboToken) {
            event->Cancelled = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterRandoMumboTokens, { "IS_RANDO" });
