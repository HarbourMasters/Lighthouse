#include "Anchor.h"
#include "Authority.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/Engine.h"
#include "port/Nametag/Nametag.h"
#include "port/Interpolation/FrameInterpolation.h"
#include "port/ObjectExtension/ObjectExtension.h"
#include "port/Enhancements/Retention/Retention.h"

extern "C" {
#include "variables.h"
#include "functions.h"
// extern PlayState* gPlayState;
}

// MARK: - Overrides

static Anchor* Instance;

void Anchor::Init() {
    Instance = new Anchor();
    Instance->RegisterHooks();
}

Anchor* Anchor::GetInstance() {
    return Instance;
}

void Anchor::Enable() {
    Network::Enable(CVarGetString(CVAR_REMOTE_ANCHOR("Host"), "anchor.hm64.org"),
                    CVarGetInteger(CVAR_REMOTE_ANCHOR("Port"), 43383));
    ownClientId = CVarGetInteger(CVAR_REMOTE_ANCHOR("LastClientId"), 0);
    roomState.ownerClientId = 0;
}

void Anchor::Disable() {
    Network::Disable();

    Authority_Reset();
    dummies.clear();
    for (auto& [clientId, client] : clients) {
        if (client.dummy != nullptr) {
            client.dummy->dummy_free();
            free(client.dummy);
        }
    }
    clients.clear();
    RefreshClientActors();
}

void Anchor::OnConnected() {
    SendPacket_Handshake();
    RegisterHooks();

    // Realtime note/jinjo collection sync depends on those retention systems running, so force
    // them on while connected — without touching the user's CVar (their setting is preserved).
    port_noteRetention_setForced(1);
    port_jinjoRetention_setForced(1);

    if (IsSaveLoaded()) {
        SendPacket_RequestTeamState();
        hasRequestedTeamState = true;
    }
}

void Anchor::OnDisconnected() {
    Authority_Reset();
    RegisterHooks();

    port_noteRetention_setForced(0);
    port_jinjoRetention_setForced(0);
}

void Anchor::ProcessOutgoingPackets() {
    // Copy all queued packets while holding the lock, then send them after releasing
    std::queue<nlohmann::json> packetsToSend;
    {
        std::lock_guard<std::mutex> lock(outgoingPacketQueueMutex);
        packetsToSend.swap(outgoingPacketQueue);
    }

    // Send packets without holding the lock
    while (!packetsToSend.empty()) {
        nlohmann::json payload = packetsToSend.front();
        packetsToSend.pop();

        if (!payload.contains("quiet")) {
            SPDLOG_DEBUG("[Anchor] Sending payload:\n{}", payload.dump());
        }
        Network::SendJsonToRemote(payload);
    }
}

void Anchor::SendJsonToRemote(nlohmann::json payload) {
    if (!isConnected) {
        return;
    }

    payload["clientId"] = ownClientId;
    if (!payload.contains("quiet")) {
        SPDLOG_DEBUG("[Anchor] Queuing payload:\n{}", payload.dump());
    }

    if (payload["type"] == HANDSHAKE) {
        Network::SendJsonToRemote(payload);
        return;
    }

    // Queue the packet to be sent on the network thread
    std::lock_guard<std::mutex> lock(outgoingPacketQueueMutex);
    outgoingPacketQueue.push(payload);
}

void Anchor::OnIncomingJson(nlohmann::json payload) {
    // If it doesn't contain a type, it's not a valid payload
    if (!payload.contains("type")) {
        return;
    }

    // If it's not a quiet payload, log it
    if (!payload.contains("quiet")) {
        SPDLOG_DEBUG("[Anchor] Received payload:\n{}", payload.dump());
    }

    std::string packetType = payload["type"].get<std::string>();

    // Ignore packets from mismatched clients, except for ALL_CLIENT_STATE, UPDATE_CLIENT_STATE, and
    // PLAYER_UPDATE(_FULL)
    if (packetType != ALL_CLIENT_STATE && packetType != UPDATE_CLIENT_STATE && packetType != PLAYER_UPDATE &&
        packetType != PLAYER_UPDATE_FULL) {
        if (payload.contains("clientId")) {
            uint32_t clientId = payload["clientId"].get<uint32_t>();
            if (clients.contains(clientId) && clients[clientId].clientVersion != clientVersion) {
                return;
            }
        }
    }

    // Queue all packets to be processed on the game thread
    std::lock_guard<std::mutex> lock(incomingPacketQueueMutex);
    incomingPacketQueue.push(payload);
}

void Anchor::ProcessIncomingPacketQueue() {
    // Copy all queued packets while holding the lock, then process them after releasing
    std::queue<nlohmann::json> packetsToProcess;
    {
        std::lock_guard<std::mutex> lock(incomingPacketQueueMutex);
        packetsToProcess.swap(incomingPacketQueue);
    }

    // Process packets without holding the lock
    while (!packetsToProcess.empty()) {
        nlohmann::json payload = packetsToProcess.front();
        packetsToProcess.pop();

        std::string packetType = payload["type"].get<std::string>();

        isProcessingIncomingPacket = true;

        try {
            // packetType here is a string so we can't use a switch statement
            if (packetType == ALL_CLIENT_STATE)
                HandlePacket_AllClientState(payload);
            else if (packetType == AUTHORITY_STATE)
                HandlePacket_AuthorityState(payload);
            else if (packetType == DAMAGE_PLAYER)
                HandlePacket_DamagePlayer(payload);
            else if (packetType == DISABLE_ANCHOR)
                HandlePacket_DisableAnchor(payload);
            else if (packetType == ENTRANCE_DISCOVERED)
                HandlePacket_EntranceDiscovered(payload);
            else if (packetType == GAME_COMPLETE)
                HandlePacket_GameComplete(payload);
            else if (packetType == GIVE_ITEM)
                HandlePacket_GiveItem(payload);
            else if (packetType == PLAYER_ANIM)
                HandlePacket_PlayerAnimChange(payload);
            else if (packetType == PLAYER_SUBRANGE)
                HandlePacket_PlayerSubRangeChange(payload);
            else if (packetType == PLAYER_TRANSFORM)
                HandlePacket_PlayerTransformChange(payload);
            else if (packetType == PLAYER_UPDATE || packetType == PLAYER_UPDATE_FULL)
                HandlePacket_PlayerUpdate(payload);
            else if (packetType == PLAYER_SFX)
                HandlePacket_PlayerSfx(payload);
            else if (packetType == UPDATE_TEAM_STATE)
                HandlePacket_UpdateTeamState(payload);
            else if (packetType == REQUEST_TEAM_STATE)
                HandlePacket_RequestTeamState(payload);
            else if (packetType == REQUEST_TELEPORT)
                HandlePacket_RequestTeleport(payload);
            else if (packetType == SERVER_MESSAGE)
                HandlePacket_ServerMessage(payload);
            else if (packetType == SET_CHECK_STATUS)
                HandlePacket_SetCheckStatus(payload);
            else if (packetType == SET_FLAG)
                HandlePacket_SetFlag(payload);
            else if (packetType == ITEM_COUNT)
                HandlePacket_SetItemCount(payload);
            else if (packetType == SET_ABILITY)
                HandlePacket_SetAbility(payload);
            else if (packetType == SCOPED_FLAG)
                HandlePacket_ScopedFlag(payload);
            else if (packetType == REQUEST_SCOPED_STATE)
                HandlePacket_RequestScopedState(payload);
            else if (packetType == SCOPED_STATE)
                HandlePacket_ScopedState(payload);
            else if (packetType == COLLECT_ITEM)
                HandlePacket_CollectItem(payload);
            else if (packetType == CARRY_THROW)
                HandlePacket_CarryThrow(payload);
            else if (packetType == BREAK_OBJECT)
                HandlePacket_BreakObject(payload);
            else if (packetType == EGG_TOLL)
                HandlePacket_EggToll(payload);
            else if (packetType == PUZZLE_STEP)
                HandlePacket_PuzzleStep(payload);
            else if (packetType == HUT_SMASH)
                HandlePacket_HutSmash(payload);
            else if (packetType == JIGGY_CRANE)
                HandlePacket_JiggyCrane(payload);
            else if (packetType == PEDESTAL_OWNER)
                HandlePacket_PedestalOwner(payload);
            else if (packetType == JIGGY_SPAWN)
                HandlePacket_SpawnJiggy(payload);
            else if (packetType == TELEPORT_TO)
                HandlePacket_TeleportTo(payload);
            else if (packetType == UNSET_FLAG)
                HandlePacket_UnsetFlag(payload);
            else if (packetType == MAP_LOAD)
                HandlePacket_MapLoad(payload);
            else if (packetType == UPDATE_CLIENT_STATE)
                HandlePacket_UpdateClientState(payload);
            else if (packetType == UPDATE_ROOM_STATE)
                HandlePacket_UpdateRoomState(payload);
            else if (packetType == VILE_EAT_REQUEST)
                HandlePacket_VileEatRequest(payload);
            else if (packetType == VILE_EAT_RESULT)
                HandlePacket_VileEatResult(payload);
            else if (packetType == VILE_GAME_STATE)
                HandlePacket_VileGameState(payload);
            else if (packetType == VILE_HOLE_STATE)
                HandlePacket_VileHoleState(payload);
            else if (packetType == VILE_UPDATE)
                HandlePacket_VileUpdate(payload);
            else if (packetType == FIGHT_UPDATE)
                HandlePacket_FightUpdate(payload);
            else if (packetType == FIGHT_EVENT)
                HandlePacket_FightEvent(payload);
            else if (packetType == FIGHT_STATE)
                HandlePacket_FightState(payload);
        } catch (const std::exception& e) {
            SPDLOG_ERROR("[Anchor] Exception while processing incoming packet {}", e.what());
            SPDLOG_ERROR("[Anchor] Packet: {}", payload.dump());
        }

        isProcessingIncomingPacket = false;
    }
}

// MARK: - Misc/Helpers

struct DummyPlayerClientId {
    uint32_t clientId;
};

// Kills all existing anchor actors and respawns them with the new client data
static ObjectExtension::Register<DummyPlayerClientId> DummyPlayerClientIdRegister;

uint32_t Anchor::GetDummyPlayerClientId(const Actor* actor) {
    const DummyPlayerClientId* clientId = ObjectExtension::GetInstance().Get<DummyPlayerClientId>(actor);
    return clientId != nullptr ? clientId->clientId : 0;
}

void Anchor::SetDummyPlayerClientId(const Actor* actor, uint32_t clientId) {
    ObjectExtension::GetInstance().Set<DummyPlayerClientId>(actor, DummyPlayerClientId{ clientId });
}

void Anchor::DrawDummies(OnPlayerDraw* event) {
    if (!isConnected)
        return;
    for (const auto& [id, dummy] : dummies) {
        FrameInterpolation_RecordOpenChild(clients[id].name.c_str(), 0);
        dummy->Draw(event->gfx, event->mtx, event->vtx);
        FrameInterpolation_RecordCloseChild();
    }
}

void Anchor::ClearDummies() {
    for (auto& [id, dummy] : dummies) {
        // Despawn the stand-in properly (shadow unlink included) rather than leaking it into the
        // map. On the map-teardown path the markers were already detached (actorArray_free →
        // port_anchorDummies_onActorsFreed), making this a no-op; it does real work for live
        // actors (disconnect mid-map).
        dummy->dummy_despawnActor();
    }
    dummies.clear();
}

// actorArray_free is tearing down every actor and marker wholesale — and that path fires no
// OnActorDestroy events — so the stand-in markers are about to dangle (a freed marker resolves
// to non-null garbage, not nullptr). Forget them all; dummies that should still exist respawn
// their stand-ins lazily on the next update.
extern "C" void port_anchorDummies_onActorsFreed(void) {
    Anchor* anchor = Anchor::GetInstance();
    if (anchor == nullptr) {
        return;
    }
    for (auto& [clientId, client] : anchor->clients) {
        if (client.dummy != nullptr) {
            client.dummy->dummy_detachActor();
        }
    }
}

// Takes the map explicitly rather than reading gsworld_getMap(): during the
// OnMapLoad event the new map hasn't been committed yet, so gsworld_getMap()
// still reports the map being left.
void Anchor::PopulateDummies(GameMap map) {
    for (const auto& [clientId, client] : clients) {
        if (client.map == map && !client.self && !dummies.contains(clientId) && client.online) {
            client.dummy->dummy_reset();
            RegisterDummy(client.dummy, clientId);
        }
    }
}

std::unordered_map<uint32_t, DummyPlayer*>* Anchor::GetDummies() {
    return &dummies;
}

void Anchor::UpdateDummies() {
    if (IsSaveLoaded() && isConnected) {
        for (const auto& [id, dummy] : dummies) {
            dummy->dummy_update();
        }
    }
}

void Anchor::OnActorDestroyed(Actor* actor) {
    // The engine destroyed a dummy's stand-in behind our back (or our own despawn is mid-flight):
    // forget the marker so nothing dereferences it. The dummy stays registered — its update
    // lazily respawns the stand-in if it should still exist.
    if (actor == nullptr || actor->marker == nullptr) {
        return;
    }
    for (auto& [clientId, client] : clients) {
        if (client.dummy != nullptr && client.dummy->dummy_getMarker() == actor->marker) {
            client.dummy->dummy_detachActor();
            return;
        }
    }
}

void Anchor::RemoveDummy(uint32_t clientId) {
    if (dummies.contains(clientId)) {
        // Mid-map removal (client left the map / went offline): take the stand-in with it.
        dummies[clientId]->dummy_despawnActor();
        dummies.erase(clientId);
    }
}

// Per-level clears for the temporary-persistence session stores (defined in the respective
// packet modules, alongside their team-state snapshot/restore).
extern void port_breakable_clearForLevel(int32_t levelId);
extern void port_hutSmash_clearForLevel(int32_t levelId);
extern void port_eggToll_clearForLevel(int32_t levelId);
extern void port_puzzleStep_clearForLevel(int32_t levelId);

// Temporary-persistence state (broken windows/grates, smashed huts, egg tolls, puzzle steps) is
// only valid while its level stays continuously occupied: vanilla persists none of it, so once
// the last player leaves a level the records must reset or the level stays "used up" for the
// whole session. Rather than electing the last leaver or first returner to broadcast a clear,
// every client applies this rule to its own copy whenever anyone's location changes — all copies
// converge on the same event stream, a last-occupant crash still sweeps (going offline removes
// them from occupancy), and late joiners can't resurrect stale records because team-state
// snapshots are served by a live (already-swept) client. Note/jinjo retention and spawned
// jiggies are deliberately NOT swept: those mirror vanilla-persistent state.
void Anchor::SweepUnoccupiedLevelState(GameMap selfMap) {
    // level_e ids are small (1..0xD); 0x20 gives the same headroom the jinjo retention slots use.
    bool occupied[0x20] = { false };
    auto markOccupied = [&occupied](s32 map) {
        if (map <= 0 || map >= MAP_NUM_MAPS) {
            return; // unknown/boot map: map_getLevel on a bad id is unsafe
        }
        s32 level = (s32)map_getLevel((enum map_e)map);
        if (level > 0 && level < 0x20) {
            occupied[level] = true;
        }
    };
    markOccupied((s32)selfMap);
    for (auto& [clientId, client] : clients) {
        if (!client.self && client.online && client.isSaveLoaded) {
            markOccupied((s32)client.map);
        }
    }
    for (s32 level = 1; level < 0x20; level++) {
        if (!occupied[level]) {
            port_breakable_clearForLevel(level);
            port_hutSmash_clearForLevel(level);
            port_eggToll_clearForLevel(level);
            port_puzzleStep_clearForLevel(level);
        }
    }
}

void Anchor::RegisterDummy(DummyPlayer* dummy, uint32_t clientID) {
    dummies.emplace(clientID, dummy);
}

void Anchor::EvaluateDummyForClient(uint32_t clientId) {
    if (!clients.contains(clientId))
        return;
    AnchorClient& client = clients[clientId];
    if (client.dummy == nullptr)
        return;
    bool shouldBeActive = IsSaveLoaded() && client.online && !client.self && client.map == gsworld_getMap();
    bool isActive = dummies.contains(clientId);

    if (shouldBeActive && !isActive) {
        client.dummy->dummy_reset();
        RegisterDummy(client.dummy, clientId);
    } else if (!shouldBeActive && isActive) {
        RemoveDummy(clientId);
    }
}

void Anchor::RefreshClientActors() {
    if (!IsSaveLoaded() || !shouldRefreshActors) {
        return;
    }

    shouldRefreshActors = false;

    spawningDummyPlayerForClientId = 0;
}

bool Anchor::IsSaveLoaded() {
    // Attract-mode demos and file playback run real gameplay logic (collecting items, setting
    // flags) against a throwaway save. Never treat those as a loaded save, or connecting during
    // a demo syncs the demo's pickups and flags to teammates.
    s32 gameMode = getGameMode();
    if (gameMode == GAME_MODE_6_FILE_PLAYBACK || gameMode == GAME_MODE_7_ATTRACT_DEMO) {
        return false;
    }
    auto map = gsworld_getMap();
    return map != MAP_1E_CS_START_NINTENDO && map != MAP_1F_CS_START_RAREWARE && map != MAP_91_FILE_SELECT;
    /* if (gPlayState == nullptr) {
         return false;
     }

     if (GET_PLAYER(gPlayState) == nullptr) {
         return false;
     }

     if (gSaveContext.fileNum < 0 || gSaveContext.fileNum > 2) {
         return false;
     }

     if (gSaveContext.gameMode != GAMEMODE_NORMAL) {
         return false;
     }*/

    // return true;
}

bool Anchor::ShouldShowNotifications() {
    return CVarGetInteger(CVAR_REMOTE_ANCHOR("Notifications"), 1) != 0;
}

std::string Anchor::GetClientName(uint32_t clientId) {
    auto it = clients.find(clientId);
    if (it != clients.end() && !it->second.name.empty()) {
        return it->second.name;
    }
    return "A teammate";
}
