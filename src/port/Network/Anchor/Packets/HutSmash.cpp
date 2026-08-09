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

/**
 * HUT_SMASH
 */

std::map<std::array<int32_t, 4>, int32_t> sHuts;

extern "C" int32_t port_hutSmash_get(int32_t x, int32_t y, int32_t z) {
    auto it = sHuts.find({ (int32_t)gsworld_getMap(), x, y, z });
    return it != sHuts.end() ? it->second : -1;
}

extern "C" int32_t port_hutSmash_countForCurrentLevel(void) {
    int32_t level = (int32_t)map_getLevel(gsworld_getMap());
    int32_t count = 0;
    for (const auto& [key, loot] : sHuts) {
        if ((int32_t)map_getLevel((enum map_e)key[0]) == level) {
            count++;
        }
    }
    return count;
}

extern "C" void port_hutSmash_record(int32_t x, int32_t y, int32_t z, int32_t loot) {
    int32_t map = (int32_t)gsworld_getMap();
    std::array<int32_t, 4> key = { map, x, y, z };
    if (sHuts.find(key) != sHuts.end()) {
        return;
    }
    sHuts[key] = loot;
    Anchor::GetInstance()->SendPacket_HutSmash(x, y, z, loot, map);
}

void Anchor::SendPacket_HutSmash(s32 x, s32 y, s32 z, s32 loot, s32 map) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = HUT_SMASH;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["x"] = x;
    payload["y"] = y;
    payload["z"] = z;
    payload["loot"] = loot;
    payload["map"] = map;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_HutSmash(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s32 x = payload.at("x").get<s32>();
    s32 y = payload.at("y").get<s32>();
    s32 z = payload.at("z").get<s32>();
    s32 loot = payload.at("loot").get<s32>();
    s32 map = payload.at("map").get<s32>();

    sHuts.emplace(std::array<int32_t, 4>{ map, x, y, z }, loot);
}

std::vector<int32_t> port_hutSmash_snapshot() {
    std::vector<int32_t> flat;
    flat.reserve(sHuts.size() * 5);
    for (const auto& [key, loot] : sHuts) {
        flat.push_back(key[0]);
        flat.push_back(key[1]);
        flat.push_back(key[2]);
        flat.push_back(key[3]);
        flat.push_back(loot);
    }
    return flat;
}

void port_hutSmash_restore(const std::vector<int32_t>& flat) {
    sHuts.clear();
    for (size_t i = 0; i + 5 <= flat.size(); i += 5) {
        sHuts[{ flat[i], flat[i + 1], flat[i + 2], flat[i + 3] }] = flat[i + 4];
    }
}

void port_hutSmash_clearForLevel(int32_t levelId) {
    std::erase_if(sHuts,
                  [levelId](const auto& kv) { return (int32_t)map_getLevel((enum map_e)kv.first[0]) == levelId; });
}

void RegisterHutSmash_Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sHuts.clear(); });
}

static RegisterShipInitFunc initHutSmash(RegisterHutSmash_Init, {});
