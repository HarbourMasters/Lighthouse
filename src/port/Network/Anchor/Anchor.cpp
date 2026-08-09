#include "Anchor.h"
#include "Authority.h"
#include <cstring>
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/Engine.h"
#include "port/Nametag/Nametag.h"
#include "port/Interpolation/FrameInterpolation.h"
#include "port/ObjectExtension/ObjectExtension.h"
#include "port/Enhancements/Retention/Retention.h"

#include "variables.h"
#include "functions.h"

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
    roomState = RoomState{};
}

bool Anchor::IsGlobalRoom() {
    return strcmp(CVarGetString(CVAR_REMOTE_ANCHOR("RoomId"), ""), "lh-global") == 0;
}

bool Anchor::IsWorldSyncActive() {
    return isConnected && !IsGlobalRoom() && roomState.syncItemsAndFlags != 0;
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
    PlayerColors_reset();
    RefreshClientActors();
}

void Anchor::OnConnected() {
    SendPacket_Handshake();
    RegisterHooks();

    port_noteRetention_setForced(IsGlobalRoom() ? 0 : 1);
    port_jinjoRetention_setForced(IsGlobalRoom() ? 0 : 1);

    if (IsSaveLoaded()) {
        SendPacket_RequestTeamState();
        hasRequestedTeamState = true;
        reloadMapOnTeamState = true;
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

bool Anchor::AllowedWithoutGameSync(const std::string& packetType) {
    return packetType == HANDSHAKE || packetType == ALL_CLIENT_STATE || packetType == UPDATE_CLIENT_STATE ||
           packetType == UPDATE_ROOM_STATE || packetType == MAP_LOAD || packetType == PLAYER_UPDATE ||
           packetType == PLAYER_UPDATE_FULL || packetType == PLAYER_ANIM || packetType == PLAYER_SUBRANGE ||
           packetType == PLAYER_TRANSFORM || packetType == PLAYER_SFX || packetType == SERVER_MESSAGE;
}

void Anchor::SendJsonToRemote(nlohmann::json payload) {
    if (!isConnected) {
        return;
    }

    if (!roomState.syncItemsAndFlags && !AllowedWithoutGameSync(payload.value("type", std::string()))) {
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

    // Same rule inbound: a peer on an older build must not push world state into a room
    // that isn't syncing it.
    if (!roomState.syncItemsAndFlags && !AllowedWithoutGameSync(packetType)) {
        return;
    }

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
            else if (packetType == WATER_RISE)
                HandlePacket_WaterRise(payload);
            else if (packetType == PUZZLE_COUNT)
                HandlePacket_PuzzleCount(payload);
            else if (packetType == HUT_SMASH)
                HandlePacket_HutSmash(payload);
            else if (packetType == JIGGY_CRANE)
                HandlePacket_JiggyCrane(payload);
            else if (packetType == PEDESTAL_OWNER)
                HandlePacket_PedestalOwner(payload);
            else if (packetType == JIGGY_SPAWN)
                HandlePacket_SpawnJiggy(payload);
            else if (packetType == HONEYCOMB_SPAWN)
                HandlePacket_SpawnHoneycomb(payload);
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

// Roughly the top of Banjo's head.
static constexpr f32 kNametagHeight = 155.0f;

// The menu exposes the nametag range as a multiplier of this, the same way Extended Draw
// Distance scales off its own base.
static constexpr f32 kNametagRangeUnit = 3000.0f;

void Anchor::DrawDummies(OnPlayerDraw* event) {
    if (!isConnected)
        return;
    const bool showNametags = CVarGetInteger(CVAR_REMOTE_ANCHOR("Nametags"), 1) != 0;
    const f32 nametagRange = kNametagRangeUnit * CVarGetFloat(CVAR_REMOTE_ANCHOR("NametagScale"), 1.0f);
    for (const auto& [id, dummy] : dummies) {
        FrameInterpolation_RecordOpenChild(clients[id].name.c_str(), 0);
        dummy->Draw(event->gfx, event->mtx, event->vtx);
        FrameInterpolation_RecordCloseChild();

        // A nameless client would otherwise get an empty tag box floating over them.
        const std::string label = showNametags ? GetNametagLabel(id) : std::string();
        if (dummy->dummy_isVisible() && !label.empty()) {
            f32 position[3];
            dummy->dummy_getPosition(position);
            position[1] += kNametagHeight;
            const f32 fade = Nametag::FadeForDistance(position[0], position[1], position[2], nametagRange);
            Nametag::Push(id, position[0], position[1], position[2], label.c_str(), fade);
        }
    }
}

void Anchor::ClearDummies() {
    for (auto& [id, dummy] : dummies) {
        dummy->dummy_despawnActor(); // skips if already detached by map teardown
    }
    dummies.clear();
}

// Presence only: true in any room, including the global one. For dummy-player visuals.
extern "C" s32 port_anchor_isConnected(void) {
    Anchor* anchor = Anchor::GetInstance();
    return (anchor != nullptr && anchor->isConnected) ? 1 : 0;
}

// Lets decomp gate Anchor-only catch-up paths so single player keeps vanilla behaviour.
extern "C" s32 port_anchor_isWorldSyncActive(void) {
    Anchor* anchor = Anchor::GetInstance();
    return (anchor != nullptr && anchor->IsWorldSyncActive()) ? 1 : 0;
}

// actorArray_free tears down actors/markers without firing OnActorDestroy; forget them all.
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
    // Stand-in was destroyed externally; forget its marker, dummy stays registered.
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

// Queues the honeycomb spawn the switch press performs.
extern "C" void __baMarker_8028BA00(s32 honeycombId);

// The GV cactus and RBB boathouse honeycombs only exist once their switch is
// beak-busted: marker.c sets a map flag and spawns the actor. A teammate's press
// reaches us as flag state, so spawn the same actor the local press would have.
void Anchor::RevealSwitchHoneycomb() {
    s32 uid;
    s32 flag;

    switch (gsworld_getMap()) {
        case MAP_12_GV_GOBIS_VALLEY:
            uid = HONEYCOMB_B_GV_CACTUS;
            flag = 0xD;
            break;
        case MAP_36_RBB_BOATHOUSE:
            uid = HONEYCOMB_F_RBB_BOAT_HOUSE;
            flag = 0;
            break;
        default:
            return;
    }

    if (!mapSpecificFlags_get(flag) || honeycombscore_get((enum honeycomb_e)uid)) {
        return;
    }
    __baMarker_8028BA00(uid);
}

void Anchor::RemoveDummy(uint32_t clientId) {
    if (dummies.contains(clientId)) {
        dummies[clientId]->dummy_despawnActor();
        dummies.erase(clientId);
    }
}

extern "C" s32 port_cutsceneWarp_getReturnMap(void);
extern void port_breakable_clearForLevel(int32_t levelId);
extern void port_hutSmash_clearForLevel(int32_t levelId);
extern void port_eggToll_clearForLevel(int32_t levelId);
extern void port_puzzleStep_clearForLevel(int32_t levelId);
extern void port_carriedSync_clearForLevel(int32_t levelId);

s32 Anchor_LevelOfMap(s32 map) {
    if (map <= 0 || map >= MAP_NUM_MAPS) {
        return 0;
    }
    s32 level = (s32)map_getLevel((enum map_e)map);
    return (level > 0 && level < 0x20) ? level : 0;
}

void Anchor::SweepUnoccupiedLevelState(GameMap selfMap) {
    // Nothing is shared without world sync, so these stores hold only our own progress —
    // dropping them on a level change would undo single-player state.
    if (!IsWorldSyncActive()) {
        return;
    }

    s32 selfLevel = Anchor_LevelOfMap((s32)selfMap);
    if (selfLevel == 0 || selfLevel == (s32)LEVEL_D_CUTSCENE || selfMap == MAP_91_FILE_SELECT) {
        return;
    }

    bool occupied[0x20] = { false }; // level_e ids are small; matches jinjo retention slot count
    auto markOccupied = [&occupied](s32 map) {
        s32 level = Anchor_LevelOfMap(map);
        if (level != 0) {
            occupied[level] = true;
        }
    };
    markOccupied((s32)selfMap);
    markOccupied(port_cutsceneWarp_getReturnMap());
    for (auto& [clientId, client] : clients) {
        if (!client.self && client.online && client.isSaveLoaded) {
            markOccupied((s32)client.map);
            markOccupied(client.cutsceneReturnMap);
        }
    }
    for (s32 level = 1; level < 0x20; level++) {
        if (!occupied[level]) {
            port_breakable_clearForLevel(level);
            port_hutSmash_clearForLevel(level);
            port_eggToll_clearForLevel(level);
            port_puzzleStep_clearForLevel(level);
            port_carriedSync_clearForLevel(level);
        }
    }
}

void Anchor::RegisterDummy(DummyPlayer* dummy, uint32_t clientID) {
    dummies.emplace(clientID, dummy);
}

// Pushes a client's chosen model colours onto their stand-in so the next draw picks them up.
void Anchor::ApplyClientCosmetics(uint32_t clientId) {
    if (!clients.contains(clientId)) {
        return;
    }
    AnchorClient& client = clients[clientId];
    if (client.dummy == nullptr) {
        return;
    }
    client.dummy->dummy_setOwner(clientId);
    client.dummy->dummy_setColors(client.colors);
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
    s32 gameMode = getGameMode();
    if (gameMode == GAME_MODE_6_FILE_PLAYBACK || gameMode == GAME_MODE_7_ATTRACT_DEMO) {
        return false;
    }
    auto map = gsworld_getMap();
    return map != MAP_1E_CS_START_NINTENDO && map != MAP_1F_CS_START_RAREWARE && map != MAP_91_FILE_SELECT;
}

bool Anchor::ShouldShowNotifications() {
    return CVarGetInteger(CVAR_REMOTE_ANCHOR("Notifications"), 1) != 0;
}

// Public global room shows player number instead of custom name
std::string Anchor::GetNametagLabel(uint32_t clientId) {
    auto it = clients.find(clientId);
    if (it == clients.end()) {
        return "";
    }
    if (!IsGlobalRoom()) {
        return it->second.name;
    }
    if (it->second.joinOrder == 0) {
        return "Player";
    }
    return "Player " + std::to_string(it->second.joinOrder);
}

std::string Anchor::GetClientName(uint32_t clientId) {
    auto it = clients.find(clientId);
    if (it != clients.end() && !it->second.name.empty()) {
        return it->second.name;
    }
    return "A teammate";
}
