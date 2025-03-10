#include <ultra64.h>

#include "core1/core1.h"
#include "functions.h"
#include "variables.h"

#include "music.h"

#include "version.h"

#ifdef LIGHTHOUSE_P
#include "compat.h"
#else
#include "n_libaudio.h"



u8 n_alCSPGetChannelState(ALCSPlayer *, u8);
extern void n_alSeqPlayerSetChannelMode(N_ALSeqPlayer *arg0, u8 arg1);
extern void n_alCSPSetChannelVolume(ALCSPlayer *, u8, u8);
extern void n_alSeqPlayerSetChannelPressure(ALCSPlayer *, u8);
#endif

u8 soundfont2ctl_ROM_START[];
u8 soundfont2ctl_ROM_END[];
u8 soundfont2tbl_ROM_START[];

//musictrack.c

/* dependent functions */
void musicTrack_play(u8, enum comusic_e);
void musicTrack_setVolume(u8, s16);
int musicTrack_isStopped(u8);


void n_alCSPSetTempoTransition(ALCSPlayer *, f32, f32);
u16 musicTrack_getChannelMask(s32 arg0);
void musicTrack_stopAll(void);

/* .data */
MusicTrackMeta D_musicTrackMeta[0xB0] = {{"Blank", 15000},
                                         {"Scrap", 15000},
                                         {"Jungle 2", 20000},
                                         {"Snow 2", 20000},
                                         {"Bells", 21000},
                                         {"Beach", 20000},
                                         {"Swamp", 15000},
                                         {"Crab Cave", 20000},
                                         {"Title", 15000},
                                         {"Notes", 15000},
                                         {"Jinjo", 15000},
                                         {"Feather", 15000},
                                         {"Egg", 15000},
                                         {"Jigpiece", 28000},
                                         {"Sky", 0x7FFF},
                                         {"Spooky", 21000},
                                         {"Training", 15000},
                                         {"Lighthouse", 24000},
                                         {"Crab", 15000},
                                         {"Shell", 0x7FFF},
                                         {"Feather Inv", 15000},
                                         {"Extra life", 15000},
                                         {"Honeycomb", 15000},
                                         {"Empty honey piece", 15000},
                                         {"Extra honey", 15000},
                                         {"Mystery", 15000},
                                         {"You lose", 20000},
                                         {"Termite nest", 0x7FFF},
                                         {"Outside whale", 15000},
                                         {"Spell", 15000},
                                         {"Witch House", 23000},
                                         {"In whale", 0x4650},
                                         {"Desert", 20000},
                                         {"In spooky", 0x4650},
                                         {"Grave", 24000},
                                         {"Church", 28000},
                                         {"Sphinx", 20000},
                                         {"Invulnerabilty", 28000},
                                         {"Collapse", 15000},
                                         {"Snake", 15000},
                                         {"Sandcastle", 15000},
                                         {"Summer", 20000},
                                         {"Winter", 0x6978},
                                         {"Right", 28000},
                                         {"Wrong", 0x7D00},
                                         {"Achieve", 0x7D00},
                                         {"Autumn", 0x55F0},
                                         {"Default forest", 0x7530},
                                         {"5 Jinjos", 15000},
                                         {"Game over", 15000},
                                         {"Nintendo", 15000},
                                         {"Ship", 24000},
                                         {"Shark", 15000},
                                         {"Ship inside", 24000},
                                         {"100 Notes", 15000},
                                         {"Door Open", 15000},
                                         {"Organ sequence", 0x4650},
                                         {"Advent", 15000},
                                         {"Slalom", 15000},
                                         {"Race win", 15000},
                                         {"Race lose", 15000},
                                         {"Jigsaw magic", 15000},
                                         {"Oh dear", 15000},
                                         {"Up", 15000},
                                         {"Down", 15000},
                                         {"Shamen Hut", 0x4A38},
                                         {"Jig 10", 0x61A8},
                                         {"Carpet", 15000},
                                         {"Squirrel", 15000},
                                         {"Hornet", 15000},
                                         {"Treetop", 0x7D00},
                                         {"Turtle Shell", 0x61A8},
                                         {"House Summer", 15000},
                                         {"House Autumn", 15000},
                                         {"Out Buildings", 15000},
                                         {"Hornet 2", 15000},
                                         {"Cabins", 15000},
                                         {"Rain", 15000},
                                         {"Jigsaw Open", 15000},
                                         {"Jigsaw Close", 15000},
                                         {"Witch 1", 23000},
                                         {"Witch 2", 23000},
                                         {"Witch 3", 23000},
                                         {"Witch 4", 23000},
                                         {"Witch 5", 23000},
                                         {"Mr Vile", 15000},
                                         {"Bridge", 0x55F0},
                                         {"Turbo Talon Trot", 28000},
                                         {"Long legs", 28000},
                                         {"Witch 6", 23000},
                                         {"Boggy sad", 15000},
                                         {"Boggy happy", 15000},
                                         {"Quit", 15000},
                                         {"Witch 7", 23000},
                                         {"Witch 8", 23000},
                                         {"Spring", 0x4650},
                                         {"Squirrel attic", 0x6590},
                                         {"Lights", 15000},
                                         {"Box", 0x4268},
                                         {"Witch 9", 23000},
                                         {"Open up", 15000},
                                         {"Puzzle complete", 0x61A8},
                                         {"Xmas tree", 15000},
                                         {"Puzzle in", 15000},
                                         {"Lite tune", 15000},
                                         {"Open extra", 15000},
                                         {"Ouija", 0x7148},
                                         {"Wozza", 15000},
                                         {"Intro", 20000},
                                         {"Gnawty", 15000},
                                         {"Banjo's Pad", 15000},
                                         {"Pause", 15000},
                                         {"Cesspit", 0x61A8},
                                         {"Quiz", 15000},
                                         {"Frog", 20000},
                                         {"GameBoy", 15000},
                                         {"Lair", 15000},
                                         {"Red Extra", 0x7D00},
                                         {"Gold Extra", 0x7D00},
                                         {"Egg Extra", 0x7D00},
                                         {"Note door", 15000},
                                         {"Cheaty", 15000},
                                         {"Fairy", 20000},
                                         {"Skull", 0x61A8},
                                         {"Square Grunty", 0x61A8},
                                         {"Square Banjo", 0x61A8},
                                         {"Square Joker", 0x7530},
                                         {"Square Music", 0x61A8},
                                         {"Lab", 20000},
                                         {"Fade Up", 0x61A8},
                                         {"Puzzle Out", 15000},
                                         {"Secret Gobi", 20000},
                                         {"Secret Beach", 20000},
                                         {"Secret Ice", 20000},
                                         {"Secret Spooky", 20000},
                                         {"Secret Squirrel", 20000},
                                         {"Secret Egg", 20000},
                                         {"Jinjup", 0x7D00},
                                         {"Turbo Talon Trot short", 28000},
                                         {"Fade Down", 0x61A8},
                                         {"Big Jinjo", 0x7D00},
                                         {"T1000", 15000},
                                         {"Credits", 15000},
                                         {"T1000x", 20000},
                                         {"Big Door", 20000},
                                         {"Descent", 20000},
                                         {"Wind up", 20000},
                                         {"Air", 20000},
                                         {"Do jig", 20000},
                                         {"Picture", 28000},
                                         {"Piece up", 20000},
                                         {"Piece down", 20000},
                                         {"Spin", 20000},
                                         {"BarBQ", 15000},
                                         {"Chord1", 20000},
                                         {"Chord2", 20000},
                                         {"Chord3", 20000},
                                         {"Chord4", 20000},
                                         {"Chord5", 20000},
                                         {"Chord6", 20000},
                                         {"Chord7", 20000},
                                         {"Chord8", 20000},
                                         {"Chord9", 20000},
                                         {"Chord10", 20000},
                                         {"Shock1", 20000},
                                         {"Shock2", 20000},
                                         {"Shock3", 20000},
                                         {"Shock4", 20000},
                                         {"Sad grunt", 20000},
                                         {"Podium", 20000},
                                         {"Endbit", 20000},
                                         {"Rock", 20000},
                                         {"Last Bit", 20000},
                                         {"Unnamed piece", 15000},
                                         {"Unnamed piece", 15000},
                                         0};
s32 currentChannelMask = 0;
s32 currentTempo = 0;

/* .bss */
MusicTrack musicTracks[6];
MusicTrack **musicTrackCache;
ALSeqpConfig seqPlayerConfig;
u16 totalMusicTracks; // called as u16 someplaces and s16 others
ALBank *soundBank;
structBs channelVolumes[0x20];

/* .code */
void musicInstruments_init(void) {
  s32 size;
  ALBankFile *bnk_f; // sp38
  s32 i;
  f32 tmpf1;

  size = soundfont2ctl_ROM_END - soundfont2ctl_ROM_START;
  bnk_f = heap_malloc(size);
  osWritebackDCacheAll();
  osPiStartDma(audioManager_getDmaIoMesg(), 0, 0, (u32)soundfont2ctl_ROM_START,
               bnk_f, size, audioManager_getDmaQueue());
  osRecvMesg(audioManager_getDmaQueue(), 0, 1); // osRecvMesg
  totalMusicTracks = 0xAD;
  musicTrackCache =
      (MusicTrack **)heap_malloc(totalMusicTracks * sizeof(MusicTrack *));
  for (i = 0; i < totalMusicTracks; i++) {
    musicTrackCache[i] = NULL;
  }
  seqPlayerConfig.maxVoices = 0x18;
  seqPlayerConfig.maxEvents = 0x55;
  seqPlayerConfig.maxChannels = 0x10;
  seqPlayerConfig.heap = audioManager_getHeap();
  seqPlayerConfig.initOsc = NULL;
  seqPlayerConfig.updateOsc = NULL;
  seqPlayerConfig.stopOsc = NULL;
  initOscStateList(&seqPlayerConfig);
  for (i = 0; i < 6; i++) {
    n_alCSPNew(&musicTracks[i].cseqp, &seqPlayerConfig);
  }

  alBnkfNew(bnk_f, soundfont2tbl_ROM_START);
  soundBank = bnk_f->bankArray[0];
  for (i = 0; i < 6; i++) {
    alCSPSetBank(&musicTracks[i].cseqp, soundBank);
  }

  for (i = 0; i < 6; i++) {
    musicTracks[i].unk2 = 0;
    musicTracks[i].unk3 = 0;
    musicTracks[i].index_cpy = 0;
    musicTracks[i].unk17C = 0.0f;
    musicTracks[i].unk180 = 1.0f;
  }
  musicTrack_stopAll();
}

ALBank *music_get_sound_bank(void) { return soundBank; }

void musicTrack_load(s32 arg0) { // music track load
  if (musicTrackCache[arg0] == NULL) {
    assetCache_releaseAnimation();
#if VERSION == VERSION_USA_1_0
    musicTrackCache[arg0] = assetcache_get(arg0 + 0x1516);
#elif VERSION == VERSION_PAL
    musicTrackCache[arg0] = assetcache_get(arg0 + 0xd74);
#endif
  }
}

void musicTrack_unload(s32 arg0) {
  s32 i;
  if (musicTrackCache[arg0] != NULL) {
    i = 0;
    for (i = 0; i != 6; i++) {
      if (musicTracks[i].index == arg0)
        return;
    }
    assetcache_release(musicTrackCache[arg0]);
    musicTrackCache[arg0] = 0;
  }
}

void musicTrack_unloadAll(void) {
  s32 i;
  for (i = 0; i < totalMusicTracks; i++) {
    musicTrack_unload(i);
  }
}

void musicTrack_set(u8 arg0, enum comusic_e arg1) {
  s32 i;
  if (arg1 == -1) {
    if (arg1 != musicTracks[arg0].index)
      alCSPStop(&musicTracks[arg0].cseqp);
    musicTracks[arg0].index = arg1;

  } else {
    if (-1 != musicTracks[arg0].index) {
      musicTrack_set(arg0, -1);
    }
    musicTracks[arg0].unk2 = 0;
    musicTracks[arg0].unk3 = 0;
    musicTracks[arg0].index = arg1;
    for (i = 0; i < 0xe; i++) {
      musicTracks[arg0].unk184[i] = 0;
      musicTracks[arg0].unk192[i] = 0;
    }
    musicTrack_load(musicTracks[arg0].index);
    n_alCSeqNew(&musicTracks[arg0].cseq,
                musicTrackCache[musicTracks[arg0].index]);

    musicTracks[arg0].cseqp.chanMask = musicTrack_getChannelMask(arg0);
    alCSPSetSeq(&musicTracks[arg0].cseqp, &musicTracks[arg0].cseq);
    alCSPPlay(&musicTracks[arg0].cseqp);
    alCSPSetVol(&musicTracks[arg0].cseqp, musicTracks[arg0].unk0);
    if (player_is_present() &&
        player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      n_alCSPSetTempoTransition(&musicTracks[arg0].cseqp, 0.0f, 1.0f);
    } else {
      n_alCSPSetTempoTransition(&musicTracks[arg0].cseqp,
                                musicTracks[arg0].unk17C,
                                musicTracks[arg0].unk180);
    }
  }
}

//clang-format off
s32 musicTrack_getIndex(u8 arg0){
    return musicTracks[arg0].index;
}
//clang-format on

void musicTrack_play(u8 arg0, enum comusic_e arg1) {
  s32 sp2C;
  s32 sp24;
  volatile s64 sp20;

  sp2C = musicTracks[arg0].index;
  if (arg1 == sp2C || sp2C == -1) {
    musicTrack_set(arg0, arg1);
  } else {
    musicTrack_set(arg0, -1);
    sp20 = osGetTime();
    while (musicTracks[arg0].cseqp.state != AL_STOPPED) {
      osGetTime();
    };
    musicTrack_unload(sp2C);
    musicTrack_set(arg0, arg1);
  }
}

//clang-format off
s32 musicTrack_getState(u8 arg0){
    return musicTracks[arg0].cseqp.state;
}
//clang-format on

void musicTrack_stopAll(void) {
  s32 i, allStopped;
  volatile s64 sp2C;

  for (i = 0; i < 6; i++) {
    musicTrack_set(i, -1);
  }
  sp2C = osGetTime();

  do {
    allStopped = 0;
    for (i = 0; i < 6; i++) {
      if (musicTrack_getState(i) != AL_STOPPED)
        allStopped++;
    }
    osGetTime();
  } while (allStopped);
}

void musicTrack_setIndexCopy(u8 arg0, enum comusic_e arg1) {
  musicTracks[arg0].index_cpy = arg1;
  musicTracks[arg0].unk2 = 1;
  musicTracks[arg0].unk3 = 0;
  musicTracks[arg0].unk0 = D_musicTrackMeta[arg1].unk4;
}

void musicTrack_resetIndexCopy(u8 arg0) {
  s32 indx;
  indx = musicTracks[arg0].index;
  if (indx == 0x2D || indx == 0x3D) {
    musicTracks[arg0].unk2 = 1;
    musicTracks[arg0].unk3 = 0;
    musicTracks[arg0].index_cpy = musicTracks[arg0].index;
  } else {
    musicTracks[arg0].index_cpy = -1;
    musicTracks[arg0].unk3 = 1;
    musicTracks[arg0].unk2 = 1;
    musicTracks[arg0].unk0 = 0;
  }
}

void musicTrack_setVolumeAndIndexCopy(u8 arg0, s16 arg1) {
  musicTracks[arg0].unk3 = 1;
  musicTracks[arg0].unk2 = 1;
  musicTracks[arg0].unk0 = arg1;
  musicTracks[arg0].index_cpy = musicTracks[arg0].index;
}

// musicTrack_setVolume
void musicTrack_setVolume(u8 arg0, s16 arg1) {
  musicTracks[arg0].unk0 = arg1;
  alCSPSetVol(&musicTracks[arg0].cseqp, arg1);
  if (musicTracks[arg0].unk3 && arg1) {
    musicTrack_setVolumeAndIndexCopy(arg0, arg1);
  } else if (!musicTracks[arg0].unk3 && arg1 == 0) {
    if (musicTrack_isStopped(arg0) == 0)
      musicTrack_resetIndexCopy(arg0);
  }
}

// musicTrack_setTempo
void musicTrack_setTempo(u8 arg0, s32 tempo) {
  if (musicTrack_isStopped(arg0) == 0) {
    if (!musicTracks[arg0].unk2) {
      alCSPSetTempo(&musicTracks[arg0].cseqp, tempo);
    }
  }
}

void musicTrack_setTempoTransition(u8 arg0, f32 arg1, f32 arg2) {
  musicTracks[arg0].unk17C = arg1;
  musicTracks[arg0].unk180 = arg2;
  if (musicTrack_isStopped(arg0) == 0) {
    if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      n_alCSPSetTempoTransition(&musicTracks[arg0].cseqp, 0.0f, 1.0f);
    } else {
      n_alCSPSetTempoTransition(&musicTracks[arg0].cseqp, arg1, arg2);
    }
  }
}

//clang-format off
void musicTrack_getTicks(u8 arg0){
    alCSeqGetTicks(&musicTracks[arg0].cseq);
}
//clang-format on

void musicTrack_update(void) {
  s32 i;

  for (i = 0; i < 6; i++) {
    switch (musicTracks[i].cseqp.state) {
    case AL_PLAYING: // L8024FF94
      if (musicTracks[i].unk2) {
        alCSPStop(&(musicTracks[i].cseqp));

        if (musicTracks[i].unk3)
          musicTracks[i].unk2 = 0;
      }
      break;

    case AL_STOPPED: // L8024FFBC
      if (musicTracks[i].unk2) {
        if (musicTracks[i].unk3) {
          alCSPPlay(&musicTracks[i].cseqp);
        } else {
          musicTrack_play(i, musicTracks[i].index_cpy);
        }
        musicTracks[i].unk3 = 0;
        musicTracks[i].unk2 = 0;
        musicTrack_setVolume(i, musicTracks[i].unk0);
      }
      break;
    case AL_STOPPING: // L80250008
      break;
    }
  }
}

s32 musicTrack_getVolume(enum comusic_e track_id) {
  return D_musicTrackMeta[track_id].unk4;
}

void musicTrack_setVolumeById(enum comusic_e track_id, u16 arg1) {
  D_musicTrackMeta[track_id].unk4 = arg1;
}

// song_getName
char *musicTrack_getName(enum comusic_e track_id) {
  return D_musicTrackMeta[track_id].name;
}

int musicTrack_isStopped(u8 arg0) {
  return (musicTracks[arg0].cseqp.state == AL_STOPPED &&
          musicTracks[arg0].unk3 == 0);
}

s32 musicTrack_getCount(void) { return *(s16 *)&totalMusicTracks; }

N_ALCSPlayer *musicTrack_getPlayer(s32 arg0) {
  return &musicTracks[arg0].cseqp;
}

void musicTrack_noop1(s32 arg0) {}

void musicTrack_noop2(s32 arg0) {}

void musicTrack_setEvent(ALCSeq *arg0, s32 arg1, s32 arg2) {
  u8 i;
  for (i = 0; i < 6; i++) {
    if (arg0 == &musicTracks[i].cseq) {
      musicTracks[i].unk184[arg1 - 0x6A] = 1;
      musicTracks[i].unk192[arg1 - 0x6A] = arg2;
      return;
    }
  }
}

void musicTrack_setEventById(u8 arg0, s32 arg1, s32 arg2) {
  musicTracks[arg0].unk184[arg1 - 0x6A] = arg2;
}

s32 musicTrack_getEvent(u8 arg0, s32 arg1, s32 *arg2) {
  if (arg2 != 0) {
    *arg2 = musicTracks[arg0].unk192[arg1 - 0x6A];
  }
  return musicTracks[arg0].unk184[arg1 - 0x6A];
}

void musicTrack_setChannelVolume(s32 arg0, s16 chan, s16 arg2, f32 arg3) {
  s32 i;
  ALCSPlayer *sp28;
  f32 tmpf;
  s32 mask;

  sp28 = musicTrack_getPlayer(arg0);
  mask = osSetIntMask(OS_IM_NONE);
  tmpf = (!musicTrack_isStopped(arg0)) ? n_alCSPGetChannelState(sp28, chan)
                                       : 127.0f;

  if (arg3 < (2.0f / FRAMERATE)) {
    arg3 = (2.0f / FRAMERATE);
  }

  for (i = 0; i < 0x20; i++) {
    if ((channelVolumes[i].unk8 == channelVolumes[i].unk10) ||
        (channelVolumes[i].unk0 == arg0 && chan == channelVolumes[i].chan)) {
      channelVolumes[i].unk0 = arg0;
      channelVolumes[i].chan = chan;
      channelVolumes[i].unk8 = tmpf;
      channelVolumes[i].unkC = (arg2 - tmpf) / ((arg3 * (float)FRAMERATE) / 2);
      channelVolumes[i].unk10 = arg2;
      osSetIntMask(mask);
      return;
    }
  }
  osSetIntMask(mask);
}

void musicTrack_setTempoSmooth(s32 arg0, s32 arg1, f32 arg2) {
  ALCSPlayer *sp24;
  s32 i;
  s32 sp1C;
  f32 tempo;

  sp24 = musicTrack_getPlayer(arg0);
  sp1C = osSetIntMask(1);
  tempo = alCSPGetTempo(sp24);
  if (arg2 < (2.0f / FRAMERATE)) {
    arg2 = (2.0f / FRAMERATE);
  }
  for (i = 0; i < 0x20; i++) {
    if (channelVolumes[i].unk8 == channelVolumes[i].unk10 ||
        (channelVolumes[i].unk0 == arg0 && -1 == channelVolumes[i].chan)) {
      channelVolumes[i].unk0 = arg0;
      channelVolumes[i].chan = -1;
      channelVolumes[i].unk8 = tempo;
      channelVolumes[i].unkC = (arg1 - tempo) / ((arg2 * (float)FRAMERATE) / 2);
      channelVolumes[i].unk10 = arg1;
      osSetIntMask(sp1C);
      return;
    }
  }
  osSetIntMask(sp1C);
}

u16 musicTrack_getChannelMask(s32 arg0) {
  ALCSPlayer *sp24;
  s32 i;
  s32 sp1C;
  f32 tmpf;

  if (arg0 != 0)
    return ~0;
  currentChannelMask = (currentTempo = -1);
  sp1C = osSetIntMask(1);
  for (i = 0; i < 0x20; i++) {
    channelVolumes[i].unk8 = -1.0f;
    channelVolumes[i].unk10 = -1.0f;
  }
  osSetIntMask(sp1C);
  map_handleMusicEvents();
  if (currentChannelMask == -1) {
    currentChannelMask = 0xFFFF;
  }
  return currentChannelMask;
}

void musicTrack_setChannelMask(s32 arg0, u16 chan_mask, f32 arg2) {
  s32 chan;
  if (currentChannelMask != chan_mask) {
    if (currentChannelMask == -1) {
      arg2 = 0.0f;
    }
    currentChannelMask = chan_mask;
    for (chan = 0; chan < 16; chan++) {
      if (chan_mask & (1 << chan)) {
        musicTrack_setChannelVolume(arg0, chan, 0x7F, arg2);
      } else {
        musicTrack_setChannelVolume(arg0, chan, 0, arg2);
      }
    }
  } // L802505E4
}

void musicTrack_setTempoSmoothById(s32 arg0, s32 arg1, f32 arg2) {
  if (arg1 != currentTempo) {
    if (currentTempo == -1) {
      arg2 = 0.0f;
    }
    currentTempo = arg1;
    musicTrack_setTempoSmooth(arg0, arg1, arg2);
  }
}

void musicTrack_updateChannels(void) {
  N_ALCSPlayer *csplayer;
  s32 i;
  s32 channel;

  for (i = 0; i < 0x20; i++) {
    csplayer = musicTrack_getPlayer(channelVolumes[i].unk0);
    if ((channelVolumes[i].unk8 != channelVolumes[i].unk10) &&
        (musicTrack_isStopped((u8)channelVolumes[i].unk0) == 0)) {
      if (channelVolumes[i].unkC >= 0.0f) {
        channelVolumes[i].unk8 =
            MIN(channelVolumes[i].unk8 + channelVolumes[i].unkC,
                channelVolumes[i].unk10);
      } else {
        channelVolumes[i].unk8 =
            MAX(channelVolumes[i].unk8 + channelVolumes[i].unkC,
                channelVolumes[i].unk10);
      }
      if (channelVolumes[i].chan == -1) {
        alCSPSetTempo(csplayer, (s32)channelVolumes[i].unk8);
      } else {
        n_alCSPSetChannelVolume(csplayer, channelVolumes[i].chan,
                                channelVolumes[i].unk8);
        channel = channelVolumes[i].chan;

        if (((csplayer->chanMask) & (1 << channel))) {
          if (channelVolumes[i].unk8 == 0.0) {
            n_alSeqPlayerSetChannelPressure(csplayer, channelVolumes[i].chan);
          }
        } else {
          if (channelVolumes[i].unk8 != 0.0f) {
            n_alSeqPlayerSetChannelMode(csplayer, channelVolumes[i].chan);
          }
        }
      }
    }
  }
}
