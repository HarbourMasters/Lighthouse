#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "spdlog/spdlog.h"

extern "C" {
void func_80332BB0(void);
}

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_JIGGIES].optionValue

void PlaySpawnSound(RandoCheckId randoCheckId) {
    RandoSaveCheck randoSaveCheck = Rando::Logic::GetShuffledObject(randoCheckId);
    if (randoSaveCheck.obtained) {
        return;
    }
    if (randoSaveCheck.randoItemId != RI_JIGGY) {
        coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 28000);
        return;
    }
    midichannel_incOrDecCounter(false);
    func_8025A55C(0, 4000, 5);
    coMusicPlayer_playMusic(COMUSIC_3D_JIGGY_SPAWN, 0x7FFF);
    timedFunc_set_0(3.5f, func_80332BB0);
}

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
        case RC_MMM_JIGGY_MOTZAND:
            jiggyscore_setSpawned(JIGGY_60_MMM_MOTZHAND, true);
            return;
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
        if (randoCustomActor != NULL && jiggyId != JIGGY_3E_GV_GRABBA && jiggyId != JIGGY_0B_TTC_JINJO) {
            PlaySpawnSound(randoCheckId);
        }
        if (jiggyId != JIGGY_17_CC_CLANKER_RAISED && jiggyId != JIGGY_1B_CC_TOOTH) {
            ApplyCustomActorPhysics(randoCheckId, randoCustomActor, false);
        }
        // Stand in for the jiggyscore_setSpawned this override skips, for the
        // jiggies the world reads back as state rather than as a collectable.
        MarkWorldStateFromJiggySpawn(randoCheckId);
        *should = true;
    })

    // The witch switches' lair cutscenes play their own jingle
    COND_VB_SHOULD(VB_JIGGY_SPAWN_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        jiggy_e jiggyId = (jiggy_e)va_arg(args, int);
        s32* track = va_arg(args, s32*);

        if (!IS_RANDO || !OPTION_ENABLED) {
            return;
        }

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByJiggyId(jiggyId);

        if (randoCheckId == RC_UNKNOWN || !Rando::Logic::IsCheckShuffled(randoCheckId)) {
            return;
        }

        if (Rando::Logic::GetShuffledObject(randoCheckId).randoItemId != RI_JIGGY) {
            *track = COMUSIC_2B_DING_B;
        }
    })

    // The last jiggy placed in this world, however many it holds. No marker means the jiggy was already collected.
    COND_VB_SHOULD(VB_LAST_JIGGY_FANFARE, EVENT_PRIORITY_NORMAL, true, {
        ActorMarker* marker = va_arg(args, ActorMarker*);

        if (!IS_RANDO || !OPTION_ENABLED) {
            return;
        }

        const RandoCheckId collecting = marker != NULL ? (RandoCheckId)marker->randoCheckId : RC_UNKNOWN;
        const int32_t level = map_getLevel(gsworld_getMap());
        bool inWorld = marker == NULL;
        for (const auto& object : Rando::Logic::shuffledPool) {
            if (!object.isShuffled || object.randoItemId != RI_JIGGY ||
                Rando::StaticData::Checks[object.randoCheckId].worldId != level) {
                continue;
            }
            if (object.randoCheckId == collecting) {
                inWorld = true;
            } else if (!object.obtained) {
                *should = false;
                return;
            }
        }
        *should = inWorld;
    })
}
