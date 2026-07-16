// Backports Banjo-Tooie's Honeyback reward. Once all 24 empty honeycombs are
// collected, health gradually refills one honeycomb at a time, starting a short
// while after the last damage was taken.

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

#include "enums.h"
#include "functions.h"
#include "gc/gctransition.h"

#define CVAR_NAME CVAR_ENHANCEMENT("Backports.Honeyback")

// GameFrameUpdate fires once per 30 Hz game tick.
static const int TICKS_PER_SECOND = 30;
static const int ALL_HONEYCOMBS = 24;
// Matches Tooie's HONEYBACK cheat: ~2s after the last damage, then 1 health about every 3s.
static const int DAMAGE_DELAY_TICKS = 2 * TICKS_PER_SECOND;
static const int REGEN_PERIOD_TICKS = 3 * TICKS_PER_SECOND;

static int sPrevHealth = -1;
static int sDamageDelay = 0;
static int sRegenTimer = 0;

void RegisterHoneyback_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NAME, 0), [](IEvent* event) {
        if (getGameMode() != GAME_MODE_3_NORMAL || gctransition_active() || gcdialog_hasCurrentTextId() ||
            honeycombscore_get_total() < ALL_HONEYCOMBS) {
            sPrevHealth = -1;
            sDamageDelay = 0;
            sRegenTimer = 0;
            return;
        }

        const int cur = item_getCount(ITEM_14_HEALTH);
        const int max = item_getCount(ITEM_15_HEALTH_TOTAL);

        // Re-arm tracking on the first eligible tick (or after a reload).
        if (sPrevHealth < 0) {
            sPrevHealth = cur;
        }

        // Took damage since last tick: hold off regen for a few seconds.
        if (cur < sPrevHealth) {
            sDamageDelay = DAMAGE_DELAY_TICKS;
            sRegenTimer = 0;
        }
        sPrevHealth = cur;

        // Don't revive the player, and do nothing when already full.
        if (cur <= 0 || cur >= max) {
            sRegenTimer = 0;
            return;
        }

        if (sDamageDelay > 0) {
            sDamageDelay--;
            return;
        }

        if (++sRegenTimer >= REGEN_PERIOD_TICKS) {
            sRegenTimer = 0;
            item_adjustByDiffWithHud(ITEM_14_HEALTH, 1);
            coMusicPlayer_playMusic(COMUSIC_16_HONEYCOMB_COLLECTED, 28000);
            sPrevHealth = item_getCount(ITEM_14_HEALTH);
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterHoneyback_Init, { CVAR_NAME });
