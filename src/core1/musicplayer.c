// BanjoDecomp: musicplayer.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "version.h"

#include "core1/core1.h"

void func_8025AE50(s32, f32);
void func_8025AC20(enum comusic_e, s32, s32, f32, char*, s32);
void func_8025AC7C(enum comusic_e, s32, s32, f32, void *, const char *, s32);
void func_8025A7DC(enum comusic_e);

/* .bss */
CoMusic *comusicTracks = NULL; // Pointer to the first CoMusic struct. Additional are saved after in memory. The first track is the primary track.
bool sProcessVolumeChange = FALSE;

/* .code */
/**
 * @brief returns a pointer to the CoMusic struct with the corresponding track_id
 * if it exists OR the first free CoMusic struct. This also skips the first CoMusic in the search.
 *
 * @param track_id
 * @return CoMusic*
 */
CoMusic *comusic_findTrack(enum comusic_e track_id) {
    CoMusic *i_ptr, *first_free = NULL;

    for (i_ptr = &comusicTracks[1]; i_ptr < &comusicTracks[NUM_MUSIC_SLOTS - 1]; i_ptr++) {
        if (i_ptr->track_id == track_id) {
            return i_ptr;
        }

        if ((first_free == NULL) && (i_ptr->track_id < 0)) {
            first_free = i_ptr;
        }
    }

    return first_free;
}

void comusic_func_80259914(CoMusic *this, s32 arg1, s32 arg2) {
    s32 sp2C;
    s32 i;
    struct12s *tmp;

    freelist_clear(this->unk18);
    for(i = 0; i < 0xE; i++){
        this->unk1C[i] = 0;
    }

    tmp = (struct12s *)freelist_next(&this->unk18, &sp2C);
    tmp->volume_1 = arg1;
    tmp->volume_2 = arg2;
}

void comusic_func_80259994(CoMusic *this, s32 volume) {
    comusic_func_80259914(this, volume, volume);
}

void comusic_func_802599B4(CoMusic *this) {
    comusic_func_80259994(this, gcMusic_getDefaultVolumeForTrack(this->track_id));

    this->track_id = -1;
    this->unk14 = 0;
    this->unk15 = FALSE;

    musicSlot_func_8024FC1C(this - comusicTracks, -1);
}

void coMusicPlayer_init(void) {
    CoMusic *i_ptr;
    int i;

    if (comusicTracks != NULL) {
        coMusicPlayer_free();
    }

    comusicTracks = (CoMusic *) bk_malloc(NUM_MUSIC_SLOTS * sizeof(CoMusic));

    for (i_ptr = &comusicTracks[0]; i_ptr < &comusicTracks[NUM_MUSIC_SLOTS]; i_ptr++) {
        i_ptr->track_id = -1;
        i_ptr->volume = 0;
        i_ptr->volume_change = 0;
        i_ptr->destination_volume = 0;
        i_ptr->unk4 = 0.0f;
        i_ptr->unk14 = 0;
        i_ptr->unk15 = FALSE;
        i_ptr->volume_change_wait_timer = 0.0f;
        i_ptr->unk18 = (FREE_LIST(struct12s) *) freelist_new(sizeof(struct12s), 4);

        for (i = 0; i < 0xE; i++) {
            i_ptr->unk1C[i] = 0;
        }
    }
}

void coMusicPlayer_free(void) {
    CoMusic *i_ptr;

    musicSlot_stopAll();
    musicTrack_releaseAll();

    for (i_ptr = &comusicTracks[0]; i_ptr < &comusicTracks[NUM_MUSIC_SLOTS]; i_ptr++) {
        freelist_free(i_ptr->unk18);
    }

    bk_free(comusicTracks);
    comusicTracks = NULL;
}

s32 coMusicPlayer_getTrackCount(void) {
    CoMusic *i_ptr;
    s32 count = 0;

    for (i_ptr = &comusicTracks[0]; i_ptr < &comusicTracks[NUM_MUSIC_SLOTS]; i_ptr++) {
        if (i_ptr->track_id >= 0) {
            count++;
        }
    }

    return count;
}

void coMusicPlayer_update(void) {
    s32 music_slot;
    CoMusic *i_ptr;
    f32 dt;

    dt = time_getDelta();

    for (i_ptr = &comusicTracks[0]; i_ptr < &comusicTracks[NUM_MUSIC_SLOTS]; i_ptr++) {
        if (i_ptr->track_id >= 0) {
            music_slot = i_ptr - comusicTracks;
            i_ptr->unk4 = ml_min_f(i_ptr->unk4 + dt, 600.0f);

            if ((i_ptr->unk4 > 1.0f) && musicSlot_hasStopped(music_slot)) {
                func_8025A7DC(i_ptr->track_id);
            }
        }
    }

    func_8024FF34();

    if (!sProcessVolumeChange) {
        return;
    }

    sProcessVolumeChange = FALSE;

    for (i_ptr = &comusicTracks[0]; i_ptr < &comusicTracks[NUM_MUSIC_SLOTS]; i_ptr++) {
        if (i_ptr->track_id < 0) {
            continue;
        }

        if (i_ptr->volume_change != 0) {
            music_slot = i_ptr - comusicTracks;

            // The volume change begins only after the wait timer expires
            if (i_ptr->volume_change_wait_timer > 0.0f) {
                i_ptr->volume_change_wait_timer -= time_getDelta();
                sProcessVolumeChange = TRUE;
                continue;
            }

            // Process volume decrease
            if (i_ptr->volume_change < 0) {
                i_ptr->volume += i_ptr->volume_change;

                if (i_ptr->unk15 && (i_ptr->destination_volume == 0) && (i_ptr->volume <= 0)) {
                    comusic_func_802599B4(i_ptr);
                    continue;
                } else {
                    if (i_ptr->volume <= i_ptr->destination_volume) {
                        i_ptr->volume = i_ptr->destination_volume;
                        i_ptr->volume_change = 0;
                    } else {
                        sProcessVolumeChange = TRUE;
                    }
                    musicSlot_setVolume(music_slot, i_ptr->volume);
                }

                continue;
            }

            // Process volume increase
            if (i_ptr->volume < i_ptr->destination_volume) {
                if (i_ptr->volume == 0) {
                    i_ptr->unk4 = 0.0f;
                }
                i_ptr->volume += i_ptr->volume_change;
                if (i_ptr->volume >= i_ptr->destination_volume) {
                    i_ptr->volume = i_ptr->destination_volume;
                    i_ptr->volume_change = 0;
                } else {
                    sProcessVolumeChange = TRUE;
                }
                musicSlot_setVolume(music_slot, i_ptr->volume);

                continue;
            }

            i_ptr->volume_change = 0;
        }
    }
}

void func_80259EA8(CoMusic *this, s32 *arg1, s32 *arg2){
    int i;
    int cnt = freelist_size(this->unk18);
    s32 tmp_s1 = 0x7FFF;
    s32 tmp_s2 = 0x40000000;
    struct12s *tmp_ptr;

    for(i = 1; i < cnt; i++){
        if (freelist_elementIsAlive(this->unk18, i)) {
            tmp_ptr = (struct12s*)freelist_at(this->unk18, i);

            if (tmp_ptr->volume_1 < tmp_s1 || (tmp_s1 == tmp_ptr->volume_1 && tmp_ptr->volume_2 < tmp_s2)) {
                tmp_s1 = tmp_ptr->volume_1;
                tmp_s2 = tmp_ptr->volume_2;
            }//L80259F40
        }
    }
    *arg1 = tmp_s1;
    *arg2 = tmp_s2;
}

void func_80259F7C(CoMusic *self, s32 *arg1, s32 *arg2, s32 *arg3) {
    struct12s *temp_v0;
    f32 pad;
    s32 sp34;
    s32 var_s2;

    var_s2 = *arg1;
    sp34 = *arg2;
    if ((*arg3 != 0) && !freelist_elementIsAlive(self->unk18, *arg3)) {
        *arg3 = 0;
    }

    if (var_s2 < 0) {
        temp_v0 = (struct12s *)freelist_at(self->unk18, 1);
        if (temp_v0->volume_1 < gcMusic_getDefaultVolumeForTrack(self->track_id)) {
            var_s2 = gcMusic_getDefaultVolumeForTrack(self->track_id);
        } else {
            var_s2 = temp_v0->volume_1;
        }

        if (*arg3 != 0) {
            temp_v0 = (struct12s *)freelist_at(self->unk18, *arg3);
            *arg2 = temp_v0->volume_2;
            freelist_freeElement(self->unk18, *arg3);
            *arg3 = 0;
            func_80259EA8(self, arg1, &sp34);
            return;
        }
    }

    if (*arg3 == 0) {
        temp_v0 = (struct12s *)freelist_at(self->unk18, 1);
        if ((temp_v0->volume_1 < var_s2) || ((var_s2 == temp_v0->volume_1) && (sp34 >= temp_v0->volume_2))) {
            comusic_func_80259914(self, var_s2, sp34);
        } else {
            freelist_next(&self->unk18, arg3);
        }
    }

    if (*arg3 != 0) {
        temp_v0 = (struct12s *)freelist_at(self->unk18, *arg3);
        temp_v0->volume_1 = var_s2;
        temp_v0->volume_2 = sp34;
    }

    func_80259EA8(self, arg1, arg2);
}

void func_8025A104(enum comusic_e track_id, s32 volume) {
    if (track_id != comusicTracks[0].track_id) {
        musicSlot_func_8024FC1C(0, track_id);
    }

    musicSlot_setVolume(0, volume);
    comusicTracks[0].track_id = (s16) track_id;
    comusicTracks[0].volume = volume;
    comusicTracks[0].volume_change_wait_timer = 0.0f;
    comusicTracks[0].volume_change = 0;
    comusicTracks[0].unk4 = 0.0f;
    comusicTracks[0].unk15 = FALSE;
    comusic_func_80259994(&comusicTracks[0], volume);
}

void func_8025A1A8(enum comusic_e arg0){
    if (arg0 == comusicTracks[0].track_id) {
        return;
    }

    musicSlot_func_8024FC1C(0, arg0);

    comusicTracks[0].track_id = (s16) arg0;
    comusicTracks[0].volume = gcMusic_getDefaultVolumeForTrack(arg0);
    comusicTracks[0].volume_change_wait_timer = 0.0f;
    comusicTracks[0].volume_change = 0;
    comusicTracks[0].unk4 = 0.0f;
    comusicTracks[0].unk15 = FALSE;

    comusic_func_80259994(&comusicTracks[0], comusicTracks[0].volume);
}

void func_8025A23C(enum comusic_e track_id) {
    CoMusic *last_music_track = &comusicTracks[NUM_MUSIC_SLOTS - 1];
    s32 volume;

    if (track_id == last_music_track->track_id){
        return;
    }

    musicSlot_func_8024FC1C(NUM_MUSIC_SLOTS - 1, track_id);
    last_music_track->track_id = (s16) track_id;
    volume = gcMusic_getDefaultVolumeForTrack(track_id);
    last_music_track->volume = volume;
    last_music_track->volume_change = 0;
    last_music_track->unk15 = FALSE;
    last_music_track->volume_change_wait_timer = 0.0f;
    last_music_track->unk4 = 0.0f;
    comusic_func_80259994(last_music_track, volume);
}

void func_8025A2B0(void) {
    comusic_func_802599B4(&comusicTracks[NUM_MUSIC_SLOTS - 1]);
}

void func_8025A2D8(void) {
    comusic_func_802599B4(&comusicTracks[0]);
}

void func_8025A2FC(s32 arg0, s32 arg1){
    s32 i;

    func_8025A55C(arg0, arg1, 1);
    for (i = 1; i < NUM_MUSIC_SLOTS - 1; i++){
        s16 val = comusicTracks[i].track_id;
        if (val >= 0){
            func_8025ABB8(val, arg0, arg1, 1);
        }
    }
}

void func_8025A388(s32 arg0, s32 arg1) {
    s32 i;

    if (comusicTracks->unk14 == 0){
        func_8025A55C(arg0, arg1, 1);
    }

    for (i = 1; i < NUM_MUSIC_SLOTS - 1; i++) {
        if (comusicTracks[i].track_id >= 0 && comusicTracks[i].unk14 == 0) {
            func_8025ABB8(comusicTracks[i].track_id, arg0, arg1, 1);
        }
    }
}

void func_8025A430(s32 arg0, s32 arg1, s32 arg2){
    s32 i;

    func_8025A55C(arg0, arg1, arg2);

    for (i = 1; i < NUM_MUSIC_SLOTS - 1; i++) {
        s16 val = comusicTracks[i].track_id;

        if (val >= 0) {
            func_8025ABB8(val, arg0, arg1, arg2);
        }
    }
}

void func_8025A4C4(s32 arg0, s32 arg1, s32 *arg2) {
    if (comusicTracks[0].track_id < 0) {
        return;
    }

    func_80259F7C(&comusicTracks[0], &arg0, &arg1, arg2);

    if (arg0 != comusicTracks[0].volume) {
        if (comusicTracks[0].volume < arg0) {
            comusicTracks[0].volume_change = arg1;
        } else {
            comusicTracks[0].volume_change = -arg1;
        }

        comusicTracks[0].destination_volume = arg0;
        sProcessVolumeChange = TRUE;
    }
}

void func_8025A55C(s32 arg0, s32 arg1, s32 arg2) {
    func_8025A4C4(arg0, arg1, &comusicTracks->unk1C[arg2]);
}

void func_8025A58C(u32 arg0, u32 arg1) {
    func_8025A55C(arg0, arg1, 0);
}

void comusic_playMusic(enum comusic_e track_id, s32 volume, bool force_reinit) {
    CoMusic *track;
    s32 index;

    if (volume == -1) {
        volume = gcMusic_getDefaultVolumeForTrack(track_id);
    }

    track = comusic_findTrack(track_id);

    if (track == NULL) {
        return;
    }

    index = track - comusicTracks;

    // Initialize the track if comusic_findTrack returned an empty
    if (track->track_id < 0 || force_reinit) {
        switch (track_id) {
            case COMUSIC_15_EXTRA_LIFE_COLLECTED:
                if (gsworld_getMap() == MAP_10_BGS_MR_VILE) {
                    break;
                }
            case COMUSIC_3B_MINIGAME_VICTORY:
            case COMUSIC_3C_MINIGAME_LOSS:
                func_8025AE50(4000, 2.0f);
        }

        track->track_id = track_id;
        track->volume_change = 0;
        track->unk15 = FALSE;
        track->unk4 = 0.0f;
        comusic_func_80259994(track, volume);
        musicSlot_func_8024FC1C(index, track_id);
    }

    musicSlot_setVolume(index, volume);
    track->volume = volume;
}

// If comusic_findTrack returns the track, then do not re-initialize; just adust the volume
void coMusicPlayer_playMusicWeak(enum comusic_e track_id, s32 volume) {
    comusic_playMusic(track_id, volume, FALSE);
}

// Plays the track and reinitializes it
void coMusicPlayer_playMusic(enum comusic_e track_id, s32 volume) {
    comusic_playMusic(track_id, volume, TRUE);
}

//comusic_queueTrack
void comusic_playTrack(enum comusic_e track_id) {
    CoMusic *trackPtr;
    s32 indx;

    trackPtr = comusic_findTrack(track_id);

    if (trackPtr == NULL) {
        return;
    }

    indx = trackPtr - comusicTracks;

    if (trackPtr->track_id < 0) {
        trackPtr->track_id = track_id;
        trackPtr->volume_change = 0;
        trackPtr->unk4 = 0.0f;
        musicSlot_func_8024FC1C( indx, track_id);
        comusic_func_80259994(trackPtr, trackPtr->volume = gcMusic_getDefaultVolumeForTrack(track_id));
    }
}

void func_8025A788(enum comusic_e track_id, f32 delay1, f32 delay2) {
    timedFunc_set_1(delay1, (GenFunction_1) comusic_playTrack, track_id);
    timedFunc_set_1(delay1 + delay2, (GenFunction_1) func_8025A7DC, track_id);
}

void func_8025A7DC(enum comusic_e track_id) {
    CoMusic *trackPtr;

    trackPtr = comusic_findTrack(track_id);

    if (trackPtr != NULL && trackPtr->track_id >= 0){
        comusic_func_802599B4(trackPtr);
    }
}

s32 func_8025A818(void) {
    if (comusicTracks[0].destination_volume == 0 && comusicTracks[0].volume <= 0){
        comusic_func_802599B4(&comusicTracks[0]);
        return 1;
    }

    return 0;
}

s32 func_8025A864(enum comusic_e track_id){
    CoMusic *trackPtr;

    trackPtr = comusic_findTrack(track_id);

    if (trackPtr != NULL && trackPtr->destination_volume == 0 && trackPtr->volume <= 0){
        comusic_func_802599B4(trackPtr);
        return 1;
    }

    return 0;
}

void func_8025A8B8(enum comusic_e track_id, s32 arg1){
    CoMusic *trackPtr = comusic_findTrack(track_id);

    if (trackPtr != NULL){
        trackPtr->unk14 = arg1;
    }
}

void func_8025A8E4(s32 arg0) {
    if (comusicTracks[0].track_id < 0) {
        return;
    }

    comusicTracks[0].unk14 = arg0;
}

void func_8025A904(void){
    CoMusic *i_ptr;

    for (i_ptr = &comusicTracks[0]; i_ptr < &comusicTracks[NUM_MUSIC_SLOTS]; i_ptr++) {
        if (i_ptr->track_id >= 0) {
            comusic_func_802599B4(i_ptr);
        }
    }
}

//dequeue_allTracks
void func_8025A96C(void){
    CoMusic *i_ptr;

    for (i_ptr = &comusicTracks[1]; i_ptr < &comusicTracks[NUM_MUSIC_SLOTS]; i_ptr++) {
        if (i_ptr->track_id >= 0) {
            comusic_func_802599B4(i_ptr);
        }
    }
}

//dequeue_allTracks
void func_8025A9D4(void){
    CoMusic *i_ptr;

    for (i_ptr = &comusicTracks[0]; i_ptr < &comusicTracks[NUM_MUSIC_SLOTS]; i_ptr++) {
        if (i_ptr->track_id >= 0 && !i_ptr->unk14) {
            comusic_func_802599B4(i_ptr);
        }
    }
}

//dequeue_nonmainTracks
void func_8025AA48(void){
    CoMusic *i_ptr;

    for (i_ptr = &comusicTracks[1]; i_ptr < &comusicTracks[NUM_MUSIC_SLOTS]; i_ptr++) {
        if (i_ptr->track_id >= 0 && !i_ptr->unk14) {
            comusic_func_802599B4(i_ptr);
        }
    }
}

//dequeue_track?
void func_8025AABC(enum comusic_e track_id) {
    CoMusic *trackPtr;

    trackPtr = comusic_findTrack(track_id);

    if (!trackPtr) {
        return;
    }

    trackPtr->unk15 = TRUE;

    if (!trackPtr->volume) {
        comusic_func_802599B4(trackPtr);
    }
}

void func_8025AB00(void) {
    comusicTracks[0].unk15 = TRUE;

    if (comusicTracks[0].volume){
        return;
    }

    comusic_func_802599B4(&comusicTracks[0]);
}

void comusic_8025AB44(enum comusic_e track_id, s32 arg1, s32 arg2) {
    func_8025AC20(track_id, arg1, arg2, 0.0f, "comusic.c", VER_SELECT(926, 927, 0, 0));
}

void comusic_8025AB78(enum comusic_e track_id, s32 arg1, s32 arg2, s32 arg3) {
    CoMusic *track = comusic_findTrack(track_id); // [port] null check
    if (!track) return;
    func_8025AC7C(track_id, arg1, arg2, 0.0f, &track->unk1C[arg3], "comusic.c", VER_SELECT(931, 932, 0, 0));
}

void func_8025ABB8(enum comusic_e track_id, s32 arg1, s32 arg2, s32 arg3) {
    func_8025AC7C(track_id, arg1, arg2, 0.0f, &(comusic_findTrack(track_id)->unk1C[arg3]), "comusic.c", VER_SELECT(938, 939, 0, 0));
}

void func_8025AC20(enum comusic_e track_id, s32 arg1, s32 arg2, f32 arg3, char* arg4, s32 char5) {
    func_8025AC7C(track_id, arg1, arg2, 0.0f, comusic_findTrack(track_id)->unk1C, "comusic.c", VER_SELECT(945, 946, 0, 0));
}

void func_8025AC7C(enum comusic_e track_id, s32 arg1, s32 arg2, f32 arg3, void *arg4, const char *filename, s32 arg6){
    CoMusic *trackPtr;
    u32 slot_index;
    s32 *fadeSlot = (s32 *)arg4; // [port] passed as void* for 64-bit pointer safety

    //get track location
    trackPtr = comusic_findTrack(track_id);
    if(trackPtr == NULL)
        return;

    //check if track is loaded in slot
    if(trackPtr->track_id < 0){ //Track not loaded
        if(arg1 == 0)
            return;
        slot_index = (trackPtr - comusicTracks);
        musicSlot_func_8024FC1C(slot_index, track_id);
        trackPtr->track_id = track_id;
        trackPtr->volume = 0;
        trackPtr->unk15 = FALSE;
        trackPtr->unk4 = 0.0f;
        comusic_func_80259994(trackPtr, 0);
        musicSlot_setVolume(slot_index, 0);
    }
    func_80259F7C(trackPtr,&arg1, &arg2, fadeSlot);
    trackPtr->volume_change_wait_timer = arg3;
    trackPtr->volume_change = (trackPtr->volume < arg1)? arg2: -arg2;
    trackPtr->destination_volume = arg1;
    sProcessVolumeChange = TRUE;
}

bool comusic_isTrackQueued(enum comusic_e track_id) {
    CoMusic *track = comusic_findTrack(track_id);
    return ((track == NULL) || (track->track_id == -1)) ? FALSE : TRUE;
}

bool comusic_isPrimaryTrack(enum comusic_e track_id) {
    return comusicTracks[0].track_id == track_id;
}

s32 comusic_getTrackPosition(enum comusic_e track_id) {
    CoMusic *track = comusic_findTrack(track_id);
    return track - comusicTracks;
}

void func_8025AE0C(s32 arg0, f32 time) {
    func_8025A58C(0, arg0);
    timedFunc_set_2(time, (GenFunction_2)func_8025A58C, -1, arg0);
}

void func_8025AE50(s32 arg0, f32 time) {
    func_8025A430(0, arg0, 6);
    timedFunc_set_3(time, (GenFunction_3)func_8025A430, -1, arg0, 6);
}

void func_8025AEA0(enum comusic_e track_id, s32 arg1) {
    CoMusic *track = comusic_findTrack(track_id);

    if (!track) {
        return;
    }

    musicSlot_setTempo(track - comusicTracks, arg1);
}

int func_8025AEEC(void) {
    s32 out = musicSlot_func_802501A0(0, 0x6A, NULL);

    if (out) {
        musicSlot_func_80250170(0, 0x6A, 0);
    }

    return out;
}

void comusic_defrag(void) {
    CoMusic *i_ptr;

    if (!comusicTracks) {
        return;
    }

    for (i_ptr = &comusicTracks[0]; i_ptr < &comusicTracks[NUM_MUSIC_SLOTS]; i_ptr++) {
        i_ptr->unk18 = (FREE_LIST(struct12s) *) freelist_defrag(i_ptr->unk18);
    }

    comusicTracks = (CoMusic *) defrag(comusicTracks);
}
