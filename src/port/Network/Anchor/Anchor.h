#pragma once
#ifdef __cplusplus

#include <mutex>
#include <queue>
#include <map>
#include <libultraship/libultraship.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Network/Anchor/DummyPlayer.h"
#include "port/Network/Network.h"
#include "port/build.h"

extern "C" {
#include "prop.h"
#include "variables.h"
//#include "z64.h"
}

// void DummyPlayer_Init(Actor* actor, PlayState* play);
// void DummyPlayer_Update(Actor* actor, PlayState* play);
// void DummyPlayer_Draw(Actor* actor, PlayState* play);
// void DummyPlayer_Destroy(Actor* actor, PlayState* play);

typedef struct {
    uint32_t clientId;
    std::string name;
    // Color_RGB8 color;
    std::string clientVersion;
    std::string teamId;
    bool online;
    bool self;
    uint32_t seed;
    bool isSaveLoaded;
    bool isGameComplete;
    GameMap map, prevMap;
    s32 exit, prevExit;

    DummyPlayer* dummy;
} AnchorClient;

typedef struct {
    uint32_t ownerClientId;
    u8 pvpMode;           // 0 = off, 1 = on, 2 = on with friendly fire
    u8 showLocationsMode; // 0 = none, 1 = team, 2 = all
    u8 teleportMode;      // 0 = off, 1 = team, 2 = all
    u8 syncItemsAndFlags; // 0 = off, 1 = on
    bool isRomhack;
    std::string romhackName;
} RoomState;

class Anchor : public Network {
private:
    uint32_t spawningDummyPlayerForClientId = 0;
    bool shouldRefreshActors = false;
    bool justLoadedSave = false;
    bool isHandlingUpdateTeamState = false;
    bool isProcessingIncomingPacket = false;
    std::queue<nlohmann::json> incomingPacketQueue;
    std::mutex incomingPacketQueueMutex;
    std::queue<nlohmann::json> outgoingPacketQueue;
    std::mutex outgoingPacketQueueMutex;
    std::unordered_map<uint32_t, DummyPlayer*> dummies;

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

    void PrepDirectionPayload(nlohmann::json& payload);
    void PrepTransformationPayload(nlohmann::json& payload);
    void PrepAnimStatePayload(nlohmann::json& payload);
    void PrepAnimSubRangePayload(nlohmann::json& payload);

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
    void SendPacket_RequestTeamState();
    void SendPacket_RequestTeleport(u32 clientId);
    void SendPacket_SetCheckStatus(/*RandomizerCheck rc*/);
    void SendPacket_SetFlag(s16 sceneNum, s16 flagType, s16 flag);
    void SendPacket_TeleportTo(u32 clientId);
    void SendPacket_UnsetFlag(s16 sceneNum, s16 flagType, s16 flag);
    void SendPacket_UpdateClientState();
    void SendPacket_UpdateRoomState();
    void SendPacket_UpdateTeamState();
    void SendPacket_VileEatRequest(u8 holeId);
    void SendPacket_VileEatResult(u32 eaterClientId, u8 pieceType, u8 correctType);
    void SendPacket_VileGameState();
    void SendPacket_VileHoleState(u8 holeId, u8 holeState, u8 pieceType, u32 eaterClientId);
    void SendPacket_VileUpdate(const f32 position[3], f32 pitch, f32 yaw, f32 roll, u8 animMode);
    void OnActorDestroyed(Actor* actor);
    void SendToCurrentMapPlayers(nlohmann::json& payload);

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
