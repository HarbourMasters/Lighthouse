#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "spdlog/spdlog.h"

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_JIGGIES].optionValue

// Some jiggies double as the world's only memory of what opened them up: raising
// Clanker and finishing his rings are both recalled purely from
// jiggyscore_isSpawned on the jiggy that appeared. Vanilla records that in
// jiggy_spawn via jiggyscore_setSpawned, which the override below never reaches,
// and the check-obtained path only fires once the reward is actually picked up.
static void MarkWorldStateFromJiggySpawn(RandoCheckId randoCheckId) {
    RandoInf randoInfFlag = RANDO_INF_UNKNOWN;
    switch (randoCheckId) {
        case RC_CC_JIGGY_CLANKER_RAISED:
            randoInfFlag = RANDO_INF_CLANKER_RAISED;
            break;
        case RC_CC_JIGGY_RINGS:
            randoInfFlag = RANDO_INF_MINIGAME_RINGS_COMPLETED;
            break;
        default:
            return;
    }
    CALL_EVENT(SetRandoInfFlag, randoInfFlag, 1);
}

void Rando::ObjectBehavior::InitJiggyBehavior() {
    COND_VB_SHOULD(VB_NAPPER_SET_JIGGY_POSITION, EVENT_PRIORITY_NORMAL, true, {
        if (!IS_RANDO && !OPTION_ENABLED) {
            return;
        }

        if (RANDO_SAVE_CHECKS[RC_MMM_JIGGY_MANSION_TABLE].obtained) {
            *should = false;
        }
    })

    COND_VB_SHOULD(VB_OVERRIDE_JIGGY_SPAWN, EVENT_PRIORITY_NORMAL, true, {
        jiggy_e jiggyId = (jiggy_e)va_arg(args, int);
        f32* position = va_arg(args, f32*);

        if (!IS_RANDO && !OPTION_ENABLED) {
            return;
        }

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByJiggyId(jiggyId);

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        if (!Rando::Logic::IsCheckShuffled(randoCheckId)) {
            return;
        }

        int32_t spawnPosition[3];
        spawnPosition[0] = (int32_t)position[0];
        spawnPosition[1] = (int32_t)position[1];
        spawnPosition[2] = (int32_t)position[2];

        if (jiggyId == JIGGY_26_BGS_TANKTUP) {
            spawnPosition[0] -= 300;
            spawnPosition[1] += 100;
            spawnPosition[2] -= 300;
        }

        Actor* randoCustomActor = CustomObject::ShouldCreateCustomActorEX(randoCheckId, spawnPosition, false);
        if (jiggyId != JIGGY_17_CC_CLANKER_RAISED && jiggyId != JIGGY_1B_CC_TOOTH) {
            ApplyCustomActorPhysics(randoCheckId, randoCustomActor, false);
        }
        // Stand in for the jiggyscore_setSpawned this override skips, for the
        // jiggies the world reads back as state rather than as a collectable.
        MarkWorldStateFromJiggySpawn(randoCheckId);
        *should = true;
    })
}
