#ifdef LIGHTHOUSE_P

#include "pc_audio.h"

#ifdef __cplusplus
extern "C" {
#endif

void n_alCSPNew(N_ALCSPlayer *seqp, ALSeqpConfig *c){}
void alBnkfNew(ALBankFile *file, u8 *table) {}
void alCSPSetBank(ALCSPlayer *seqp, ALBank *b) {}
void alCSPStop(ALCSPlayer *seqp){}
void n_alCSeqNew(ALCSeq *seq, u8 *ptr){}
void alCSPSetSeq(ALCSPlayer *seqp, ALCSeq *seq){}
void alCSPPlay(ALCSPlayer *seqp) {}
void alCSPSetVol(ALCSPlayer *seqp, s16 vol){}
void n_alCSPSetTempoTransition(ALCSPlayer *seqp, f32 arg1, f32 arg2){}
void alCSPSetTempo(ALCSPlayer *seqp, s32 tempo){}
void n_alSeqPlayerSetChannelPressure(N_ALSeqPlayer *arg0, u8 arg1){}
void n_alSeqPlayerSetChannelMode(N_ALSeqPlayer *arg0, u8 arg1){}
void alHeapInit(ALHeap *hp, u8 *base, s32 len){}
void n_alInit(N_ALGlobals *g, ALSynConfig *config){}
Acmd *n_alAudioFrame(Acmd *cmdList, s32 *cmdLen, s16 *outBuf, s32 outLen){return 0;}
void *alHeapDBAlloc(u8 *file, s32 line, ALHeap *hp, s32 num, s32 size){}
void alLink(ALLink *ln, ALLink *to){}
void alUnlink(ALLink *ln){}
void alEvtqNew(ALEventQueue *evtq, ALEventListItem *items, s32 itemCount){}
s32 alCSeqGetTicks(ALCSeq *seq) { return 0;}
u8 n_alCSPGetChannelState(N_ALCSPlayer *seqp, u8 chan) { return 0;}
s32 alCSPGetTempo(ALCSPlayer *seqp) { return 0;}
void n_alCSPSetChannelVolume(ALCSPlayer *seqp, u8 chan, u8 arg2) {}
f32 alCents2Ratio(s32 cents) { return 0;}
void n_alSynAddSndPlayer(ALPlayer *client){}
void alEvtqPostEvent(ALEventQueue *evtq, ALEvent *evt, ALMicroTime delta){}
ALMicroTime alEvtqNextEvent(ALEventQueue *evtq, ALEvent *evt){return 0;}
s32 n_alSynAllocVoice(N_ALVoice *voice, ALVoiceConfig *vc) { return 0; }
void n_alSynSetVol(N_ALVoice *v, s16 volume, ALMicroTime t){}
void n_alSynStartVoice(N_ALVoice *v, ALWaveTable *table){}
void n_alSynSetPan(N_ALVoice *v, u8 pan){}
void n_alSynSetPitch(N_ALVoice *v, f32 pitch){}
void n_alSynSetFXMix(N_ALVoice *v, u8 fxmix){}
void n_alSynStopVoice(N_ALVoice *v){}
void n_alSynFreeVoice(N_ALVoice *voice){}
N_ALSynth *n_syn = NULL;


#ifdef __cplusplus
}
#endif

#endif