#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Network/Anchor/Authority.h"

#include "functions.h"
extern "C" {
void* func_8038A26C(s32 mesh);
void func_8038A2B8(void);
void func_8038A82C(void* tile);
void func_8038A964(void);
}

/**
 * TUMBLAR_UPDATE
 *
 * The player on Tumblar runs his game for everyone in the shed. Every frame they send his position, which the others'
 * Tumblar follows, and each tile he steps on ("tile"), which the others' game replays. When the rider is off him
 * mid-run, whoever gets on next carries the run on with the time and letters left.
 */

// Where the teammate running the game has him.
static f32 sTumblarX = 0.0f;
static f32 sTumblarZ = 0.0f;
static bool sHasTumblar = false;

static s32 sTumblarStandTime = 0;
// The game's state (MMM_D_8038C510.unk8), from VB_TUMBLAR_GAME_START.
static s32 sShedState = 0;
// The last tile stepped on, ours or a teammate's.
static s32 sLastTile = 0;
// The last rider's tile once we take him over. It stays stepped until he's on another (their MMM_D_8038C510.unk4).
static s32 sHandoffTile = 0;
static bool sCarryingOn = false;
static bool sReplayingTile = false;

bool TumblarUpdate_IsGameRunning() {
    return sShedState == 3;
}

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

void Anchor::SendPacket_TumblarTile(s32 tile) {
    if (!IsSaveLoaded() || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = TUMBLAR_UPDATE;
    payload["tile"] = tile;

    SendToCurrentMapPlayers(payload);
}

// Tile 0 starts a new run, which resets every tile and letter.
void Anchor::HandlePacket_TumblarUpdate(nlohmann::json& payload) {
    if (gsworld_getMap() != MAP_24_MMM_TUMBLARS_SHED) {
        return;
    }
    if (payload.contains("tile")) {
        sLastTile = payload.value("tile", 0);
        if (sLastTile == 0) {
            func_8038A2B8();
            func_8038A964();
            return;
        }
        void* tile = func_8038A26C(sLastTile);
        if (tile != NULL) {
            sReplayingTile = true;
            func_8038A82C(tile);
            sReplayingTile = false;
        }
        return;
    }
    sTumblarX = payload.value("x", 0.0f);
    sTumblarZ = payload.value("z", 0.0f);
    sHasTumblar = true;
}

void Anchor_RegisterTumblarHooks(bool connected) {
    // One player at a time rides Tumblar and runs his game: standing on him claims it while the game is ready or
    // running, and a start waits for every teammate in the shed to accept. A loss, or 15 frames off him before the
    // start or mid-run, frees him for anyone; a win keeps him until we leave, since our pickup ends a teammate's game.
    COND_VB_SHOULD(VB_TUMBLAR_RIDE, EVENT_PRIORITY_NORMAL, connected, {
        if (!*should) {
            return;
        }
        if (!NetAuthority_IsClaimed(NET_ACTIVITY_MMM_TUMBLAR) && (sShedState == 1 || sShedState == 3)) {
            NetAuthority_Claim(NET_ACTIVITY_MMM_TUMBLAR);
            if (sShedState == 3) {
                sHandoffTile = sLastTile;
            }
        }
        // While a teammate holds him, we can't ride him.
        if (!NetAuthority_IsSelf(NET_ACTIVITY_MMM_TUMBLAR)) {
            *should = false;
            return;
        }
        sTumblarStandTime = globalTimer_getTime();
    });
    COND_VB_SHOULD(VB_TUMBLAR_GAME_START, EVENT_PRIORITY_NORMAL, connected, {
        sShedState = va_arg(args, s32);
        if (*should) {
            if (!NetAuthority_IsConfirmed(NET_ACTIVITY_MMM_TUMBLAR)) {
                *should = false;
            } else if (ChallengeTimer_IsShowing()) {
                // A teammate's run is still on our hourglass, so ours carries it on, without the intro.
                sCarryingOn = true;
                sHandoffTile = sLastTile;
                levelSpecificFlags_set(LEVEL_FLAG_2F_MMM_UNKNOWN, true);
            } else {
                // A teammate's earlier run can have left tiles and letters lit here.
                func_8038A2B8();
                func_8038A964();
                sLastTile = 0;
                Anchor::GetInstance()->SendPacket_TumblarTile(0);
            }
        } else if (NetAuthority_IsClaimed(NET_ACTIVITY_MMM_TUMBLAR) && NetAuthority_IsSelf(NET_ACTIVITY_MMM_TUMBLAR) &&
                   (sShedState == 4 || ((sShedState == 1 || sShedState == 3) &&
                                        globalTimer_getTime() - sTumblarStandTime > 15))) { // frames
            NetAuthority_Release(NET_ACTIVITY_MMM_TUMBLAR);
        }
    });
    COND_VB_SHOULD(VB_TUMBLAR_HOURGLASS_RESET, EVENT_PRIORITY_NORMAL, connected, {
        if (sCarryingOn) {
            sCarryingOn = false;
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
    // Only a confirmed rider's steps count, and each goes to the others' game.
    COND_VB_SHOULD(VB_TUMBLAR_TILE_STEP, EVENT_PRIORITY_NORMAL, connected, {
        const s32 tile = va_arg(args, s32);
        if (tile != sHandoffTile) {
            sHandoffTile = 0;
        }
        if (!*should) {
            return;
        }
        if (!NetAuthority_IsConfirmed(NET_ACTIVITY_MMM_TUMBLAR) || tile == sHandoffTile) {
            *should = false;
            return;
        }
        sLastTile = tile;
        Anchor::GetInstance()->SendPacket_TumblarTile(tile);
    });
    // A teammate's X tile knocks off only them.
    COND_VB_SHOULD(VB_TUMBLAR_KNOCK_OFF, EVENT_PRIORITY_NORMAL, connected, {
        if (sReplayingTile) {
            *should = false;
        }
    });
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        sHasTumblar = false;
        sShedState = 0;
        sLastTile = 0;
        sHandoffTile = 0;
    });
}
