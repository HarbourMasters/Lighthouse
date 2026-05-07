#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Rando/Logic/Logic.h"

#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/enhancements/events/PortEnhancements.h"
#include "port/enhancements/events/hooks/Events.h"

std::vector<actor_e> junkItemList;

std::map<actor_e, const char*> junkOptionsMap = {
    { ACTOR_50_HONEYCOMB, CVAR_RANDOMIZER_SETTING("Junk.HealthRefill") },
    { ACTOR_52_BLUE_EGG, CVAR_RANDOMIZER_SETTING("Junk.BlueEggs") },
    { ACTOR_129_RED_FEATHER, CVAR_RANDOMIZER_SETTING("Junk.RedFeathers") },
    { ACTOR_370_GOLD_FEATHER, CVAR_RANDOMIZER_SETTING("Junk.GoldFeathers") },
};

void UpdateJunkList() {
    junkItemList.clear();

    if (CVarGetInteger(Rando::StaticData::Options[RO_SPAWN_JUNK].cvar, 0) == RO_GENERIC_OFF) {
        return;
    }

    for (auto& [actorId, cvar] : junkOptionsMap) {
        if (CVarGetInteger(cvar, 0)) {
            junkItemList.push_back(actorId);
        }
    }

    if (junkItemList.empty()) {
        junkItemList.push_back(ACTOR_50_HONEYCOMB);
    }
}

actor_e GetActorIdByShuffledObjectState(Rando::StaticData::RandoShuffledPool shuffledObject) {
    actor_e randoActorId = (actor_e)Rando::StaticData::Items[shuffledObject.randoItemId].actorId;

    if (CVarGetInteger(Rando::StaticData::Options[RO_SPAWN_JUNK].cvar, 0) == RO_GENERIC_ON) {
        randoActorId = shuffledObject.obtained ? junkItemList[(rand() % junkItemList.size())]
                                               : (actor_e)Rando::StaticData::Items[shuffledObject.randoItemId].actorId;
    } else {
        randoActorId = shuffledObject.obtained ? ACTOR_1_UNKNOWN
                                               : (actor_e)Rando::StaticData::Items[shuffledObject.randoItemId].actorId;
    }

    return randoActorId;
}
