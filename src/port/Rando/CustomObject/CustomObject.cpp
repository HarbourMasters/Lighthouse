#include "CustomObject.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/ObjectBehavior/ObjectBehavior.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "actor.h"
#include "include/core1/sns.h"

#include "spdlog/spdlog.h"

extern "C" {
extern u8 D_80385FF0[0xE];

typedef struct chjiggy_s {
    u32 unk0;
    u32 index;
} ActorLocal_Jiggy;

typedef struct {
    enum mumbotoken_e uid;
} ActorLocal_MumboToken;

typedef struct {
    enum honeycomb_e uid;
    s32 unk4;
} ActorLocal_EmptyHoneycomb;

s32 item_adjustByDiffWithHud(enum item_e item, s32 diff);
void fxSparkle_musicNote(s16 position[3]);
void player_getPosition(f32 dst[3]);
void ml_vec3f_to_vec3h(s16 dst[3], f32 src[3]);

void gcparade_beginFinalParade(void);

void coMusicPlayer_playMusic(enum comusic_e track_id, s32 volume);
void marker_despawn(ActorMarker* marker);
Actor* actor_new(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags);
extern ActorInfo chJinjoBlue;
extern ActorInfo chJinjoGreen;
extern ActorInfo chJinjoYellow;
extern ActorInfo chJinjoPink;
extern ActorInfo chJinjoOrange;
extern ActorInfo chJiggy;
extern ActorInfo chEmptyHoneycomb;
extern ActorInfo chMumboToken;
extern ActorInfo sumusicNote;

extern ActorInfo chHoneycomb;
extern ActorInfo chBlueEgg;
extern ActorInfo chRedFeather;
extern ActorInfo chGoldFeather;

extern ActorInfo chSnsEgg;
extern ActorInfo chIceKey;
}

typedef struct {
    int32_t position[3];
    RandoCheckId randoCheckId;
    bool isSpawned;
} QueuedRandoProp;

std::vector<QueuedRandoProp> randoActorQueue;
std::vector<RandoCheckId> randoSpawnedCheckIds;
bool shouldRemoveEX = false;

// clang-format off
std::map<actor_e, std::pair<ActorInfo, int32_t>> actorInfoMap = {
    { ACTOR_2D_MUMBO_TOKEN,     { chMumboToken,       ACTOR_FLAG_UNKNOWN_6 } },
    { ACTOR_46_JIGGY,           { chJiggy,          ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_21 } },
    { ACTOR_47_EMPTY_HONEYCOMB, { chEmptyHoneycomb,       ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_21 } },
    { ACTOR_51_MUSIC_NOTE,      { sumusicNote,      ACTOR_FLAG_UNKNOWN_21 } },
    { ACTOR_5E_JINJO_YELLOW,    { chJinjoYellow,    ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_8 } },
    { ACTOR_5F_JINJO_ORANGE,    { chJinjoOrange,    ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_8 } },
    { ACTOR_60_JINJO_BLUE,      { chJinjoBlue,      ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_8 } },
    { ACTOR_61_JINJO_PINK,      { chJinjoPink,      ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_8 } },
    { ACTOR_62_JINJO_GREEN,     { chJinjoGreen,     ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_8 } },
    { ACTOR_50_HONEYCOMB,       { chHoneycomb,       ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_21 } },
    { ACTOR_52_BLUE_EGG,        { chBlueEgg,       ACTOR_FLAG_UNKNOWN_21 } },
    { ACTOR_129_RED_FEATHER,    { chRedFeather,       ACTOR_FLAG_UNKNOWN_21 } },
    { ACTOR_370_GOLD_FEATHER,   { chGoldFeather,       ACTOR_FLAG_UNKNOWN_21 } },
    { ACTOR_25E_SNS_EGG,        { chSnsEgg,       ACTOR_FLAG_UNKNOWN_9 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_15 } },
    { ACTOR_25D_ICE_KEY,        { chIceKey,       ACTOR_FLAG_UNKNOWN_9 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_15 } },
};
// clang-format on

int32_t currentMap = -1;

void CustomObject::ResetRandoSpawnQueue() {
    for (auto& queue : randoActorQueue) {
        queue.isSpawned = false;
    }
}

void CustomObject::ClearRandoActorListEX() {
    if ((gsworld_getMap() == MAP_7_TTC_TREASURE_TROVE_COVE && currentMap == gsworld_getMap()) ||
        (gsworld_getMap() == MAP_1B_MMM_MAD_MONSTER_MANSION && currentMap == gsworld_getMap())) {
        CustomObject::ResetRandoSpawnQueue();
        randoSpawnedCheckIds.clear();
    }
    if (currentMap != gsworld_getMap()) {
        currentMap = gsworld_getMap();
        randoActorQueue.clear();
        randoSpawnedCheckIds.clear();
    }
}

bool CustomObject::CheckSpawnedIdList(RandoCheckId randoCheckId) {
    for (auto& spawn : randoSpawnedCheckIds) {
        if (spawn == randoCheckId) {
            return true;
        }
    }
    return false;
}

void CustomObject::RemoveSpawnedIdFromList(RandoCheckId randoCheckId) {
    for (int i = 0; i < randoSpawnedCheckIds.size(); i++) {
        if (randoSpawnedCheckIds[i] == randoCheckId) {
            randoSpawnedCheckIds.erase(randoSpawnedCheckIds.begin() + i);
            break;
        }
    }
}

Actor* CustomObject::SetCustomActorParametersEX(RandoCheckId randoCheckId, Actor* customActor) {
    RandoSaveCheck shuffledObject = Rando::Logic::GetShuffledObject(randoCheckId);
    customActor->marker->randoCheckId = randoCheckId;

    switch (shuffledObject.randoItemId) {
        case RI_EMPTY_HONEYCOMB:
            ActorLocal_EmptyHoneycomb* honeycombLocal;
            honeycombLocal = (ActorLocal_EmptyHoneycomb*)&customActor->local;
            honeycombLocal->uid = (honeycomb_e)Rando::Logic::GetShuffledObject(randoCheckId).randoCollectionId;
            break;
        case RI_JIGGY:
            ActorLocal_Jiggy* jiggyLocal;
            jiggyLocal = (ActorLocal_Jiggy*)&customActor->local;
            jiggyLocal->index = shuffledObject.randoCollectionId;
            break;
        case RI_MUMBO_TOKEN:
            ActorLocal_MumboToken* tokenLocal;
            tokenLocal = (ActorLocal_MumboToken*)&customActor->local;
            tokenLocal->uid = (mumbotoken_e)Rando::Logic::GetShuffledObject(randoCheckId).randoCollectionId;
            break;
        case RI_STOP_N_SWOP_EGG:
        case RI_STOP_N_SWOP_KEY:
            customActor->actorTypeSpecificField = shuffledObject.randoCollectionId;
            customActor->scale = 0.5f;
            if (shuffledObject.randoItemId == RI_STOP_N_SWOP_KEY) {
                customActor->position_y += 50.0f;
            }
            break;
        default:
            break;
    }
    return customActor;
}

Actor* CustomObject::SpawnCustomActorEX(RandoCheckId randoCheckId, int32_t position[3], ActorInfo* actorInfo,
                                        int32_t flags) {
    if (randoCheckId == RC_UNKNOWN) {
        return NULL;
    }

    Actor* customActor = actor_new(position, 0, actorInfo, flags);

    if (customActor != NULL) {
        // Music notes don't cast shadows, custom or otherwise.
        if (actorInfo->actorId == ACTOR_51_MUSIC_NOTE) {
            customActor->unk124_6 = 0;
        }
        customActor = SetCustomActorParametersEX(randoCheckId, customActor);
        randoSpawnedCheckIds.push_back(randoCheckId);
    }
    return customActor;
}

void CustomObject::FlushRandoSpawnQueue() {
    if (gsworld_getMap() == MAP_91_FILE_SELECT) {
        return;
    }

    if (randoActorQueue.empty()) {
        return;
    }

    for (auto& queue : randoActorQueue) {
        if (queue.isSpawned) {
            continue;
        }

        if (CustomObject::CheckSpawnedIdList(queue.randoCheckId)) {
            queue.isSpawned = true;
            continue;
        }

        RandoSaveCheck randoSaveCheck = Rando::Logic::GetShuffledObject(queue.randoCheckId);
        if (randoSaveCheck.randoCheckId == RC_UNKNOWN) {
            queue.isSpawned = true;
            continue;
        }

        actor_e randoActorId = (actor_e)Rando::StaticData::Items[randoSaveCheck.randoItemId].actorId;
        if (randoSaveCheck.obtained) {
            randoActorId = GetRandomJunkActorId(randoSaveCheck);
        }

        if (randoActorId == ACTOR_1_UNKNOWN) {
            queue.isSpawned = true;
            continue;
        }

        Actor* customActor = CustomObject::SpawnCustomActorEX(randoSaveCheck.randoCheckId, queue.position,
                                                              &actorInfoMap.at(randoActorId).first,
                                                              actorInfoMap.at(randoActorId).second);

        if (customActor != NULL && randoSaveCheck.obtained &&
            RANDO_SAVE_OPTIONS[RO_SPAWN_JUNK].optionValue == RO_GENERIC_ON) {
            customActor->marker->unk14_21 = true;
            customActor->scale = 1.0f;
        }
        queue.isSpawned = true;
    }
}

void CustomObject::AddPropToSpawnQueueEX(int32_t position[3], RandoCheckId randoCheckId) {
    randoActorQueue.push_back({ { position[0], position[1], position[2] }, randoCheckId, false });
}

Actor* CustomObject::ShouldCreateCustomActorEX(RandoCheckId randoCheckId, int32_t position[3], bool isProp,
                                               Actor* refActor) {
    if (randoCheckId == RC_UNKNOWN) {
        return NULL;
    }

    if (CustomObject::CheckSpawnedIdList(randoCheckId)) {
        return NULL;
    }

    RandoSaveCheck randoSaveCheck = Rando::Logic::GetShuffledObject(randoCheckId);
    actor_e randoActorId = (actor_e)Rando::StaticData::Items[randoSaveCheck.randoItemId].actorId;
    if (!randoSaveCheck.isShuffled) {
        return NULL;
    }

    if (isProp) {
        CustomObject::AddPropToSpawnQueueEX(position, randoCheckId);
        return NULL;
    }

    if (randoSaveCheck.obtained) {
        if (refActor != nullptr) {
            randoActorId = (actor_e)refActor->modelCacheIndex;
        } else {
            randoActorId = GetRandomJunkActorId(randoSaveCheck);
        }
    }

    if (randoSaveCheck.randoCheckId == RC_UNKNOWN) {
        return NULL;
    }

    if (randoActorId == ACTOR_1_UNKNOWN) {
        return NULL;
    }

    return CustomObject::SpawnCustomActorEX(randoCheckId, position, &actorInfoMap.at(randoActorId).first,
                                            actorInfoMap.at(randoActorId).second);
}

void CustomObject::ResolveCustomActorCollisionEX(RandoCheckId randoCheckId) {
    RandoSaveCheck shuffledObject = Rando::Logic::GetShuffledObject(randoCheckId);
    if (shuffledObject.randoCheckId == RC_UNKNOWN) {
        return;
    }

    int32_t spawnPosition[3];
    int16_t sparklePos[3];
    f32 playerPosF[3];
    player_getPosition(playerPosF);
    ml_vec3f_to_vec3h(sparklePos, playerPosF);
    spawnPosition[0] = (int32_t)playerPosF[0];
    spawnPosition[1] = (int32_t)playerPosF[1];
    spawnPosition[2] = (int32_t)playerPosF[2];

    switch (shuffledObject.randoItemId) {
        case RI_JIGGY:
            if (CVarGetInteger(CVAR_ENHANCEMENT("Cutscenes.SkipJiggyDance"), 0)) {
                fxSparkle_musicNote(sparklePos);
            }
            break;
        case RI_JINJO_BLUE:
        case RI_JINJO_GREEN:
        case RI_JINJO_ORANGE:
        case RI_JINJO_PINK:
        case RI_JINJO_YELLOW:
            if (Rando::StaticData::Checks[shuffledObject.shuffledCheckId].worldId == map_getLevel(gsworld_getMap())) {
                int32_t jinjoMarkerId =
                    GetJinjoActorMarkerId((actor_e)Rando::StaticData::Items[shuffledObject.randoItemId].actorId);
                item_adjustByDiffWithHud(ITEM_12_JINJOS, (1 << ((jinjoMarkerId + 6) & 0x1F)));
            } else {
                if (Rando::Logic::ShouldSpawnJinjoJiggy(
                        Rando::StaticData::Checks[shuffledObject.shuffledCheckId].worldId)) {
                    RandoCheckId jiggyCheckId = Rando::StaticData::GetJinjoJiggyCheckByLevelId(
                        Rando::StaticData::Checks[shuffledObject.shuffledCheckId].worldId);

                    if (jiggyCheckId != RC_UNKNOWN) {
                        Actor* customActor = ShouldCreateCustomActorEX(jiggyCheckId, spawnPosition, false);
                        if (customActor != NULL) {
                            ApplyCustomActorPhysics(jiggyCheckId, customActor, true);
                        }
                    }
                }
            }
            break;
        case RI_MUSIC_NOTE:
            D_80385FF0[Rando::StaticData::Checks[shuffledObject.shuffledCheckId].worldId]++;
            if (Rando::StaticData::Checks[shuffledObject.shuffledCheckId].worldId == map_getLevel(gsworld_getMap())) {
                item_set(ITEM_C_NOTE, D_80385FF0[map_getLevel(gsworld_getMap())]);
            }

            UpdateSaveDataNoteScores();
            fxSparkle_musicNote(sparklePos);
            coMusicPlayer_playMusic(COMUSIC_9_NOTE_COLLECTED, 16000);
            break;
        case RI_STOP_N_SWOP_EGG:
        case RI_STOP_N_SWOP_KEY:
            if (Rando::Logic::GetTotalSnsItemsCollected() >= 7) {
                gcparade_beginFinalParade();
            }
            break;
        default:
            break;
    }
}

void CustomObject::CheckObtainedEX(RandoCheckId randoCheckId, bool isInit) {
    for (auto& pool : Rando::Logic::shuffledPool) {
        if (pool.randoCheckId == randoCheckId && !pool.obtained) {
            pool.obtained = true;
            shouldRemoveEX = true;
            RANDO_SAVE_CHECKS[pool.randoCheckId].obtained = true;
            CustomObject::RemoveSpawnedIdFromList(randoCheckId);
            if (isInit) {
                CustomObject::ResolveCustomActorCollisionEX(randoCheckId);
            } else {
                Rando::StaticData::SendCollisionNotification(pool.randoCheckId);
            }
            Rando::StaticData::ModifyRandoInfFlagState(randoCheckId);
            Rando::Logic::RefreshReachableRegions();
            // Broadcast real collects only (not save-load/remote apply, both isInit).
            if (!isInit) {
                CALL_EVENT(OnRandoCheckObtained, (int32_t)randoCheckId, (int32_t)gsworld_getMap());
            }
            break;
        }
    }
}

void CustomObject::ObjectCollectedEX(Prop* prop) {
    shouldRemoveEX = false;
    CustomObject::CheckObtainedEX((RandoCheckId)prop->actorProp.marker->randoCheckId);

    if (shouldRemoveEX) {
        CustomObject::ResolveCustomActorCollisionEX((RandoCheckId)prop->actorProp.marker->randoCheckId);

        if (Rando::Logic::GetShuffledObject((RandoCheckId)prop->actorProp.marker->randoCheckId).randoItemId ==
            RI_MUSIC_NOTE) {
            marker_despawn(prop->actorProp.marker);
        }
    }
}
