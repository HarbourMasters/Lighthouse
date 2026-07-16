#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
void func_8031D04C(enum map_e arg0, s32 exit_id);
// ba_lookdir.c: instantaneously reposition the player within the current map. Sets both the
// current AND previous player position (so no movement delta is produced) and resettles ground
// collision + camera — the same warp the in-map cauldrons/parade use. Contrast player_setPosition,
// which only moves the current position: the physics step then sweeps the player from the stale
// previous position to the new one, dragging them through level geometry at high speed.
void func_8028F85C(f32 arg0[3]);
}

// Exit id the game reserves for "spawn at an explicit position" (see func_8028E4B0);
// player_setWarpDestination arms it with the destination before the transition starts.
#define EXIT_WARP_DESTINATION 0x63

/**
 * TELEPORT_TO
 *
 * See REQUEST_TELEPORT for more information, this is the second part of the process.
 * Carries the sender's live map/position/yaw so the requester's map transition opens
 * directly at the sender instead of at a map entrance.
 */

void Anchor::SendPacket_TeleportTo(uint32_t clientId) {
    if (!IsSaveLoaded()) {
        return;
    }

    f32 pos[3];
    player_getPosition(pos);

    nlohmann::json payload;
    payload["type"] = TELEPORT_TO;
    payload["targetClientId"] = clientId;
    payload["map"] = gsworld_getMap();
    payload["exit"] = gsworld_getExit();
    payload["pos"] = pos;
    payload["yaw"] = player_getYaw();

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_TeleportTo(nlohmann::json& payload) {
    if (!IsSaveLoaded()) {
        return;
    }

    GameMap map = payload.at("map").get<GameMap>();
    std::vector<f32> pos = payload.at("pos").get<std::vector<f32>>();
    f32 yaw = payload.at("yaw").get<f32>();

    // Already in their map: place the player directly, no transition needed. Packets are
    // handled inside GameFrameUpdate, so this runs on the game thread. Set the facing first so the
    // camera resettle inside func_8028F85C uses it, then do the instantaneous, sweep-free reposition.
    if (map == (GameMap)gsworld_getMap()) {
        yaw_set(yaw);
        yaw_setIdeal(yaw);
        yaw_applyIdeal();
        func_8028F85C(pos.data());
        return;
    }

    player_setWarpDestination(pos.data(), yaw, payload.value("exit", 0));
    func_8031D04C(map, EXIT_WARP_DESTINATION);
}
