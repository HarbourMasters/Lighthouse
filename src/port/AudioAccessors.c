// Thin C accessors for audio state — avoids decomp header conflicts in C++.
// Called from LairContinuity.cpp, ShipUtils.cpp, etc.

#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "music.h"

extern CoMusic* D_80276E30;
extern MusicTrack D_80281720[6];
extern MusicTrackMeta D_80275D40[];

// --- port_setMusicSlotStopped: force a slot to AL_STOPPED (spin-wait deadlock fix) ---
void port_setMusicSlotStopped(int i) {
    D_80281720[i].cseqp.state = AL_STOPPED;
}

// --- LairContinuity accessors ---
s32 lc_getSlotIndex(u8 slot) {
    return D_80281720[slot].index;
}
void lc_getSlotTicks(u8 slot, s32* ticks) {
    *ticks = alCSeqGetTicks(&D_80281720[slot].cseq);
}
void lc_queueStop(u8 slot) {
    D_80281720[slot].index_cpy = -1;
    D_80281720[slot].unk2 = 1;
    D_80281720[slot].unk3 = 0;
    D_80281720[slot].unk0 = 0;
}
s32 lc_getTrackVolume(s32 trackIndex) {
    return (trackIndex >= 0 && trackIndex < 0xB0) ? D_80275D40[trackIndex].unk4 : 0;
}
s32 lc_getDestMapTrack(void) {
    return func_803226E8(map_get());
}
bool lc_isSlotPlayer(u8 slot, void* player) {
    return player == &D_80281720[slot].cseqp;
}
void lc_setHwVolume(u8 slot, s16 vol) {
    func_8024FD28(slot, vol);
}
void lc_setCoMusicAlive(u8 slot, s32 vol) {
    D_80276E30[slot].volume = vol;
    D_80276E30[slot].unk12 = 0;
}
