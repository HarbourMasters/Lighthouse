#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
#include "macros.h"
#include "variables.h"
//extern PlayState* gPlayState;
}

/**
 * PLAYER_UPDATE
 *
 * Contains real-time data necessary to update other clients in the same scene as the player
 *
 * Sent every frame to other clients within the same scene
 *
 * Note: This packet is sent _a lot_, so please do not include any unnecessary data in it
 */

void Anchor::SendPacket_PlayerSubRangeChange(f32 duration, f32 end) {

}

void Anchor::SendPacket_PlayerAnimChange(AssetID anim_id, f32 duration, AnimControl control, f32 start_position, bool smooth) {

}

void Anchor::SendPacket_PlayerUpdate() {
    if (!IsSaveLoaded()) {
        return;
    }

    uint32_t currentPlayerCount = 0;
    for (auto& [clientId, client] : clients) {
        if (client.map == gsworld_getMap() && client.online && client.isSaveLoaded && !client.self) {
            currentPlayerCount++;
        }
    }
    if (currentPlayerCount == 0) {
        return;
    }

    //Player* player = GET_PLAYER(gPlayState);
    nlohmann::json payload;

    payload["type"] = PLAYER_UPDATE;
    payload["map"] = gsworld_getMap();
    payload["exit"] = gsworld_getExit();
    f32 pos[3];
    player_getPosition(pos);
    payload["pos"] = pos;
    payload["rot"] = { pitch_get(), roll_get(), player_getYaw() };
    //std::vector<int> jointArray;
    //for (size_t i = 0; i < 24; i++) {
    //    Vec3s joint = player->skelAnime.jointTable[i];
    //    jointArray.push_back(joint.x);
    //    jointArray.push_back(joint.y);
    //    jointArray.push_back(joint.z);
    //}
    //payload["prevTransl"] = player->skelAnime.prevTransl;
    //payload["movementFlags"] = player->skelAnime.movementFlags;
    //payload["jointTable"] = jointArray;
    //payload["upperLimbRot"] = player->upperLimbRot;
    //payload["currentBoots"] = player->currentBoots;
    //payload["currentShield"] = player->currentShield;
    //payload["currentTunic"] = player->currentTunic;
    //payload["stateFlags1"] = player->stateFlags1;
    //payload["stateFlags2"] = player->stateFlags2 & ~PLAYER_STATE2_DISABLE_DRAW;
    //payload["buttonItem0"] = gSaveContext.equips.buttonItems[0];
    //payload["itemAction"] = player->itemAction;
    //payload["heldItemAction"] = player->heldItemAction;
    //payload["modelGroup"] = player->modelGroup;
    //payload["invincibilityTimer"] = player->invincibilityTimer;
    //payload["unk_862"] = player->unk_862;
    //payload["unk_85C"] = player->unk_85C;
    //payload["actionVar1"] = player->av1.actionVar1;
    //payload["quiet"] = true;

    //for (auto& [clientId, client] : clients) {
    //    if (client.sceneNum == gPlayState->sceneNum && client.online && client.isSaveLoaded && !client.self) {
    //        payload["targetClientId"] = clientId;
    //        SendJsonToRemote(payload);
    //    }
    //}
}

void Anchor::HandlePacket_PlayerUpdate(nlohmann::json payload) {
    uint32_t clientId = payload["clientId"].get<uint32_t>();

    if (clients.contains(clientId)) {
        auto& client = clients[clientId];

        //if (client.dummy->dummy_getTransformation() != payload.value("linkAge", TRANSFORM_1_BANJO)) {
        //    shouldRefreshActors = true;
        //}

        client.map = payload.value("map", (GameMap)-1);
        client.exit = payload.value("exit", (s32)0);
    }
}
