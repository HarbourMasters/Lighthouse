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

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"
#include "port/Rando/Rando.h"
#include "port/Rando/CustomObject/CustomObject.h"

extern "C" {
#include "enums.h"
#include "actor.h"
#include "prop.h"
#include "functions.h"
}

// True if a teammate (or we) dynamically spawned this jiggy this session and it isn't collected — so
// it will (re)appear from the team's spawned-jiggy record on map entry. Defined in SpawnJiggy.cpp.
extern "C" int32_t port_jiggySpawn_isRecorded(int32_t jiggyId);

// Anchor forces retention on while connected, separate from the user's CVar so their setting is
// preserved. CVAR_VALUE / applyEnabled() — and thus every COND_HOOK gate — respect it.
static bool sForcedByAnchor = false;
#define CVAR_JINJO_RETENTION CVAR_ENHANCEMENT("Gameplay.JinjoRetention")
#define CVAR_VALUE (CVarGetInteger(CVAR_JINJO_RETENTION, 0) || sForcedByAnchor)

constexpr u8 kAllJinjos = 0x1F; // all five color bits collected

using retention::activeSlot;
using retention::systemActive;

static bool applyEnabled() {
    return CVAR_VALUE;
}

// Anchor sets this on connect/disconnect. Re-runs the init so the COND_HOOK gates re-evaluate.
extern "C" void port_jinjoRetention_setForced(int32_t forced) {
    sForcedByAnchor = forced != 0;
    ShipInit::Init(CVAR_JINJO_RETENTION);
}

static JinjoRetentionSaveData* store() {
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

static bool levelInRange(int32_t level) {
    return level > 0 && level < JINJO_RETENTION_LEVEL_SLOTS;
}

static u8 collectedBits(int32_t level) {
    JinjoRetentionSaveData* s = store();
    return (s != nullptr && levelInRange(level)) ? s->collected[level] : 0;
}

static void setCollectedBits(int32_t level, u8 bits) {
    JinjoRetentionSaveData* s = store();
    if (s != nullptr && levelInRange(level)) {
        s->collected[level] = bits;
    }
}

// The level's jinjo jiggy (the "all five jinjos" reward).
static enum jiggy_e jinjoJiggy(int32_t level) {
    return (enum jiggy_e)(10 * level - 9);
}

static u8 jinjoBitFromMarker(int32_t markerId) {
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

static u8 jinjoBitFromActor(int32_t actorId) {
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

static int32_t jinjoActorFromBit(u8 bit) {
    switch (bit) {
        case 1 << 0: return ACTOR_60_JINJO_BLUE;
        case 1 << 1: return ACTOR_62_JINJO_GREEN;
        case 1 << 2: return ACTOR_5F_JINJO_ORANGE;
        case 1 << 3: return ACTOR_61_JINJO_PINK;
        case 1 << 4: return ACTOR_5E_JINJO_YELLOW;
        default: return 0;
    }
}

// Dev/test (RemoteCollectSim): pick a live jinjo actor in the current map so the simulator can
// drive applyRemoteCollect against a real target. Returns its colour bit, or 0 if none.
extern "C" int32_t port_jinjoRetention_debugPickLive(void) {
    static const enum actor_e kJinjoActors[] = { ACTOR_60_JINJO_BLUE, ACTOR_62_JINJO_GREEN, ACTOR_5F_JINJO_ORANGE,
                                                 ACTOR_61_JINJO_PINK, ACTOR_5E_JINJO_YELLOW };
    for (enum actor_e id : kJinjoActors) {
        Actor* a = actorArray_findActorFromActorId(id);
        if (a != nullptr && a->marker != nullptr) {
            return jinjoBitFromActor(id);
        }
    }
    return 0;
}

// Apply a teammate's jinjo pickup: record the colour bit for the collector's level; if we're
// in the same map, update the HUD count and despawn our copy of that jinjo.
extern "C" void port_jinjoRetention_applyRemoteCollect(int32_t map, int32_t bit, int32_t sameMap) {
    int32_t level = map_getLevel((enum map_e)map);
    JinjoRetentionSaveData* s = store();
    if (s != nullptr && levelInRange(level)) {
        s->collected[level] |= (u8)bit;
    }
    // ITEM_12_JINJOS is the current level's jinjo set, not a per-map count. Refresh the HUD for
    // anyone in the same level — sub-areas are distinct maps, so a teammate collecting in another
    // sub-area must still update our count, even though only the same map has a live actor to despawn.
    if (level == (int32_t)level_get()) {
        item_set(ITEM_12_JINJOS, collectedBits(level));
    }
    if (sameMap) {
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
static bool retentionActiveForLevel(int32_t level) {
    if (!applyEnabled() || !levelInRange(level)) {
        return false;
    }
    // Stranded-jiggy: all jinjos recorded but the jiggy is neither collected nor currently spawned,
    // so it can only be re-earned by re-collecting the jinjos — keep retention off so they respawn.
    // If the jiggy IS already spawned, or a teammate spawned it this session (it'll re-appear from the
    // team's spawned-jiggy record on entry, even if it hasn't this frame yet), keep retention on: the
    // jinjos stay suppressed and the player just collects the available jiggy.
    if (collectedBits(level) == kAllJinjos && !jiggyscore_isCollected(jinjoJiggy(level)) &&
        !jiggyscore_isSpawned(jinjoJiggy(level)) && !port_jiggySpawn_isRecorded(jinjoJiggy(level))) {
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
        if (!systemActive() || !applyEnabled() || !levelInRange(ev->levelId)) {
            return;
        }
        int32_t level = ev->levelId;
        // [port] Reconcile the recorded jinjo bits with the jiggy's actual state on entry:
        if (jiggyscore_isCollected(jinjoJiggy(level))) {
            // Jiggy earned — force all five recorded so no jinjo respawns (covers a jiggy collected
            // with the record out of sync, e.g. a teammate finished it).
            if (collectedBits(level) != kAllJinjos) {
                setCollectedBits(level, kAllJinjos);
            }
        } else if (collectedBits(level) == kAllJinjos && !jiggyscore_isSpawned(jinjoJiggy(level)) &&
                   !port_jiggySpawn_isRecorded(jinjoJiggy(level))) {
            // Orphaned: all jinjos recorded but the jiggy is neither collected nor spawned, so it must
            // be re-earned. The frozen 0x1F record meant re-collected jinjos never persisted; clear it
            // so they respawn fresh and re-collecting accumulates + persists (re-spawning the jiggy on
            // the fifth). The spawned/recorded checks leave a jiggy a teammate spawned this session
            // alone — it re-appears from the team's spawned-jiggy record on entry rather than being
            // wrongly treated as stranded (which would clear the jinjos before it respawns).
            setCollectedBits(level, 0);
        }
        if (!retentionActiveForLevel(level)) {
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

    COND_VB_SHOULD(VB_OVERRIDE_BUNDLE_SPAWN, EVENT_PRIORITY_NORMAL, CVAR_VALUE, {
        (void)va_arg(args, int);
        BundleInfo* bundleInfo = va_arg(args, BundleInfo*);
        (void)va_arg(args, s32);
        (void)va_arg(args, f32*);
        (void)va_arg(args, Actor**);

        if (!systemActive() || bundleInfo == nullptr) {
            return;
        }
        u8 bit = jinjoBitFromActor(bundleInfo->actor_id);
        if (bit == 0) {
            return;
        }
        int32_t level = level_get();
        if (!retentionActiveForLevel(level) || !(collectedBits(level) & bit)) {
            return;
        }
        *should = true;
    });
}

static RegisterShipInitFunc initJinjoRetention(RegisterJinjoRetention_Init, { CVAR_JINJO_RETENTION });
