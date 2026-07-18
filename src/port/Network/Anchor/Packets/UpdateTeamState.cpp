#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/UI/Notification.h"
#include "port/Enhancements/Retention/Retention.h"
#include "port/Rando/Rando.h"
#include <algorithm>
#include <vector>

extern "C" {
#include "variables.h"
#include "functions.h"
}

// Temporary persistence
extern std::vector<int32_t> port_breakable_snapshotBroken();
extern void port_breakable_restoreBroken(const std::vector<int32_t>& flat);
extern std::vector<int32_t> port_carriedSync_snapshotCollected();
extern void port_carriedSync_restoreCollected(const std::vector<int32_t>& flat);
extern std::vector<int32_t> port_eggToll_snapshot();
extern void port_eggToll_restore(const std::vector<int32_t>& flat);
extern std::vector<int32_t> port_puzzleStep_snapshot();
extern void port_puzzleStep_restore(const std::vector<int32_t>& flat);
extern std::vector<int32_t> port_puzzleCount_snapshot();
extern void port_puzzleCount_restore(const std::vector<int32_t>& flat);
extern std::vector<int32_t> port_puzzlePos_snapshot();
extern void port_puzzlePos_restore(const std::vector<int32_t>& flat);
extern std::vector<int32_t> port_jiggySpawn_snapshot();
extern void port_jiggySpawn_restore(const std::vector<int32_t>& flat);
extern std::vector<int32_t> port_hutSmash_snapshot();
extern void port_hutSmash_restore(const std::vector<int32_t>& flat);

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
    payload["state"]["savedItems"] = ScoreBytes(saveditem_getSizeAndPtr);
    payload["state"]["abilities"] = ScoreBytes(ability_getSizeAndPtr);
    // Time scores use a (s32*, void**) accessor, so pack inline rather than via ScoreBytes.
    s32 tsSize;
    void* tsAddr;
    timeScores_getSizeAndPtr(&tsSize, &tsAddr);
    payload["state"]["timeScores"] = std::vector<u8>((u8*)tsAddr, (u8*)tsAddr + tsSize);
    payload["state"]["volatileFlags"] = ScoreBytes(volatileFlag_getSizeAndPtr);
    // In-memory session sets (never saved): broken non-persistent objects + collected worms/acorns.
    payload["state"]["brokenObjects"] = port_breakable_snapshotBroken();
    payload["state"]["carriedCollected"] = port_carriedSync_snapshotCollected();
    payload["state"]["eggTolls"] = port_eggToll_snapshot();
    payload["state"]["puzzleSteps"] = port_puzzleStep_snapshot();
    payload["state"]["puzzleCounts"] = port_puzzleCount_snapshot();
    payload["state"]["puzzlePos"] = port_puzzlePos_snapshot();
    payload["state"]["spawnedJiggies"] = port_jiggySpawn_snapshot();
    payload["state"]["huts"] = port_hutSmash_snapshot();

    if (IS_RANDO) {
        std::vector<u8> checks(RC_MAX, 0);
        for (s32 rc = RC_UNKNOWN + 1; rc < RC_MAX; rc++) {
            checks[rc] = RANDO_SAVE_CHECKS[rc].obtained ? 1 : 0;
        }
        payload["state"]["randoChecks"] = checks;

        std::vector<int32_t> randoFlags(RANDO_INF_MAX, 0);
        for (s32 i = RANDO_INF_UNKNOWN + 1; i < RANDO_INF_MAX; i++) {
            randoFlags[i] = RANDO_SAVE_FLAGS[i].flagState;
        }
        payload["state"]["randoFlags"] = randoFlags;
    }

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
        if (state.contains("noteRetention")) {
            ApplyTeamBytes(state["noteRetention"], port_noteRetention_getSizeAndPtr);
        }
        if (state.contains("jinjoRetention")) {
            ApplyTeamBytes(state["jinjoRetention"], port_jinjoRetention_getSizeAndPtr);
        }
        if (state.contains("abilities")) {
            ApplyTeamBytes(state["abilities"], ability_getSizeAndPtr);
        }
        // In-memory session sets (never saved)
        if (state.contains("brokenObjects")) {
            port_breakable_restoreBroken(state["brokenObjects"].get<std::vector<int32_t>>());
        }
        if (state.contains("carriedCollected")) {
            port_carriedSync_restoreCollected(state["carriedCollected"].get<std::vector<int32_t>>());
        }
        if (state.contains("eggTolls")) {
            port_eggToll_restore(state["eggTolls"].get<std::vector<int32_t>>());
        }
        if (state.contains("puzzleSteps")) {
            port_puzzleStep_restore(state["puzzleSteps"].get<std::vector<int32_t>>());
        }
        if (state.contains("puzzlePos")) {
            port_puzzlePos_restore(state["puzzlePos"].get<std::vector<int32_t>>());
        }
        if (state.contains("puzzleCounts")) {
            port_puzzleCount_restore(state["puzzleCounts"].get<std::vector<int32_t>>());
        }
        if (state.contains("spawnedJiggies")) {
            port_jiggySpawn_restore(state["spawnedJiggies"].get<std::vector<int32_t>>());
        }
        if (state.contains("huts")) {
            port_hutSmash_restore(state["huts"].get<std::vector<int32_t>>());
        }

        // Recompute for the overwrites
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

        // Randomizer catch-up
        if (IS_RANDO && IsSaveLoaded()) {
            if (state.contains("randoChecks")) {
                auto checks = state["randoChecks"].get<std::vector<u8>>();
                s32 n = std::min((s32)checks.size(), (s32)RC_MAX);
                for (s32 rc = RC_UNKNOWN + 1; rc < n; rc++) {
                    if (checks[rc]) {
                        AdoptRemoteCheck(rc);
                    }
                }
            }
            if (state.contains("randoFlags")) {
                auto flags = state["randoFlags"].get<std::vector<int32_t>>();
                s32 n = std::min((s32)flags.size(), (s32)RANDO_INF_MAX);
                for (s32 i = RANDO_INF_UNKNOWN + 1; i < n; i++) {
                    RANDO_SAVE_FLAGS[i].flagState = flags[i];
                }
            }
        }

        SweepUnoccupiedLevelState((GameMap)gsworld_getMap());

        Notification::Emit({
            .message = "Save updated from team",
        });

        // Adopting team state overwrites flags via a direct byte copy, so actors already spawned (the
        // world we're standing in) never re-check them. Hold the map load when loading a file already
        // connected to a room (with backup timeout allowing load and reloading once received). Otherwise,
        // force reload on receipt to apply changed state.
        if (teamStateHoldArmed) {
            teamStateHoldArmed = false;
            if (teamStateHoldMapLoaded && IsSaveLoaded()) {
                transitionToMap(gsworld_getMap(), gsworld_getExit(), 1);
            }
        } else if (reloadMapOnTeamState && IsSaveLoaded()) {
            reloadMapOnTeamState = false;
            transitionToMap(gsworld_getMap(), gsworld_getExit(), 1);
        }
    }

    if (payload.contains("queue")) {
        std::lock_guard<std::mutex> lock(incomingPacketQueueMutex);
        for (auto& item : payload["queue"]) {
            incomingPacketQueue.push(nlohmann::json::parse(item.get<std::string>()));
        }
    }

    isHandlingUpdateTeamState = false;
}
