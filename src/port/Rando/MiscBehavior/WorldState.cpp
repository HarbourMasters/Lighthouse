#include "MiscBehavior.h"
#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "spdlog/spdlog.h"
#include "enums.h"

#include "functions.h"
#include "port/ShipUtils.h"

// These read per-file save data, so they must not be evaluated before a file is selected.
#define EMPTY_HONEYCOMB_OPTION_ENABLED \
    (selectedFileNum != DEFAULT_FILE_NUM && RANDO_SAVE_OPTIONS[RO_SHUFFLE_EMPTY_HONEYCOMBS].optionValue)
#define JIGGY_OPTION_ENABLED (selectedFileNum != DEFAULT_FILE_NUM && RANDO_SAVE_OPTIONS[RO_SHUFFLE_JIGGIES].optionValue)
#define MUMBO_TOKENS_OPTION_ENABLED \
    (selectedFileNum != DEFAULT_FILE_NUM && RANDO_SAVE_OPTIONS[RO_SHUFFLE_MUMBO_TOKENS].optionValue)

bool isPauseMenu = false;

void Rando::StaticData::ModifyRandoInfFlagState(RandoCheckId randoCheckId) {
    RandoInf randoInfFlag = RANDO_INF_UNKNOWN;

    switch (randoCheckId) {
        case RC_CC_JIGGY_CLANKER_RAISED:
            randoInfFlag = RANDO_INF_CLANKER_RAISED;
            break;
        case RC_CC_JIGGY_RINGS:
            randoInfFlag = RANDO_INF_MINIGAME_RINGS_COMPLETED;
            break;
        case RC_RBB_JIGGY_SNORKEL:
            randoInfFlag = RANDO_INF_ANCHOR_RAISED;
            break;
        case RC_GV_JIGGY_WATER_PYRAMID:
        case RC_GV_MUMBO_TOKEN_INSIDE_WATER_PYRAMID:
            if (RANDO_SAVE_CHECKS[RC_GV_JIGGY_WATER_PYRAMID].obtained &&
                RANDO_SAVE_CHECKS[RC_GV_MUMBO_TOKEN_INSIDE_WATER_PYRAMID].obtained) {
                randoInfFlag = RANDO_INF_WATER_PYRAMID_DRAINED;
            }
            break;
        default:
            break;
    }

    if (randoInfFlag != RANDO_INF_UNKNOWN) {
        CALL_EVENT(SetRandoInfFlag, randoInfFlag, 1);
    }
}

void Rando::MiscBehavior::InitWorldStateBehavior() {
    REGISTER_LISTENER(SetRandoInfFlag, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        SetRandoInfFlag* ev = (SetRandoInfFlag*)event;

        if (!IS_RANDO) {
            return;
        }

        RandoInf flagId = (RandoInf)ev->flagId;

        if (flagId < RANDO_INF_UNKNOWN || flagId >= RANDO_INF_MAX) {
            return;
        }

        RANDO_SAVE_FLAGS[(RandoInf)flagId].flagState = ev->flagState;
    })

    COND_VB_SHOULD(VB_XMAS_TREE_JIGGY_COLLIDABLE, EVENT_PRIORITY_NORMAL, true, {
        Actor* jiggyActor = va_arg(args, Actor*);

        if (!IS_RANDO || !JIGGY_OPTION_ENABLED) {
            return;
        }

        if (jiggyActor != NULL && jiggyActor->marker != NULL && jiggyActor->marker->randoCheckId != RC_UNKNOWN) {
            *should = true;
        }
    })

    COND_VB_SHOULD(VB_COLLECTABLE_ALREADY_HELD, EVENT_PRIORITY_NORMAL, true, {
        Actor* collectable = va_arg(args, Actor*);

        if (!IS_RANDO || collectable == NULL || collectable->marker == NULL) {
            return;
        }

        RandoCheckId randoCheckId = (RandoCheckId)collectable->marker->randoCheckId;

        if (randoCheckId == RC_UNKNOWN || randoCheckId >= RC_MAX) {
            return;
        }

        *should = RANDO_SAVE_CHECKS[randoCheckId].obtained;
    })

    COND_VB_SHOULD(VB_JIGGYSCORE_LEVEL_TOTAL, EVENT_PRIORITY_NORMAL, true, {
        s32 level = va_arg(args, s32);
        s32* result = va_arg(args, s32*);

        if (!IS_RANDO || !JIGGY_OPTION_ENABLED) {
            return;
        }

        s32 total = 0;

        if (level > 0 && level < 0xB) {
            for (s32 jiggyId = (level - 1) * 10 + 1; jiggyId < level * 10 + 1; jiggyId++) {
                if (port_jiggyscore_isCollectedRaw((jiggy_e)jiggyId)) {
                    total++;
                }
            }
        }

        *result = total;
        *should = false;
    })

    COND_VB_SHOULD(VB_HONEYCOMBSCORE_LEVEL_TOTAL, EVENT_PRIORITY_NORMAL, true, {
        s32 level = va_arg(args, s32);
        s32* result = va_arg(args, s32*);

        if (!IS_RANDO || !EMPTY_HONEYCOMB_OPTION_ENABLED) {
            return;
        }

        s32 total = 0;

        if (level > 0 && level != LEVEL_6_LAIR && level < 0xC) {
            s32 first = level * 2 - 1;
            first = (level < 7) ? first : first - 2;
            s32 end = (level * 2 - 1 == 0x15) ? first + 6 : first + 2;

            for (s32 honeycombId = first; honeycombId < end; honeycombId++) {
                if (port_honeycombscore_getRaw((honeycomb_e)honeycombId)) {
                    total++;
                }
            }
        }

        *result = total;
        *should = false;
    })

    REGISTER_LISTENER(OnActorSpawn, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;

        if (!IS_RANDO) {
            return;
        }

        map_e currentMap = gsworld_getMap();
        level_e currentLevel = map_getLevel(currentMap);

        switch (currentLevel) {
            case LEVEL_1_MUMBOS_MOUNTAIN:
                if (RANDO_SAVE_CHECKS[RC_MM_JIGGY_CHIMPY].obtained) {
                    if (ev->actorId == ACTOR_F_CHIMPY) {
                        event->Cancelled = true;
                        ev->result = NULL;
                    }
                }
                mapSpecificFlags_set(MM_SPECIFIC_FLAG_0_CHIMPY_STUMP_RAISED,
                                     RANDO_SAVE_CHECKS[RC_MM_JIGGY_CHIMPY].obtained);
                mapSpecificFlags_set(MM_SPECIFIC_FLAG_2_ORANGE_HAS_BEEN_RETURNED,
                                     RANDO_SAVE_CHECKS[RC_MM_JIGGY_CHIMPY].obtained);
                mapSpecificFlags_set(MM_SPECIFIC_FLAG_3_CHIMPY_HAS_LEFT,
                                     RANDO_SAVE_CHECKS[RC_MM_JIGGY_CHIMPY].obtained);
                break;
            case LEVEL_3_CLANKERS_CAVERN:
                if (currentMap == MAP_22_CC_INSIDE_CLANKER &&
                    RANDO_SAVE_FLAGS[RANDO_INF_MINIGAME_RINGS_COMPLETED].flagState) {
                    func_8034E71C((Struct73s*)func_8034C5AC(0x131), 0x190, 12.0f);
                }
                break;
            case LEVEL_9_RUSTY_BUCKET_BAY:
                if (ev->actorId == 0x18F) {
                    mapSpecificFlags_set(0, RANDO_SAVE_CHECKS[RC_RBB_EMPTY_HONEYCOMB_BOAT_HOUSE].obtained);
                }
                break;
            case LEVEL_A_MAD_MONSTER_MANSION:
                if (ev->actorId == ACTOR_39_NAPPER && RANDO_SAVE_CHECKS[RC_MMM_JIGGY_MANSION_TABLE].obtained) {
                    event->Cancelled = true;
                    ev->result = NULL;
                }
                break;
            default:
                break;
        }
    })

    REGISTER_LISTENER(OnIsJiggyScoreCollected, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnIsJiggyScoreCollected* ev = (OnIsJiggyScoreCollected*)event;

        if (!IS_RANDO || !JIGGY_OPTION_ENABLED) {
            return;
        }

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByJiggyId(ev->jiggyId);

        if (randoCheckId == RC_UNKNOWN || !RANDO_SAVE_CHECKS[randoCheckId].isShuffled) {
            return;
        }

        level_e currentLevel = map_getLevel(gsworld_getMap());

        switch (ev->jiggyId) {
            case JIGGY_42_GV_WATER_PYRAMID:
                if (currentLevel == LEVEL_7_GOBIS_VALLEY) {
                    event->Cancelled = true;
                    ev->result = RANDO_SAVE_FLAGS[RANDO_INF_WATER_PYRAMID_DRAINED].flagState;
                    return;
                }
                break;
            default:
                break;
        }

        event->Cancelled = true;
        ev->result = RANDO_SAVE_CHECKS[randoCheckId].obtained;
    })

    REGISTER_LISTENER(OnIsJiggyScoreSpawned, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnIsJiggyScoreSpawned* ev = (OnIsJiggyScoreSpawned*)event;

        if (!IS_RANDO || !JIGGY_OPTION_ENABLED) {
            return;
        }

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByJiggyId(ev->jiggyId);

        if (randoCheckId != RC_UNKNOWN) {
            event->Cancelled = true;
            if (randoCheckId == RC_MMM_JIGGY_TUMBLARS_PUZZLE) {
                ev->result = mapSpecificFlags_get(MMM_SPECIFIC_FLAG_TUMBLAR_BROKEN);
            } else if (randoCheckId == RC_CC_JIGGY_CLANKER_RAISED) {
                // Clanker's height and his rings' water level are both recalled from
                // this query, so it has to answer for the world event and not just
                // for a reward still sitting in the level.
                ev->result = RANDO_SAVE_FLAGS[RANDO_INF_CLANKER_RAISED].flagState ||
                             CustomObject::CheckSpawnedIdList(randoCheckId);
            } else if (randoCheckId == RC_CC_JIGGY_RINGS) {
                ev->result = RANDO_SAVE_FLAGS[RANDO_INF_MINIGAME_RINGS_COMPLETED].flagState ||
                             CustomObject::CheckSpawnedIdList(randoCheckId);
            } else {
                ev->result = CustomObject::CheckSpawnedIdList(randoCheckId) || RANDO_SAVE_CHECKS[randoCheckId].obtained;
            }
        }
    })

    // Drop the id-keyed requirement for the MMM floorboard honeycomb (marker.c)
    COND_VB_SHOULD(VB_HONEYCOMB_PUMPKIN_REQUIREMENT, EVENT_PRIORITY_NORMAL, true, {
        (void)args;
        if (!IS_RANDO || !EMPTY_HONEYCOMB_OPTION_ENABLED) {
            return;
        }
        *should = false;
    })

    REGISTER_LISTENER(OnIsHoneycombScoreCollected, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnIsHoneycombScoreCollected* ev = (OnIsHoneycombScoreCollected*)event;

        if (!IS_RANDO || !EMPTY_HONEYCOMB_OPTION_ENABLED) {
            return;
        }

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByHoneycombId((honeycomb_e)ev->honeycombId);

        if (randoCheckId == RC_UNKNOWN || !RANDO_SAVE_CHECKS[randoCheckId].isShuffled) {
            return;
        }

        event->Cancelled = true;

        if (ev->honeycombId == HONEYCOMB_17_SM_COLLIWOBBLE) {
            ev->result = false;
            return;
        }

        ev->result = RANDO_SAVE_CHECKS[randoCheckId].obtained;
    })

    REGISTER_LISTENER(OnIsMumboTokenScoreCollected, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnIsMumboTokenScoreCollected* ev = (OnIsMumboTokenScoreCollected*)event;

        if (!IS_RANDO || !MUMBO_TOKENS_OPTION_ENABLED) {
            return;
        }

        if (getGameMode() == GAME_MODE_4_PAUSED) {
            return;
        }

        for (auto& saveCheck : RANDO_SAVE_CHECKS) {
            Rando::StaticData::RandoStaticItem randoItem = Rando::StaticData::Items[saveCheck.randoItemId];

            if (randoItem.randoItemType != RITYPE_MUMBO_TOKEN) {
                continue;
            }

            if (saveCheck.randoCollectionId == ev->tokenId) {
                event->Cancelled = true;
                ev->result = saveCheck.obtained;
                break;
            }
        }
    })
}