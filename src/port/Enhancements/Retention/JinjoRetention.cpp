// Jinjo Collection Retention
//
// Vanilla resets ITEM_12_JINJOS (per-level jinjo count) every level entry; this persists
// per-level collected colors across visits.
//
#include <libultraship/bridge.h>
#include <libultraship/bridge/consolevariablebridge.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"
#include "port/Rando/Rando.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include "enums.h"
#include "actor.h"
#include "prop.h"
#include "functions.h"

extern "C" int32_t port_jiggySpawn_isRecorded(int32_t jiggyId);

static bool sForcedByAnchor = false;
#define CVAR_JINJO_RETENTION CVAR_ENHANCEMENT("Gameplay.JinjoRetention")
#define CVAR_VALUE (CVarGetInteger(CVAR_JINJO_RETENTION, 0) || sForcedByAnchor)

constexpr u8 kAllJinjos = 0x1F; // all five color bits collected

using retention::activeSlot;
using retention::systemActive;

static bool applyEnabled() {
    return CVAR_VALUE;
}

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
        case 1 << 0:
            return ACTOR_60_JINJO_BLUE;
        case 1 << 1:
            return ACTOR_62_JINJO_GREEN;
        case 1 << 2:
            return ACTOR_5F_JINJO_ORANGE;
        case 1 << 3:
            return ACTOR_61_JINJO_PINK;
        case 1 << 4:
            return ACTOR_5E_JINJO_YELLOW;
        default:
            return 0;
    }
}

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

extern "C" void port_jinjoRetention_applyRemoteCollect(int32_t map, int32_t bit, int32_t sameMap) {
    int32_t level = map_getLevel((enum map_e)map);
    JinjoRetentionSaveData* s = store();
    if (s != nullptr && levelInRange(level)) {
        s->collected[level] |= (u8)bit;
    }
    // ITEM_12_JINJOS is per-level, not per-map; sub-areas are separate maps but share the count.
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

// Called from the actual pickup (__chJinjo_802CDBA8), not broad-phase collision.
extern "C" void port_jinjoRetention_onLocalJinjoCollected(int32_t markerId) {
    if (!systemActive()) {
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

// False when retention is off, or when the jiggy is stranded (all 5 recorded but not
// collected/spawned).
static bool retentionActiveForLevel(int32_t level) {
    if (!applyEnabled() || !levelInRange(level)) {
        return false;
    }
    if (collectedBits(level) == kAllJinjos && !jiggyscore_isCollected(jinjoJiggy(level)) &&
        !jiggyscore_isSpawned(jinjoJiggy(level)) && !port_jiggySpawn_isRecorded(jinjoJiggy(level))) {
        return false;
    }
    return true;
}

void RegisterJinjoRetention_Init() {
    // Seed ITEM_12_JINJOS from saved bits on entry.
    COND_HOOK(OnSetJiggyList, EVENT_PRIORITY_NORMAL, CVAR_VALUE, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;
        if (!systemActive() || !applyEnabled() || !levelInRange(ev->levelId)) {
            return;
        }
        int32_t level = ev->levelId;
        if (jiggyscore_isCollected(jinjoJiggy(level))) {
            // Jiggy earned but record out of sync — force all five.
            if (collectedBits(level) != kAllJinjos) {
                setCollectedBits(level, kAllJinjos);
            }
        } else if (collectedBits(level) == kAllJinjos && !jiggyscore_isSpawned(jinjoJiggy(level)) &&
                   !port_jiggySpawn_isRecorded(jinjoJiggy(level))) {
            // Orphaned: all recorded but jiggy neither collected nor spawned.
            setCollectedBits(level, 0);
        }
        if (!retentionActiveForLevel(level)) {
            return;
        }
        u8 bits = collectedBits(ev->levelId);
        if (bits != 0) {
            item_adjustByDiffWithoutHud(ITEM_12_JINJOS, bits - item_getCount(ITEM_12_JINJOS));
        }
    });

    // Suppress respawn of already-collected jinjos; null result + Cancelled = no spawn.
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

static void RegisterRetentionSaving_Init() {
    // bcopy's scratch-slot save buffer doesn't carry our bits; sync them in.
    COND_HOOK(OnSaveFileSave, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        OnSaveFileSave* ev = (OnSaveFileSave*)event;
        SaveData* buf = (SaveData*)ev->saveBuffer;
        JinjoRetentionSaveData* live = store();
        if (buf != nullptr && live != nullptr && &buf->shipSaveData.jinjoRetention != live) {
            buf->shipSaveData.jinjoRetention = *live;
        }
    });
}

static RegisterShipInitFunc initRetentionSaving(RegisterRetentionSaving_Init);
