#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Network/Anchor/Authority.h"

#include "functions.h"

/**
 * TUMBLAR_UPDATE
 *
 * Tumblar's position, sent every frame by the player running his game to the others in the shed. Their Tumblar
 * follows it, and nobody else can ride him.
 */

// Where the teammate running the game has him.
static f32 sTumblarX = 0.0f;
static f32 sTumblarZ = 0.0f;
static bool sHasTumblar = false;

static s32 sTumblarStandTime = 0;

void Anchor::SendPacket_TumblarUpdate(const f32 position[3]) {
    if (!IsSaveLoaded() || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = TUMBLAR_UPDATE;
    payload["x"] = position[0];
    payload["z"] = position[2];
    payload["quiet"] = true;

    SendToCurrentMapPlayers(payload);
}

void Anchor::HandlePacket_TumblarUpdate(nlohmann::json& payload) {
    if (gsworld_getMap() != MAP_24_MMM_TUMBLARS_SHED) {
        return;
    }
    sTumblarX = payload.value("x", 0.0f);
    sTumblarZ = payload.value("z", 0.0f);
    sHasTumblar = true;
}

void Anchor_RegisterTumblarHooks(bool connected) {
    // One player runs Tumblar's game at a time: standing on him claims it and waits for every teammate in the shed to
    // accept. A loss or stepping off before the start releases it; a win keeps it until we leave, since our pickup
    // ends a teammate's game.
    COND_VB_SHOULD(VB_TUMBLAR_GAME_START, EVENT_PRIORITY_NORMAL, connected, {
        const s32 state = va_arg(args, s32);
        if (*should) {
            if (!NetAuthority_IsClaimed(NET_ACTIVITY_MMM_TUMBLAR)) {
                NetAuthority_Claim(NET_ACTIVITY_MMM_TUMBLAR);
            }
            sTumblarStandTime = globalTimer_getTime();
            if (!NetAuthority_IsConfirmed(NET_ACTIVITY_MMM_TUMBLAR)) {
                *should = false;
            }
        } else if (NetAuthority_IsClaimed(NET_ACTIVITY_MMM_TUMBLAR) && NetAuthority_IsSelf(NET_ACTIVITY_MMM_TUMBLAR) &&
                   (state == 4 || (state == 1 && globalTimer_getTime() - sTumblarStandTime > 15))) { // frames
            NetAuthority_Release(NET_ACTIVITY_MMM_TUMBLAR);
        }
    });

    // While a teammate holds his game, Tumblar is theirs: we can't ride him, and ours goes where theirs is.
    COND_VB_SHOULD(VB_TUMBLAR_RIDE, EVENT_PRIORITY_NORMAL, connected, {
        if (!NetAuthority_IsSelf(NET_ACTIVITY_MMM_TUMBLAR)) {
            *should = false;
        }
    });
    // The position sent is the one he ended last frame on.
    COND_VB_SHOULD(VB_TUMBLAR_MOVE, EVENT_PRIORITY_NORMAL, connected, {
        f32* position = va_arg(args, f32*);
        if (NetAuthority_IsSelf(NET_ACTIVITY_MMM_TUMBLAR)) {
            sHasTumblar = false;
            if (NetAuthority_IsClaimed(NET_ACTIVITY_MMM_TUMBLAR)) {
                Anchor::GetInstance()->SendPacket_TumblarUpdate(position);
            }
            return;
        }
        if (sHasTumblar) {
            position[0] = sTumblarX;
            position[2] = sTumblarZ;
        }
        *should = false;
    });
}
