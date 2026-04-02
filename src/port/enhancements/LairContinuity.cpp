// Lair Audio Continuity Enhancement
//
// When enabled, lair music plays seamlessly through room transitions within
// Gruntilda's Lair. Tracks in the same group share sequence data, so seeking
// to the saved tick position sounds continuous.
//
// Hook points in the decomp C code call into this module:
//   lairContinuity_onTrackSet()      — from func_8024FC1C (track set/stop)
//   lairContinuity_onTick()          — from func_8024FF34 (per-frame update)
//   lairContinuity_onPreStop()       — from func_8024FF34 AL_PLAYING->stop
//   lairContinuity_onTrackStart()    — from func_8024FA98 (track start)
//   lairContinuity_consumeSeek()     — from n_csplayer.c AL_SEQP_PLAY_EVT
//   lairContinuity_hasPendingSeek()  — from audio_musicplayer.c fade skip

#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"

extern "C" {
#include "enums.h"
}

// Thin C accessors in audio_instruments.c — avoids decomp header conflicts.
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

#define CVAR_ENABLED CVarGetInteger(CVAR_ENHANCEMENT("Audio.LairContinuity"), 0)

// Lair tracks grouped by adjacent floors — seeking within a group sounds seamless.
// Group 0 = not a lair track, groups 1-3 = adjacent lair floors.
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

static int32_t sSavedTicks[4] = {}; // per-group tick positions (index 1-3)
static int32_t sLastSavedTicks = 0; // most recent tick from any group
static int32_t sPendingSeek = 0;    // ticks to seek to on next track start
static uint8_t sPendingSeekSlot = 0xFF;
static int8_t sDeferredStopSlot = -1; // slot with deferred stop, or -1

static void resetState() {
    sSavedTicks[0] = sSavedTicks[1] = sSavedTicks[2] = sSavedTicks[3] = 0;
    sLastSavedTicks = 0;
    sPendingSeek = 0;
    sPendingSeekSlot = 0xFF;
    sDeferredStopSlot = -1;
}

// Called from func_8024FC1C when setting a track index on a hardware slot.
// Returns true if the call was handled (deferred) and the caller should return early.
extern "C" bool lairContinuity_onTrackSet(uint8_t slot, int32_t newTrackId) {
    if (!CVAR_ENABLED)
        return false;

    // Stop request for a lair track
    if (newTrackId == -1) {
        int group = lairTrackGroup(lc_getSlotIndex(slot));
        if (group > 0) {
            int destGroup = lairTrackGroup(lc_getDestMapTrack());
            if (destGroup > 0) {
                // Lair-to-lair: defer the stop, save tick position
                int32_t ticks;
                lc_getSlotTicks(slot, &ticks);
                sSavedTicks[group] = ticks;
                sLastSavedTicks = ticks;
                sDeferredStopSlot = slot;
                return true; // caller returns without stopping
            }
            // Lair-to-non-lair: clear all saved state, let stop proceed
            resetState();
        }
        return false;
    }

    // New track request on a slot with a deferred stop
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
        // Queue stop — func_8024FC1C fallthrough will overwrite index_cpy with new track
        lc_queueStop(slot);
    }

    return false;
}

// Called at the start of func_8024FF34 (per-frame hardware tick).
extern "C" void lairContinuity_onTick(void) {
    if (sDeferredStopSlot < 0)
        return;

    // If the current map is no longer a lair map, force-stop the deferred slot.
    if (lairTrackGroup(lc_getDestMapTrack()) == 0) {
        lc_queueStop((uint8_t)sDeferredStopSlot);
        resetState();
        return;
    }

    // Keep the deferred track alive: maintain volume and prevent comusic fade
    uint8_t slot = (uint8_t)sDeferredStopSlot;
    int32_t index = lc_getSlotIndex(slot);
    int32_t vol = lc_getTrackVolume(index);
    lc_setHwVolume(slot, (int16_t)vol);
    lc_setCoMusicAlive(slot, vol);
}

// Called from func_8024FF34 just before alCSPStop when a playing track has unk2 set.
extern "C" void lairContinuity_onPreStop(int32_t slotIndex) {
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

// Called from func_8024FA98 when starting a new track on a stopped slot.
extern "C" void lairContinuity_onTrackStart(uint8_t slot, int32_t newTrackId) {
    if (!CVAR_ENABLED || newTrackId < 0)
        return;
    int newGroup = lairTrackGroup(newTrackId);
    if (newGroup > 0 && sLastSavedTicks > 0) {
        sPendingSeek = sSavedTicks[newGroup] ? sSavedTicks[newGroup] : sLastSavedTicks;
        sPendingSeekSlot = slot;
    }
}

// Called from n_csplayer.c AL_SEQP_PLAY_EVT handler.
extern "C" int32_t lairContinuity_consumeSeek(void* player) {
    if (sPendingSeekSlot >= 6)
        return 0;
    if (!lc_isSlotPlayer(sPendingSeekSlot, player))
        return 0;
    int32_t ticks = sPendingSeek;
    sPendingSeek = 0;
    sPendingSeekSlot = 0xFF;
    return ticks;
}

// Called from audio_musicplayer.c to check if fade-in should be skipped.
extern "C" int lairContinuity_hasPendingSeek(void) {
    return sPendingSeek > 0;
}
