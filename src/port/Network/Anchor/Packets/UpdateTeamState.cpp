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

// Snapshot a decomp byte-array score/flag section into a JSON byte array.
static std::vector<u8> ScoreBytes(void (*getSizeAndPtr)(s32*, u8**)) {
    s32 size;
    u8* addr;
    getSizeAndPtr(&size, &addr);
    return std::vector<u8>(addr, addr + size);
}

void Anchor::SendPacket_UpdateTeamState() {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    json payload;
    payload["type"] = UPDATE_TEAM_STATE;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["queue"] = json::array();
    payload["state"]["fileProgressFlags"] = ScoreBytes(fileProgressFlag_getSizeAndPtr);
    payload["state"]["jiggies"] = ScoreBytes(jiggyscore_getSizeAndPtr);
    payload["state"]["honeycombs"] = ScoreBytes(honeycombscore_getSizeAndPtr);
    payload["state"]["mumboTokens"] = ScoreBytes(mumboscore_getSizeAndPtr);
    payload["state"]["noteScores"] = ScoreBytes(itemscore_noteScores_getSizeAndPtr);
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

// Overwrites a local byte section with the authoritative team-state array. Team state is
// authoritative: a client that requests it adopts the sender's state, replacing its own
// (no additive merge — joining a session means accepting that session's progress).
static void ApplyTeamBytes(nlohmann::json& bytes, void (*getSizeAndPtr)(s32*, u8**)) {
    s32 size;
    u8* addr;
    getSizeAndPtr(&size, &addr);
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
        // Authoritative overwrite of each section — direct byte copy bypasses the setters,
        // so no OnGameFlagSet / collectible events fire from adopting team state.
        if (state.contains("fileProgressFlags")) {
            ApplyTeamBytes(state["fileProgressFlags"], fileProgressFlag_getSizeAndPtr);
        }
        if (state.contains("volatileFlags")) {
            ApplyTeamBytes(state["volatileFlags"], volatileFlag_getSizeAndPtr);
        }
        if (state.contains("jiggies")) {
            ApplyTeamBytes(state["jiggies"], jiggyscore_getSizeAndPtr);
        }
        if (state.contains("honeycombs")) {
            ApplyTeamBytes(state["honeycombs"], honeycombscore_getSizeAndPtr);
        }
        if (state.contains("mumboTokens")) {
            ApplyTeamBytes(state["mumboTokens"], mumboscore_getSizeAndPtr);
        }
        if (state.contains("noteScores")) {
            ApplyTeamBytes(state["noteScores"], itemscore_noteScores_getSizeAndPtr);
        }

        // The overwrites above bypass the setters, so recompute the cached HUD counts the
        // same way the save-load path does (jiggy/honeycomb/mumbo refreshers). Guarded
        // because team state can arrive before the game has fully loaded.
        if (IsSaveLoaded()) {
            if (state.contains("jiggies")) {
                func_8034798C();
            }
            if (state.contains("honeycombs")) {
                func_80347958();
            }
            if (state.contains("mumboTokens")) {
                func_80347984();
            }
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
