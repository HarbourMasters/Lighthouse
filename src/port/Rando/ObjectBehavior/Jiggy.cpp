#include "ObjectBehavior.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Rando/CustomCollectible/CustomCollectible.h"

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_JIGGIES].optionValue

bool OverrideJiggySpawn(f32 position[3], jiggy_e jiggyId) {
    RandoCheckId randoCheckId = Rando::StaticData::GetCheckByJiggyId(jiggyId);

    if (randoCheckId == RC_UNKNOWN || !Rando::Logic::IsCheckShuffled(randoCheckId)) {
        return false;
    }

    int32_t spawnPosition[3] = { (int32_t)position[0], (int32_t)position[1], (int32_t)position[2] };

    if (jiggyId == JIGGY_26_BGS_TANKTUP) {
        spawnPosition[0] -= 300;
        spawnPosition[1] += 100;
        spawnPosition[2] -= 300;
    }

    Actor* actor = CustomCollectible::Spawn(spawnPosition, randoCheckId);
    if (jiggyId != JIGGY_17_CC_CLANKER_RAISED && jiggyId != JIGGY_1B_CC_TOOTH) {
        ApplyCustomActorPhysics(randoCheckId, actor, false);
    }
    return true;
}

bool OverrideJiggyActorSpawn(OnActorSpawn* ev) {
    if (ev->actorId != ACTOR_46_JIGGY) {
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

void RegisterRandoJiggies() {
    COND_VB_SHOULD(VB_NAPPER_SET_JIGGY_POSITION, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, {
        if (RANDO_SAVE_CHECKS[RC_MMM_JIGGY_MANSION_TABLE].eligible) {
            *should = false;
        }
    });

    // Jiggies can be spawned through 2 separate code paths, that's why there's 2 hooks with slight
    // variations on how it identifies the jiggy.
    COND_VB_SHOULD(VB_OVERRIDE_JIGGY_SPAWN, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, {
        jiggy_e jiggyId = (jiggy_e)va_arg(args, int);
        f32* position = va_arg(args, f32*);

        bool replaceJiggy = OverrideJiggySpawn(position, jiggyId);
        if (replaceJiggy) {
            *should = true;
        }
    });

    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;

        bool replaceJiggy = OverrideJiggyActorSpawn(ev);
        if (replaceJiggy) {
            event->Cancelled = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterRandoJiggies, { "IS_RANDO" });
