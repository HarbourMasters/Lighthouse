#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include <algorithm>
#include <vector>

#include "functions.h"

/**
 * REQUEST_SCOPED_STATE / SCOPED_STATE
 *
 * On map load, a client asks the team for current level/map flag state; same-scope teammates reply.
 */

void Anchor::SendPacket_RequestScopedState(GameMap map) {
    nlohmann::json payload;
    payload["type"] = REQUEST_SCOPED_STATE;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["level"] = (s32)map_getLevel((enum map_e)map);
    payload["map"] = (s32)map;
    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_RequestScopedState(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s32 reqLevel = payload.at("level").get<s32>();
    s32 reqMap = payload.at("map").get<s32>();

    nlohmann::json response;
    response["type"] = SCOPED_STATE;
    response["targetClientId"] = payload.at("clientId").get<uint32_t>();
    bool any = false;

    if ((s32)map_getLevel(gsworld_getMap()) == reqLevel) {
        s32 size;
        u8* addr;
        levelSpecificFlags_getSizeAndPtr(&size, &addr);
        std::vector<u8> levelFlags(addr, addr + size);
        for (s32 idx = 0; idx < size * 8; idx++) {
            if (Anchor_ScopedFlagExcluded(ANCHOR_FLAGSPACE_LEVEL_SPECIFIC, idx)) {
                levelFlags[idx >> 3] &= ~(1 << (idx & 7));
            }
        }
        response["level"] = reqLevel;
        response["levelFlags"] = levelFlags;
        any = true;
    }
    if ((s32)gsworld_getMap() == reqMap) {
        u32 mapFlags = (u32)mapSpecificFlags_getAll();
        for (s32 idx = 0; idx < 32; idx++) {
            if (Anchor_ScopedFlagExcluded(ANCHOR_FLAGSPACE_MAP_SPECIFIC, idx)) {
                mapFlags &= ~(1u << idx);
            }
        }
        response["map"] = reqMap;
        response["mapFlags"] = mapFlags;
        any = true;
    }

    if (any) {
        SendJsonToRemote(response);
    }
}

void Anchor::HandlePacket_ScopedState(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    if (payload.contains("levelFlags") && (s32)map_getLevel(gsworld_getMap()) == payload.at("level").get<s32>()) {
        auto bytes = payload["levelFlags"].get<std::vector<u8>>();
        s32 size;
        u8* addr;
        levelSpecificFlags_getSizeAndPtr(&size, &addr);
        s32 count = std::min(size, (s32)bytes.size());
        for (s32 i = 0; i < count; i++) {
            addr[i] |= bytes[i];
        }
    }
    if (payload.contains("mapFlags") && (s32)gsworld_getMap() == payload.at("map").get<s32>()) {
        u32 mapFlags = payload["mapFlags"].get<u32>();
        mapSpecificFlags_setAll(mapSpecificFlags_getAll() | mapFlags);
        RevealSwitchHoneycomb();
    }
}
