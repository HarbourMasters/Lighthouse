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

void Anchor_MarkMapFlagsRemote(uint32_t mask) {
    sMapFlagSetRemotely |= mask;
}

/**
 * SCOPED_FLAG
 *
 * Realtime sync of a transient level-/map flag; ctx = sender's level/map id.
 */

// count > 1 sends a multi-bit field: value holds its bits, lowest first.
void Anchor::SendPacket_ScopedFlag(u8 space, s16 index, u32 value, s32 count) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = SCOPED_FLAG;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["space"] = space;
    payload["index"] = index;
    payload["value"] = value;
    if (count > 1) {
        payload["count"] = count;
    }

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
    u32 values = payload.at("value").get<u32>();
    s32 count = payload.value("count", 1);
    s32 ctx = payload.at("ctx").get<s32>();

    if (space == ANCHOR_FLAGSPACE_LEVEL_SPECIFIC) {
        if ((s32)map_getLevel(gsworld_getMap()) == ctx) {
            ApplyRemote([&] {
                for (s32 i = 0; i < count; i++) {
                    levelSpecificFlags_set(index + i, (values >> i) & 1);
                }
            });
        }
    } else if (space == ANCHOR_FLAGSPACE_MAP_SPECIFIC) {
        if ((s32)gsworld_getMap() == ctx) {
            ApplyRemote([&] {
                for (s32 i = 0; i < count; i++) {
                    const s32 bit = index + i;
                    const u8 value = (values >> i) & 1;
                    // Only a write that sets the flag marks it; one we already had stays ours.
                    const bool wasSet = bit >= 0 && bit < 32 && mapSpecificFlags_get(bit);
                    mapSpecificFlags_set(bit, value);
                    if (value && !wasSet && bit >= 0 && bit < 32) {
                        sMapFlagSetRemotely |= (1u << bit);
                    }
                }
            });
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
