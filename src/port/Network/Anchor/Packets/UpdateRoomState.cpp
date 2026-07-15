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

    if (IsGlobalRoom()) {
        // The global room is display-only: everyone just sees each other's dummies. Force every
        // gameplay setting off. This state is also what the HANDSHAKE ships (Handshake.cpp), so the
        // very first client to connect creates the server-side room with syncing disabled.
        payload["pvpMode"] = 0;
        payload["showLocationsMode"] = 0;
        payload["teleportMode"] = 0;
        payload["syncItemsAndFlags"] = 0;
        payload["shareConsumables"] = 0;
        payload["isRomHack"] = false;
        payload["romhackName"] = "";
        return payload;
    }

    payload["pvpMode"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.PvpMode"), 1);
    payload["showLocationsMode"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.ShowLocationsMode"), 1);
    payload["teleportMode"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.TeleportMode"), 1);
    payload["syncItemsAndFlags"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.SyncItemsAndFlags"), 1);
    payload["shareConsumables"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.ShareConsumables"), 0);
    payload["isRomHack"] = port_isRomhack();
    payload["romhackName"] = Lighthouse::CurrentRomhackLabel();

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

    // The global room never syncs anything, so ignore whatever room state is on the wire and pin
    // every gameplay setting off locally. Romhack identity is irrelevant with no syncing, so skip
    // the mismatch warning too (players run all sorts of mods in the public room).
    if (IsGlobalRoom()) {
        roomState.ownerClientId = payload["state"].value("ownerClientId", (uint32_t)0);
        roomState.pvpMode = 0;
        roomState.showLocationsMode = 0;
        roomState.teleportMode = 0;
        roomState.syncItemsAndFlags = 0;
        roomState.shareConsumables = 0;
        roomState.isRomhack = false;
        roomState.romhackName.clear();
        return;
    }

    roomState.isRomhack = payload["state"]["isRomHack"].get<bool>();
    roomState.romhackName = payload["state"]["romhackName"].get<std::string>();
    const std::string localLabel = Lighthouse::CurrentRomhackLabel();
    // Romhack mismatches are no longer a hard gate — warn the player once per
    // distinct mismatch and let them decide whether to keep playing.
    if (roomState.romhackName != localLabel) {
        if (roomState.romhackName != lastWarnedRomhackLabel) {
            lastWarnedRomhackLabel = roomState.romhackName;
            std::string msg = "There's a romhack mismatch between your client and the server:\n\n";
            msg += Lighthouse::DescribeRomhackMismatch(port_isRomhack(), localLabel, roomState.isRomhack,
                                                       roomState.romhackName);
            msg += "\n\nYou can still play together, but items, flags, and custom content\n"
                   "may not sync correctly. To avoid desyncs, enable or disable the\n"
                   "appropriate mod(s) in the Mod Menu so both sides match, then reconnect.";
            LighthouseGui::RegisterPopup("Romhack Mismatch Warning", msg);
        }
    } else {
        // Back in agreement — clear the guard so a later mismatch warns again.
        lastWarnedRomhackLabel.clear();
    }

    roomState.ownerClientId = payload["state"]["ownerClientId"].get<uint32_t>();
    roomState.pvpMode = payload["state"]["pvpMode"].get<u8>();
    roomState.showLocationsMode = payload["state"]["showLocationsMode"].get<u8>();
    roomState.teleportMode = payload["state"]["teleportMode"].get<u8>();
    roomState.syncItemsAndFlags = payload["state"]["syncItemsAndFlags"].get<u8>();
    roomState.shareConsumables = payload["state"].value("shareConsumables", (u8)0);
}
