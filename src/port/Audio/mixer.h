#ifndef MIXER_H
#define MIXER_H

#include <stdint.h>
#include <ultra64.h>

#undef aSegment
#undef aClearBuffer
#undef aSetBuffer
#undef aLoadBuffer
#undef aSaveBuffer
#undef aDMEMMove
#undef aMix
#undef aEnvMixer
#undef aResample
#undef aInterleave
#undef aSetVolume
#undef aSetVolume32
#undef aSetLoop
#undef aLoadADPCM
#undef aADPCMdec
#undef aPoleFilter

void aClearBufferImpl(uint16_t addr, int nbytes);
void aLoadADPCMImpl(int num_entries_times_16, const int16_t* book_source_addr);
void aDMEMMoveImpl(uint16_t in_addr, uint16_t out_addr, int nbytes);
void aSetLoopImpl(ADPCM_STATE* adpcm_loop_state);
void aADPCMdecImpl(uint8_t flags, ADPCM_STATE state, int nbytes, uint16_t inofs, uint16_t outofs);
void aResampleImpl(uint8_t flags, uint16_t pitch, RESAMPLE_STATE state, uint16_t inofs, uint8_t outflag);
void aLoadBufferImpl(const void* source_addr, uint16_t dest_addr, uint16_t nbytes);
void aSaveBufferImpl(uint16_t source_addr, int16_t* dest_addr, uint16_t nbytes);
void aInterleaveImpl(void);
void aMixImpl(uint8_t flags, int16_t gain, uint16_t in_addr, uint16_t out_addr);
void aEnvMixerImpl(uint8_t flags, ENVMIX_STATE state, int16_t some_vol);
void aSetVolumeImpl(uint8_t flags, int16_t v, int16_t t, int16_t r);
void aPoleFilterImpl(uint8_t flags, uint16_t gain, uint8_t dmem_shift, void* state);
void aDisableImpl(uint16_t outp, uint32_t b, uint32_t c);

#define aDisable(pkt, o, b, c) ((void)(pkt), aDisableImpl(o, b, c))
#define aClearBuffer(pkt, d, c) ((void)(pkt), aClearBufferImpl(d, c))
#define aLoadBuffer(pkt, c, d, s) ((void)(pkt), aLoadBufferImpl((void*)(s), d, c))
#define aSaveBuffer(pkt, c, s, d) ((void)(pkt), aSaveBufferImpl(s, (int16_t*)(d), c))
#define aLoadADPCM(pkt, c, d) ((void)(pkt), aLoadADPCMImpl(c, (int16_t*)(d)))
#define aDMEMMove(pkt, i, o, c) ((void)(pkt), aDMEMMoveImpl(i, o, c))
#define aSetLoop(pkt, a) ((void)(pkt), aSetLoopImpl((void*)(a)))
#define aADPCMdec(pkt, s, f, c, i, o) ((void)(pkt), aADPCMdecImpl(f, (void*)(s), c, i, o))
#define aResample(pkt, s, f, p, i, o) ((void)(pkt), aResampleImpl(f, p, (void*)(s), i, o))
#define aInterleave(pkt) ((void)(pkt), aInterleaveImpl())
#define aMix(pkt, f, g, i, o) ((void)(pkt), aMixImpl(f, g, i, o))
#define aEnvMixer(pkt, f, t, s) ((void)(pkt), aEnvMixerImpl(f, (void*)(s), t))
#define aSetVolume(pkt, f, v, t, r) ((void)(pkt), aSetVolumeImpl(f, v, t, r))
#define aPoleFilter(pkt, f, g, t, s) ((void)(pkt), aPoleFilterImpl(f, g, t, s))
#define aPlayMP3(pkt, a, b, c, r) ((void)(pkt), aPlayMP3Impl((void*)(a), b, (void*)(c), r))

#endif
