#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/Romhack/RomhackConfig.h"
#include "port/Romhack/RomhackCompat.h"
#include "port/Rando/Rando.h"
#include "port/UI/LighthouseGui.hpp"
#include "port/UI/LighthouseModals.h"

#include "variables.h"

/**
 * UPDATE_ROOM_STATE
 */

nlohmann::json Anchor::PrepRoomState() {
    nlohmann::json payload;
    payload["ownerClientId"] = ownClientId;

    if (IsGlobalRoom()) {
        payload["pvpMode"] = 0;
        payload["showLocationsMode"] = 0;
        payload["teleportMode"] = 0;
        payload["syncItemsAndFlags"] = 0;
        payload["shareConsumables"] = 0;
        payload["isRomHack"] = false;
        payload["romhackName"] = "";
        payload["isRando"] = false;
        payload["seed"] = 0;
        return payload;
    }

    payload["pvpMode"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.PvpMode"), 1);
    payload["showLocationsMode"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.ShowLocationsMode"), 1);
    payload["teleportMode"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.TeleportMode"), 1);
    payload["syncItemsAndFlags"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.SyncItemsAndFlags"), 1);
    payload["shareConsumables"] = CVarGetInteger(CVAR_REMOTE_ANCHOR("RoomSettings.ShareConsumables"), 0);
    payload["isRomHack"] = port_isRomhack();
    payload["romhackName"] = Lighthouse::CurrentRomhackLabel();
    payload["isRando"] = (bool)IS_RANDO;
    payload["seed"] = (int32_t)(IS_RANDO ? RANDO_SEED : 0);

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
        lastWarnedRomhackLabel.clear();
    }

    roomState.ownerClientId = payload["state"]["ownerClientId"].get<uint32_t>();
    roomState.pvpMode = payload["state"]["pvpMode"].get<u8>();
    roomState.showLocationsMode = payload["state"]["showLocationsMode"].get<u8>();
    roomState.teleportMode = payload["state"]["teleportMode"].get<u8>();
    roomState.syncItemsAndFlags = payload["state"]["syncItemsAndFlags"].get<u8>();
    roomState.shareConsumables = payload["state"].value("shareConsumables", (u8)0);
    roomState.isRando = payload["state"].value("isRando", false);
    roomState.seed = payload["state"].value("seed", (int32_t)0);

    CheckRandoRoomCompatibility();
}

void Anchor::CheckRandoRoomCompatibility() {
    if (IsGlobalRoom() || !isConnected || !IsSaveLoaded()) {
        return;
    }

    const bool localRando = IS_RANDO;
    const int32_t localSeed = localRando ? (int32_t)RANDO_SEED : 0;

    std::string msg;
    if (roomState.isRando && !localRando) {
        msg = "This is a randomizer room, but the save you loaded is not a randomizer file.\n\n"
              "Items, flags, and checks will not line up - you'll receive progress you can't use\n"
              "and may corrupt the shared session. Load the matching randomizer file, or\n"
              "disconnect before continuing.";
    } else if (!roomState.isRando && localRando) {
        msg = "You loaded a randomizer file, but this room is running a vanilla game.\n\n"
              "Shuffled checks won't match what teammates send, so progress will desync.\n"
              "Reconnect to a randomizer room, or load a vanilla file to match this one.";
    } else if (roomState.isRando && localRando && roomState.seed != localSeed) {
        msg = "Seed mismatch: your randomizer seed differs from the room's.\n\n"
              "Checks are shuffled differently per seed, so syncing will scatter items to the\n"
              "wrong locations. Everyone on a team must generate/load from the same seed.\n";
        msg += "\n    Your seed:  " + std::to_string(localSeed);
        msg += "\n    Room seed:  " + std::to_string(roomState.seed);
    }

    if (msg.empty()) {
        lastWarnedRandoState.clear();
        return;
    }

    std::string sig = std::to_string(roomState.isRando) + ":" + std::to_string(roomState.seed) + "|" +
                      std::to_string(localRando) + ":" + std::to_string(localSeed);
    if (sig == lastWarnedRandoState) {
        return;
    }
    lastWarnedRandoState = sig;
    LighthouseGui::RegisterPopup("Randomizer Mismatch Warning", msg);
}
