// BanjoDecomp: code_11AC0.c
#include <ultra64.h>
#include <n_audio/PR/n_libaudio.h>
#include "core1/core1.h"
#include "core1/music.h"
#include "functions.h"
#include "variables.h"
#include "port/ShipUtils.h" // gPortResetPending
#include "version.h"

#include <libultra/exception.h>
#include "port/Patches/Patches.h"

#if VERSION == VERSION_USA_1_0
#define MUSIC_TRACK_ASSET_BASE_ID   0x1516
#elif VERSION == VERSION_PAL
#define MUSIC_TRACK_ASSET_BASE_ID   0xD74
#else
#define MUSIC_TRACK_ASSET_BASE_ID   0
#endif

u8 func_8025F4A0(N_ALCSPlayer *seqp, u8 chan); // n_audio, get channel tempo
void func_8025F5C0(N_ALCSPlayer *seqp, u8 chan); // n_audio
void func_8025F570(N_ALCSPlayer *seqp, u8 chan); // n_audio
void func_8025F3F0(ALCSPlayer *, f32, f32); // audio
void func_8025F510(ALCSPlayer *seqp, u8 chan, u8 tempo); // audio, alCSPSetTempo for channel

extern u8 *soundfont2ctl_ROM_START;
extern u8 *soundfont2ctl_ROM_END;
extern u8 *soundfont2tbl_ROM_START;

MusicTrackMeta musicTrackInfo[176] = {
    { "Blank", 15000 },
    { "Scrap", 15000 },
    { "Jungle 2", 20000 },
    { "Snow 2", 20000 },
    { "Bells", 21000 },
    { "Beach", 20000 },
    { "Swamp", 15000 },
    { "Crab Cave", 20000 },
    { "Title", 15000 },
    { "Notes", 15000 },
    { "Jinjo", 15000 },
    { "Feather", 15000 },
    { "Egg", 15000 },
    { "Jigpiece", 28000 },
    { "Sky", 32767 },
    { "Spooky", 21000 },
    { "Training", 15000 },
    { "Lighthouse", 24000 },
    { "Crab", 15000 },
    { "Shell", 32767 },
    { "Feather Inv", 15000 },
    { "Extra life", 15000 },
    { "Honeycomb", 15000 },
    { "Empty honey piece", 15000 },
    { "Extra honey", 15000 },
    { "Mystery", 15000 },
    { "You lose", 20000 },
    { "Termite nest", 32767 },
    { "Outside whale", 15000 },
    { "Spell", 15000 },
    { "Witch House", 23000 },
    { "In whale", 18000 },
    { "Desert", 20000 },
    { "In spooky", 18000 },
    { "Grave", 24000 },
    { "Church", 28000 },
    { "Sphinx", 20000 },
    { "Invulnerabilty", 28000 },
    { "Collapse", 15000 },
    { "Snake", 15000 },
    { "Sandcastle", 15000 },
    { "Summer", 20000 },
    { "Winter", 27000 },
    { "Right", 28000 },
    { "Wrong", 32000 },
    { "Achieve", 32000 },
    { "Autumn", 22000 },
    { "Default forest", 30000 },
    { "5 Jinjos", 15000 },
    { "Game over", 15000 },
    { "Nintendo", 15000 },
    { "Ship", 24000 },
    { "Shark", 15000 },
    { "Ship inside", 24000 },
    { "100 Notes", 15000 },
    { "Door Open", 15000 },
    { "Organ sequence", 18000 },
    { "Advent", 15000 },
    { "Slalom", 15000 },
    { "Race win", 15000 },
    { "Race lose", 15000 },
    { "Jigsaw magic", 15000 },
    { "Oh dear", 15000 },
    { "Up", 15000 },
    { "Down", 15000 },
    { "Shamen Hut", 19000 },
    { "Jig 10", 25000 },
    { "Carpet", 15000 },
    { "Squirrel", 15000 },
    { "Hornet", 15000 },
    { "Treetop", 32000 },
    { "Turtle Shell", 25000 },
    { "House Summer", 15000 },
    { "House Autumn", 15000 },
    { "Out Buildings", 15000 },
    { "Hornet 2", 15000 },
    { "Cabins", 15000 },
    { "Rain", 15000 },
    { "Jigsaw Open", 15000 },
    { "Jigsaw Close", 15000 },
    { "Witch 1", 23000 },
    { "Witch 2", 23000 },
    { "Witch 3", 23000 },
    { "Witch 4", 23000 },
    { "Witch 5", 23000 },
    { "Mr Vile", 15000 },
    { "Bridge", 22000 },
    { "Turbo Talon Trot", 28000 },
    { "Long legs", 28000 },
    { "Witch 6", 23000 },
    { "Boggy sad", 15000 },
    { "Boggy happy", 15000 },
    { "Quit", 15000 },
    { "Witch 7", 23000 },
    { "Witch 8", 23000 },
    { "Spring", 18000 },
    { "Squirrel attic", 26000 },
    { "Lights", 15000 },
    { "Box", 17000 },
    { "Witch 9", 23000 },
    { "Open up", 15000 },
    { "Puzzle complete", 25000 },
    { "Xmas tree", 15000 },
    { "Puzzle in", 15000 },
    { "Lite tune", 15000 },
    { "Open extra", 15000 },
    { "Ouija", 29000 },
    { "Wozza", 15000 },
    { "Intro", 20000 },
    { "Gnawty", 15000 },
    { "Banjo's Pad", 15000 },
    { "Pause", 15000 },
    { "Cesspit", 25000 },
    { "Quiz", 15000 },
    { "Frog", 20000 },
    { "GameBoy", 15000 },
    { "Lair", 15000 },
    { "Red Extra", 32000 },
    { "Gold Extra", 32000 },
    { "Egg Extra", 32000 },
    { "Note door", 15000 },
    { "Cheaty", 15000 },
    { "Fairy", 20000 },
    { "Skull", 25000 },
    { "Square Grunty", 25000 },
    { "Square Banjo", 25000 },
    { "Square Joker", 30000 },
    { "Square Music", 25000 },
    { "Lab", 20000 },
    { "Fade Up", 25000 },
    { "Puzzle Out", 15000 },
    { "Secret Gobi", 20000 },
    { "Secret Beach", 20000 },
    { "Secret Ice", 20000 },
    { "Secret Spooky", 20000 },
    { "Secret Squirrel", 20000 },
    { "Secret Egg", 20000 },
    { "Jinjup", 32000 },
    { "Turbo Talon Trot short", 28000 },
    { "Fade Down", 25000 },
    { "Big Jinjo", 32000 },
    { "T1000", 15000 },
    { "Credits", 15000 },
    { "T1000x", 20000 },
    { "Big Door", 20000 },
    { "Descent", 20000 },
    { "Wind up", 20000 },
    { "Air", 20000 },
    { "Do jig", 20000 },
    { "Picture", 28000 },
    { "Piece up", 20000 },
    { "Piece down", 20000 },
    { "Spin", 20000 },
    { "BarBQ", 15000 },
    { "Chord1", 20000 },
    { "Chord2", 20000 },
    { "Chord3", 20000 },
    { "Chord4", 20000 },
    { "Chord5", 20000 },
    { "Chord6", 20000 },
    { "Chord7", 20000 },
    { "Chord8", 20000 },
    { "Chord9", 20000 },
    { "Chord10", 20000 },
    { "Shock1", 20000 },
    { "Shock2", 20000 },
    { "Shock3", 20000 },
    { "Shock4", 20000 },
    { "Sad grunt", 20000 },
    { "Podium", 20000 },
    { "Endbit", 20000 },
    { "Rock", 20000 },
    { "Last Bit", 20000 },
    { "Unnamed piece", 15000 },
    { "Unnamed piece", 15000 },
    0
};

s32 D_802762C0 = 0;
s32 D_802762C4 = 0;

MusicSlot sMusicSlots[NUM_MUSIC_SLOTS];
MidiBin **sMIDIAssets;
ALSeqpConfig sMusicInstrumentsSeqConfig;
u16 sNumMIDIAssets;
ALBank *sMusicSoundBank;
ChanTempoState sChanTempoStates[NUM_CHANNEL_TEMPO_STATES];

void musicInstruments_init(void) {
    ALBankFile *bnk_f;
    u32 size;
    int i;

    sNumMIDIAssets = COMUSIC_NUM_TRACKS;
    sMIDIAssets = (MidiBin **) bk_malloc(sNumMIDIAssets * sizeof(MidiBin *));
    for (i = 0; i < sNumMIDIAssets; i++) {
        sMIDIAssets[i] = NULL;
    }

    for (i = 0; i < NUM_MUSIC_SLOTS; i++) {
        sMusicSlots[i].track_id = -1;
        sMusicSlots[i].unk2 = FALSE;
        sMusicSlots[i].unk3 = FALSE;
        sMusicSlots[i].index_cpy = 0;
        sMusicSlots[i].unk17C = 0.0f;
        sMusicSlots[i].unk180 = 1.0f;
        sMusicSlots[i].cseqp.state = AL_STOPPED;
    }

    // [port] Parse N64 big-endian ctl binary into native 64-bit structs
    extern ALBankFile *port_alBnkfNew(u8 *ctlData, s32 ctlSize, u8 *tblData);
    size = soundfont2ctl_ROM_END - soundfont2ctl_ROM_START;
    bnk_f = port_alBnkfNew(soundfont2ctl_ROM_START, size, soundfont2tbl_ROM_START);

    sMusicInstrumentsSeqConfig.maxVoices = 24;
    sMusicInstrumentsSeqConfig.maxEvents = 85;
    sMusicInstrumentsSeqConfig.maxChannels = 16;
    sMusicInstrumentsSeqConfig.heap = audioManager_getALHeapInfo();
    sMusicInstrumentsSeqConfig.initOsc = NULL;
    sMusicInstrumentsSeqConfig.updateOsc = NULL;
    sMusicInstrumentsSeqConfig.stopOsc = NULL;
    audioManager_setupSeqp(&sMusicInstrumentsSeqConfig);

    for (i = 0; i < NUM_MUSIC_SLOTS; i++) {
        n_alCSPNew((N_ALCSPlayer *) &sMusicSlots[i].cseqp, &sMusicInstrumentsSeqConfig);
    }

    sMusicSoundBank = bnk_f->bankArray[0];
    for (i = 0; i < NUM_MUSIC_SLOTS; i++) {
        alCSPSetBank(&sMusicSlots[i].cseqp, sMusicSoundBank);
    }

    musicSlot_stopAll();
}

ALBank *musicInstruments_getSoundBank(void) {
    return sMusicSoundBank;
}

void musicTrack_load(enum comusic_e track_id) {
    if (sMIDIAssets[track_id] == NULL) {
        assetcache_func_8033B788();
        sMIDIAssets[track_id] = assetcache_get(MUSIC_TRACK_ASSET_BASE_ID + track_id);
    }
}

void musicTrack_release(enum comusic_e track_id) {
    int i;

    port_lockAudio();
    if (sMIDIAssets[track_id] != NULL) {
        for (i = 0; i < NUM_MUSIC_SLOTS; i++) {
            if (sMusicSlots[i].track_id == track_id) {
                port_unlockAudio();
                return;
            }
        }
        assetcache_release(sMIDIAssets[track_id]);
        sMIDIAssets[track_id] = NULL;
    }
    port_unlockAudio();
}

void musicTrack_releaseAll(void) {
    int i;

    for (i = 0; i < sNumMIDIAssets; i++) {
        musicTrack_release(i);
    }
}

void musicSlot_loadTrack(u8 index, enum comusic_e track_id) {
    int i;

    // [port] This function directly mutates the sequence player's target (n_alCSeqNew) and
    // chanMask while the free-running audio worker reads them in n_alAudioFrame. Unlike
    // the rest of the audio code, it has no osSetIntMask bracket. Guard it with the audio lock.
    port_lockAudio();
    if (track_id == -1) {
        if (track_id != sMusicSlots[index].track_id) {
            alCSPStop((ALCSPlayer *) &sMusicSlots[index].cseqp);
        }
        sMusicSlots[index].track_id = track_id;
    } else {
        if (sMusicSlots[index].track_id != -1) {
            musicSlot_loadTrack(index, -1);
        }
        sMusicSlots[index].unk2 = FALSE;
        sMusicSlots[index].unk3 = FALSE;
        sMusicSlots[index].track_id = track_id;
        for (i = 0; i < 14; i++) {
            sMusicSlots[index].unk184[i] = 0;
            sMusicSlots[index].unk192[i] = 0;
        }
        musicTrack_load(sMusicSlots[index].track_id);
        if (track_id >= 0 && track_id < COMUSIC_NUM_TRACKS && sMIDIAssets != NULL) { // [port] bounds guard
            n_alCSeqNew(&sMusicSlots[index].cseq, (u8 *) sMIDIAssets[sMusicSlots[index].track_id]);
        }
        sMusicSlots[index].cseqp.chanMask = musicSlot_func_80250474(index);
        alCSPSetSeq(&sMusicSlots[index].cseqp, &sMusicSlots[index].cseq);
        alCSPPlay(&sMusicSlots[index].cseqp);
        alCSPSetVol(&sMusicSlots[index].cseqp, sMusicSlots[index].volume);

        if (player_is_present() && (player_getWaterState() == BSWATERGROUP_2_UNDERWATER)) {
            func_8025F3F0(&sMusicSlots[index].cseqp, 0.0f, 1.0f);
        } else {
            func_8025F3F0(&sMusicSlots[index].cseqp, sMusicSlots[index].unk17C, sMusicSlots[index].unk180);
        }
    }
    port_unlockAudio();
}

enum comusic_e musicSlot_getTrack(u8 index) {
    return sMusicSlots[index].track_id;
}

void musicSlot_func_8024FA98(u8 index, enum comusic_e track_id) {
    s32 current_track_id;
    volatile OSTime time;

    current_track_id = sMusicSlots[index].track_id;
    if ((current_track_id == track_id) || (current_track_id == -1)) {
        musicSlot_loadTrack(index, track_id);
    } else {
        musicSlot_loadTrack(index, -1);
        time = osGetTime();
        while (sMusicSlots[index].cseqp.state != AL_STOPPED) {
            if (gPortResetPending) break; // [port] avoid deadlock during reset
            osGetTime();
        };
        musicTrack_release(current_track_id);
        musicSlot_loadTrack(index, track_id);
    }
}

s32 musicSlot_getSlotSeqpState(u8 index) {
    return sMusicSlots[index].cseqp.state;
}

void musicSlot_stopAll(void) {
    int i;
    bool still_running;
    volatile OSTime time;

    for (i = 0; i < NUM_MUSIC_SLOTS; i++) {
        musicSlot_loadTrack(i, -1);
    }

    time = osGetTime();

    do {
        // [port] During reset, audio callback may not run to clear player states
        if (gPortResetPending) break;
        still_running = FALSE;
        for (i = 0; i < NUM_MUSIC_SLOTS; i++) {
            if (musicSlot_getSlotSeqpState(i) != AL_STOPPED)
                still_running++;
        }
        osGetTime();
    } while(still_running);

}

void musicSlot_func_8024FC1C(u8 index, enum comusic_e track_id) {
    sMusicSlots[index].index_cpy = track_id;
    sMusicSlots[index].unk2 = TRUE;
    sMusicSlots[index].unk3 = FALSE;
    if (track_id >= 0 && track_id < 176) { // [port] bounds guard
        sMusicSlots[index].volume = musicTrackInfo[track_id].volume;
    } else {
        sMusicSlots[index].volume = 0;
    }
}

void musicSlot_func_8024FC6C(u8 index) {
    enum comusic_e track_id = sMusicSlots[index].track_id;

    if ((track_id == COMUSIC_2D_PUZZLE_SOLVED_FANFARE) || (track_id == COMUSIC_3D_JIGGY_SPAWN)) {
        sMusicSlots[index].unk2 = TRUE;
        sMusicSlots[index].unk3 = FALSE;
        sMusicSlots[index].index_cpy = sMusicSlots[index].track_id;
    } else {
        sMusicSlots[index].index_cpy = -1;
        sMusicSlots[index].unk3 = TRUE;
        sMusicSlots[index].unk2 = TRUE;
        sMusicSlots[index].volume = 0;
    }
}

void musicSlot_func_8024FCE0(u8 index, s16 volume) {
    sMusicSlots[index].unk3 = TRUE;
    sMusicSlots[index].unk2 = TRUE;
    sMusicSlots[index].volume = volume;
    sMusicSlots[index].index_cpy = sMusicSlots[index].track_id;
}

void musicSlot_setVolume(u8 index, s16 volume) {
    sMusicSlots[index].volume = volume;
    alCSPSetVol(&sMusicSlots[index].cseqp, volume);

    if (sMusicSlots[index].unk3 && volume) {
        musicSlot_func_8024FCE0(index, volume);
    } else if (!sMusicSlots[index].unk3 && (volume == 0)) {
        if (!musicSlot_hasStopped(index)) {
            musicSlot_func_8024FC6C(index);
        }
    }
}

void musicSlot_setTempo(u8 index, s32 tempo) {
    if (!musicSlot_hasStopped(index)) {
        if (!sMusicSlots[index].unk2) {
            alCSPSetTempo(&sMusicSlots[index].cseqp, tempo);
        }
    }
}

void musicSlot_func_8024FE44(u8 index, f32 arg1, f32 arg2) {
    sMusicSlots[index].unk17C = arg1;
    sMusicSlots[index].unk180 = arg2;
    if (!musicSlot_hasStopped(index)) {
        if(player_getWaterState() == BSWATERGROUP_2_UNDERWATER){
            func_8025F3F0(&sMusicSlots[index].cseqp, 0.0f, 1.0f);
        }else{
            func_8025F3F0(&sMusicSlots[index].cseqp, arg1, arg2);
        }
    }
}

s32 musicSlot_getCSeqTicks(u8 index) {
    return alCSeqGetTicks(&sMusicSlots[index].cseq);
}

void func_8024FF34(void) {
    s32 i;

    for (i = 0; i < NUM_MUSIC_SLOTS; i++) {
        switch (sMusicSlots[i].cseqp.state) {
            case AL_PLAYING:
                if (sMusicSlots[i].unk2) {
                    alCSPStop(&sMusicSlots[i].cseqp);

                    if (sMusicSlots[i].unk3) {
                        sMusicSlots[i].unk2 = FALSE;
                    }
                }
                break;

            case AL_STOPPED:
                if (sMusicSlots[i].unk2) {
                    if (sMusicSlots[i].unk3) {
                        alCSPPlay(&sMusicSlots[i].cseqp);
                    } else {
                        musicSlot_func_8024FA98(i, sMusicSlots[i].index_cpy);
                    }
                    sMusicSlots[i].unk3 = FALSE;
                    sMusicSlots[i].unk2 = FALSE;
                    musicSlot_setVolume(i, sMusicSlots[i].volume);
                }
                break;

            case AL_STOPPING:
                break;
        }
    }
}

s32 gcMusic_getDefaultVolumeForTrack(enum comusic_e track_id) {
    if (track_id >= 0 && track_id < 176) { // [port] bounds guard
        return musicTrackInfo[track_id].volume;
    }
    return 0;
}

void gcMusic_setDefaultVolumeForTrack(enum comusic_e track_id, u16 volume) {
    if (track_id >= 0 && track_id < 176) { // [port] bounds guard
        musicTrackInfo[track_id].volume = volume;
    }
}

char *gcMusic_getNameForTrack(enum comusic_e track_id) {
    if (track_id >= 0 && track_id < 176) { // [port] bounds guard
        return musicTrackInfo[track_id].name;
    }
    return NULL;
}

bool musicSlot_hasStopped(u8 index) {
    return (sMusicSlots[index].cseqp.state == AL_STOPPED && !sMusicSlots[index].unk3);
}

s32 gcMusic_getAssetCount(void) {
    return (s16) sNumMIDIAssets;
}

N_ALCSPlayer *musicSlot_getCSeqp(s32 index) {
    return (N_ALCSPlayer *) &sMusicSlots[index].cseqp;
}

void musicSlot_func_802500F4(s32 index) {}

void musicSlot_func_802500FC(s32 index) {}

void musicSlot_func_80250104(ALCSeq *cseq, s32 arg1, s32 chan) {
    u8 i;

    for (i = 0; i < NUM_MUSIC_SLOTS; i++) {
        if (cseq == &sMusicSlots[i].cseq) {
            sMusicSlots[i].unk184[arg1 - 0x6A] = 1;
            sMusicSlots[i].unk192[arg1 - 0x6A] = chan;
            return;
        }
    }
}

void musicSlot_func_80250170(u8 index, s32 arg1, s32 arg2) {
    sMusicSlots[index].unk184[arg1 - 0x6A] = arg2;
}

s32 musicSlot_func_802501A0(u8 index, s32 arg1, s32 *chan) {
    if (chan) {
        *chan = sMusicSlots[index].unk192[arg1 - 0x6A];
    }
    return sMusicSlots[index].unk184[arg1 - 0x6A];
}

void musicSlot_setChannelTempoChange(s32 index, s16 chan, s16 target_tempo, f32 transition_speed) {
    int i;
    N_ALCSPlayer *csplayer;
    f32 tempo;
    OSIntMask old_int_mask;

    csplayer = musicSlot_getCSeqp(index);
    old_int_mask = osSetIntMask(OS_IM_NONE);
    tempo = (!musicSlot_hasStopped(index)) ? func_8025F4A0(csplayer, chan) : 127.0f;

    if (transition_speed < (2.0f / FRAMERATE)) {
        transition_speed = (2.0f / FRAMERATE);
    }

    for (i = 0; i < NUM_CHANNEL_TEMPO_STATES; i++) {
        if ((sChanTempoStates[i].tempo == sChanTempoStates[i].target_tempo) || ((sChanTempoStates[i].slot_id == index) && (sChanTempoStates[i].chan) == chan)) {
            sChanTempoStates[i].slot_id = index;
            sChanTempoStates[i].chan = chan;
            sChanTempoStates[i].tempo = tempo;
            sChanTempoStates[i].change_step = (target_tempo - tempo) / ((transition_speed * (f32) FRAMERATE) / 2);
            sChanTempoStates[i].target_tempo = target_tempo;
            osSetIntMask(old_int_mask);
            return;
        }
    }

    osSetIntMask(old_int_mask);
}

void musicSlot_setTempoChange(s32 index, s32 target_tempo, f32 transition_speed) {
    N_ALCSPlayer *csplayer;
    int i;
    OSIntMask old_int_mask;
    f32 tempo;

    csplayer = musicSlot_getCSeqp(index);
    old_int_mask = osSetIntMask(OS_IM_NONE);
    tempo = alCSPGetTempo((ALCSPlayer *) csplayer);

    if (transition_speed < (2.0f / FRAMERATE)) {
        transition_speed = (2.0f / FRAMERATE);
    }

    for (i = 0; i < NUM_CHANNEL_TEMPO_STATES; i++) {
        if ((sChanTempoStates[i].tempo == sChanTempoStates[i].target_tempo) || ((sChanTempoStates[i].slot_id == index) && (sChanTempoStates[i].chan == -1))) {
            sChanTempoStates[i].slot_id = index;
            sChanTempoStates[i].chan = -1;
            sChanTempoStates[i].tempo = tempo;
            sChanTempoStates[i].change_step = (target_tempo - tempo) / ((transition_speed * (f32) FRAMERATE) / 2);
            sChanTempoStates[i].target_tempo = target_tempo;
            osSetIntMask(old_int_mask);
            return;
        }
    }

    osSetIntMask(old_int_mask);
}

u16 musicSlot_func_80250474(s32 index) {
    int i;
    OSIntMask old_int_mask;

    if (index != 0) {
        return -1;
    }

    D_802762C0 = D_802762C4 = -1;

    old_int_mask = osSetIntMask(OS_IM_NONE);
    for (i = 0; i < NUM_CHANNEL_TEMPO_STATES; i++) {
        sChanTempoStates[i].tempo = -1.0f;
        sChanTempoStates[i].target_tempo = -1.0f;
    }
    osSetIntMask(old_int_mask);

    core1_ce60_func_8024AF48();

    if (D_802762C0 == -1) {
        D_802762C0 = 0xFFFF;
    }

    return D_802762C0;
}

void musicSlot_stepToChannelMask(s32 index, u16 chan_mask, f32 transition_speed) {
    s32 chan;

    if (D_802762C0 != chan_mask) {
        if (D_802762C0 == -1) {
            transition_speed = 0.0f;
        }
        D_802762C0 = chan_mask;
        for (chan = 0; chan < 16; chan++) {
            if (chan_mask & (1 << chan)) {
                musicSlot_setChannelTempoChange(index, chan, 127, transition_speed);
            } else {
                musicSlot_setChannelTempoChange(index, chan, 0, transition_speed);
            }
        }
    }
}

void musicSlot_stepToTempo(s32 index, s32 target_tempo, f32 transition_speed) {
    if (target_tempo != D_802762C4) {
        if (D_802762C4 == -1) {
            transition_speed = 0.0f;
        }
        D_802762C4 = target_tempo;
        musicSlot_setTempoChange(index, target_tempo, transition_speed);
    }
}

// n_audio ??
void func_80250650(void) {
    N_ALCSPlayer *csplayer;
    int i;
    s32 channel;

    for (i = 0; i < NUM_CHANNEL_TEMPO_STATES; i++) {
        csplayer = musicSlot_getCSeqp(sChanTempoStates[i].slot_id);

        if ((sChanTempoStates[i].tempo != sChanTempoStates[i].target_tempo) && !musicSlot_hasStopped(sChanTempoStates[i].slot_id)) {
            if (sChanTempoStates[i].change_step >= 0.0f) {
                sChanTempoStates[i].tempo = MIN(sChanTempoStates[i].tempo + sChanTempoStates[i].change_step, sChanTempoStates[i].target_tempo);
            } else {
                sChanTempoStates[i].tempo = MAX(sChanTempoStates[i].tempo + sChanTempoStates[i].change_step, sChanTempoStates[i].target_tempo);
            }

            if (sChanTempoStates[i].chan == -1) {
                alCSPSetTempo((ALCSPlayer *) csplayer, (s32) sChanTempoStates[i].tempo);
            } else {
                func_8025F510((ALCSPlayer *) csplayer, sChanTempoStates[i].chan, sChanTempoStates[i].tempo);
                channel = sChanTempoStates[i].chan;

                if (((csplayer->chanMask) & (1 << channel))) {
                    if (sChanTempoStates[i].tempo == 0.0) {
                        func_8025F5C0(csplayer, sChanTempoStates[i].chan);
                    }
                } else {
                    if (sChanTempoStates[i].tempo != 0.0f) {
                        func_8025F570(csplayer, sChanTempoStates[i].chan);
                    }
                }
            }
        }
    }
}
