#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <array>
#include <map>
#include <vector>

#include "functions.h"
extern "C" {
#include "RBB/ch/eggtoll1.h"

extern ActorArray* suBaddieActorArray;
extern chRBBEggTollInfo chRBBEggTollInfoTable[4];
void chRBBEggToll_setState(Actor* self, s32 next_state);
}

/**
 * EGG_TOLL
 */

std::map<std::array<int32_t, 2>, int32_t> sStages;
// The tolls a teammate paid, by tollId bit, until we pay one ourselves.
static u32 sTollReplay = 0;

static bool IsTollReplay(Actor* actor) {
    return actor != NULL && actor->modelCacheIndex == ACTOR_172_RBB_EGG_TOLL &&
           (sTollReplay & (1 << ((ActorLocal_ChRBBEggToll*)&actor->local)->tollId));
}

void recordStage(int32_t map, int32_t secondaryId, int32_t stage) {
    std::array<int32_t, 2> key = { map, secondaryId };
    auto it = sStages.find(key);
    if (it == sStages.end() || stage > it->second) {
        sStages[key] = stage;
    }
}

static int32_t getStage(int32_t map, int32_t secondaryId) {
    if (!Anchor::GetInstance()->IsWorldSyncActive()) {
        return 0;
    }
    auto it = sStages.find({ map, secondaryId });
    return it != sStages.end() ? it->second : 0;
}

static s32 countForStage(s32 tollIdx, s32 stage) {
    s32 count = 0;
    if (stage >= 1) {
        count += chRBBEggTollInfoTable[tollIdx].eggCost1;
    }
    if (stage >= 2) {
        count += chRBBEggTollInfoTable[tollIdx].eggCost2;
    }
    return count;
}

// A paid stage plays out through state 3 to state 4, which re-arms a two-stage toll or stays paid. A toll yet to set up
// takes its stage in OnEggTollSetUp, and one just restored snaps to it as its update runs.
static void remoteApply(s32 secondaryId, s32 stage) {
    if (suBaddieActorArray == NULL) {
        return;
    }
    for (s32 i = 0; i < suBaddieActorArray->cnt; i++) {
        Actor* actor = &suBaddieActorArray->data[i];
        if (actor->marker == NULL || actor->marker->id != MARKER_182_RBB_EGG_TOLL) {
            continue;
        }
        if (actor->secondaryId != secondaryId) {
            continue;
        }
        auto* local = (ActorLocal_ChRBBEggToll*)&actor->local;
        if (actor->state != CH_RBB_EGG_TOLL_STATE_0_NOT_INIT && stage > local->tollNum) {
            local->eggsPaid = countForStage(local->tollId, stage);
            if (actor->volatile_initialized) {
                local->tollNum = stage - 1;
                sTollReplay |= 1 << local->tollId;
                chRBBEggToll_setState(actor, CH_RBB_EGG_TOLL_STATE_3_TOLL_COMPLETE);
            } else {
                local->tollNum = stage;
                chRBBEggToll_setState(actor, CH_RBB_EGG_TOLL_STATE_4_UNK);
            }
        }
        return;
    }
}

void Anchor::SendPacket_EggToll(s16 secondaryId, s32 stage, s32 map) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = EGG_TOLL;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["sec"] = secondaryId;
    payload["stage"] = stage;
    payload["map"] = map;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_EggToll(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 secondaryId = payload.at("sec").get<s16>();
    s32 stage = payload.at("stage").get<s32>();
    s32 map = payload.at("map").get<s32>();

    recordStage(map, secondaryId, stage);
    if ((s32)gsworld_getMap() == map) {
        remoteApply(secondaryId, stage);
    }
}

std::vector<int32_t> port_eggToll_snapshot() {
    std::vector<int32_t> flat;
    flat.reserve(sStages.size() * 3);
    for (const auto& [key, stage] : sStages) {
        flat.push_back(key[0]);
        flat.push_back(key[1]);
        flat.push_back(stage);
    }
    return flat;
}

void port_eggToll_restore(const std::vector<int32_t>& flat) {
    sStages.clear();
    for (size_t i = 0; i + 3 <= flat.size(); i += 3) {
        sStages[{ flat[i], flat[i + 1] }] = flat[i + 2];
    }
}

void port_eggToll_clearForLevel(int32_t levelId) {
    std::erase_if(sStages,
                  [levelId](const auto& kv) { return (int32_t)map_getLevel((enum map_e)kv.first[0]) == levelId; });
}

void RegisterEggToll_Init() {
    // At boot, with the rest of this file: the stages do nothing until world sync is live, and a replay that starts
    // while connected plays out if the connection drops.
    COND_HOOK(OnSaveLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) { sStages.clear(); });

    // A toll's paid stage has no flag, so the team keeps it by secondaryId. A teammate's stage was recorded as it
    // landed, so its replay sends nothing.
    COND_HOOK(OnEggTollPaid, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        Actor* toll = ((OnEggTollPaid*)event)->actor;
        auto* local = (ActorLocal_ChRBBEggToll*)&toll->local;
        const s32 map = (s32)gsworld_getMap();
        const s32 stage = local->tollNum;
        if (stage <= getStage(map, toll->secondaryId)) {
            return;
        }
        sTollReplay &= ~(1 << local->tollId);
        if (!Anchor::GetInstance()->IsWorldSyncActive()) {
            return;
        }
        recordStage(map, toll->secondaryId, stage);
        Anchor::GetInstance()->SendPacket_EggToll((s16)toll->secondaryId, stage, map);
    });

    COND_VB_SHOULD(VB_PLAY_PUZZLE_CUTSCENE, EVENT_PRIORITY_NORMAL, true, {
        if (IsTollReplay(va_arg(args, Actor*))) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsTollReplay(actor) && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });

    COND_HOOK(OnEggTollSetUp, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        Actor* toll = ((OnEggTollSetUp*)event)->actor;
        auto* local = (ActorLocal_ChRBBEggToll*)&toll->local;
        const s32 stage = getStage((s32)gsworld_getMap(), toll->secondaryId);
        if (stage > 0) {
            local->tollNum = stage;
            local->eggsPaid = countForStage(local->tollId, stage);
            chRBBEggToll_setState(toll, CH_RBB_EGG_TOLL_STATE_4_UNK);
        }
    });

    COND_HOOK(OnMapLoaded, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        OnMapLoaded* ev = (OnMapLoaded*)event;
        if (!Anchor::GetInstance()->IsWorldSyncActive()) {
            return;
        }
        for (const auto& [key, stage] : sStages) {
            if (key[0] == (int32_t)ev->nextMap) {
                remoteApply(key[1], stage);
            }
        }
    });
}

static RegisterShipInitFunc initEggToll(RegisterEggToll_Init, {});
