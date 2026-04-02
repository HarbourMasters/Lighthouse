// Lair Audio Continuity Enhancement
//
// When enabled, lair music plays seamlessly through room transitions within
// Gruntilda's Lair. Tracks in the same group share sequence data, so seeking
// to the saved tick position sounds continuous.
//
// Listens to VB_MUSIC_SET_TRACK (to defer the vanilla stop) plus the
// OnMusicTick/OnMusicPreStop/OnMusicTrackStart notifications fired from
// audio_instruments.c. The seek/pending functions are called directly from
// n_csplayer.c and audio_musicplayer.c via Patches.h.

#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"
#include "port/enhancements/events/hooks/list/EngineEvent.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
}

// Thin C accessors in AudioAccessors.c — avoids decomp header conflicts.
extern "C" {
int32_t lc_getSlotIndex(uint8_t slot);
void lc_getSlotTicks(uint8_t slot, int32_t* ticks);
void lc_queueStop(uint8_t slot);
void lc_setCoMusicAlive(uint8_t slot, int32_t vol);
int32_t lc_getTrackVolume(int32_t trackIndex);
int32_t lc_getDestMapTrack(void);
bool lc_isSlotPlayer(uint8_t slot, void* player);
void lc_setHwVolume(uint8_t slot, int16_t vol);
}

#define CVAR_NAME CVAR_ENHANCEMENT("Audio.LairContinuity")
#define CVAR_ENABLED CVarGetInteger(CVAR_NAME, 0)

// VB_MUSIC_SET_TRACK args (still uses VB's void* args because it gates vanilla logic)
typedef struct {
    uint8_t slot;
    int32_t trackId;
} MusicTrackArgs;

// Lair tracks grouped by adjacent floors — seeking within a group sounds seamless.
static int lairTrackGroup(int32_t trackIndex) {
    switch (trackIndex) {
        case COMUSIC_1E_GL_MM_VERSION:
        case COMUSIC_50_GL_TTC_VERSION:
        case COMUSIC_51_GL_CCW_VERSION:
            return 1;
        case COMUSIC_52_GL_BGS_RBB_VERSION:
        case COMUSIC_53_GL_FP_VERSION_A:
            return 2;
        case COMUSIC_54_GL_GV_VERSION:
        case COMUSIC_59_GL_FP_VERSION_B:
        case COMUSIC_5D_GL_MMM_VERSION:
        case COMUSIC_5E_GL_MMM_RBB_VERSION:
        case COMUSIC_63_GL_FF_VERSION:
            return 3;
        default:
            return 0;
    }
}

static int32_t sSavedTicks[4] = {};
static int32_t sLastSavedTicks = 0;
static int32_t sPendingSeek = 0;
static uint8_t sPendingSeekSlot = 0xFF;
static int8_t sDeferredStopSlot = -1;

static void resetState() {
    sSavedTicks[0] = sSavedTicks[1] = sSavedTicks[2] = sSavedTicks[3] = 0;
    sLastSavedTicks = 0;
    sPendingSeek = 0;
    sPendingSeekSlot = 0xFF;
    sDeferredStopSlot = -1;
}

static void onTrackSet(VanillaBehavior* ev) {
    MusicTrackArgs* args = (MusicTrackArgs*)ev->args;
    uint8_t slot = args->slot;
    int32_t newTrackId = args->trackId;

    if (newTrackId == -1) {
        int group = lairTrackGroup(lc_getSlotIndex(slot));
        if (group > 0) {
            int destGroup = lairTrackGroup(lc_getDestMapTrack());
            if (destGroup > 0) {
                int32_t ticks;
                lc_getSlotTicks(slot, &ticks);
                sSavedTicks[group] = ticks;
                sLastSavedTicks = ticks;
                sDeferredStopSlot = slot;
                *ev->should = false; // cancel vanilla stop
                return;
            }
            resetState();
        }
        return;
    }

    if (newTrackId >= 0 && sDeferredStopSlot == (int8_t)slot) {
        int newGroup = lairTrackGroup(newTrackId);
        int oldGroup = lairTrackGroup(lc_getSlotIndex(slot));
        if (oldGroup > 0) {
            int32_t ticks;
            lc_getSlotTicks(slot, &ticks);
            sSavedTicks[oldGroup] = ticks;
            sLastSavedTicks = ticks;
        }
        sDeferredStopSlot = -1;
        if (newGroup > 0) {
            sPendingSeek = sSavedTicks[newGroup] ? sSavedTicks[newGroup] : sLastSavedTicks;
            sPendingSeekSlot = slot;
        }
        lc_queueStop(slot);
    }
}

static void onTick() {
    if (!CVAR_ENABLED || sDeferredStopSlot < 0)
        return;

    if (lairTrackGroup(lc_getDestMapTrack()) == 0) {
        lc_queueStop((uint8_t)sDeferredStopSlot);
        resetState();
        return;
    }

    uint8_t slot = (uint8_t)sDeferredStopSlot;
    int32_t index = lc_getSlotIndex(slot);
    int32_t vol = lc_getTrackVolume(index);
    lc_setHwVolume(slot, (int16_t)vol);
    lc_setCoMusicAlive(slot, vol);
}

static void onPreStop(int32_t slotIndex) {
    if (!CVAR_ENABLED || sDeferredStopSlot < 0)
        return;
    int group = lairTrackGroup(lc_getSlotIndex((uint8_t)slotIndex));
    if (group > 0) {
        int32_t ticks;
        lc_getSlotTicks((uint8_t)slotIndex, &ticks);
        sSavedTicks[group] = ticks;
        sLastSavedTicks = ticks;
    }
}

static void onTrackStart(uint8_t slot, int32_t trackId) {
    if (!CVAR_ENABLED || trackId < 0)
        return;
    int newGroup = lairTrackGroup(trackId);
    if (newGroup > 0 && sLastSavedTicks > 0) {
        sPendingSeek = sSavedTicks[newGroup] ? sSavedTicks[newGroup] : sLastSavedTicks;
        sPendingSeekSlot = slot;
    }
}

// Called directly from n_csplayer.c via Patches.h
extern "C" int32_t lairAudio_consumePendingSeek(void* player) {
    if (sPendingSeekSlot >= 6)
        return 0;
    if (!lc_isSlotPlayer(sPendingSeekSlot, player))
        return 0;
    int32_t ticks = sPendingSeek;
    sPendingSeek = 0;
    sPendingSeekSlot = 0xFF;
    return ticks;
}

// Called directly from audio_musicplayer.c via Patches.h
extern "C" int lairAudio_hasPendingSeek(void) {
    return sPendingSeek > 0;
}

void RegisterLairContinuity_Init() {
    COND_VB_SHOULD(VB_MUSIC_SET_TRACK, CVAR_ENABLED, {
        if (ev->id != VB_MUSIC_SET_TRACK)
            return;
        onTrackSet(ev);
    });

    // TODO: swap to COND_HOOK when available
    REGISTER_LISTENER(OnMusicTick, EVENT_PRIORITY_NORMAL, [](IEvent*) { onTick(); });

    // TODO: swap to COND_HOOK when available
    REGISTER_LISTENER(OnMusicPreStop, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnMusicPreStop* ev = (OnMusicPreStop*)event;
        onPreStop(ev->slot);
    });

    // TODO: swap to COND_HOOK when available
    REGISTER_LISTENER(OnMusicTrackStart, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnMusicTrackStart* ev = (OnMusicTrackStart*)event;
        onTrackStart(ev->slot, ev->trackId);
    });
}

static RegisterShipInitFunc initFunc(RegisterLairContinuity_Init, { CVAR_NAME });
