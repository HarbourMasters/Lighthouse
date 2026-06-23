// Jinjo Collection Retention
//
// Vanilla resets ITEM_12_JINJOS on every level entry (itemscore_levelReset), so jinjos
// respawn each visit and must all be collected in one go to earn the jiggy. This persists
// the per-level set of collected jinjo colors so individually collected jinjos stay
// collected across visits.
//
// How it works:
//   - Saving is ALWAYS ON: collecting a jinjo records its color bit for the current
//     level (same 5-bit layout as ITEM_12_JINJOS).
//   - Application is behind an enhancement toggle (default off): when on, ITEM_12_JINJOS
//     is seeded on level load from the saved bits, and already collected jinjos are not
//     respawned. Collecting the remaining jinjos then completes the vanilla 0x1f mask and
//     spawns the jiggy as usual.
//
// Stranded-jiggy guard: if all 5 of a level's jinjos are recorded but that level's jinjo
// jiggy has not been obtained (player got all 5 but skipped the jiggy), retention is not
// applied for that level -- the jinjos respawn so the jiggy can still be earned.
//
#include <libultraship/bridge.h>
#include <libultraship/bridge/consolevariablebridge.h>

#include "port/UI/cvar_prefixes.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/JinjoRetention/JinjoRetention.h"
#include "port/Rando/Rando.h"

extern "C" {
#include "enums.h"
#include "actor.h"
#include "prop.h"
#include "functions.h"
}

#define CVAR_JINJO_RETENTION CVAR_ENHANCEMENT("Gameplay.JinjoRetention")
// Anchor forces retention on while connected, separate from the user's CVar so their setting is
// preserved. CVAR_VALUE / applyEnabled() — and thus every COND_HOOK gate — respect it.
static bool sForcedByAnchor = false;
#define CVAR_VALUE (CVarGetInteger(CVAR_JINJO_RETENTION, 0) || sForcedByAnchor)

constexpr u8 kAllJinjos = 0x1F; // all five color bits collected

// gameFile_saveData index of the live save slot for the current game, or -1.
int32_t activeSlot() {
    if (selectedFileNum == DEFAULT_FILE_NUM || selectedFileNum < 0 || selectedFileNum >= 4) {
        return -1;
    }
    return (int32_t)selectedFileNum;
}

// Don't run during demos, Bottles bonus games, or rando files.
bool systemActive() {
    int32_t slot = activeSlot();
    if (slot < 0 || gameFile_saveData[slot].shipSaveData.fileType == FILE_TYPE_SAVE_RANDO) {
        return false;
    }
    switch (getGameMode()) {
        case GAME_MODE_7_ATTRACT_DEMO:
        case GAME_MODE_8_BOTTLES_BONUS:
        case GAME_MODE_9_BANJO_AND_KAZOOIE:
        case GAME_MODE_A_SNS_PICTURE:
            return false;
        default:
            return true;
    }
}

bool applyEnabled() {
    return CVAR_VALUE;
}

// Anchor sets this on connect/disconnect. Re-runs the init so the COND_HOOK gates re-evaluate.
extern "C" void port_jinjoRetention_setForced(int32_t forced) {
    sForcedByAnchor = forced != 0;
    ShipInit::Init(CVAR_JINJO_RETENTION);
}

JinjoRetentionSaveData* store() {
    int32_t slot = activeSlot();
    return slot >= 0 ? &gameFile_saveData[slot].shipSaveData.jinjoRetention : nullptr;
}

extern "C" void port_jinjoRetention_getSizeAndPtr(int32_t* size, uint8_t** addr) {
    JinjoRetentionSaveData* s = store();
    if (s == nullptr) {
        *size = 0;
        *addr = nullptr;
        return;
    }
    *size = (int32_t)sizeof(JinjoRetentionSaveData);
    *addr = (uint8_t*)s;
}

bool levelInRange(int32_t level) {
    return level > 0 && level < JINJO_RETENTION_LEVEL_SLOTS;
}

u8 collectedBits(int32_t level) {
    JinjoRetentionSaveData* s = store();
    return (s != nullptr && levelInRange(level)) ? s->collected[level] : 0;
}

u8 jinjoBitFromMarker(int32_t markerId) {
    switch (markerId) {
        case MARKER_5A_JINJO_BLUE:
        case MARKER_5B_JINJO_GREEN:
        case MARKER_5C_JINJO_ORANGE:
        case MARKER_5D_JINJO_PINK:
        case MARKER_5E_JINJO_YELLOW:
            return (u8)(1 << ((markerId + 6) & 0x1F));
        default:
            return 0;
    }
}

u8 jinjoBitFromActor(int32_t actorId) {
    switch (actorId) {
        case ACTOR_60_JINJO_BLUE:
            return 1 << 0;
        case ACTOR_62_JINJO_GREEN:
            return 1 << 1;
        case ACTOR_5F_JINJO_ORANGE:
            return 1 << 2;
        case ACTOR_61_JINJO_PINK:
            return 1 << 3;
        case ACTOR_5E_JINJO_YELLOW:
            return 1 << 4;
        default:
            return 0;
    }
}

int32_t jinjoActorFromBit(u8 bit) {
    switch (bit) {
        case 1 << 0: return ACTOR_60_JINJO_BLUE;
        case 1 << 1: return ACTOR_62_JINJO_GREEN;
        case 1 << 2: return ACTOR_5F_JINJO_ORANGE;
        case 1 << 3: return ACTOR_61_JINJO_PINK;
        case 1 << 4: return ACTOR_5E_JINJO_YELLOW;
        default: return 0;
    }
}

// Apply a teammate's jinjo pickup: record the colour bit for the collector's level; if we're
// in the same map, update the HUD count and despawn our copy of that jinjo.
extern "C" void port_jinjoRetention_applyRemoteCollect(int32_t map, int32_t bit, int32_t sameMap) {
    int32_t level = map_getLevel((enum map_e)map);
    JinjoRetentionSaveData* s = store();
    if (s != nullptr && levelInRange(level)) {
        s->collected[level] |= (u8)bit;
    }
    if (sameMap) {
        item_set(ITEM_12_JINJOS, collectedBits(level));
        int32_t actorId = jinjoActorFromBit((u8)bit);
        if (actorId != 0) {
            Actor* a = actorArray_findActorFromActorId((enum actor_e)actorId);
            if (a != nullptr && a->marker != nullptr) {
                marker_despawn(a->marker);
            }
        }
    }
}

// Called from the jinjo's actual pickup (__chJinjo_802CDBA8), not broad-phase collision, so
// retention records and the network broadcast fire only on a real collection.
extern "C" void port_jinjoRetention_onLocalJinjoCollected(int32_t markerId) {
    if (!applyEnabled() || !systemActive()) {
        return;
    }
    u8 bit = jinjoBitFromMarker(markerId);
    if (bit == 0) {
        return;
    }
    int32_t level = level_get();
    JinjoRetentionSaveData* s = store();
    bool wasSet = (s != nullptr) && levelInRange(level) && ((s->collected[level] & bit) != 0);
    if (s != nullptr && levelInRange(level)) {
        s->collected[level] |= bit;
    }
    if (!wasSet) {
        CALL_EVENT(OnCollectibleCollected, ANCHOR_COLLECTIBLE_JINJO, bit);
    }
}

// Whether retention should seed/suppress jinjos for this level. False when retention is
// off, and false in the stranded-jiggy case so the jinjos respawn and the jiggy is still
// earnable. Seeding and suppression both gate on this so ITEM_12_JINJOS stays consistent.
bool retentionActiveForLevel(int32_t level) {
    if (!applyEnabled() || !levelInRange(level)) {
        return false;
    }
    if (collectedBits(level) == kAllJinjos && !jiggyscore_isCollected((enum jiggy_e)(10 * level - 9))) {
        return false;
    }
    return true;
}

void RegisterJinjoRetention_Init() {
    // Collection is recorded in port_jinjoRetention_onLocalJinjoCollected, called from the
    // jinjo's real pickup — not here on broad-phase OnActorCollision (which fires on mere
    // proximity, before/without an actual collection).

    // Seed ITEM_12_JINJOS from the saved bits so prior progress carries across visits and
    // the HUD reflects it. Mirrors note retention's OnSetJiggyList seeding.
    COND_HOOK(OnSetJiggyList, EVENT_PRIORITY_NORMAL, CVAR_VALUE, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;
        if (!systemActive() || !retentionActiveForLevel(ev->levelId)) {
            return;
        }
        u8 bits = collectedBits(ev->levelId);
        if (bits != 0) {
            item_set(ITEM_12_JINJOS, bits);
        }
    });

    // Suppress respawning already-collected jinjos. OnActorSpawn is cancellable; returning
    // a null result with Cancelled means no actor spawns.
    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, CVAR_VALUE, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;
        if (!systemActive()) {
            return;
        }
        u8 bit = jinjoBitFromActor(ev->actorId);
        if (bit == 0) {
            return;
        }
        int32_t level = level_get();
        if (!retentionActiveForLevel(level) || !(collectedBits(level) & bit)) {
            return;
        }
        ev->result = nullptr;
        event->Cancelled = true;
    });

    // The engine's save double-buffers into a scratch slot via bcopy, which doesn't carry
    // our bits. Sync the live slot's jinjoRetention into the buffer about to be serialized.
    COND_HOOK(OnSaveFileSave, EVENT_PRIORITY_HIGH, CVAR_VALUE, [](IEvent* event) {
        OnSaveFileSave* ev = (OnSaveFileSave*)event;
        SaveData* buf = (SaveData*)ev->saveBuffer;
        JinjoRetentionSaveData* live = store();
        if (buf != nullptr && live != nullptr && &buf->shipSaveData.jinjoRetention != live) {
            buf->shipSaveData.jinjoRetention = *live;
        }
    });
}

static RegisterShipInitFunc initJinjoRetention(RegisterJinjoRetention_Init, { CVAR_JINJO_RETENTION });
