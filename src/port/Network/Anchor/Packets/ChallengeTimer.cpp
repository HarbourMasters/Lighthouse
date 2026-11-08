#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/RomhackConfig.h"

#include "functions.h"

/**
 * CHALLENGE_TIMER
 *
 * A teammate's timed challenge shows its countdown on our hourglass, but only while the hourglass is free and nothing
 * of ours in the map is waiting on it. Our own start replaces it.
 */

static s32 sOwnJiggy = 0;
// The teammate's countdown on our hourglass: who sent it, and when.
static uint32_t sShownClient = 0;
static u32 sShownTime = 0;
// Counted through pauses too, when the global timer stops.
static u32 sFrames = 0;
// Set around our writes for a teammate's countdown, so they don't clear its mark.
static bool sWritingHourglass = false;

constexpr s32 kResendFrames = 30;
constexpr s32 kLeaseFrames = 90;
constexpr s32 kDriftTicks = 15;

// Something of ours in this map acts on the hourglass running out. A map that isn't listed, or any map in a romhack,
// counts as armed.
static bool ChallengeTimer_ReaderArmed() {
    if (port_isRomhack()) {
        return true;
    }
    switch (gsworld_getMap()) {
        case MAP_D_BGS_BUBBLEGLOOP_SWAMP:
            return mapSpecificFlags_get(BGS_SPECIFIC_FLAG_3_WALKWAY_JIGGY_TIMER_RUNNING) ||
                   mapSpecificFlags_get(BGS_SPECIFIC_FLAG_C_MAZE_JIGGY_TIMER_RUNNING);
        case MAP_22_CC_INSIDE_CLANKER:
            // Ring 1 only arms the game; passing it starts the hourglass.
            return maClankerRings_isMinigameActive() >= 2;
        default:
            return true;
    }
}

static bool ChallengeTimer_IsShowing() {
    if (sShownClient != 0 && item_empty(ITEM_6_HOURGLASS)) {
        sShownClient = 0;
    }
    return sShownClient != 0;
}

static void ChallengeTimer_Hide() {
    if (!ChallengeTimer_IsShowing()) {
        return;
    }
    item_set(ITEM_6_HOURGLASS, 0);
    item_set(ITEM_0_HOURGLASS_TIMER, 0);
    sShownClient = 0;
}

void Anchor::SendPacket_ChallengeTimer(s32 ticks) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags || GetCurrentMapPlayers() == 0) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = CHALLENGE_TIMER;
    payload["ticks"] = ticks;
    payload["map"] = (s32)gsworld_getMap();

    SendToCurrentMapPlayers(payload);
}

// ticks 0 ends the countdown.
void Anchor::HandlePacket_ChallengeTimer(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags || payload.value("map", -1) != (s32)gsworld_getMap()) {
        return;
    }

    const uint32_t clientId = payload.value("clientId", 0u);
    const s32 ticks = payload.value("ticks", 0);
    const bool theirs = ChallengeTimer_IsShowing() && sShownClient == clientId;
    if (ticks <= 0) {
        if (theirs) {
            ChallengeTimer_Hide();
        }
        return;
    }
    if (ChallengeTimer_ReaderArmed() || (!theirs && !item_empty(ITEM_6_HOURGLASS))) {
        return;
    }

    sWritingHourglass = true;
    if (!theirs || std::abs(item_getCount(ITEM_0_HOURGLASS_TIMER) - ticks) > kDriftTicks) {
        item_set(ITEM_0_HOURGLASS_TIMER, ticks);
    }
    if (!theirs) {
        item_set(ITEM_6_HOURGLASS, 1);
    }
    sWritingHourglass = false;
    sShownClient = clientId;
    sShownTime = sFrames;
}

void RegisterChallengeTimer_Init() {
    // At boot, with the rest of this file: a teammate's countdown has to go after a disconnect too, and the sends do
    // nothing until a save is loaded in a syncing room.
    COND_HOOK(OnChallengeTimerSet, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        if (item_empty(ITEM_6_HOURGLASS)) {
            return;
        }
        sOwnJiggy = ((OnChallengeTimerSet*)event)->jiggyId;
        Anchor::GetInstance()->SendPacket_ChallengeTimer(item_getCount(ITEM_0_HOURGLASS_TIMER));
    });
    // Any other write to the hourglass, or one that adds time, makes the countdown on it ours.
    COND_VB_SHOULD(VB_ITEM_ADJUST_BY_DIFF, EVENT_PRIORITY_NORMAL, true, {
        const s32 item = va_arg(args, s32);
        const s32* diff = va_arg(args, s32*);
        if (!sWritingHourglass && (item == ITEM_6_HOURGLASS || (item == ITEM_0_HOURGLASS_TIMER && *diff > 0))) {
            sShownClient = 0;
        }
    });
    // A teammate's countdown stays in their map, and goes if they stop sending. Ours ends for them when it stops here
    // or we leave.
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        if (sOwnJiggy != 0) {
            Anchor::GetInstance()->SendPacket_ChallengeTimer(0);
            sOwnJiggy = 0;
        }
        ChallengeTimer_Hide();
    });
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        sFrames++;
        if (ChallengeTimer_IsShowing() && sFrames - sShownTime > kLeaseFrames) {
            ChallengeTimer_Hide();
        }
        if (sOwnJiggy == 0) {
            return;
        }
        if (item_empty(ITEM_6_HOURGLASS) || ChallengeTimer_IsShowing()) {
            Anchor::GetInstance()->SendPacket_ChallengeTimer(0);
            sOwnJiggy = 0;
        } else if (sFrames % kResendFrames == 0) {
            Anchor::GetInstance()->SendPacket_ChallengeTimer(item_getCount(ITEM_0_HOURGLASS_TIMER));
        }
    });
}

static RegisterShipInitFunc initChallengeTimer(RegisterChallengeTimer_Init, {});
