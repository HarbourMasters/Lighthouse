#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "functions.h"
#include "macros.h"
#include "variables.h"
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

void Anchor::SendToCurrentMapPlayers(nlohmann::json& payload) {
    for (auto& [clientId, client] : clients) {
        if (client.map == gsworld_getMap() && client.online && client.isSaveLoaded && !client.self) {
            payload["targetClientId"] = clientId;
            SendJsonToRemote(payload);
        }
    }
}

void Anchor::SendPacket_PlayerSubRangeChange(f32 duration, f32 end) {
    if (!IsSaveLoaded() || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = PLAYER_SUBRANGE;
    payload["duration"] = duration;
    payload["end"] = end;

    SendToCurrentMapPlayers(payload);
}

void Anchor::SendPacket_PlayerTransformChange(Transformation tf_id, uint32_t targetClientId) {
    if (!IsSaveLoaded()) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = PLAYER_TRANSFORM;
    payload["id"] = tf_id;
    if (targetClientId != 0) {
        payload["targetClientId"] = targetClientId;
    }
    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_PlayerSubRangeChange(nlohmann::json& payload) {
    uint32_t clientId = payload["clientId"].get<uint32_t>();

    if (clients.contains(clientId)) {
        auto& client = clients[clientId];
        client.dummy->dummyAnim_setEndAndDuration(payload.value("end", 1.0f), payload.value("duration", 0.0f));
    }
}

void Anchor::HandlePacket_PlayerTransformChange(nlohmann::json& payload) {
    uint32_t clientId = payload["clientId"].get<uint32_t>();

    if (clients.contains(clientId)) {
        auto& client = clients[clientId];
        client.dummy->dummy_setTransformation(payload.value("id", TRANSFORM_1_BANJO));
        client.dummy->dummy_updateModel();
    }
}

void Anchor::HandlePacket_PlayerAnimChange(nlohmann::json& payload) {
    uint32_t clientId = payload["clientId"].get<uint32_t>();

    if (clients.contains(clientId)) {
        auto& client = clients[clientId];
        client.dummy->dummyAnim_playForDuration(payload.value("id", ASSET_0_NONE), payload.value("duration", 0.0f),
                                                payload.value("control", ANIMCTRL_ONCE), payload.value("start", 0.0f),
                                                payload.value("subrange_end", 1.0f), payload.value("smooth", false));
    }
}

void Anchor::SendPacket_PlayerAnimChange(AssetID anim_id, f32 duration, AnimControl control, f32 start_position,
                                         f32 subrange_end, bool smooth) {
    if (!IsSaveLoaded() || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = PLAYER_ANIM;
    payload["id"] = anim_id;
    payload["duration"] = duration;
    payload["control"] = control;
    payload["start"] = start_position;
    payload["subrange_end"] = subrange_end;
    payload["smooth"] = smooth;

    SendToCurrentMapPlayers(payload);
}

void Anchor::SendPacket_PlayerAnimReset() {
    if (!IsSaveLoaded() || GetCurrentMapPlayers() == 0) {
        return;
    }
}

bool Anchor::GetCurrentMapPlayers() {
    uint32_t currentPlayerCount = 0;
    for (auto& [clientId, client] : clients) {
        if (client.map == gsworld_getMap() && client.online && client.isSaveLoaded && !client.self) {
            currentPlayerCount++;
        }
    }
    return currentPlayerCount;
}

void Anchor::SendPacket_PlayerUpdate(bool full, uint32_t targetClientId) {
    // A targeted update is a join-time snapshot for one specific client, so it bypasses
    // the "is anyone else in my map?" gate — the recipient was just added and may be the
    // only other player here.
    if (!IsSaveLoaded() || (targetClientId == 0 && GetCurrentMapPlayers() == 0)) {
        return;
    }

    f32 pos[3];
    f32 velocity_min;
    f32 velocity_max;
    f32 duration_min;
    f32 duration_max;
    f32 duration_scale;
    u8 scalable_duration;
    f32 velocity[3];
    f32 animMinDuration;
    f32 animMaxDuration;
    player_getPosition(pos);
    baphysics_get_velocity(velocity);
    baanim_getVelocityMapRanges(&velocity_min, &velocity_max, &duration_min, &duration_max);
    baanim_getDurationRange(&animMinDuration, &animMaxDuration);

    nlohmann::json payload;
    payload["map"] = gsworld_getMap();
    payload["exit"] = gsworld_getExit();
    payload["pos"] = pos;
    payload["rot"] = { pitch_get(), roll_get(), player_getYaw() };
    AnimUpdateType updateType = baanim_getUpdateType();
    payload["updateType"] = updateType;
    payload["velocity"] = velocity;
    payload["direction"] = baModel_getDirection();
    payload["velRanges"] = { velocity_min, velocity_max, duration_min, duration_max };
    payload["durationScale"] = baanim_getDurationScale();
    payload["scalable"] = baanim_isScalableDuration();
    payload["duration"] = anctrl_getDuration(baanim_getAnimCtrlPtr());
    payload["durationRange"] = { animMinDuration, animMaxDuration };
    {
        f32 sub_start, sub_end;
        anctrl_getSubRange(baanim_getAnimCtrlPtr(), &sub_start, &sub_end);
        payload["subrange_end"] = sub_end;
    }
    payload["kazooieVisible"] = func_8029DFBC();     // Kazooie visibility (Kazooie popped out)
    payload["modelSquint"] = func_8029DFA4();        // squint
    payload["modelWink"] = func_8029DFB0();          // wink
    payload["modelMouth1"] = func_8029DFE0();        // mouth
    payload["modelMouth2"] = func_8029DFEC();        // mouth 2
    payload["modelEyeBlendUpper"] = func_8029DFC8(); // eye blend upper
    payload["modelEyeBlendLower"] = func_8029DFD4(); // eye blend lower

    if (full) {
        payload["anim_id"] = anctrl_getIndex(baanim_getAnimCtrlPtr());
        payload["anim_control"] = anctrl_getPlaybackType(baanim_getAnimCtrlPtr());
    }
    payload["type"] = full ? PLAYER_UPDATE_FULL : PLAYER_UPDATE;

    if (targetClientId != 0) {
        payload["targetClientId"] = targetClientId;
        SendJsonToRemote(payload);
    } else {
        SendToCurrentMapPlayers(payload);
    }
}

void Anchor::HandlePacket_PlayerUpdate(nlohmann::json& payload) {
    uint32_t clientId = payload["clientId"].get<uint32_t>();

    if (clients.contains(clientId)) {
        auto& client = clients[clientId];
        if (client.self) {
            return;
        }

        client.map = payload.value("map", MAP_0_UNKNOWN);
        client.exit = payload.value("exit", (s32)0);
        // Self-heal: a missed MAP_LOAD (e.g. it arrived while we were still in the
        // previous map) would otherwise leave this client's dummy unregistered
        // forever, since single-map levels produce no further map loads.
        EvaluateDummyForClient(clientId);
        std::vector<f32> pos = payload["pos"].get<std::vector<f32>>();
        client.dummy->dummy_setPoisition(pos.data());
        std::vector<f32> rot = payload["rot"].get<std::vector<f32>>();
        client.dummy->dummy_setPitch(rot[0]);
        client.dummy->dummy_setRoll(rot[1]);
        {
            PlayerModelDirection dir = (PlayerModelDirection)payload.value("direction", (int)PLAYER_MODEL_DIR_BANJO);
            client.dummy->dummy_setDirection(dir);
            // Kazooie-direction models face 180° opposite to the base player yaw,
            // matching _baModel_updateModelYaw on the sender side. Apply the flip
            // unconditionally every frame so per-frame yaw updates don't undo it.
            if (dir == PLAYER_MODEL_DIR_KAZOOIE) {
                client.dummy->dummy_setYaw(mlNormalizeAngle(rot[2] + 180.0f));
            } else {
                client.dummy->dummy_setYaw(rot[2]);
            }
        }
        client.dummy->dummyAnim_setUpdateType(payload.value("updateType", BAANIM_UPDATE_0_NONE));
        std::vector<f32> velocity = payload["velocity"].get<std::vector<f32>>();
        client.dummy->dummyAnim_setVelocity(velocity.data());
        std::vector<f32> velocityRanges = payload["velRanges"].get<std::vector<f32>>();
        client.dummy->dummyAnim_setVelocityMapRanges(velocityRanges[0], velocityRanges[1], velocityRanges[2],
                                                     velocityRanges[3]);
        client.dummy->dummyAnim_setScalableDuration(payload.value("durationScale", 0.0f),
                                                    payload.value("scalable", false));
        std::vector<f32> durationRange = payload["durationRange"].get<std::vector<f32>>();
        client.dummy->dummyAnim_setDurationRange(durationRange[0], durationRange[1]);

        client.dummy->dummy_getAnimCtrl()->animation_duration = payload.value("duration", 0.0f);
        anctrl_setSubRange(client.dummy->dummy_getAnimCtrl(), 0.0f, payload.value("subrange_end", 1.0f));
        if (payload.value("type", PLAYER_UPDATE) == PLAYER_UPDATE_FULL && payload.contains("anim_id")) {
            client.dummy->dummyAnim_playForDuration((AssetID)payload.value("anim_id", (int)ASSET_0_NONE),
                                                    payload.value("duration", 0.0f),
                                                    (AnimControl)payload.value("anim_control", (int)ANIMCTRL_LOOP),
                                                    0.0f, payload.value("subrange_end", 1.0f), false);
        }
        client.dummy->setModelSubStates(payload.value("kazooieVisible", false), payload.value("modelSquint", false),
                                        payload.value("modelWink", false), payload.value("modelMouth1", false),
                                        payload.value("modelMouth2", false), payload.value("modelEyeBlendUpper", 0.0f),
                                        payload.value("modelEyeBlendLower", 0.0f));
    }
}
