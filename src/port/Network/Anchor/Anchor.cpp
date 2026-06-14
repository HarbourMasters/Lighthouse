#include "Anchor.h"
#include "Authority.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/Engine.h"
#include "port/Nametag/Nametag.h"
#include "port/Interpolation/FrameInterpolation.h"
#include "port/ObjectExtension/ObjectExtension.h"

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

    if (IsSaveLoaded()) {
        SendPacket_RequestTeamState();
    }
}

void Anchor::OnDisconnected() {
    Authority_Reset();
    RegisterHooks();
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
        dummy->dummy_detachActor();
    }
    dummies.clear();
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
    // for (auto& [clientId, client] : clients) {
    //     if (client.dummy != nullptr && client.dummy->getDummyActor() == actor) {
    //         client.dummy->dummy_detachActor();
    //         RemoveDummy(clientId);
    //         return;
    //     }
    // }
}

void Anchor::RemoveDummy(uint32_t clientId) {
    if (dummies.contains(clientId)) {
        dummies.erase(clientId);
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
