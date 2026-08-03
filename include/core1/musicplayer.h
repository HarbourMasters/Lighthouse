#ifndef BANJO_KAZOOIE_CORE1_MUSICPLAYER_H
#define BANJO_KAZOOIE_CORE1_MUSICPLAYER_H

#include <ultra64.h>
#include "bool.h"
#include "enums.h"
#include "core2/fla.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_MUSIC_SLOTS            6



typedef struct struct_12_s{
    s32 volume_1; // volume 1
    s32 volume_2; // volume 2
} struct12s;

typedef struct comusic_s {
    f32 volume_change_wait_timer;
    f32 unk4;
    s32 volume;
    s32 destination_volume;
    s16 track_id; //trackId
    s16 volume_change;
    u8 unk14;
    u8 unk15;
    u8 pad16[0x2];
    FREE_LIST(struct12s) *unk18;
    s32 unk1C[0xE];
} CoMusic;

CoMusic *comusic_findTrack(enum comusic_e track_id);

void coMusicPlayer_init(void);
void coMusicPlayer_free(void);
s32 coMusicPlayer_getTrackCount(void);
void coMusicPlayer_update(void);


bool comusic_isTrackQueued(enum comusic_e track_id);
bool comusic_isPrimaryTrack(enum comusic_e track_id);
s32 comusic_getTrackPosition(enum comusic_e track_id);
void comusic_defrag(void);

#ifdef __cplusplus
}
#endif

#endif
