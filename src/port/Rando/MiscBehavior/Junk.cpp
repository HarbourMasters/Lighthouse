#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Rando/Logic/Logic.h"

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

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

actor_e GetRandomJunkActorId(RandoSaveCheck shuffledObject) {
    if (CVarGetInteger(Rando::StaticData::Options[RO_SPAWN_JUNK].cvar, 0) == RO_GENERIC_ON) {
        UpdateJunkList();
        return junkItemList[(rand() % junkItemList.size())];
    }

    return ACTOR_1_UNKNOWN;
}
