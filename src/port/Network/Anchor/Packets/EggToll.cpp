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

#include "port/Patches/Patches.h"

/**
 * EGG_TOLL
 */

std::map<std::array<int32_t, 2>, int32_t> sStages;

void recordStage(int32_t map, int32_t secondaryId, int32_t stage) {
    std::array<int32_t, 2> key = { map, secondaryId };
    auto it = sStages.find(key);
    if (it == sStages.end() || stage > it->second) {
        sStages[key] = stage;
    }
}

extern "C" int32_t port_eggToll_getStage(int32_t map, int32_t secondaryId) {
    auto it = sStages.find({ map, secondaryId });
    return it != sStages.end() ? it->second : 0;
}

extern "C" void port_eggToll_onAdvance(int32_t map, int32_t secondaryId, int32_t stage) {
    recordStage(map, secondaryId, stage);
    Anchor::GetInstance()->SendPacket_EggToll((s16)secondaryId, stage, map);
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
        port_eggToll_remoteApply(map, secondaryId, stage);
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
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sStages.clear(); });

    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnMapLoad* ev = (OnMapLoad*)event;
        for (const auto& [key, stage] : sStages) {
            if (key[0] == (int32_t)ev->nextMap) {
                port_eggToll_remoteApply(key[0], key[1], stage);
            }
        }
    });
}

static RegisterShipInitFunc initEggToll(RegisterEggToll_Init, {});
