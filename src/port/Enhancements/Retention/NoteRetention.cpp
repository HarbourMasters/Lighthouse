// Note Collection Retention
//
// Notes are identified by (mapId, spawn-order index) from the map's deterministic cube parse.
// Bundle notes store identity in actor->local (survives sub-area save/restore, which copies
// the actor struct but assigns a new marker).
//
#include "port/ObjectExtension/ObjectExtension.h"
#include <libultraship/bridge.h>
#include <libultraship/bridge/consolevariablebridge.h>

#include "port/UI/cvar_prefixes.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/Rando/Rando.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "functions.h"
extern "C" {
#include "enums.h"
#include "actor.h"
#include "prop.h"

extern f32 gBundle_yaw;
extern ActorInfo sumusicNote;
}

namespace {

#define CVAR_NOTE_RETENTION CVAR_ENHANCEMENT("Gameplay.NoteRetention")
static bool sForcedByAnchor = false;
#define CVAR_VALUE (CVarGetInteger(CVAR_NOTE_RETENTION, 0) || sForcedByAnchor)

constexpr s32 kNoteSpriteAsset = ASSET_6D6_SPRITE_MUSIC_NOTE;

// Per-actor note identity. Parenthesized ctor avoids brace-init commas breaking event macros.
struct NoteRetentionData {
    int32_t mapId;
    int32_t noteIndex;
    NoteRetentionData(int32_t map = 0, int32_t index = 0) : mapId(map), noteIndex(index) {
    }
};
ObjectExtension::Register<NoteRetentionData> NoteRetentionDataRegister;

// Identity for bundle notes, stored in actor->local (survives sub-area restore; a new marker
// is assigned, so an extension wouldn't). magic distinguishes our data from garbage.
constexpr uint32_t kNoteLocalMagic = 0x4E4F5445u; // 'NOTE'
struct NoteLocal {
    uint32_t magic;
    int32_t mapId;
    int32_t noteIndex;
};

NoteLocal* bundleNoteLocal(Actor* actor) {
    if (actor == nullptr) {
        return nullptr;
    }
    NoteLocal* nl = reinterpret_cast<NoteLocal*>(actor->local);
    return (nl->magic == kNoteLocalMagic) ? nl : nullptr;
}

struct QueuedNote {
    int32_t pos[3];
    int32_t mapId;
    int32_t noteIndex;
};

int32_t noteCounter = 0;
std::vector<QueuedNote> noteActorQueue;
std::unordered_map<int64_t, ActorMarker*> activeNoteSet;

// Notes taken since entering this level: reproduces the prop "taken" bit they lost to actors.
std::unordered_set<int64_t> collectedThisVisit;

int64_t noteKey(int32_t mapId, int32_t noteIndex) {
    return ((int64_t)mapId << 32) | (uint32_t)noteIndex;
}

using retention::activeSlot;
using retention::systemActive;

// Level whose note count is waiting to be seeded; -1 when nothing is pending.
int32_t sPendingSeedLevel = -1;

bool applyEnabled() {
    return CVAR_VALUE;
}

// Rando drives note props on this same VB when they're shuffled, writing *should for every one
// of them — hand the class over rather than race it on listener order.
bool randoOwnsNoteProps() {
    return IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_MUSIC_NOTES].optionValue;
}

NoteRetentionSaveData* store() {
    int32_t slot = activeSlot();
    return slot >= 0 ? &gameFile_saveData[slot].shipSaveData.noteRetention : nullptr;
}

bool indexInRange(int32_t mapId, int32_t index) {
    return mapId >= 0 && mapId < NOTE_RETENTION_MAP_SLOTS && index >= 0 && index < NOTE_RETENTION_NOTES_PER_MAP;
}

bool isCollected(int32_t mapId, int32_t index) {
    NoteRetentionSaveData* s = store();
    if (s == nullptr || !indexInRange(mapId, index)) {
        return false;
    }
    return (s->collected[mapId][index >> 3] >> (index & 7)) & 1;
}

void setCollected(int32_t mapId, int32_t index) {
    NoteRetentionSaveData* s = store();
    if (s == nullptr || !indexInRange(mapId, index)) {
        return;
    }
    s->collected[mapId][index >> 3] |= (uint8_t)(1 << (index & 7));
}

int32_t countCollectedForLevel(int32_t levelId) {
    NoteRetentionSaveData* s = store();
    if (s == nullptr) {
        return 0;
    }
    int32_t total = 0;
    for (int32_t mapId = 0; mapId < NOTE_RETENTION_MAP_SLOTS; mapId++) {
        // Skip empty maps: map_getLevel crashes on non-existent ids (e.g. MAP_0_UNKNOWN).
        int32_t mapTotal = 0;
        for (int32_t b = 0; b < NOTE_RETENTION_BYTES_PER_MAP; b++) {
            uint8_t byte = s->collected[mapId][b];
            while (byte) {
                mapTotal += byte & 1;
                byte >>= 1;
            }
        }
        if (mapTotal == 0) {
            continue;
        }
        if (map_getLevel((enum map_e)mapId) == (enum level_e)levelId) {
            total += mapTotal;
        }
    }
    return total;
}

bool isInParade() {
    return volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE) ||
           volatileFlag_get(VOLATILE_FLAG_C1_IN_FINAL_CHARACTER_PARADE);
}

} // namespace

extern "C" void port_noteRetention_getSizeAndPtr(int32_t* size, uint8_t** addr) {
    NoteRetentionSaveData* s = store();
    if (s == nullptr) {
        *size = 0;
        *addr = nullptr;
        return;
    }
    *size = (int32_t)sizeof(NoteRetentionSaveData);
    *addr = (uint8_t*)s;
}

// Called from the actual pickup (MARKER_5F_MUSIC_NOTE), not broad-phase collision.
extern "C" void port_noteRetention_onLocalNoteCollected(void* markerPtr) {
    if (!systemActive()) {
        return;
    }
    ActorMarker* marker = (ActorMarker*)markerPtr;
    if (marker == nullptr || marker->id != MARKER_5F_MUSIC_NOTE) {
        return;
    }
    int32_t mapId;
    int32_t noteIndex;
    NoteRetentionData* data = ObjectExtension::GetInstance().Get<NoteRetentionData>(marker);
    if (data != nullptr) {
        mapId = data->mapId;
        noteIndex = data->noteIndex;
        ObjectExtension::GetInstance().Remove<NoteRetentionData>(marker);
    } else if (NoteLocal* nl = bundleNoteLocal(marker_getActor(marker))) {
        mapId = nl->mapId;
        noteIndex = nl->noteIndex;
        nl->magic = 0; // consumed
    } else {
        return; // not one of ours
    }
    bool wasCollected = isCollected(mapId, noteIndex);
    setCollected(mapId, noteIndex);
    collectedThisVisit.insert(noteKey(mapId, noteIndex));
    activeNoteSet.erase(noteKey(mapId, noteIndex));
    if (!wasCollected) {
        CALL_EVENT(OnCollectibleCollected, ANCHOR_COLLECTIBLE_NOTE, noteIndex);
    }
}

extern "C" void port_noteRetention_applyRemoteCollect(int32_t mapId, int32_t noteIndex, int32_t sameMap) {
    bool already = isCollected(mapId, noteIndex);
    setCollected(mapId, noteIndex);
    if (!already) {
        // ITEM_C_NOTE is per-level; item_inc also bumps the high score and pause total.
        int32_t level = map_getLevel((enum map_e)mapId);
        if (level == (int32_t)level_get()) {
            item_inc(ITEM_C_NOTE);
        } else {
            // Different level: can't touch live ITEM_C_NOTE, still bump its high score.
            itemscore_noteScores_setLevel((enum level_e)level, countCollectedForLevel(level));
        }
    }
    if (sameMap) {
        // Erase before despawn: OnActorDestroy's hook also erases this key.
        auto it = activeNoteSet.find(noteKey(mapId, noteIndex));
        if (it != activeNoteSet.end()) {
            ActorMarker* m = it->second;
            activeNoteSet.erase(it);
            if (m != nullptr) {
                marker_despawn(m);
            }
        }
    }
}

// Called from gsworld_load before cube parsing; fires on every re-parse (map load, intra-
// level warps). Resets the index counter.
extern "C" void port_noteRetention_beginMapLoad(int32_t mapId) {
    (void)mapId;
    noteCounter = 0;
    noteActorQueue.clear();
}

extern "C" void port_noteRetention_onActorsFreed(void) {
    for (auto& [key, marker] : activeNoteSet) {
        ObjectExtension::GetInstance().Remove<NoteRetentionData>(marker);
    }
    activeNoteSet.clear();
    noteActorQueue.clear();
}

extern "C" void port_noteRetention_setForced(int32_t forced) {
    sForcedByAnchor = forced != 0;
    ShipInit::Init(CVAR_NOTE_RETENTION);
}

extern "C" int32_t port_noteRetention_debugPickLive(int32_t mapId) {
    for (auto& [key, marker] : activeNoteSet) {
        int32_t keyMap = (int32_t)(key >> 32);
        int32_t index = (int32_t)(uint32_t)(key & 0xFFFFFFFF);
        if (keyMap == mapId && !isCollected(keyMap, index)) {
            return index;
        }
    }
    return -1;
}

void RegisterNoteRetention_Init() {
    // Bundle notes continue the map's index counter; identity stored in actor->local.
    COND_VB_SHOULD(VB_OVERRIDE_BUNDLE_SPAWN, EVENT_PRIORITY_NORMAL, true, {
        bundle_e bundleId = (bundle_e)va_arg(args, int);
        BundleInfo* bundleInfo = va_arg(args, BundleInfo*);
        va_arg(args, s32); // index within the bundle (unused)
        f32* position = va_arg(args, f32*);
        Actor** actorOut = va_arg(args, Actor**);

        if (!systemActive() || bundleInfo == nullptr || bundleInfo->actor_id != ACTOR_51_MUSIC_NOTE) {
            return; // not our note bundle
        }

        int32_t mapId = (int32_t)gsworld_getMap();
        int32_t noteIndex = noteCounter++;
        if (!indexInRange(mapId, noteIndex)) {
            return; // out of addressable range
        }

        // We own this note: suppress the vanilla bundle spawn.
        *should = true;

        int64_t key = noteKey(mapId, noteIndex);
        if (activeNoteSet.count(key)) {
            return; // a live actor already exists (re-trigger this load)
        }
        if ((applyEnabled() && isCollected(mapId, noteIndex)) || collectedThisVisit.count(key)) {
            return;
        }

        int32_t pos[3];
        pos[0] = (int32_t)position[0];
        pos[1] = (int32_t)position[1];
        pos[2] = (int32_t)position[2];
        Actor* note = actor_new(pos, 0, &sumusicNote, ACTOR_FLAG_UNKNOWN_21);
        if (note == nullptr) {
            return;
        }
        NoteLocal* nl = reinterpret_cast<NoteLocal*>(note->local);
        nl->magic = kNoteLocalMagic;
        nl->mapId = mapId;
        nl->noteIndex = noteIndex;

        ApplyBundleActorPhysics(note, (int32_t)bundleId, bundleInfo, gBundle_yaw);

        activeNoteSet[key] = note->marker;
        *actorOut = note;
    });

    // Seed the level's note counter from collected notes.
    COND_HOOK(OnSetJiggyList, EVENT_PRIORITY_NORMAL, CVAR_VALUE, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;
        sPendingSeedLevel = ev->levelId;
    });

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVAR_VALUE, [](IEvent*) {
        if (sPendingSeedLevel < 0) {
            return;
        }
        if (getGameMode() != GAME_MODE_3_NORMAL) {
            // Demo or playback: drop the seed, its score state is scratch. Anything else
            // (transition, pause) just waits for normal play to resume.
            if (func_802E4A08()) {
                sPendingSeedLevel = -1;
            }
            return;
        }
        const int32_t level = sPendingSeedLevel;
        sPendingSeedLevel = -1;
        if (!systemActive() || !applyEnabled() || isInParade()) {
            return;
        }
        item_adjustByDiffWithoutHud(ITEM_C_NOTE, countCollectedForLevel(level) - item_getCount(ITEM_C_NOTE));
    });

    // Seeding ITEM_C_NOTE re-triggers vanilla high-score dialogs; suppress them.
    COND_VB_SHOULD(VB_OVERRIDE_DIALOG_SHOW, EVENT_PRIORITY_NORMAL, true, {
        s32 textId = va_arg(args, s32);
        // Bottles' 50-note-door line assumes vanilla note-door costs, which
        // romhacks retune, so it is wrong for them whether or not retention is
        // on. In vanilla it stays: it's an aide for new players.
        if (textId == 0xF74 && port_isRomhack()) {
            *should = true;
        } else if (applyEnabled()) {
            switch (textId) {
                case 0xD9C: // Bottles' first-note text: "you can't take notes with you"
                case 0xF76: // "you just beat your high score"
                    // 0xF78 (every note in the level) is left alone here; hacks that
                    // want it gone suppress it per-hack, as Gruntch does.
                    *should = true;
                    break;
                default:
                    break;
            }
        }
    });
}

static RegisterShipInitFunc initNoteRetention(RegisterNoteRetention_Init, { CVAR_NOTE_RETENTION });

// Always registered: the sprite-prop pickup path has no marker to record through, so notes must
// be actors even when the enhancement isn't applying. Only the suppression below is gated.
static void RegisterRetentionSaving_Init() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent*) { collectedThisVisit.clear(); });

    // The visit's taken notes have to die with the note counter.
    REGISTER_LISTENER(OnLevelReset, EVENT_PRIORITY_NORMAL, [](IEvent*) { collectedThisVisit.clear(); });

    // The cube parse rebuilds static notes every map load, so a restored copy would double them
    // on a sub-area round trip. Bundle notes have no parse to come from and must survive.
    REGISTER_LISTENER(OnLoadActorSaveState, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnLoadActorSaveState* ev = (OnLoadActorSaveState*)event;
        if (!systemActive() || ev->actor == nullptr) {
            return;
        }
        if ((int32_t)ev->actor->modelCacheIndex != ACTOR_51_MUSIC_NOTE) {
            return;
        }
        if (ev->actor->is_bundle && bundleNoteLocal(ev->actor) != nullptr) {
            return; // keep restored bundle notes alive
        }
        event->Cancelled = true;
    });

    COND_VB_SHOULD(VB_OVERRIDE_PROP_SPAWN, EVENT_PRIORITY_NORMAL, true, {
        s16* spawnPosition = va_arg(args, s16*);
        s32 spriteAsset = va_arg(args, s32);

        if (!systemActive() || spriteAsset != kNoteSpriteAsset) {
            return;
        }

        int32_t mapId = (int32_t)gsworld_getMap();
        int32_t noteIndex = noteCounter++;
        if (!indexInRange(mapId, noteIndex)) {
            return; // out of addressable range
        }

        // Counter already advanced, so indices stay aligned; rando spawns these itself.
        if (randoOwnsNoteProps()) {
            return;
        }

        // We own this note: suppress the static sprite prop.
        *should = true;

        if (activeNoteSet.count(noteKey(mapId, noteIndex))) {
            return; // a live actor already exists (re-parse)
        }

        if ((applyEnabled() && isCollected(mapId, noteIndex)) || collectedThisVisit.count(noteKey(mapId, noteIndex))) {
            return;
        }

        QueuedNote queued;
        queued.pos[0] = spawnPosition[0];
        queued.pos[1] = spawnPosition[1];
        queued.pos[2] = spawnPosition[2];
        queued.mapId = mapId;
        queued.noteIndex = noteIndex;
        noteActorQueue.push_back(queued);
    });

    // Flush queued notes into actors. actor_new does not re-fire OnActorSpawn.
    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        if (!systemActive() || noteActorQueue.empty()) {
            return;
        }
        std::vector<QueuedNote> pending;
        pending.swap(noteActorQueue);
        for (auto& q : pending) {
            int32_t pos[3];
            pos[0] = q.pos[0];
            pos[1] = q.pos[1];
            pos[2] = q.pos[2];
            Actor* note = actor_new(pos, 0, &sumusicNote, ACTOR_FLAG_UNKNOWN_21);
            if (note != nullptr) {
                note->unk124_6 = 0;
            }
            ActorMarker* marker = (note != nullptr) ? note->marker : nullptr;
            if (marker != nullptr) {
                // Key on the marker (stable across the actor's life).
                ObjectExtension::GetInstance().Set<NoteRetentionData>(marker, NoteRetentionData(q.mapId, q.noteIndex));
                activeNoteSet[noteKey(q.mapId, q.noteIndex)] = marker;
            }
        }
    });

    COND_HOOK(OnActorDestroy, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        OnActorDestroy* ev = (OnActorDestroy*)event;
        if (ev->actor == nullptr) {
            return;
        }
        if (ev->actor->marker != nullptr) {
            NoteRetentionData* data = ObjectExtension::GetInstance().Get<NoteRetentionData>(ev->actor->marker);
            if (data != nullptr) {
                activeNoteSet.erase(noteKey(data->mapId, data->noteIndex));
            }
            ObjectExtension::GetInstance().Remove<NoteRetentionData>(ev->actor->marker);
        }
        if (NoteLocal* nl = bundleNoteLocal(ev->actor)) {
            activeNoteSet.erase(noteKey(nl->mapId, nl->noteIndex));
            nl->magic = 0;
        }
    });

    // bcopy's scratch-slot save buffer doesn't carry our bits; sync them in.
    COND_HOOK(OnSaveFileSave, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        OnSaveFileSave* ev = (OnSaveFileSave*)event;
        SaveData* buf = (SaveData*)ev->saveBuffer;
        NoteRetentionSaveData* live = store();
        if (buf != nullptr && live != nullptr && &buf->shipSaveData.noteRetention != live) {
            buf->shipSaveData.noteRetention = *live;
        }
    });
}

static RegisterShipInitFunc initRetentionSaving(RegisterRetentionSaving_Init);
