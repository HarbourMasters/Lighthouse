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

std::vector<int32_t> HutSmash_Snapshot() {
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

void HutSmash_Restore(const std::vector<int32_t>& flat) {
    sHuts.clear();
    for (size_t i = 0; i + 5 <= flat.size(); i += 5) {
        sHuts[{ flat[i], flat[i + 1], flat[i + 2], flat[i + 3] }] = flat[i + 4];
    }
}

void HutSmash_ClearForLevel(int32_t levelId) {
    std::erase_if(sHuts,
                  [levelId](const auto& kv) { return (int32_t)map_getLevel((enum map_e)kv.first[0]) == levelId; });
}

void RegisterHutSmash_Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sHuts.clear(); });
}

static RegisterShipInitFunc initHutSmash(RegisterHutSmash_Init, {});

static void recordSmash(const OnHutSmash* ev) {
    int32_t map = (int32_t)gsworld_getMap();
    std::array<int32_t, 4> key = { map, ev->x, ev->y, ev->z };
    if (!Anchor::GetInstance()->IsWorldSyncActive() || sHuts.find(key) != sHuts.end()) {
        return;
    }
    sHuts[key] = ev->loot;
    Anchor::GetInstance()->SendPacket_HutSmash(ev->x, ev->y, ev->z, ev->loot, map);
}

static void RegisterHutSmashSync_Init() {
    const bool connected = Anchor::GetInstance()->isConnected;

    COND_VB_SHOULD(VB_HUT_SMASH_LOOT, EVENT_PRIORITY_NORMAL, connected, {
        const s32 x = va_arg(args, s32);
        const s32 y = va_arg(args, s32);
        const s32 z = va_arg(args, s32);
        s32* loot = va_arg(args, s32*);
        if (!Anchor::GetInstance()->IsWorldSyncActive()) {
            return;
        }
        auto it = sHuts.find({ (int32_t)gsworld_getMap(), x, y, z });
        if (it != sHuts.end()) {
            *loot = it->second;
        }
    });

    // The team's smash count for the level replaces the local one.
    COND_VB_SHOULD(VB_HUT_SMASH_INDEX, EVENT_PRIORITY_NORMAL, connected, {
        s32* index = va_arg(args, s32*);
        if (!Anchor::GetInstance()->IsWorldSyncActive()) {
            return;
        }
        int32_t level = (int32_t)map_getLevel(gsworld_getMap());
        int32_t count = 0;
        for (const auto& [key, loot] : sHuts) {
            if ((int32_t)map_getLevel((enum map_e)key[0]) == level) {
                count++;
            }
        }
        *index = count;
    });

    COND_HOOK(OnHutSmash, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) { recordSmash((OnHutSmash*)event); });
}

static RegisterShipInitFunc initHutSmashSync(RegisterHutSmashSync_Init, { ANCHOR_CONNECTED_PATH });
