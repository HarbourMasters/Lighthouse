#include "MiscBehavior.h"
#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "spdlog/spdlog.h"
#include "enums.h"

extern "C" {
s32 getGameMode(void);
void mapSpecificFlags_set(s32 i, s32 val);
s32 mapSpecificFlags_get(s32 i);

enum map_e gsworld_getMap(void);
enum level_e map_getLevel(enum map_e map);

Struct70s* func_8034C5AC(s32 arg0);
void func_8034E71C(Struct73s* arg0, s32 arg1, f32 arg2);
}

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

        if (flagId < RANDO_INF_UNKNOWN && flagId > RANDO_INF_MAX) {
            return;
        }

        RANDO_SAVE_FLAGS[(RandoInf)flagId].flagState = ev->flagState;
    })

    REGISTER_LISTENER(OnGetLevelSpecificFlag, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnGetLevelSpecificFlag* ev = (OnGetLevelSpecificFlag*)event;

        if (!IS_RANDO) {
            return;
        }

        map_e currentMap = gsworld_getMap();

        switch (ev->flagId) {
            case LEVEL_FLAG_29_FP_XMAS_TREE_COMPLETE:
                if (currentMap == MAP_53_FP_CHRISTMAS_TREE) {
                    return;
                }
                event->Cancelled = true;
                ev->result = 1;
                break;
            default:
                break;
        }
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

        if (!IS_RANDO && !JIGGY_OPTION_ENABLED) {
            return;
        }

        if (getGameMode() == GAME_MODE_4_PAUSED) {
            return;
        }

        map_e currentMap = gsworld_getMap();
        level_e currenLevel = map_getLevel(currentMap);

        for (auto& randoSaveCheck : RANDO_SAVE_CHECKS) {
            if (randoSaveCheck.randoItemId != RITYPE_JIGGY) {
                continue;
            }

            if (randoSaveCheck.randoCollectionId == ev->jiggyId) {
                if (ev->jiggyId == JIGGY_5D_MMM_NAPPER) {
                    if (currentMap == MAP_26_MMM_NAPPERS_ROOM &&
                        randoSaveCheck.randoCheckId != RC_MMM_JIGGY_MANSION_TABLE) {
                        event->Cancelled = true;
                        ev->result = RANDO_SAVE_CHECKS[RC_MMM_JIGGY_MANSION_TABLE].obtained;
                        break;
                    }
                }
                if (ev->jiggyId == JIGGY_42_GV_WATER_PYRAMID) {
                    if (currenLevel == LEVEL_7_GOBIS_VALLEY) {
                        event->Cancelled = true;
                        ev->result = RANDO_SAVE_FLAGS[RANDO_INF_WATER_PYRAMID_DRAINED].flagState;
                        break;
                    }
                }
                if (ev->jiggyId == JIGGY_2E_FP_PRESENTS) {
                    if (currenLevel == LEVEL_5_FREEZEEZY_PEAK) {
                        event->Cancelled = true;
                        ev->result = RANDO_SAVE_CHECKS[RC_FP_JIGGY_IGLOO].obtained;
                        break;
                    }
                }

                event->Cancelled = true;
                ev->result = randoSaveCheck.obtained;
                break;
            }
        }
    })

    REGISTER_LISTENER(OnIsJiggyScoreSpawned, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnIsJiggyScoreSpawned* ev = (OnIsJiggyScoreSpawned*)event;

        if (!IS_RANDO && !JIGGY_OPTION_ENABLED) {
            return;
        }

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByJiggyId(ev->jiggyId);

        if (randoCheckId != RC_UNKNOWN) {
            event->Cancelled = true;
            if (randoCheckId == RC_MMM_JIGGY_TUMBLARS_PUZZLE) {
                ev->result = mapSpecificFlags_get(MMM_SPECIFIC_FLAG_TUMBLAR_BROKEN);
            } else if (randoCheckId == RC_CC_JIGGY_CLANKER_RAISED) {
                ev->result = RANDO_SAVE_FLAGS[RANDO_INF_CLANKER_RAISED].flagState;
            } else {
                ev->result = CustomObject::CheckSpawnedIdList(randoCheckId);
            }
        }
    })

    REGISTER_LISTENER(OnIsHoneycombScoreCollected, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnIsHoneycombScoreCollected* ev = (OnIsHoneycombScoreCollected*)event;

        if (!IS_RANDO && !EMPTY_HONEYCOMB_OPTION_ENABLED) {
            return;
        }

        if (getGameMode() == GAME_MODE_4_PAUSED) {
            return;
        }

        for (auto& saveCheck : RANDO_SAVE_CHECKS) {
            Rando::StaticData::RandoStaticItem randoItem = Rando::StaticData::Items[saveCheck.randoItemId];

            if (randoItem.randoItemType != RITYPE_EMPTY_HONEYCOMB) {
                continue;
            }

            if (saveCheck.randoCollectionId == ev->honeycombId) {
                event->Cancelled = true;

                if (ev->honeycombId == HONEYCOMB_17_SM_COLLIWOBBLE) {
                    ev->result = false;
                } else {
                    ev->result = saveCheck.obtained;
                }

                break;
            }
        }
    })

    REGISTER_LISTENER(OnIsMumboTokenScoreCollected, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnIsMumboTokenScoreCollected* ev = (OnIsMumboTokenScoreCollected*)event;

        if (!IS_RANDO && !MUMBO_TOKENS_OPTION_ENABLED) {
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