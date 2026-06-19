// Note Collection Retention
//
// Persists which individual music notes the player has collected so they don't respawn
// on revisit. Works for vanilla and romhacks because each note is identified
// by (mapId, spawn-order index) within the map's deterministic cube-prop parse.
//
// How it works:
//   - Static note sprite-props are replaced with ACTOR_51_MUSIC_NOTE actors so a
//     stable spawn-order index can be attached to each via ObjectExtension.
//   - Dynamically-spawned note bundles (hut/switch triggers) are also replaced with
//     our own actors, given indices that continue AFTER the static notes for the map.
//     Their identity resides in actor->local (not ObjectExtension) because bundle notes
//     persist across sub-area save/restore, which copies the actor struct but assigns
//     a fresh marker, so a marker-keyed extension wouldn't survive.
//   - Saving is ALWAYS ON: collecting a note records (mapId, index) to the save.
//   - Application is behind an enhancement toggle (default off): when on, already
//     collected notes are not respawned, and ITEM_C_NOTE is seeded on level load
//     from the collected count so totals stay reachable. Note-door logic is left
//     to the vanilla per-level high score, unchanged.
//
#include "port/ObjectExtension/ObjectExtension.h"
#include <libultraship/bridge.h>
#include <libultraship/bridge/consolevariablebridge.h>

#include "port/UI/cvar_prefixes.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/NoteRetention/NoteRetention.h"
#include "port/Rando/Rando.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include <vector>
#include <unordered_map>

extern "C" {
#include "enums.h"
#include "actor.h"
#include "prop.h"
#include "functions.h"

extern f32 gBundle_yaw;
extern ActorInfo sumusicNote;
}

namespace {

#define CVAR_NOTE_RETENTION CVAR_ENHANCEMENT("Gameplay.NoteRetention")
#define CVAR_VALUE CVarGetInteger(CVAR_NOTE_RETENTION, 0)

// The note sprite asset id passed through VB_OVERRIDE_PROP_SPAWN identifies notes.
constexpr s32 kNoteSpriteAsset = ASSET_6D6_SPRITE_MUSIC_NOTE;

// Per-actor note identity, attached via ObjectExtension. The constructor lets us
// build instances with parentheses, since brace-init commas break the event macros.
struct NoteRetentionData {
    int32_t mapId;
    int32_t noteIndex;
    NoteRetentionData(int32_t map = 0, int32_t index = 0) : mapId(map), noteIndex(index) {
    }
};
ObjectExtension::Register<NoteRetentionData> NoteRetentionDataRegister;

// Identity for bundle notes, stored in the actor's local data block. Unlike static
// notes, bundle notes are restored across sub-area transitions by the engine's actor
// save-state, which memcpy's the whole Actor struct (so local survives) but assigns
// a new marker (so an extension wouldn't).
// magic distinguishes our data from the uninitialized garbage actor_new leaves
// in local, and from notes we didn't spawn.
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
    bool spawned;
};

int32_t noteCounter = 0;
std::vector<QueuedNote> noteActorQueue;
std::unordered_map<int64_t, ActorMarker*> activeNoteSet;

int64_t noteKey(int32_t mapId, int32_t noteIndex) {
    return ((int64_t)mapId << 32) | (uint32_t)noteIndex;
}

// Active slot is wobbly
int32_t activeSlot() {
    if (selectedFileNum == DEFAULT_FILE_NUM || selectedFileNum < 0 || selectedFileNum >= 4) {
        return -1;
    }
    return (int32_t)selectedFileNum;
}

bool fileValid() {
    return activeSlot() >= 0;
}

// Don't run during demos, Bottles bonus games, or rando files
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
    return CVarGetInteger(CVAR_NOTE_RETENTION, 0) != 0;
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
        // Count this map's collected bits first; only real maps that have been
        // played ever get bits set. Skipping empties keeps us from calling
        // map_getLevel on non-existent map ids (e.g. MAP_0_UNKNOWN), which crashes.
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

} // namespace

// Called from gsworld_load (the single entry that parses a map's cubes) before any
// cube is read. This is the reliable once-per-parse-pass signal -- it fires on every
// map load AND on intra-level warps/sub-area transitions that re-parse the cubes,
// even when OnMapLoad doesn't. Resetting the counter here keeps spawn-order indices
// stable across re-parses. The live-actor set is NOT cleared here: our note actors
// persist across map changes (they're only torn down by actorArray_free), so the set
// must persist too, or returning to a map would duplicate its still-live notes.
extern "C" void port_noteRetention_beginMapLoad(int32_t mapId) {
    (void)mapId;
    noteCounter = 0;
    noteActorQueue.clear();
}

// Called from actorArray_free. At this point every note actor is gone, so drop the
// live-actor set and detach the ObjectExtension data.
extern "C" void port_noteRetention_onActorsFreed(void) {
    for (auto& [key, marker] : activeNoteSet) {
        ObjectExtension::GetInstance().Remove<NoteRetentionData>(marker);
    }
    activeNoteSet.clear();
    noteActorQueue.clear();
}

void RegisterNoteRetention_Init() {
    COND_VB_SHOULD(VB_OVERRIDE_PROP_SPAWN, EVENT_PRIORITY_NORMAL, true, {
        s16* spawnPosition = va_arg(args, s16*);
        s32 spriteAsset = va_arg(args, s32);

        if (!systemActive() || spriteAsset != kNoteSpriteAsset) {
            return;
        }

        int32_t mapId = (int32_t)gsworld_getMap();
        int32_t noteIndex = noteCounter++;
        if (!indexInRange(mapId, noteIndex)) {
            return; // out of addressable range: leave as a vanilla sprite prop
        }

        // From here we own this note: suppress the static sprite prop.
        *should = true;

        if (activeNoteSet.count(noteKey(mapId, noteIndex))) {
            return; // a live actor already exists (re-parse): don't duplicate it
        }

        if (applyEnabled() && isCollected(mapId, noteIndex)) {
            return; // already collected and retention on: don't respawn it
        }

        QueuedNote queued;
        queued.pos[0] = spawnPosition[0];
        queued.pos[1] = spawnPosition[1];
        queued.pos[2] = spawnPosition[2];
        queued.mapId = mapId;
        queued.noteIndex = noteIndex;
        queued.spawned = false;
        noteActorQueue.push_back(queued);
    });

    // Flush queued notes into actors. actor_new does not re-fire OnActorSpawn, so no recursion.
    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, CVAR_VALUE, [](IEvent* event) {
        if (!systemActive() || noteActorQueue.empty()) {
            return;
        }
        for (auto& q : noteActorQueue) {
            if (q.spawned) {
                continue;
            }
            int32_t pos[3];
            pos[0] = q.pos[0];
            pos[1] = q.pos[1];
            pos[2] = q.pos[2];
            Actor* note = actor_new(pos, 0, &sumusicNote, ACTOR_FLAG_UNKNOWN_21);
            ActorMarker* marker = (note != nullptr) ? note->marker : nullptr;
            if (marker != nullptr) {
                // Key on the marker (stable across the actor's life and what the collision event hands us).
                ObjectExtension::GetInstance().Set<NoteRetentionData>(marker, NoteRetentionData(q.mapId, q.noteIndex));
                activeNoteSet[noteKey(q.mapId, q.noteIndex)] = marker;
            }
            q.spawned = true;
        }
    });

    // Note bundles get spawn-order indices that continue after the map's static
    // note count. We override the vanilla bundle spawn to substitute our own actor (carrying
    // identity in local) and re-apply the vanilla fly-out physics. Detection is by the
    // bundle's spawned actor type, so it stays romhack-agnostic.
    COND_VB_SHOULD(VB_OVERRIDE_BUNDLE_SPAWN, EVENT_PRIORITY_NORMAL, true, {
        bundle_e bundleId = (bundle_e)va_arg(args, int);
        BundleInfo* bundleInfo = va_arg(args, BundleInfo*);
        va_arg(args, s32); // index within the bundle (unused: we use our own counter)
        f32* position = va_arg(args, f32*);
        Actor** actorOut = va_arg(args, Actor**);

        if (!systemActive() || bundleInfo == nullptr || bundleInfo->actor_id != ACTOR_51_MUSIC_NOTE) {
            return; // not our note bundle: leave it to vanilla (or rando)
        }

        int32_t mapId = (int32_t)gsworld_getMap();
        int32_t noteIndex = noteCounter++;
        if (!indexInRange(mapId, noteIndex)) {
            return; // out of addressable range: let vanilla spawn it normally
        }

        // From here we own this note: suppress the vanilla bundle spawn.
        *should = true;

        int64_t key = noteKey(mapId, noteIndex);
        if (activeNoteSet.count(key)) {
            return; // a live actor already exists (re-trigger this load): don't duplicate
        }
        if (applyEnabled() && isCollected(mapId, noteIndex)) {
            return; // already collected and retention on: don't respawn it
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

    // Prevent restoration from save states, as actors get re-added by the cubeprop parse.
    COND_HOOK(OnLoadActorSaveState, EVENT_PRIORITY_NORMAL, CVAR_VALUE, [](IEvent* event) {
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

    // Record collection. Don't cancel: vanilla still increments ITEM_C_NOTE and despawns.
    COND_HOOK(OnActorCollision, EVENT_PRIORITY_NORMAL, CVAR_VALUE, [](IEvent* event) {
        OnActorCollision* ev = (OnActorCollision*)event;
        if (!systemActive() || !ev->propId->markerFlag) {
            return;
        }
        ActorMarker* marker = ev->propId->actorProp.marker;
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
        setCollected(mapId, noteIndex);
        activeNoteSet.erase(noteKey(mapId, noteIndex));
    });

    // Seed the level's note counter from collected notes so totals stay reachable.
    COND_HOOK(OnSetJiggyList, EVENT_PRIORITY_NORMAL, CVAR_VALUE, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;
        if (!systemActive() || !applyEnabled()) {
            return;
        }
        item_set(ITEM_C_NOTE, countCollectedForLevel(ev->levelId));
    });

    // With retention applied, the vanilla note-score messages become misleading or fire
    // spuriously (seeding ITEM_C_NOTE on load re-triggers the high-score milestones), so
    // suppress them while the CVar is on.
    COND_VB_SHOULD(VB_OVERRIDE_DIALOG_SHOW, EVENT_PRIORITY_NORMAL, true, {
        s32 textId = va_arg(args, s32);
        if (!applyEnabled()) {
            return;
        }
        switch (textId) {
            case 0xD9C: // Bottles' first-note text: "you can't take notes with you"
            case 0xF76: // "you just beat your high score"
            case 0xF74: // milestone: 50 notes (Mumbo's Mountain)
                *should = true;
                break;
            default:
                break;
        }
    });

    // Clean up per-marker data when actors are destroyed (marker pointers get reused).
    COND_HOOK(OnActorDestroy, EVENT_PRIORITY_NORMAL, CVAR_VALUE, [](IEvent* event) {
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

    // The engine's save double-buffers into a scratch slot via bcopy, which doesn't
    // reliably carry our note bits. Sync the live active slot's noteRetention into the
    // buffer about to be serialized.
    COND_HOOK(OnSaveFileSave, EVENT_PRIORITY_HIGH, CVAR_VALUE, [](IEvent* event) {
        OnSaveFileSave* ev = (OnSaveFileSave*)event;
        SaveData* buf = (SaveData*)ev->saveBuffer;
        NoteRetentionSaveData* live = store();
        if (buf != nullptr && live != nullptr && &buf->shipSaveData.noteRetention != live) {
            buf->shipSaveData.noteRetention = *live;
        }
    });
}

static RegisterShipInitFunc initNoteRetention(RegisterNoteRetention_Init);
