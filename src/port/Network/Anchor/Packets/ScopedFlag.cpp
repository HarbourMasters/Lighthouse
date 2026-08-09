#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h"

static uint32_t sMapFlagSetRemotely = 0;

extern "C" int32_t port_mapFlag_wasSetRemotely(int32_t index) {
    return (index >= 0 && index < 32 && (sMapFlagSetRemotely & (1u << index))) ? 1 : 0;
}

/**
 * SCOPED_FLAG
 *
 * Realtime sync of a transient level-/map flag; ctx = sender's level/map id.
 */

void Anchor::SendPacket_ScopedFlag(u8 space, s16 index, u8 value) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = SCOPED_FLAG;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["space"] = space;
    payload["index"] = index;
    payload["value"] = value;

    if (space == ANCHOR_FLAGSPACE_LEVEL_SPECIFIC) {
        payload["ctx"] = (s32)map_getLevel(gsworld_getMap());
        SendToCurrentLevelPlayers(payload);
    } else {
        payload["ctx"] = (s32)gsworld_getMap();
        SendToCurrentMapPlayers(payload);
    }
}

void Anchor::HandlePacket_ScopedFlag(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    u8 space = payload.at("space").get<u8>();
    s16 index = payload.at("index").get<s16>();
    u8 value = payload.at("value").get<u8>();
    s32 ctx = payload.at("ctx").get<s32>();

    if (space == ANCHOR_FLAGSPACE_LEVEL_SPECIFIC) {
        if ((s32)map_getLevel(gsworld_getMap()) == ctx) {
            levelSpecificFlags_setEx(index, value, 0);
        }
    } else if (space == ANCHOR_FLAGSPACE_MAP_SPECIFIC) {
        if ((s32)gsworld_getMap() == ctx) {
            mapSpecificFlags_setEx(index, value, 0);
            if (value && index >= 0 && index < 32) {
                sMapFlagSetRemotely |= (1u << index);
            }
            RevealSwitchHoneycomb();
        }
    }
}

void RegisterScopedFlag_Init() {
    REGISTER_LISTENER(OnGameFlagSet, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto ev = reinterpret_cast<OnGameFlagSet*>(event);
        if (ev->flagSpace == ANCHOR_FLAGSPACE_MAP_SPECIFIC && ev->index >= 0 && ev->index < 32) {
            sMapFlagSetRemotely &= ~(1u << ev->index);
        }
    });
    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { sMapFlagSetRemotely = 0; });
}

static RegisterShipInitFunc initScopedFlag(RegisterScopedFlag_Init, {});
