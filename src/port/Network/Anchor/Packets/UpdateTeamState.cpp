#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/UI/Notification.h"
#include <algorithm>
#include <vector>

extern "C" {
#include "variables.h"
#include "functions.h"
}

/**
 * UPDATE_TEAM_STATE
 *
 * Pushes our full flag state to the server for teammates. Fires when the server passes
 * on a REQUEST_TEAM_STATE packet, or when this client saves the game.
 *
 * Sending assumes the team queue has been drained for this client, so we clear it.
 * Receiving replays any queued packets in order after applying the state.
 */

void Anchor::SendPacket_UpdateTeamState() {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s32 fpSize;
    u8* fpAddr;
    fileProgressFlag_getSizeAndPtr(&fpSize, &fpAddr);

    json payload;
    payload["type"] = UPDATE_TEAM_STATE;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["queue"] = json::array();
    payload["state"]["fileProgressFlags"] = std::vector<u8>(fpAddr, fpAddr + fpSize);
    // Volatile flags intentionally not sent — no consumer yet (furnace-fun sync later).
    // The receive path still applies them if a "volatileFlags" array is present.

    SendJsonToRemote(payload);
}

void Anchor::SendPacket_ClearTeamState(std::string teamId) {
    json payload;
    payload["type"] = UPDATE_TEAM_STATE;
    payload["targetTeamId"] = teamId;
    payload["queue"] = json::array();
    payload["state"] = json::object();
    SendJsonToRemote(payload);
}

// Copies up to the local bitfield's size from the JSON byte array into addr.
static void ApplyFlagBytes(nlohmann::json& bytes, u8* addr, s32 size) {
    s32 count = std::min(size, (s32)bytes.size());
    for (s32 i = 0; i < count; i++) {
        addr[i] = bytes[i].get<u8>();
    }
}

void Anchor::HandlePacket_UpdateTeamState(nlohmann::json& payload) {
    if (!roomState.syncItemsAndFlags) {
        return;
    }

    isHandlingUpdateTeamState = true;

    if (payload.contains("state")) {
        auto& state = payload["state"];
        // Direct byte copy into the bitfields — bypasses the setters, so no OnGameFlagSet.
        if (state.contains("fileProgressFlags")) {
            s32 size;
            u8* addr;
            fileProgressFlag_getSizeAndPtr(&size, &addr);
            ApplyFlagBytes(state["fileProgressFlags"], addr, size);
        }
        if (state.contains("volatileFlags")) {
            s32 size;
            u8* addr;
            volatileFlag_getSizeAndPtr(&size, &addr);
            ApplyFlagBytes(state["volatileFlags"], addr, size);
        }

        Notification::Emit({
            .message = "Save updated from team",
        });
    }

    if (payload.contains("queue")) {
        std::lock_guard<std::mutex> lock(incomingPacketQueueMutex);
        for (auto& item : payload["queue"]) {
            incomingPacketQueue.push(nlohmann::json::parse(item.get<std::string>()));
        }
    }

    isHandlingUpdateTeamState = false;
}
