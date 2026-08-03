#ifndef MUSIC_H
#define MUSIC_H

#include <ultra64.h>
#include "bool.h"
#include "enums.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <n_audio/PR/n_libaudio.h> // [port] inside extern "C" so n_audio decls keep C linkage in C++ TUs

#define NUM_CHANNEL_TEMPO_STATES   32

typedef struct chan_tempo_state_s {
    s32 slot_id;
    s32 chan;
    f32 tempo;
    f32 change_step;
    f32 target_tempo;
} ChanTempoState;

typedef struct midi_bin_s {
    u8 pad[1]; // Fake struct, contains compressed MIDI data of arbitrary size
} MidiBin;

typedef struct music_slot_s {
    s16 volume;
    u8  unk2;
    u8  unk3;
    ALCSeq cseq;
    ALCSPlayer cseqp;
    s16 track_id; // enum comusic_e
    s16 index_cpy;
    f32 unk17C;
    f32 unk180;
    u8 unk184[14];
    u8 unk192[14];
} MusicSlot;

typedef struct music_track_meta_s {
    char *name;
    u16  volume;
} MusicTrackMeta;

void musicInstruments_init(void);
ALBank *musicInstruments_getSoundBank(void);
void musicTrack_load(enum comusic_e track_id);
void musicTrack_release(enum comusic_e track_id);
void musicTrack_releaseAll(void);
void musicSlot_loadTrack(u8 index, enum comusic_e track_id);
enum comusic_e musicSlot_getTrack(u8 index);
void musicSlot_func_8024FA98(u8 index, enum comusic_e track_id);
s32 musicSlot_getSlotSeqpState(u8 index);
void musicSlot_stopAll(void);
void musicSlot_func_8024FC1C(u8 index, enum comusic_e track_id);
void musicSlot_func_8024FC6C(u8 index);
void musicSlot_func_8024FCE0(u8 index, s16 volume);
void musicSlot_setVolume(u8 index, s16 volume);
void musicSlot_setTempo(u8 index, s32 tempo);
void musicSlot_func_8024FE44(u8 index, f32 arg1, f32 arg2);
s32 musicSlot_getCSeqTicks(u8 index);
void func_8024FF34(void);
s32 gcMusic_getDefaultVolumeForTrack(enum comusic_e track_id);
void gcMusic_setDefaultVolumeForTrack(enum comusic_e track_id, u16 volume);
char *gcMusic_getNameForTrack(enum comusic_e track_id);
bool musicSlot_hasStopped(u8 index);
s32 gcMusic_getAssetCount(void);
N_ALCSPlayer *musicSlot_getCSeqp(s32 index);
void musicSlot_func_802500F4(s32 index);
void musicSlot_func_802500FC(s32 index);
void musicSlot_func_80250104(ALCSeq *cseq, s32 arg1, s32 chan);
void musicSlot_func_80250170(u8 index, s32 arg1, s32 arg2);
s32 musicSlot_func_802501A0(u8 index, s32 arg1, s32 *chan);
void musicSlot_setChannelTempoChange(s32 index, s16 chan, s16 target_tempo, f32 transition_speed);
void musicSlot_setTempoChange(s32 index, s32 target_tempo, f32 transition_speed);
u16 musicSlot_func_80250474(s32 index);
void musicSlot_stepToChannelMask(s32 index, u16 chan_mask, f32 transition_speed);
void musicSlot_stepToTempo(s32 index, s32 target_tempo, f32 transition_speed);
void func_80250650(void); // n_audio ??

#ifdef __cplusplus
}
#endif

#endif
