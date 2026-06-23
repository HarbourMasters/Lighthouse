#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/UI/Notification.h"
#include "port/Enhancements/NoteRetention/NoteRetention.h"
#include "port/Enhancements/JinjoRetention/JinjoRetention.h"
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
    payload["state"]["noteRetention"] = ScoreBytes(port_noteRetention_getSizeAndPtr);
    payload["state"]["jinjoRetention"] = ScoreBytes(port_jinjoRetention_getSizeAndPtr);
    payload["state"]["savedItems"] = ScoreBytes(saveditem_getSizeAndPtr);
    payload["state"]["abilities"] = ScoreBytes(ability_getSizeAndPtr);
    // Time scores use a (s32*, void**) accessor, so pack inline rather than via ScoreBytes.
    s32 tsSize;
    void* tsAddr;
    timeScores_getSizeAndPtr(&tsSize, &tsAddr);
    payload["state"]["timeScores"] = std::vector<u8>((u8*)tsAddr, (u8*)tsAddr + tsSize);
    payload["state"]["volatileFlags"] = ScoreBytes(volatileFlag_getSizeAndPtr);

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
        // Per-level/per-map retention sets (which notes/jinjos are already collected). Takes
        // effect on the next map load — already-spawned notes/jinjos aren't retroactively
        // despawned here (that's the realtime collection-packet feature's job).
        if (state.contains("noteRetention")) {
            ApplyTeamBytes(state["noteRetention"], port_noteRetention_getSizeAndPtr);
        }
        if (state.contains("jinjoRetention")) {
            ApplyTeamBytes(state["jinjoRetention"], port_jinjoRetention_getSizeAndPtr);
        }
        if (state.contains("abilities")) {
            ApplyTeamBytes(state["abilities"], ability_getSizeAndPtr);
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

            // Saved item counts. Mumbo tokens [0] and jiggy total [4] always sync; eggs [1]
            // / red [2] / gold [3] feathers only when the room shares consumables. Seed from
            // current local counts so ungated fields are preserved, then run the load-path
            // restorer to push the result into the runtime counts + HUD.
            if (state.contains("savedItems")) {
                auto& incoming = state["savedItems"];
                s32 size;
                u8* addr;
                saveditem_getSizeAndPtr(&size, &addr); // rebuilds the array from live counts
                u8 buf[5];
                for (s32 i = 0; i < 5; i++) {
                    buf[i] = (i < size) ? addr[i] : 0;
                }
                if (incoming.size() >= 5) {
                    buf[0] = incoming[0].get<u8>();
                    buf[4] = incoming[4].get<u8>();
                    if (roomState.shareConsumables) {
                        buf[1] = incoming[1].get<u8>();
                        buf[2] = incoming[2].get<u8>();
                        buf[3] = incoming[3].get<u8>();
                    }
                    func_803479C0(buf);
                }
            }

            // Per-level best times (truncated u16 each); copy into an aligned buffer first.
            if (state.contains("timeScores")) {
                auto incoming = state["timeScores"].get<std::vector<u8>>();
                u16 ts[0xB] = { 0 };
                size_t n = std::min(incoming.size(), sizeof(ts));
                for (size_t i = 0; i < n; i++) {
                    ((u8*)ts)[i] = incoming[i];
                }
                itemscore_timeScores_fromSaveData(ts);
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
