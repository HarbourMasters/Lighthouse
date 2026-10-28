#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <array>
#include <set>

#include "functions.h"

#include "port/Patches/Patches.h"

/**
 * BREAK_OBJECT
 */

std::set<std::array<int32_t, 5>> sBrokenObjects;

std::vector<int32_t> BreakObject_SnapshotBroken() {
    std::vector<int32_t> flat;
    flat.reserve(sBrokenObjects.size() * 5);
    for (const auto& e : sBrokenObjects) {
        flat.insert(flat.end(), e.begin(), e.end());
    }
    return flat;
}

void BreakObject_RestoreBroken(const std::vector<int32_t>& flat) {
    sBrokenObjects.clear();
    for (size_t i = 0; i + 5 <= flat.size(); i += 5) {
        sBrokenObjects.insert({ flat[i], flat[i + 1], flat[i + 2], flat[i + 3], flat[i + 4] });
    }
}

void BreakObject_ClearForLevel(int32_t levelId) {
    std::erase_if(sBrokenObjects, [levelId](const std::array<int32_t, 5>& e) {
        return (int32_t)map_getLevel((enum map_e)e[0]) == levelId;
    });
}

void Anchor::SendPacket_BreakObject(s16 markerId, s32 x, s32 y, s32 z, s32 map, bool replay) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = BREAK_OBJECT;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["marker"] = markerId;
    payload["x"] = x;
    payload["y"] = y;
    payload["z"] = z;
    payload["map"] = map;
    payload["replay"] = replay;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_BreakObject(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 markerId = payload.at("marker").get<s16>();
    s32 x = payload.at("x").get<s32>();
    s32 y = payload.at("y").get<s32>();
    s32 z = payload.at("z").get<s32>();
    s32 map = payload.at("map").get<s32>();
    bool replay = payload.contains("replay") ? payload.at("replay").get<bool>() : true;

    sBrokenObjects.insert({ map, markerId, x, y, z });
    if (replay && (s32)gsworld_getMap() == map) {
        breakable_remoteBreakAt(markerId, x, y, z);
    }
}

extern "C" void breakable_despawnBrokenRestores(s32 map);

void RegisterBreakObject_Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sBrokenObjects.clear(); });

    REGISTER_LISTENER(OnMapLoaded, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnMapLoaded* ev = (OnMapLoaded*)event;
        if (!Anchor::GetInstance()->IsWorldSyncActive()) {
            return;
        }
        breakable_despawnBrokenRestores((s32)ev->nextMap);
    });
}

static RegisterShipInitFunc initBreakObject(RegisterBreakObject_Init, {});

static void RegisterBreakObjectSync_Init() {
    const bool connected = Anchor::GetInstance()->isConnected;

    COND_VB_SHOULD(VB_BREAKABLE_BROKEN, EVENT_PRIORITY_NORMAL, connected, {
        const s32 map = va_arg(args, s32);
        const s32 markerId = va_arg(args, s32);
        const s32 x = va_arg(args, s32);
        const s32 y = va_arg(args, s32);
        const s32 z = va_arg(args, s32);
        if (Anchor::GetInstance()->IsWorldSyncActive() && sBrokenObjects.count({ map, markerId, x, y, z }) != 0) {
            *should = true;
        }
    });

    COND_HOOK(OnBreakableBroken, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (OnBreakableBroken*)event;
        s32 map = (s32)gsworld_getMap();
        if (!Anchor::GetInstance()->IsWorldSyncActive() ||
            sBrokenObjects.count({ map, ev->markerId, ev->x, ev->y, ev->z }) != 0) {
            return;
        }
        sBrokenObjects.insert({ map, ev->markerId, ev->x, ev->y, ev->z });
        Anchor::GetInstance()->SendPacket_BreakObject((s16)ev->markerId, ev->x, ev->y, ev->z, map, ev->replay);
    });
}

static RegisterShipInitFunc initBreakObjectSync(RegisterBreakObjectSync_Init, { ANCHOR_CONNECTED_PATH });
