#include "ObjectBehavior.h"
#include "port/UI/UIWidgets.hpp"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/UI/Notification.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include "include/core1/sns.h"

#define WIDGET_TEXT_COLOR(id) UIWidgets::ColorValues.at(id)
#define CVAR_NAME_SHOW_COLLISION_NOTIFICATIONS "gRandoSettings.RandoNotifications"
#define CVAR_SHOW_COLLISION_NOTIFICATIONS CVarGetInteger(CVAR_NAME_SHOW_COLLISION_NOTIFICATIONS, 0)

extern "C" {
extern ActorArray* suBaddieActorArray;
}

std::map<RandoCheckId, std::tuple<int32_t, int32_t, int32_t>> randoSaveState;

// clang-format off
std::vector<int32_t> actorSpawnWhitelist = {
    ACTOR_2D_MUMBO_TOKEN,
    ACTOR_46_JIGGY,
    ACTOR_47_EMPTY_HONEYCOMB,
    ACTOR_51_MUSIC_NOTE,
    ACTOR_5E_JINJO_YELLOW,
    ACTOR_5F_JINJO_ORANGE,
    ACTOR_60_JINJO_BLUE,
    ACTOR_61_JINJO_PINK,
    ACTOR_62_JINJO_GREEN,
    ACTOR_25E_SNS_EGG,
    ACTOR_25D_ICE_KEY,
    //ACTOR_12C_MOLEHILL,
};

std::map<int32_t, UIWidgets::Colors> randoItemColors = {
    { RI_UNKNOWN,           UIWidgets::Colors::Brown },
    { RI_EMPTY_HONEYCOMB,   UIWidgets::Colors::Yellow },
    { RI_JIGGY,             UIWidgets::Colors::Yellow },
    { RI_JINJO_BLUE,        UIWidgets::Colors::SkyBlue },
    { RI_JINJO_GREEN,       UIWidgets::Colors::Green },
    { RI_JINJO_ORANGE,      UIWidgets::Colors::Orange },
    { RI_JINJO_PINK,        UIWidgets::Colors::Pink },
    { RI_JINJO_YELLOW,      UIWidgets::Colors::Yellow },
    { RI_MOLEHILL,          UIWidgets::Colors::Cyan },
    { RI_MUMBO_TOKEN,       UIWidgets::Colors::Gray },
    { RI_MUSIC_NOTE,        UIWidgets::Colors::Yellow },
    { RI_STOP_N_SWOP_EGG,   UIWidgets::Colors::Pink },
    { RI_STOP_N_SWOP_KEY,   UIWidgets::Colors::White },
};

std::map<int32_t, UIWidgets::Colors> snsItemColors = {
    { SNS_ITEM_EGG_YELLOW,  UIWidgets::Colors::Yellow },
    { SNS_ITEM_EGG_RED,     UIWidgets::Colors::Red },
    { SNS_ITEM_EGG_GREEN,   UIWidgets::Colors::Green },
    { SNS_ITEM_EGG_BLUE,    UIWidgets::Colors::Blue },
    { SNS_ITEM_EGG_PINK,    UIWidgets::Colors::Pink },
    { SNS_ITEM_EGG_CYAN,    UIWidgets::Colors::Cyan },
    { SNS_ITEM_ICE_KEY,     UIWidgets::Colors::White },
};

std::map<int32_t, actor_e> jinjoMarkerMap = {
    { MARKER_5A_JINJO_BLUE, 	ACTOR_60_JINJO_BLUE },
    { MARKER_5B_JINJO_GREEN, 	ACTOR_62_JINJO_GREEN },
    { MARKER_5C_JINJO_ORANGE, 	ACTOR_5F_JINJO_ORANGE },
    { MARKER_5D_JINJO_PINK, 	ACTOR_61_JINJO_PINK },
    { MARKER_5E_JINJO_YELLOW, 	ACTOR_5E_JINJO_YELLOW },
};

std::vector<RandoCheckId> enemyKillOverlapList = {
    RC_CC_MUMBO_TOKEN_CHOMPA_BEHIND_CLANKERS_TAIL,
    RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_1,
    RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_2,
    RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_3,
    RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_4,
    RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_5,
    RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_6,
};
// clang-format on

bool nextActorSaveState = false;

bool IsActorWhitelisted(int32_t actorId) {
    for (auto& entry : actorSpawnWhitelist) {
        if (entry == actorId) {
            return true;
        }
    }

    if (CVarGetInteger(Rando::StaticData::Options[RO_SPAWN_JUNK].cvar, 0) == RO_GENERIC_ON) {
        for (auto& junk : junkItemList) {
            if (junk == actorId) {
                return true;
            }
        }
    }

    return false;
}

int32_t GetJinjoActorMarkerId(actor_e actorId) {
    for (auto& [marker, actor] : jinjoMarkerMap) {
        if (actor == actorId) {
            return marker;
        }
    }

    return NULL;
}

Actor* FindActorByRandoCheckId(RandoCheckId randoCheckId) {
    Actor* start;
    Actor* end;
    Actor* baddieActor = NULL;

    if (CustomObject::CheckSpawnedIdList(randoCheckId)) {
        start = suBaddieActorArray->data;
        end = start + suBaddieActorArray->cnt;
        for (baddieActor = start; baddieActor < end; baddieActor++) {

            if (baddieActor == nullptr) {
                continue;
            }

            if (baddieActor->marker == nullptr) {
                continue;
            }

            if (baddieActor->marker->randoCheckId == randoCheckId) {
                return baddieActor;
            }
        }
    }

    if (baddieActor == NULL) {
        Rando::StaticData::RandoStaticCheck randoStaticCheck = Rando::StaticData::Checks[randoCheckId];
        RandoSaveCheck randoSaveCheck = RANDO_SAVE_CHECKS[randoCheckId];
        actor_e actorId = (actor_e)Rando::StaticData::Items[randoSaveCheck.randoItemId].actorId;

        int32_t position[3];
        position[0] = randoStaticCheck.posX;
        position[1] = randoStaticCheck.posY + 50;
        position[2] = randoStaticCheck.posZ;

        Actor* newActor = CustomObject::SpawnCustomActorEX(randoCheckId, position, &actorInfoMap.at(actorId).first,
                                                           actorInfoMap.at(actorId).second);

        return newActor;
    }

    return NULL;
}

// subject is "You" or a teammate's name for Anchor remote collects.
static void EmitCheckNotification(RandoCheckId randoCheckId, const std::string& subject) {
    RandoSaveCheck randoSaveCheck = RANDO_SAVE_CHECKS[randoCheckId];
    std::string prefix;
    std::string message;
    std::string suffix = "";
    ImVec4 itemColor = WIDGET_TEXT_COLOR(randoItemColors.at(randoSaveCheck.randoItemId));

    if (randoSaveCheck.randoItemId == RI_MOLEHILL) {
        prefix = subject + " learned";
        message = abilityNameList[randoSaveCheck.randoCollectionId].c_str();
    } else if (randoSaveCheck.randoItemId == RI_STOP_N_SWOP_EGG || randoSaveCheck.randoItemId == RI_STOP_N_SWOP_KEY) {
        int32_t totalsnsItems = Rando::Logic::GetTotalSnsItemsCollected();
        prefix = subject + " collected ";
        prefix += Rando::StaticData::Items[randoSaveCheck.randoItemId].article;

        message = Rando::StaticData::Items[randoSaveCheck.randoItemId].name;
        suffix = "(";
        suffix += std::to_string(totalsnsItems);
        suffix += " / 7)";

        itemColor = WIDGET_TEXT_COLOR(snsItemColors.at(randoSaveCheck.randoCollectionId));
    } else {
        prefix = subject + " collected ";
        prefix += Rando::StaticData::Items[randoSaveCheck.randoItemId].article;
        message = Rando::StaticData::Items[randoSaveCheck.randoItemId].name;
    }

    Notification::Emit({
        .prefix = prefix,
        .prefixColor = WIDGET_TEXT_COLOR(UIWidgets::Colors::White),
        .message = message,
        .messageColor = itemColor,
        .suffix = suffix,
        .suffixColor = WIDGET_TEXT_COLOR(UIWidgets::Colors::White),
    });
}

void Rando::StaticData::SendCollisionNotification(RandoCheckId randoCheckId) {
    if (CVAR_SHOW_COLLISION_NOTIFICATIONS) {
        EmitCheckNotification(randoCheckId, "You");
    }
};

void Rando::StaticData::SendRemoteCheckNotification(RandoCheckId randoCheckId, const std::string& collectorName) {
    EmitCheckNotification(randoCheckId, collectorName);
};

bool ShouldOverrideSpawn(RandoCheckId randoCheckId) {
    if (Rando::Logic::IsCheckShuffled(randoCheckId)) {
        return true;
    }

    return false;
}

bool CheckEnemyOverlapPosition(int32_t pos[3]) {
    level_e levelId = map_getLevel(gsworld_getMap());
    bool enemyOverlap = false;

    for (auto& check : enemyKillOverlapList) {
        if (Rando::StaticData::Checks[check].worldId != levelId) {
            continue;
        }

        int32_t checkPosition[3];
        checkPosition[0] = Rando::StaticData::Checks[check].posX;
        checkPosition[1] = Rando::StaticData::Checks[check].posY;
        checkPosition[2] = Rando::StaticData::Checks[check].posZ;

        int32_t posMatches = 0;
        for (int i = 0; i < 3; i++) {
            if (pos[i] == checkPosition[i]) {
                posMatches++;
            }
        }
        if (posMatches == 3) {
            enemyOverlap = true;
        }
    }

    return enemyOverlap;
}

// CALL_EVENT expands to a braced initializer whose commas don't survive being passed through the
// COND_HOOK macro wrapper, so fire it from a plain function the listener calls instead.
static void FireClearBundleDespawnQueue() {
    CALL_EVENT(ClearBundleDespawnQueue);
}

// Entry point for the module, run once on game boot
void Rando::ObjectBehavior::Init() {
    InitBundleBehavior();
    InitJiggyBehavior();
    InitJinjoBehavior();
    InitMolehillBehavior();
    InitMusicNoteBehavior();
    InitPropBehavior();
    InitStopNSwopBehavior();

    UpdateJunkList();

    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;
        map_e currentMap = gsworld_getMap();

        CustomObject::FlushRandoSpawnQueue();
        DespawnCollectedBundles();

        if (currentMap == MAP_12_GV_GOBIS_VALLEY) {
            if (ev->actorId == ACTOR_118_GRABBA) {
                event->Cancelled = RANDO_SAVE_CHECKS[RC_GV_JIGGY_GRABBA].obtained;
                ev->result = NULL;
            }
        }

        if (!IsActorWhitelisted(ev->actorId)) {
            return;
        }

        int32_t position[3];
        position[0] = ev->posX;
        position[1] = ev->posY;
        position[2] = ev->posZ;

        if ((currentMap == MAP_B_CC_CLANKERS_CAVERN && ev->actorId != ACTOR_2D_MUMBO_TOKEN) ||
            currentMap == MAP_43_CCW_SPRING) {
            if (CheckEnemyOverlapPosition(position)) {
                return;
            }
        }

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByPosition(ev->posX, ev->posY, ev->posZ);
        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        if (!Rando::Logic::IsCheckShuffled(randoCheckId)) {
            return;
        }

        event->Cancelled = true;
        Actor* randoCustomActor = CustomObject::ShouldCreateCustomActorEX(randoCheckId, position, false);
        ev->result = randoCustomActor;
    })

    COND_HOOK(OnSaveActorSaveState, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnSaveActorSaveState* ev = (OnSaveActorSaveState*)event;

        if (!IsActorWhitelisted((actor_e)ev->actor->modelCacheIndex)) {
            return;
        }

        randoSaveState.insert(
            { (RandoCheckId)ev->actor->marker->randoCheckId,
              { ev->actor->marker->propPtr->x, ev->actor->marker->propPtr->y, ev->actor->marker->propPtr->z } });
    })

    COND_HOOK(OnLoadActorSaveState, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnLoadActorSaveState* ev = (OnLoadActorSaveState*)event;

        // Decide up front whether this restore is ours: anything we don't manage falls
        // through to the vanilla restore untouched. The predicate has to be the same one
        // the save side recorded under, junk included.
        if (!IsActorWhitelisted((actor_e)ev->actor->modelCacheIndex)) {
            return;
        }

        if (randoSaveState.empty()) {
            return;
        }

        int32_t position[3];
        position[0] = ev->posX;
        position[1] = ev->posY;
        position[2] = ev->posZ;

        RandoCheckId randoCheckId = RC_UNKNOWN;
        for (auto& [checkId, location] : randoSaveState) {
            if (std::get<0>(location) == ev->posX && std::get<1>(location) == ev->posY &&
                std::get<2>(location) == ev->posZ) {
                randoCheckId = checkId;
                break;
            }
        }

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        // The check already has a live actor. Restoring would stack a second one on
        // top of it, so drop the restore entirely.
        if (CustomObject::CheckSpawnedIdList(randoCheckId)) {
            event->Cancelled = true;
            return;
        }

        // refActor keeps an obtained check restoring the junk actor it was saved as
        // instead of rolling a fresh one.
        CustomObject::ShouldCreateCustomActorEX(randoCheckId, position, false, ev->actor);
        randoSaveState.erase(randoCheckId);
        event->Cancelled = true;
    })

    COND_HOOK(OnActorCollision, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnActorCollision* ev = (OnActorCollision*)event;
        RandoItemId randoItemId = RI_UNKNOWN;

        if (ev->propId->markerFlag) {
            Actor* markerActor = marker_getActor(ev->propId->actorProp.marker);

            if (markerActor->is_bundle && func_802C9C14(markerActor)) {
                event->Cancelled = true;
                return;
            }

            switch (ev->propId->actorProp.marker->id) {
                case MARKER_39_MUMBO_TOKEN:
                    if (RANDO_SAVE_OPTIONS[RO_SHUFFLE_MUMBO_TOKENS].optionValue == RO_GENERIC_ON) {
                        randoItemId = RI_MUMBO_TOKEN;
                    }
                    break;
                case MARKER_52_JIGGY:
                    if (RANDO_SAVE_OPTIONS[RO_SHUFFLE_JIGGIES].optionValue == RO_GENERIC_ON) {
                        randoItemId = RI_JIGGY;
                    }
                    break;
                case MARKER_53_EMPTY_HONEYCOMB:
                    if (RANDO_SAVE_OPTIONS[RO_SHUFFLE_EMPTY_HONEYCOMBS].optionValue == RO_GENERIC_ON) {
                        randoItemId = RI_EMPTY_HONEYCOMB;
                    }
                    break;
                case MARKER_5A_JINJO_BLUE:
                case MARKER_5B_JINJO_GREEN:
                case MARKER_5C_JINJO_ORANGE:
                case MARKER_5D_JINJO_PINK:
                case MARKER_5E_JINJO_YELLOW:
                    if (RANDO_SAVE_OPTIONS[RO_SHUFFLE_JINJOS].optionValue == RO_GENERIC_ON) {
                        randoItemId = Rando::StaticData::GetRandoItemByActorId(
                            jinjoMarkerMap.at(ev->propId->actorProp.marker->id));
                    }
                    break;
                case MARKER_5F_MUSIC_NOTE:
                    if (RANDO_SAVE_OPTIONS[RO_SHUFFLE_MUSIC_NOTES].optionValue == RO_GENERIC_ON) {
                        randoItemId = RI_MUSIC_NOTE;
                        event->Cancelled = true;
                    }
                    break;
                case MARKER_168_ICE_KEY:
                    randoItemId = RI_STOP_N_SWOP_KEY;
                    break;
                case MARKER_169_SNS_EGG:
                    randoItemId = RI_STOP_N_SWOP_EGG;
                    break;
                case MARKER_265_WORLD_EXIT_PAD:
                    for (int i = LEVEL_1_MUMBOS_MOUNTAIN; i < LEVEL_A_MAD_MONSTER_MANSION; i++) {
                        if (Rando::Logic::ShouldSpawnJinjoJiggy(i)) {
                            RandoCheckId randoCheckId = Rando::StaticData::GetJinjoJiggyCheckByLevelId(i);
                            if (CustomObject::CheckSpawnedIdList(randoCheckId)) {
                                continue;
                            }

                            int32_t checkSpawnPos[3];
                            checkSpawnPos[0] = (int32_t)markerActor->position_x;
                            checkSpawnPos[1] = (int32_t)markerActor->position_y + 200;
                            checkSpawnPos[2] = (int32_t)markerActor->position_z;

                            CustomObject::AddPropToSpawnQueueEX(checkSpawnPos, randoCheckId);
                        }
                    }
                    break;
                default:
                    return;
            }

            if (randoItemId != RI_UNKNOWN) {
                CustomObject::ObjectCollectedEX(ev->propId);
            }
        }
    })

    COND_HOOK(OnSetJiggyList, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        CustomObject::ClearRandoActorListEX();
        FireClearBundleDespawnQueue();
        Rando::Logic::RefreshReachableRegions();
    })

    COND_HOOK(OnFindActorFromActorId, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnFindActorFromActorId* ev = (OnFindActorFromActorId*)event;
        RandoCheckId randoCheckId = RC_UNKNOWN;
        map_e currentMap = gsworld_getMap();
        ev->result = NULL;

        switch (ev->actorId) {
            case ACTOR_46_JIGGY:
                if (currentMap == MAP_26_MMM_NAPPERS_ROOM) {
                    randoCheckId = RC_MMM_JIGGY_MANSION_TABLE;
                } else if (currentMap == MAP_5A_CCW_SUMMER_ZUBBA_HIVE || currentMap == MAP_5B_CCW_SPRING_ZUBBA_HIVE) {
                    randoCheckId = RC_CCW_JIGGY_ZUBBAS;
                }
                break;
            default:
                if (currentMap == MAP_D_BGS_BUBBLEGLOOP_SWAMP) {
                    if (CustomObject::CheckSpawnedIdList(RC_BGS_JIGGY_ELEVATED_WALKWAY)) {
                        randoCheckId = RC_BGS_JIGGY_ELEVATED_WALKWAY;
                    } else if (CustomObject::CheckSpawnedIdList(RC_BGS_JIGGY_MAZE)) {
                        randoCheckId = RC_BGS_JIGGY_MAZE;
                    }
                }
                break;
        }

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        ev->result = FindActorByRandoCheckId(randoCheckId);

        if (ev->result != NULL) {
            event->Cancelled = true;
        }
    })

    COND_HOOK(OnFindActorMarkerFromJiggyId, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnFindActorMarkerFromJiggyId* ev = (OnFindActorMarkerFromJiggyId*)event;
        RandoCheckId randoCheckId = RC_UNKNOWN;

        switch (ev->jiggyId) {
            case JIGGY_3E_GV_GRABBA:
                randoCheckId = RC_GV_JIGGY_GRABBA;
                break;
            default:
                return;
        }

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        if (CustomObject::CheckSpawnedIdList(randoCheckId)) {
            event->Cancelled = true;
            ev->result = FindActorByRandoCheckId(randoCheckId)->marker;
        }
    })

    COND_HOOK(OnFindClosestActorFromActorId, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnFindClosestActorFromActorId* ev = (OnFindClosestActorFromActorId*)event;
        RandoCheckId randoCheckId = RC_UNKNOWN;
        map_e currentMap = gsworld_getMap();
        ev->result = NULL;

        switch (ev->actorId) {
            case ACTOR_46_JIGGY:
                if (currentMap == MAP_24_MMM_TUMBLARS_SHED) {
                    randoCheckId = RC_MMM_JIGGY_TUMBLARS_PUZZLE;
                }
                break;
            default:
                break;
        }

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        ev->result = FindActorByRandoCheckId(randoCheckId);

        if (ev->result != NULL) {
            event->Cancelled = true;
        }
    })

    COND_HOOK(OnActorTick, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        OnActorTick* ev = (OnActorTick*)event;

        switch (ev->actor->actor_info->actorId) {
            case ACTOR_12E_GOBI_1:
            case ACTOR_12F_GOBI_ROPE:
            case ACTOR_130_GOBI_ROCK:
            case ACTOR_131_GOBI_2:
            case ACTOR_135_GOBI_3:
                Rando::ObjectBehavior::ModifyGobiBehavior(ev->actor);
                break;
            case ACTOR_160_BOGGY_1:
            case ACTOR_181_SCARF_SLED:
            case ACTOR_C8_BOGGY_2:
            case 0x33D: // Actor Boggy 3
                Rando::ObjectBehavior::ModifyBoggyBehavior(ev->actor);
                break;
            case ACTOR_14E_BGS_ELEVATED_WALKWAY_SWITCH:
            case ACTOR_1FB_BGS_MAZE_SWITCH:
                Rando::ObjectBehavior::ModifySwitchBehavior(ev->actor->actor_info->actorId);
                break;
            case ACTOR_33A_BLUE_PRESENT:
            case ACTOR_33B_GREEN_PRESENT:
            case ACTOR_33C_RED_PRESENT:
            case ACTOR_1ED_BLUE_PRESENT_COLLECTIBLE:
            case ACTOR_1EF_GREEN_PRESENT_COLLECTIBLE:
            case ACTOR_1F1_RED_PRESENT_COLLECTIBLE:
                Rando::ObjectBehavior::ModifyPresentBehavior(ev->actor);
                break;
            case 0x253: // FP Wozza's Cave Ice Wall
            case 0x191: // MMM Cellar SNS Entrance
            case ACTOR_243_GV_SNS_CHAMBER_DOOR:
            case ACTOR_25C_SHARKFOOD_ISLAND:
                Rando::ObjectBehavior::ModifyStopNSwopWorldBehavior(ev->actor);
                break;
            default:
                break;
        }
    })
}
