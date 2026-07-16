#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "macros.h"
#include "functions.h"
#include "variables.h"

/**
 * PLAYER_SFX
 *
 * Sound effects, only sent to other clients in the same scene as the player
 */

void Anchor::SendPacket_PlayerSfx(u16 sfxId) {
    if (!IsSaveLoaded()) {
        return;
    }

    nlohmann::json payload;

    payload["type"] = PLAYER_SFX;
    payload["sfxId"] = sfxId;
    payload["quiet"] = true;
}

void Anchor::HandlePacket_PlayerSfx(nlohmann::json& payload) {
}
