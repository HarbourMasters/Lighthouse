//placholder for compilation tests
#ifndef PC_AUDIO_H
#define PC_AUDIO_H

#include "functions.h"

#ifdef __cplusplus
extern "C" {
#endif

extern u8 soundfont2ctl_ROM_START[];
extern u8 soundfont2ctl_ROM_END[];
extern u8 soundfont2tbl_ROM_START[];

#define A_SETVOL		9
#define AL_EVTQ_END     0x7fffffff
#define ALVoiceState            N_ALVoiceState
#define AL_BANK_VERSION    0x4231	/* 'B1' */

typedef s32 (*ALDMAproc)(s32 addr, s32 len, void *state);
typedef ALDMAproc (*ALDMANew)(void *state); // placeholder/custom 
typedef s32     ALMicroTime;
typedef u8      ALPan;
typedef ALMicroTime   (*ALOscInit)(void **oscState,f32 *initVal, u8 oscType,
    u8 oscRate, u8 oscDepth, u8 oscDelay);
typedef ALMicroTime   (*ALOscUpdate)(void *oscState, f32 *updateVal);
typedef void          (*ALOscStop)(void *oscState);

typedef struct ALLink_s {
    struct ALLink_s      *next;
    struct ALLink_s      *prev;
} ALLink;

typedef struct {
    ALLink      freeList;
    ALLink      allocList;
    s32         eventCount;
} ALEventQueue;

typedef struct {
    s32         ticks;    /* MIDI, Tempo and End events must start with ticks */
    u8          status;
    u8          byte1; 
    u8          byte2;
    u32         duration;
} ALMIDIEvent;

typedef struct {
    s32         ticks;
    u8          status;
    u8          type;
    u8          len;
    u8          byte1;
    u8          byte2;
    u8          byte3;
} ALTempoEvent;

typedef struct {
    s32         ticks;
    u8          status;
    u8          type;
    u8          len;
} ALEndEvent;

typedef struct {
    struct ALVoice_s    *voice;
} ALNoteEvent;

typedef struct {
    struct ALVoice_s    *voice;
    ALMicroTime         delta;
    u8                  vol;
} ALVolumeEvent;

typedef struct {
    u8          *curPtr;               
    s32         lastTicks;
    s32	       	curTicks;
    s16         lastStatus; 
} ALSeqMarker;

typedef struct {
    ALSeqMarker         *start;
    ALSeqMarker         *end;
    s32                 count;
} ALSeqpLoopEvent;

typedef struct {
    s16                 vol;
} ALSeqpVolEvent;

typedef struct {
    u8			chan;
    u8			priority;
} ALSeqpPriorityEvent;

typedef struct {
    void		*seq;	/* pointer to a seq (could be an ALSeq or an ALCSeq). */
} ALSeqpSeqEvent;

typedef struct {
    ALMicroTime attackTime;
    ALMicroTime decayTime;
    ALMicroTime releaseTime;
    u8          attackVolume;
    u8          decayVolume;
} ALEnvelope;

typedef struct {
    u8          velocityMin;
    u8          velocityMax;
    u8          keyMin;
    u8          keyMax;
    u8          keyBase;
    s8          detune;
} ALKeyMap;

typedef struct {
    u32         start;
    u32         end;
    u32         count;
    ADPCM_STATE state;
} ALADPCMloop;

typedef struct {
    s32 order;
    s32 npredictors;
    s16 book[1];        /* Actually variable size. Must be 8-byte aligned */
} ALADPCMBook;

typedef struct {
    ALADPCMloop *loop;
    ALADPCMBook *book;
} ALADPCMWaveInfo;

typedef struct {
    u32         start;
    u32         end;
    u32         count;
} ALRawLoop;

typedef struct {
    ALRawLoop *loop;
} ALRAWWaveInfo;

typedef struct ALWaveTable_s {
    u8          *base;          /* ptr to start of wave data    */
    s32         len;            /* length of data in bytes      */
    u8          type;           /* compression type             */
    u8          flags;          /* offset/address flags         */
    union {
        ALADPCMWaveInfo adpcmWave;
        ALRAWWaveInfo   rawWave;
    } waveInfo;
} ALWaveTable;

typedef struct ALSound_s {
    ALEnvelope  *envelope;
    ALKeyMap    *keyMap;
    ALWaveTable *wavetable;     /* offset to wavetable struct           */
    ALPan       samplePan;
    u8          sampleVolume;
    u8          flags;
} ALSound;

typedef struct {
    u8          volume;         /* overall volume for this instrument   */
    ALPan       pan;            /* 0 = hard left, 127 = hard right      */
    u8          priority;       /* voice priority for this instrument   */
    u8          flags;
    u8          tremType;       /* the type of tremelo osc. to use      */
    u8          tremRate;       /* the rate of the tremelo osc.         */
    u8          tremDepth;      /* the depth of the tremelo osc         */
    u8          tremDelay;      /* the delay for the tremelo osc        */
    u8          vibType;        /* the type of tremelo osc. to use      */
    u8          vibRate;        /* the rate of the tremelo osc.         */
    u8          vibDepth;       /* the depth of the tremelo osc         */
    u8          vibDelay;       /* the delay for the tremelo osc        */
    s16         bendRange;      /* pitch bend range in cents            */
    s16         soundCount;     /* number of sounds in this array       */
    ALSound     *soundArray[1];
} ALInstrument;

typedef struct ALBank_s {
    s16                 instCount;      /* number of programs in this bank */
    u8                  flags;
    u8                  pad;
    s32                 sampleRate;     /* e.g. 44100, 22050, etc...       */
    ALInstrument        *percussion;    /* default percussion for GM       */
    ALInstrument        *instArray[1];  /* ARRAY of instruments            */
} ALBank;

typedef struct {
    ALBank		*bank;
} ALSeqpBankEvent;

typedef struct {
    struct ALVoiceState_s      *vs;
    void                       *oscState;
    u8                         chan;
} ALOscEvent;

typedef struct {
    float       unk0;
    float       unk4;
} ALUnk18Event;

typedef struct {
    struct struct_81_s * unk0;
    s32       unk4;
} ALUnk_Core1_3A70_Event;

typedef struct {
    s16                 	type;
    union {
        ALMIDIEvent     	midi;
        ALTempoEvent    	tempo;
        ALEndEvent      	end;
        ALNoteEvent     	note;
        ALVolumeEvent   	vol;
        ALSeqpLoopEvent 	loop;
        ALSeqpVolEvent  	spvol;
	    ALSeqpPriorityEvent	sppriority;
	    ALSeqpSeqEvent		spseq;
	    ALSeqpBankEvent		spbank;
        ALOscEvent      	osc;
        ALUnk18Event        unk18;
        ALUnk_Core1_3A70_Event unk3A70;
    } msg;
} ALEvent;

typedef struct {
    ALLink      node;
    ALMicroTime delta;
    ALEvent     evt;
} ALEventListItem;

typedef struct {
    u8          *base;
    u8          *cur;
    s32         len;
    s32         count;
} ALHeap;

typedef ALMicroTime (*ALVoiceHandler)(void *);

typedef struct ALPlayer_s {
    struct ALPlayer_s   *next;
    void                *clientData;    /* storage for client callback  */
    ALVoiceHandler      handler;        /* voice handler for player     */
    ALMicroTime         callTime;       /* usec requested callback      */
    s32                 samplesLeft;    /* usec remaining to callback   */
} ALPlayer;

#define AL_MAX_AUX_BUS_SOURCES       8

typedef u8 ALFxId;

typedef struct {
    s32                 maxVVoices;     /* obsolete */
    s32                 maxPVoices;
    s32                 maxUpdates;
    s32                 maxFXbusses;
    void                *dmaproc;
    ALHeap              *heap;
    s32                 outputRate;     /* output sample rate */
    ALFxId              fxType;
    s32                 *params;
} ALSynConfig;

void    alUnlink(ALLink *element);
void    alLink(ALLink *element, ALLink *after);

typedef ALDMAproc (*ALDMANew)(void *state);
void    alCopy(void *src, void *dest, s32 len);


typedef struct {
    u32      trackOffset[16];
    u32      division;
} ALCMidiHdr;

typedef struct ALCSeq_s {
    ALCMidiHdr    *base;             /* ptr to start of sequence file         */
    u32           validTracks;       /* set of flags, showing valid tracks    */
    f32           qnpt;              /* qrter notes / tick (1/division)       */
    u32           lastTicks;         /* keep track of ticks incase app wants  */
    u32           lastDeltaTicks;    /* number of delta ticks of last event   */
    u32		  deltaFlag;	     /* flag: set if delta's not subtracted   */
    u8            *curLoc[16];       /* ptr to current track location,        */
                                     /* may point to next event, or may point */
                                     /* to a backup code                      */
    u8            *curBUPtr[16];     /* ptr to next event if in backup mode   */
    u8            curBULen[16];      /* if > 0, then in backup mode           */
    u8            lastStatus[16];    /* for running status                    */
    u32           evtDeltaTicks[16]; /* delta time to next event              */
} ALCSeq;

typedef struct N_ALVoice_s {
    ALLink              node;
    struct N_PVoice_s     *pvoice;
    ALWaveTable         *table;
    void                *clientPrivate;
    s16                 state;
    s16                 priority;
    s16                 fxBus;
    s16                 unityPitch;
} N_ALVoice;

typedef struct {
    N_ALVoice    *voice;
} N_ALNoteEvent;

typedef struct {
    N_ALVoice    *voice;
    ALMicroTime         delta;
    u8                  vol;
} N_ALVolumeEvent;

typedef struct {
    struct N_ALVoiceState_s      *vs;
    void                       *oscState;
    u8                         chan;
} N_ALOscEvent;

typedef struct {
    union {
        s32 i;
        f32 f;
    }data[2];
} N_ALGenericEvent;

typedef struct {
    s16                 	type;
    union {
        ALMIDIEvent     	midi;
        ALTempoEvent    	tempo;
        ALEndEvent      	end;
        N_ALNoteEvent     	note;
        N_ALVolumeEvent   	vol;
        ALSeqpLoopEvent 	loop;
        ALSeqpVolEvent  	spvol;
        ALSeqpPriorityEvent	sppriority;
        ALSeqpSeqEvent		spseq;
        ALSeqpBankEvent		spbank;
        N_ALOscEvent      	osc;
        N_ALGenericEvent    generic;
    } msg;
} N_ALEvent;

typedef struct {
    ALInstrument        *instrument;    /* instrument assigned to this chan */
    s16                 bendRange;      /* pitch bend range in cents        */
    ALFxId              fxId;           /* type of fx assigned to this chan */
    ALPan               pan;            /* overall pan for this chan        */
    u8                  priority;       /* priority for this chan           */
    u8                  vol;            /* current volume for this chan     */
    u8                  unkA;
    u8                  fxmix;          /* current fx mix for this chan     */
    u8                  sustain;        /* current sustain pedal state      */
    f32                 pitchBend;      /* current pitch bend val in cents  */
} ALChanState;

typedef struct ALVoice_s {
    ALLink              node;
    struct PVoice_s     *pvoice;
    ALWaveTable         *table;
    void                *clientPrivate;
    s16                 state;
    s16                 priority;
    s16                 fxBus;
    s16                 unityPitch;
} ALVoice;

typedef struct ALVoiceState_s {
    struct ALVoiceState_s *next;/* MUST be first                */
    ALVoice     voice;
    ALSound     *sound;
    ALMicroTime envEndTime;     /* time of envelope segment end */
    f32         pitch;          /* currect pitch ratio          */
    f32         vibrato;        /* current value of the vibrato */
    u8          envGain;        /* current envelope gain        */
    u8          channel;        /* channel assignment           */
    u8          key;            /* note on key number           */
    u8          velocity;       /* note on velocity             */
    u8          envPhase;       /* what envelope phase          */
    u8          phase;
    u8          tremelo;        /* current value of the tremelo */
    u8          flags;          /* bit 0 tremelo flag
                                   bit 1 vibrato flag           */
} ALVoiceState;

typedef struct {
    ALPlayer    *head;          /* client list head                     */
    ALPlayer    *n_seqp1;	/* for fade in / fade out */
    ALPlayer    *n_seqp2;
    ALPlayer    *n_sndp;

    ALLink      pFreeList;      /* list of free physical voices         */
    ALLink      pAllocList;     /* list of allocated physical voices    */
    ALLink      pLameList;      /* list of voices ready to be freed     */
    s32         paramSamples;
    s32         curSamples;     /* samples from start of game           */
    ALDMANew    dma;
    ALHeap      *heap;
    struct ALParam_s    *paramList;
    struct N_ALMainBus_s  *mainBus;
    struct N_ALAuxBus_s   *auxBus;
    s32                 numPVoices;
    s32                 maxAuxBusses;
    s32                 outputRate;
    s32                 maxOutSamples;
    s32	       		sv_dramout;
    s32                 sv_first;
    ALPlayer *unk5C;
    ALPlayer *unk60;
    ALPlayer *unk64;
    ALPlayer *unk68;
    ALPlayer *unk6C;
    ALPlayer *unk70;
} N_ALSynth;

typedef struct {
    ALPlayer            node;           /* note: must be first in structure */
    N_ALSynth          *drvr;           /* reference to the client driver   */
    ALCSeq             *target;         /* current sequence                 */
    ALMicroTime         curTime;
    ALBank             *bank;           /* current ALBank                   */
    s32                 uspt;           /* microseconds per tick            */
    s32                 nextDelta;      /* microseconds to next callback    */
    s32                 state;
    u16                 chanMask;       /* active channels                  */
    s16                 vol;            /* overall sequence volume          */
    u8                  maxChannels;    /* number of MIDI channels          */
    u8                  debugFlags;     /* control which error get reported */
    N_ALEvent           nextEvent;
    ALEventQueue        evtq;
    ALMicroTime         frameTime;
    ALChanState        *chanState;      /* 16 channels for MIDI             */
    N_ALVoiceState     *vAllocHead;     /* list head for allocated voices   */
    N_ALVoiceState     *vAllocTail;     /* list tail for allocated voices   */
    N_ALVoiceState     *vFreeList;      /* list of free voice state structs */
    ALOscInit           initOsc;
    ALOscUpdate         updateOsc;
    ALOscStop           stopOsc;
} N_ALCSPlayer;

typedef struct ALSeq_s {
    u8          *base;                  /* ptr to start of sequence file   */
    u8          *trackStart;            /* ptr to first MIDI event         */
    u8          *curPtr;                /* ptr to next event to read       */
    s32         lastTicks;              /* MIDI ticks for last event       */
    s32         len;                    /* length of sequence in bytes     */
    f32         qnpt;                   /* qrter notes / tick (1/division) */
    s16         division;               /* ticks per quarter note          */
    s16         lastStatus;             /* for running status              */
} ALSeq;

typedef struct {
    ALPlayer            node;          /* note: must be first in structure */
    N_ALSynth          *drvr;          /* reference to the client driver   */
    ALSeq              *target;        /* current sequence                 */
    ALMicroTime         curTime;
    ALBank             *bank;           /* current ALBank                   */
    s32                 uspt;           /* microseconds per tick            */
    s32                 nextDelta;      /* microseconds to next callback    */
    s32                 state;
    u16                 chanMask;       /* active channels                  */
    s16                 vol;            /* overall sequence volume          */
    u8                  maxChannels;    /* number of MIDI channels          */
    u8                  debugFlags;     /* control which error get reported */
    N_ALEvent           nextEvent;
    ALEventQueue        evtq;
    ALMicroTime         frameTime;
    ALChanState        *chanState;      /* 16 channels for MIDI             */
    N_ALVoiceState     *vAllocHead;     /* list head for allocated voices   */
    N_ALVoiceState     *vAllocTail;     /* list tail for allocated voices   */
    N_ALVoiceState     *vFreeList;      /* list of free voice state structs */
    ALOscInit           initOsc;
    ALOscUpdate         updateOsc;
    ALOscStop           stopOsc;
    ALSeqMarker        *loopStart;
    ALSeqMarker        *loopEnd;
    s32                 loopCount;      /* -1 = loop forever, 0 = no loop   */
} N_ALSeqPlayer;

enum ALMsg {
    AL_SEQ_REF_EVT,	/* Reference to a pending event in the sequence. */
    AL_SEQ_MIDI_EVT,
    AL_SEQP_MIDI_EVT,
    AL_TEMPO_EVT,
    AL_SEQ_END_EVT,
    AL_NOTE_END_EVT,
    AL_SEQP_ENV_EVT,
    AL_SEQP_META_EVT,
    AL_SEQP_PROG_EVT,
    AL_SEQP_API_EVT,
    AL_SEQP_VOL_EVT,
    AL_SEQP_LOOP_EVT,
    AL_SEQP_PRIORITY_EVT,
    AL_SEQP_SEQ_EVT,
    AL_SEQP_BANK_EVT,
    AL_SEQP_PLAY_EVT,
    AL_SEQP_STOP_EVT,
    AL_SEQP_STOPPING_EVT,
    AL_TRACK_END,
    AL_CSP_LOOPSTART,
    AL_CSP_LOOPEND,
    AL_CSP_NOTEOFF_EVT,
    AL_TREM_OSC_EVT,
    AL_VIB_OSC_EVT,
    AL_UNK18_EVT
};


enum AL_MIDIstatus {
    /* For distinguishing channel number from status */
    AL_MIDI_ChannelMask         = 0x0F,
    AL_MIDI_StatusMask          = 0xF0,

    /* Channel voice messages */
    AL_MIDI_ChannelVoice        = 0x80,
    AL_MIDI_NoteOff             = 0x80,
    AL_MIDI_NoteOn              = 0x90,
    AL_MIDI_PolyKeyPressure     = 0xA0,
    AL_MIDI_ControlChange       = 0xB0,
    AL_MIDI_ChannelModeSelect   = 0xB0,
    AL_MIDI_ProgramChange       = 0xC0,
    AL_MIDI_ChannelPressure     = 0xD0,
    AL_MIDI_PitchBendChange     = 0xE0,

    /* System messages */
    AL_MIDI_SysEx               = 0xF0, /* System Exclusive */

    /* System common */
    AL_MIDI_SystemCommon            = 0xF1,
    AL_MIDI_TimeCodeQuarterFrame    = 0xF1,
    AL_MIDI_SongPositionPointer     = 0xF2,
    AL_MIDI_SongSelect              = 0xF3,
    AL_MIDI_Undefined1              = 0xF4,
    AL_MIDI_Undefined2              = 0xF5,
    AL_MIDI_TuneRequest             = 0xF6,
    AL_MIDI_EOX                     = 0xF7, /* End of System Exclusive */

    /* System real time */
    AL_MIDI_SystemRealTime  = 0xF8,
    AL_MIDI_TimingClock     = 0xF8,
    AL_MIDI_Undefined3      = 0xF9,
    AL_MIDI_Start           = 0xFA,
    AL_MIDI_Continue        = 0xFB,
    AL_MIDI_Stop            = 0xFC,
    AL_MIDI_Undefined4      = 0xFD,
    AL_MIDI_ActiveSensing   = 0xFE,
    AL_MIDI_SystemReset     = 0xFF,
    AL_MIDI_Meta            = 0xFF      /* MIDI Files only */
};

#define    AL_FX_SMALLROOM     1
#define    AL_FX_BIGROOM       2
#define    AL_FX_CHORUS        3
#define    AL_FX_FLANGE        4
#define    AL_FX_ECHO          5
#define    AL_FX_CUSTOM        6

#define AL_STOPPED      0
#define AL_PLAYING      1

typedef struct {
    ALPlayer    *head;          /* client list head                     */
    ALLink      pFreeList;      /* list of free physical voices         */
    ALLink      pAllocList;     /* list of allocated physical voices    */
    ALLink      pLameList;      /* list of voices ready to be freed     */
    s32         paramSamples;
    s32         curSamples;     /* samples from start of game           */
    ALDMANew    dma;
    ALHeap      *heap;
    
    struct ALParam_s    *paramList;
    
    struct ALMainBus_s  *mainBus;
    struct ALAuxBus_s   *auxBus;        /* ptr to array of aux bus structs */
    struct ALFilter_s   *outputFilter;  /* last filter in the filter chain */

    s32                 numPVoices;
    s32                 maxAuxBusses;
    s32                 outputRate;     /* output sample rate */
    s32                 maxOutSamples;  /* Maximum samples rsp can generate
                                           at one time at output rate */
} ALSynth;

typedef struct {
    ALSynth     drvr;
} ALGlobals;

extern ALGlobals *alGlobals;

enum    {AL_ADPCM_WAVE = 0,
    AL_RAW16_WAVE};

typedef struct {
    ALPlayer            node;           /* note: must be first in structure */
    ALSynth             *drvr;          /* reference to the client driver   */
    ALSeq               *target;        /* current sequence                 */
    ALMicroTime         curTime;
    ALBank              *bank;          /* current ALBank                   */
    s32                 uspt;           /* microseconds per tick            */
    s32                 nextDelta;      /* microseconds to next callback    */
    s32                 state;
    u16                 chanMask;       /* active channels                  */
    s16                 vol;            /* overall sequence volume          */
    u8                  maxChannels;    /* number of MIDI channels          */
    u8                  debugFlags;     /* control which error get reported */
    ALEvent             nextEvent;
    ALEventQueue        evtq;
    ALMicroTime         frameTime;
    ALChanState         *chanState;     /* 16 channels for MIDI             */
    ALVoiceState        *vAllocHead;    /* list head for allocated voices   */
    ALVoiceState        *vAllocTail;    /* list tail for allocated voices   */
    ALVoiceState        *vFreeList;     /* list of free voice state structs */
    ALOscInit           initOsc;
    ALOscUpdate         updateOsc;
    ALOscStop           stopOsc;
    ALSeqMarker         *loopStart;
    ALSeqMarker         *loopEnd;
    s32                 loopCount;      /* -1 = loop forever, 0 = no loop   */
} ALSeqPlayer;

enum AL_MIDImeta {
    AL_MIDI_META_TEMPO          = 0x51,
    AL_MIDI_META_EOT            = 0x2f
};


#define AL_CMIDI_BLOCK_CODE           0xFE
#define AL_CMIDI_LOOPSTART_CODE       0x2E
#define AL_CMIDI_LOOPEND_CODE         0x2D
#define AL_CMIDI_CNTRL_LOOPSTART      102
#define AL_CMIDI_CNTRL_LOOPEND        103
#define AL_CMIDI_CNTRL_LOOPCOUNT_SM   104
#define AL_CMIDI_CNTRL_LOOPCOUNT_BIG  105

typedef struct {
    u32         validTracks;
    s32         lastTicks;
    u32         lastDeltaTicks;
    u8          *curLoc[16];
    u8          *curBUPtr[16];
    u8          curBULen[16];
    u8          lastStatus[16];
    u32         evtDeltaTicks[16];
} ALCSeqMarker;

typedef struct {
    ALPlayer            node;           /* note: must be first in structure */
    ALSynth             *drvr;          /* reference to the client driver   */
    ALCSeq              *target;        /* current sequence                 */
    ALMicroTime         curTime;
    ALBank              *bank;          /* current ALBank                   */
    s32                 uspt;           /* microseconds per tick            */
    s32                 nextDelta;      /* microseconds to next callback    */
    s32                 state;
    u16                 chanMask;       /* active channels                  */
    s16                 vol;            /* overall sequence volume          */
    u8                  maxChannels;    /* number of MIDI channels          */
    u8                  debugFlags;     /* control which error get reported */
    ALEvent             nextEvent;
    ALEventQueue        evtq;
    ALMicroTime         frameTime;
    ALChanState         *chanState;     /* 16 channels for MIDI             */
    ALVoiceState        *vAllocHead;    /* list head for allocated voices   */
    ALVoiceState        *vAllocTail;    /* list tail for allocated voices   */
    ALVoiceState        *vFreeList;     /* list of free voice state structs */
    ALOscInit           initOsc;
    ALOscUpdate         updateOsc;
    ALOscStop           stopOsc;
} ALCSPlayer;

typedef struct {
    s32         maxVoices;         /* max number of voices to alloc    */
    s32         maxEvents;         /* max internal events to support   */
    u8          maxChannels;       /* max MIDI channels to support (16)*/
    u8          debugFlags;        /* control which error get reported */
    ALHeap      *heap;             /* ptr to initialized heap          */
    void        *initOsc;
    void        *updateOsc;
    void        *stopOsc;
} ALSeqpConfig;

#define AL_FX_BUFFER_SIZE       8192
#define AL_FRAME_INIT           -1
#define AL_USEC_PER_FRAME       16000
#define AL_MAX_PRIORITY         127
#define AL_GAIN_CHANGE_TIME     1000    

#define AL_PHASE_ATTACK         0
#define AL_PHASE_NOTEON         0
#define AL_PHASE_DECAY          1
#define AL_PHASE_SUSTAIN        2
#define AL_PHASE_RELEASE        3
#define AL_PHASE_SUSTREL        4

#define AL_STOPPED      0
#define AL_PLAYING      1
#define AL_STOPPING     2

#define AL_DEFAULT_PRIORITY     5
#define AL_DEFAULT_VOICE        0
#define AL_MAX_CHANNELS         16

#define    AL_FX_NONE          0
#define    AL_FX_SMALLROOM     1
#define    AL_FX_BIGROOM       2
#define    AL_FX_CHORUS        3
#define    AL_FX_FLANGE        4
#define    AL_FX_ECHO          5
#define    AL_FX_CUSTOM        6

typedef u8      ALFxId;
typedef void    *ALFxRef;

typedef struct ALVoiceConfig_s {
    s16                 priority;       /* voice priority               */
    s16                 fxBus;          /* bus assignment               */
    u8                  unityPitch;     /* unity pitch flag             */
} ALVoiceConfig;

typedef struct {
    ALLink      node;
    ALMicroTime delta;
    N_ALEvent     evt;
} N_ALEventListItem;

typedef struct {                /* Note: sizeof won't be correct        */
    s16         revision;       /* format revision of this file         */
    s16         bankCount;      /* number of banks                      */
    ALBank      *bankArray[1];  /* ARRAY of bank offsets                */
} ALBankFile;

typedef struct {
    N_ALSynth     drvr;
} N_ALGlobals;

typedef struct struct_81_s{
    ALLink node;
    ALSound *unk8;
    N_ALVoice voice;
    f32 unk28; //pitch?
    f32 unk2C; //vibrato?
    struct struct_81_s **unk30;
    s16 unk34;
    s8 unk36;
    u8 pad37[1];
    s32 unk38;
    u8 envPhase;
    u8 unk3D;
    u8 unk3E;
    u8 unk3F; //key
    u8 unk40;
    u8 pad41[3];
    f32 unk44; //volume
    ALMicroTime unk48; //time_remaining?
    s32 unk4C;
} N_AL_Struct81s;

typedef struct {
    ALPlayer            node;           /* note: must be first in structure */
    ALEventQueue        evtq;
    N_ALEvent           nextEvent;
    N_ALSynth           *drvr;          /* reference to the client driver   */
    s32                 target;
    void                *sndState;
    s32                 maxSounds;
    ALMicroTime         frameTime;
    ALMicroTime         nextDelta;      /* microseconds to next callback    */
    ALMicroTime         curTime;
} N_ALSndPlayer;

typedef struct {
    u16                 	type;
    union {
        ALMIDIEvent     	midi;
        ALTempoEvent    	tempo;
        ALEndEvent      	end;
        N_ALNoteEvent     	note;
        N_ALVolumeEvent   	vol;
        ALSeqpLoopEvent 	loop;
        ALSeqpVolEvent  	spvol;
        ALSeqpPriorityEvent	sppriority;
        ALSeqpSeqEvent		spseq;
        ALSeqpBankEvent		spbank;
        N_ALOscEvent      	osc;
        N_ALGenericEvent    generic;
    } msg;
} N_ALEvent2;

#define NO_SOUND_ERR_MASK          0x01
#define NOTE_OFF_ERR_MASK          0x02
#define NO_VOICE_ERR_MASK          0x04

#define	ERR_ALSEQP_NO_SOUND		100
#define	ERR_ALSEQP_NO_VOICE		101
#define	ERR_ALSEQP_MAP_VOICE		102
#define	ERR_ALSEQP_OFF_VOICE		103
#define	ERR_ALSEQP_POLY_VOICE		104
#define	ERR_ALSNDP_NO_VOICE		105
#define	ERR_ALSYN_NO_UPDATE		106
#define	ERR_ALSNDPDEALLOCATE		107
#define	ERR_ALSNDPDELETE		108
#define	ERR_ALSNDPPLAY			109
#define	ERR_ALSNDPSETSOUND		110
#define ERR_ALSNDPSETPRIORITY           111
#define ERR_ALSNDPSETPAR                112
#define	ERR_ALBNKFNEW			113
#define ERR_ALSEQNOTMIDI                114
#define ERR_ALSEQNOTMIDI0               115
#define ERR_ALSEQNUMTRACKS              116
#define ERR_ALSEQTIME                   117
#define ERR_ALSEQTRACKHDR               118
#define ERR_ALSEQSYSEX                  119
#define ERR_ALSEQMETA                   120
#define ERR_ALSEQPINVALIDPROG           121
#define ERR_ALSEQPUNKNOWNMIDI           122
#define ERR_ALSEQPUNMAP                 123
#define ERR_ALEVENTNOFREE               124
#define ERR_ALHEAPNOFREE                125
#define ERR_ALHEAPCORRUPT               126
#define ERR_ALHEAPFIRSTBLOCK            127
#define ERR_ALCSEQZEROSTATUS		128
#define ERR_ALCSEQZEROVEL		129
#define ERR_ALCSPVNOTFREE		130
#define ERR_ALSEQOVERRUN		131
#define ERR_OSAISETNEXTBUFFER_ENDADDR	132
#define ERR_ALMODDELAYOVERFLOW		133

#define AL_PAN_CENTER   64
#define AL_PAN_LEFT     0
#define AL_PAN_RIGHT    127
#define AL_VOL_FULL     127
#define AL_KEY_MIN      0
#define AL_KEY_MAX      127
#define AL_DEFAULT_FXMIX	0
#define AL_SUSTAIN      63

enum AL_MIDIctrl {
    AL_MIDI_VOLUME_CTRL         = 0x07,
    AL_MIDI_PAN_CTRL            = 0x0A,
    AL_MIDI_PRIORITY_CTRL       = 0x10, /* use general purpose controller for priority */
    AL_MIDI_FX_CTRL_0           = 0x14,
    AL_MIDI_FX_CTRL_1           = 0x15,
    AL_MIDI_FX_CTRL_2           = 0x16,
    AL_MIDI_FX_CTRL_3           = 0x17,
    AL_MIDI_FX_CTRL_4           = 0x18,
    AL_MIDI_FX_CTRL_5           = 0x19,
    AL_MIDI_FX_CTRL_6           = 0x1A,
    AL_MIDI_FX_CTRL_7           = 0x1B,
    AL_MIDI_FX_CTRL_8           = 0x1C,
    AL_MIDI_FX_CTRL_9           = 0x1D,
    AL_MIDI_SUSTAIN_CTRL        = 0x40,
    AL_MIDI_FX1_CTRL            = 0x5B,
    AL_MIDI_FX3_CTRL            = 0x5D
};

#define	A_SPNOOP		0
#define	A_ADPCM			1
#define	A_CLEARBUFF		2
#define	A_ENVMIXER		3
#define	A_LOADBUFF		4
#define	A_RESAMPLE		5
#define A_SAVEBUFF	        6
#define A_SEGMENT		7
#define A_SETBUFF		8
#define A_SETVOL		9
#define A_DMEMMOVE              10
#define A_LOADADPCM             11
#define A_MIXER		        12
#define A_INTERLEAVE            13
#define A_POLEF                 14
#define A_SETLOOP               15

typedef struct {
    u8          *offset;
    s32         len;
} ALSeqData;

typedef struct {                /* Note: sizeof won't be correct        */
    s16         revision;       /* format revision of this file         */
    s16         seqCount;       /* number of sequences                  */
    ALSeqData   seqArray[1];    /* ARRAY of sequence info               */
} ALSeqFile;

void n_alCSPNew(N_ALCSPlayer *seqp, ALSeqpConfig *config);
void alBnkfNew(ALBankFile *file, u8 *table);
void alCSPSetBank(ALCSPlayer *seqp, ALBank *b);
void alCSPStop(ALCSPlayer *seqp);
void n_alCSeqNew(ALCSeq *seq, u8 *ptr);
void alCSPSetSeq(ALCSPlayer *seqp, ALCSeq *seq);
void alCSPPlay(ALCSPlayer *seqp);
void alCSPSetVol(ALCSPlayer *seqp, s16 vol);
void n_alCSPSetTempoTransition(ALCSPlayer *seqp, f32 arg1, f32 arg2);
void alCSPSetTempo(ALCSPlayer *seqp, s32 tempo);
s32 alCSeqGetTicks(ALCSeq *seq);
u8 n_alCSPGetChannelState(N_ALCSPlayer *seqp, u8 chan);
s32 alCSPGetTempo(ALCSPlayer *seqp);
void n_alCSPSetChannelVolume(ALCSPlayer *seqp, u8 chan, u8 arg2);
f32 alCents2Ratio(s32 cents); 
void n_alSeqPlayerSetChannelPressure(N_ALSeqPlayer *arg0, u8 arg1);
void n_alSeqPlayerSetChannelMode(N_ALSeqPlayer *arg0, u8 arg1);
void alHeapInit(ALHeap *hp, u8 *base, s32 len);
void n_alInit(N_ALGlobals *g, ALSynConfig *config);
Acmd *n_alAudioFrame(Acmd *cmdList, s32 *cmdLen, s16 *outBuf, s32 outLen);
void *alHeapDBAlloc(u8 *file, s32 line, ALHeap *hp, s32 num, s32 size);
void alLink(ALLink *ln, ALLink *to);
void alUnlink(ALLink *ln);
void alEvtqNew(ALEventQueue *evtq, ALEventListItem *items, s32 itemCount);
void n_alSynAddSndPlayer(ALPlayer *client);
void alEvtqPostEvent(ALEventQueue *evtq, ALEvent *evt, ALMicroTime delta);
ALMicroTime alEvtqNextEvent(ALEventQueue *evtq, ALEvent *evt);
extern N_ALSynth *n_syn;
s32 n_alSynAllocVoice(N_ALVoice *voice, ALVoiceConfig *vc);
void n_alSynSetVol(N_ALVoice *v, s16 volume, ALMicroTime t);
void n_alSynStartVoice(N_ALVoice *v, ALWaveTable *table);
void n_alSynSetPan(N_ALVoice *v, u8 pan);
void n_alSynSetPitch(N_ALVoice *v, f32 pitch);
void n_alSynSetFXMix(N_ALVoice *v, u8 fxmix);
void n_alSynStopVoice(N_ALVoice *v);
void n_alSynFreeVoice(N_ALVoice *voice);

#ifdef __cplusplus
}
#endif


#endif // PC_AUDIO_H