#pragma once
#ifdef __cplusplus

#include <mutex>
#include <queue>
#include <map>
#include <unordered_map>
#include <vector>
#include <libultraship/libultraship.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Network/Anchor/DummyPlayer.h"
#include "port/Network/Network.h"
#include "port/build.h"

extern "C" {
#include "prop.h"
#include "variables.h"
}

typedef struct {
    uint32_t clientId;
    std::string name;
    BKPlayerColorSet colors;
    std::string clientVersion;
    std::string teamId;
    bool online;
    bool self;
    uint32_t joinOrder;
    uint32_t seed;
    bool isSaveLoaded;
    bool isGameComplete;
    GameMap map, prevMap;
    s32 exit, prevExit;
    s32 cutsceneReturnMap;

    DummyPlayer* dummy;
} AnchorClient;

struct RoomState {
    uint32_t ownerClientId = 0;
    u8 pvpMode = 0;           // 0 = off, 1 = on, 2 = on with friendly fire
    u8 showLocationsMode = 0; // 0 = none, 1 = team, 2 = all
    u8 teleportMode = 0;      // 0 = off, 1 = team, 2 = all
    u8 syncItemsAndFlags = 0; // 0 = off, 1 = on
    u8 shareConsumables = 0;  // 0 = off, 1 = on — share egg/feather counts in team state
    bool isRomhack = false;
    std::string romhackName;
    bool isRando = false;
    int32_t seed = 0; // rando seed id (0 when the room is vanilla)
};

bool Anchor_ScopedFlagExcluded(s32 space, s32 index);

class Anchor : public Network {
private:
    uint32_t spawningDummyPlayerForClientId = 0;
    bool shouldRefreshActors = false;
    bool justLoadedSave = false;
    bool isHandlingUpdateTeamState = false;
    bool isProcessingIncomingPacket = false;
    bool hasRequestedTeamState = false;
    std::queue<nlohmann::json> incomingPacketQueue;
    std::mutex incomingPacketQueueMutex;
    std::queue<nlohmann::json> outgoingPacketQueue;
    std::mutex outgoingPacketQueueMutex;
    std::unordered_map<uint32_t, DummyPlayer*> dummies;

    void FlushPendingJiggySpawns();

    nlohmann::json PrepClientState();
    nlohmann::json PrepRoomState();
    void RegisterHooks();
    void RefreshClientActors();
    void SetDummyPlayerClientId(const Actor* actor, uint32_t clientId);
    void DrawDummies(OnPlayerDraw* event);
    void ClearDummies();
    void PopulateDummies(GameMap map);
    void RegisterDummy(DummyPlayer* dummy, uint32_t clientID);
    std::unordered_map<uint32_t, DummyPlayer*>* GetDummies();
    void UpdateDummies();
    void RemoveDummy(uint32_t clientId);

    void EvaluateDummyForClient(uint32_t clientId);
    void ApplyClientCosmetics(uint32_t clientId);

    void HandlePacket_AllClientState(nlohmann::json& payload);
    void HandlePacket_AuthorityState(nlohmann::json& payload);
    void HandlePacket_DamagePlayer(nlohmann::json& payload);
    void HandlePacket_DisableAnchor(nlohmann::json& payload);
    void HandlePacket_EntranceDiscovered(nlohmann::json& payload);
    void HandlePacket_GameComplete(nlohmann::json& payload);
    void HandlePacket_GiveItem(nlohmann::json& payload);
    void HandlePacket_MapLoad(nlohmann::json& payload);
    void HandlePacket_PlayerSfx(nlohmann::json& payload);
    void HandlePacket_PlayerAnimChange(nlohmann::json& payload);
    void HandlePacket_PlayerSubRangeChange(nlohmann::json& payload);
    void HandlePacket_PlayerTransformChange(nlohmann::json& payload);
    void HandlePacket_PlayerUpdate(nlohmann::json& payload);
    void HandlePacket_RequestTeamState(nlohmann::json& payload);
    void HandlePacket_RequestTeleport(nlohmann::json& payload);
    void HandlePacket_ServerMessage(nlohmann::json& payload);
    void HandlePacket_SetCheckStatus(nlohmann::json& payload);
    void HandlePacket_SetFlag(nlohmann::json& payload);
    void HandlePacket_SetItemCount(nlohmann::json& payload);
    void HandlePacket_SetAbility(nlohmann::json& payload);
    void HandlePacket_ScopedFlag(nlohmann::json& payload);
    void HandlePacket_RequestScopedState(nlohmann::json& payload);
    void HandlePacket_ScopedState(nlohmann::json& payload);
    void HandlePacket_CollectItem(nlohmann::json& payload);
    void HandlePacket_CarryThrow(nlohmann::json& payload);
    void HandlePacket_BreakObject(nlohmann::json& payload);
    void HandlePacket_EggToll(nlohmann::json& payload);
    void HandlePacket_FightEvent(nlohmann::json& payload);
    void HandlePacket_FightState(nlohmann::json& payload);
    void HandlePacket_FightUpdate(nlohmann::json& payload);
    void HandlePacket_PuzzleStep(nlohmann::json& payload);
    void HandlePacket_PuzzleCount(nlohmann::json& payload);
    void HandlePacket_WaterRise(nlohmann::json& payload);
    void HandlePacket_HutSmash(nlohmann::json& payload);
    void HandlePacket_JiggyCrane(nlohmann::json& payload);
    void HandlePacket_PedestalOwner(nlohmann::json& payload);
    void HandlePacket_SpawnJiggy(nlohmann::json& payload);
    void HandlePacket_SpawnHoneycomb(nlohmann::json& payload);
    void HandlePacket_TeleportTo(nlohmann::json& payload);
    void HandlePacket_UnsetFlag(nlohmann::json& payload);
    void HandlePacket_UpdateClientState(nlohmann::json& payload);
    void HandlePacket_UpdateRoomState(nlohmann::json& payload);
    void HandlePacket_UpdateTeamState(nlohmann::json& payload);
    void HandlePacket_VileEatRequest(nlohmann::json& payload);
    void HandlePacket_VileEatResult(nlohmann::json& payload);
    void HandlePacket_VileGameState(nlohmann::json& payload);
    void HandlePacket_VileHoleState(nlohmann::json& payload);
    void HandlePacket_VileUpdate(nlohmann::json& payload);

public:
    uint32_t ownClientId;
    bool reloadMapOnTeamState = false;
    inline static const std::string clientVersion = (char*)gGitCommitHash;

    // Packet types //
    inline static const std::string ALL_CLIENT_STATE = "ALL_CLIENT_STATE";
    inline static const std::string AUTHORITY_STATE = "AUTHORITY_STATE";
    inline static const std::string DAMAGE_PLAYER = "DAMAGE_PLAYER";
    inline static const std::string DISABLE_ANCHOR = "DISABLE_ANCHOR";
    inline static const std::string ENTRANCE_DISCOVERED = "ENTRANCE_DISCOVERED";
    inline static const std::string GAME_COMPLETE = "GAME_COMPLETE";
    inline static const std::string GIVE_ITEM = "GIVE_ITEM";
    inline static const std::string HANDSHAKE = "HANDSHAKE";
    inline static const std::string MAP_LOAD = "MAP_LOAD";
    inline static const std::string PLAYER_ANIM = "PLAYER_ANIM";
    inline static const std::string PLAYER_SFX = "PLAYER_SFX";
    inline static const std::string PLAYER_SUBRANGE = "PLAYER_SUBRANGE";
    inline static const std::string PLAYER_TRANSFORM = "PLAYER_TRANSFORM";
    inline static const std::string PLAYER_UPDATE = "PLAYER_UPDATE";
    inline static const std::string PLAYER_UPDATE_FULL = "PLAYER_UPDATE_FULL";
    inline static const std::string REQUEST_TEAM_STATE = "REQUEST_TEAM_STATE";
    inline static const std::string REQUEST_TELEPORT = "REQUEST_TELEPORT";
    inline static const std::string SERVER_MESSAGE = "SERVER_MESSAGE";
    inline static const std::string SET_CHECK_STATUS = "SET_CHECK_STATUS";
    inline static const std::string SET_FLAG = "SET_FLAG";
    inline static const std::string ITEM_COUNT = "ITEM_COUNT";
    inline static const std::string SET_ABILITY = "SET_ABILITY";
    inline static const std::string SCOPED_FLAG = "SCOPED_FLAG";
    inline static const std::string REQUEST_SCOPED_STATE = "REQUEST_SCOPED_STATE";
    inline static const std::string SCOPED_STATE = "SCOPED_STATE";
    inline static const std::string COLLECT_ITEM = "COLLECT_ITEM";
    inline static const std::string CARRY_THROW = "CARRY_THROW";
    inline static const std::string BREAK_OBJECT = "BREAK_OBJECT";
    inline static const std::string EGG_TOLL = "EGG_TOLL";
    inline static const std::string FIGHT_EVENT = "FIGHT_EVENT";
    inline static const std::string FIGHT_STATE = "FIGHT_STATE";
    inline static const std::string FIGHT_UPDATE = "FIGHT_UPDATE";
    inline static const std::string PUZZLE_STEP = "PUZZLE_STEP";
    inline static const std::string PUZZLE_COUNT = "PUZZLE_COUNT";
    inline static const std::string WATER_RISE = "WATER_RISE";
    inline static const std::string HUT_SMASH = "HUT_SMASH";
    inline static const std::string JIGGY_CRANE = "JIGGY_CRANE";
    inline static const std::string PEDESTAL_OWNER = "PEDESTAL_OWNER";
    inline static const std::string JIGGY_SPAWN = "JIGGY_SPAWN";
    inline static const std::string HONEYCOMB_SPAWN = "HONEYCOMB_SPAWN";
    inline static const std::string TELEPORT_TO = "TELEPORT_TO";
    inline static const std::string UNSET_FLAG = "UNSET_FLAG";
    inline static const std::string UPDATE_CLIENT_STATE = "UPDATE_CLIENT_STATE";
    inline static const std::string UPDATE_ROOM_STATE = "UPDATE_ROOM_STATE";
    inline static const std::string UPDATE_TEAM_STATE = "UPDATE_TEAM_STATE";
    inline static const std::string VILE_EAT_REQUEST = "VILE_EAT_REQUEST";
    inline static const std::string VILE_EAT_RESULT = "VILE_EAT_RESULT";
    inline static const std::string VILE_GAME_STATE = "VILE_GAME_STATE";
    inline static const std::string VILE_HOLE_STATE = "VILE_HOLE_STATE";
    inline static const std::string VILE_UPDATE = "VILE_UPDATE";

    std::map<uint32_t, AnchorClient> clients;
    RoomState roomState;
    std::string lastWarnedRomhackLabel;
    std::string lastWarnedRandoState;
    bool hasCheckedRandoCompat = false;

    void Enable();
    void Disable();
    void OnIncomingJson(nlohmann::json payload);
    void OnConnected();
    void OnDisconnected();
    void ProcessOutgoingPackets();
    void DrawMenu();
    void ProcessIncomingPacketQueue();
    void SendJsonToRemote(nlohmann::json packet);
    bool IsSaveLoaded();
    bool CanTeleportTo(uint32_t clientId);
    uint32_t GetDummyPlayerClientId(const Actor* actor);
    bool GetCurrentMapPlayers();

    bool IsGlobalRoom();

    // The single gate for every Anchor behaviour that touches the world. A presence-only
    // room (the global room, or a private room with "Sync Items & Flags" off) has to play
    // exactly like single player, so only dummy players are allowed past this.
    bool IsWorldSyncActive();

    // Presence/position/plumbing packets, the only ones allowed through when the room
    // is not syncing game state. Anything absent is treated as world state and dropped
    // at the send and receive choke points, so a new packet type is unsynced by default.
    static bool AllowedWithoutGameSync(const std::string& packetType);

    void AdoptRemoteCheck(s32 rc);
    void CheckRandoRoomCompatibility();

    bool ShouldShowNotifications();
    std::string GetClientName(uint32_t clientId);
    std::string GetNametagLabel(uint32_t clientId);

    void SendPacket_AuthorityState(u8 activity, bool claimed);
    void SendPacket_ClearTeamState(std::string teamId);
    void SendPacket_DamagePlayer(u32 clientId, u8 damageEffect, u8 damage);
    void SendPacket_EntranceDiscovered(u16 entranceIndex);
    void SendPacket_GameComplete();
    void SendPacket_GiveItem(u16 modId, s16 getItemId);
    void SendPacket_Handshake();
    void SendPacket_MapLoad(GameMap map, s32 exit);
    void SendPacket_PlayerAnimChange(AssetID anim_id, f32 duration, AnimControl control, f32 start_position,
                                     f32 subrange_end, bool smooth);
    void SendPacket_PlayerAnimReset();
    void SendPacket_PlayerSfx(u16 sfxId);
    void SendPacket_PlayerSubRangeChange(f32 duration, f32 end);
    // targetClientId 0 = broadcast/all current-map players; nonzero = send only to that
    // client (used to hand a late arrival our current state directly).
    void SendPacket_PlayerTransformChange(Transformation tf_id, uint32_t targetClientId = 0);
    void SendPacket_PlayerUpdate(bool full = false, uint32_t targetClientId = 0);
    void SendPacket_RequestTeamState(bool force = false);
    void SendPacket_RequestTeleport(uint32_t clientId);
    void SendPacket_SetCheckStatus(s32 rc, s32 map);
    void SendPacket_SetFlag(u8 flagSpace, s16 flag);
    void SendPacket_SetItemCount(s16 item, s32 count);
    void SendPacket_SetAbility(s16 move, u8 value);
    void SendPacket_ScopedFlag(u8 space, s16 index, u8 value);
    void SendPacket_RequestScopedState(GameMap map);
    void SendPacket_CollectItem(u8 kind, s32 id);
    void SendPacket_CarryThrow(s32 markerId, f32 start[3], f32 target[3]);
    void SendPacket_BreakObject(s16 markerId, s32 x, s32 y, s32 z, s32 map, bool replay = true);
    void SendPacket_EggToll(s16 secondaryId, s32 stage, s32 map);
    void SendPacket_FightEvent(s32 ev, s32 a, s32 b, const f32 v0[3], const f32 v1[3], const f32 v2[3]);
    void SendPacket_FightState(u32 targetClientId);
    void SendPacket_FightUpdate(const f32 pos[3], f32 yaw, s32 state, s32 phase, s32 mirror, s32 vuln);
    void SendPacket_PuzzleStep(s32 puzzleId, s32 bits, s32 map, s32 phash = 0);
    void SendPacket_PuzzleCount(s32 counterId, s32 delta, s32 map);
    void SendPacket_WaterRise(s32 map, s32 kind, s32 p1, s32 p2, f32 duration);
    void SendPacket_HutSmash(s32 x, s32 y, s32 z, s32 loot, s32 map);
    void SendPacket_JiggyCrane(s32 stage);
    void SendPacket_PedestalOwner(s32 id, bool claimed);
    void SendPacket_SpawnJiggy(s16 jiggyId, f32 x, f32 y, f32 z);
    void SendPacket_SpawnHoneycomb(s16 honeycombId, s32 bundleId, f32 x, f32 y, f32 z);
    void SendPacket_TeleportTo(uint32_t clientId);
    void SendPacket_UnsetFlag(u8 flagSpace, s16 flag);
    void SendPacket_UpdateClientState();
    void SendPacket_UpdateRoomState();
    void SendPacket_UpdateTeamState();
    void SendPacket_VileEatRequest(u8 holeId);
    void SendPacket_VileEatResult(u32 eaterClientId, u8 pieceType, u8 correctType);
    void SendPacket_VileGameState();
    void SendPacket_VileHoleState(u8 holeId, u8 holeState, u8 pieceType, u32 eaterClientId);
    void SendPacket_VileUpdate(const f32 position[3], f32 pitch, f32 yaw, f32 roll, u8 animMode);
    void OnActorDestroyed(Actor* actor);
    void RevealSwitchHoneycomb();
    void SendToCurrentMapPlayers(nlohmann::json& payload);
    void SendToCurrentLevelPlayers(nlohmann::json& payload);
    void SweepUnoccupiedLevelState(GameMap selfMap);

    static Anchor* GetInstance();
    static void Init();
};

typedef enum {
    // Starting at 5 to continue from the last value in the PlayerDamageResponseType enum
    DUMMY_PLAYER_HIT_RESPONSE_STUN = 5,
    DUMMY_PLAYER_HIT_RESPONSE_FIRE,
    DUMMY_PLAYER_HIT_RESPONSE_NORMAL,
} DummyPlayerDamageResponseType;

class AnchorRoomWindow : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override;
    void Draw() override;
    void UpdateElement() override{};
};

#endif // __cplusplus
