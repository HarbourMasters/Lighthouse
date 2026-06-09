#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/Romhack/RomhackConfig.h"
#include "port/Romhack/RomhackCompat.h"
#include "port/UI/LighthouseGui.hpp"
#include "port/UI/LighthouseModals.h"
//#include "soh/OTRGlobals.h"

extern "C" {
#include "variables.h"
// extern PlayState* gPlayState;
}

/**
 * UPDATE_ROOM_STATE
 */

nlohmann::json Anchor::PrepRoomState() {
    nlohmann::json payload;
    payload["ownerClientId"] = ownClientId;
    bool isGlobalRoom = (std::string("soh-global") == CVarGetString(CVAR_REMOTE_ANCHOR("RoomId"), ""));

    // if (isGlobalRoom) {
    //     // Global room uses hardcoded settings
    //     payload["pvpMode"] = 0;
    //     payload["showLocationsMode"] = 0;
    //     payload["teleportMode"] = 0;
    //     payload["syncItemsAndFlags"] = 0;
    // } else {
    payload["pvpMode"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.PvpMode"), 1);
    payload["showLocationsMode"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.ShowLocationsMode"), 1);
    payload["teleportMode"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.TeleportMode"), 1);
    payload["syncItemsAndFlags"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.SyncItemsAndFlags"), 1);
    payload["isRomHack"] = port_isRomhack();
    payload["romhackName"] = Lighthouse::CurrentRomhackLabel();
    //}

    return payload;
}

void Anchor::SendPacket_UpdateRoomState() {
    nlohmann::json payload;
    payload["type"] = UPDATE_ROOM_STATE;
    payload["state"] = PrepRoomState();

    Network::SendJsonToRemote(payload);
}

void Anchor::HandlePacket_UpdateRoomState(nlohmann::json& payload) {
    if (!payload.contains("state")) {
        return;
    }
    roomState.isRomhack = payload["state"]["isRomHack"].get<bool>();
    roomState.romhackName = payload["state"]["romhackName"].get<std::string>();
    const std::string localLabel = Lighthouse::CurrentRomhackLabel();
    if (roomState.romhackName != localLabel) {
        Disable();
        std::string msg = "There's a romhack mismatch between your client and the server:\n\n";
        msg += Lighthouse::DescribeRomhackMismatch(port_isRomhack(), localLabel, roomState.isRomhack,
                                                   roomState.romhackName);
        msg += "\n\nAnchor has been disabled. Please enable or disable the appropriate\n"
               "mod(s) in the Mod Menu and reconnect, or enter a new room name.";
        LighthouseGui::RegisterPopup("Incompatible Romhack State", msg);
    }

    roomState.ownerClientId = payload["state"]["ownerClientId"].get<uint32_t>();
    roomState.pvpMode = payload["state"]["pvpMode"].get<u8>();
    roomState.showLocationsMode = payload["state"]["showLocationsMode"].get<u8>();
    roomState.teleportMode = payload["state"]["teleportMode"].get<u8>();
    roomState.syncItemsAndFlags = payload["state"]["syncItemsAndFlags"].get<u8>();
}
