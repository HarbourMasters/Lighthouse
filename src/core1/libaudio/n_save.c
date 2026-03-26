#include <ultra64.h>
#include "n_synth.h"
#include "port/audio/mixer.h"

Acmd *n_alMainBusPull(s32 sampleOffset, Acmd *p);

Acmd *n_alSavePull( s32 sampleOffset, Acmd *p) 
{
    Acmd        *ptr = p;

    ptr = n_alMainBusPull(sampleOffset, ptr);
    
#ifndef N_MICRO
    aSetBuffer (ptr++, 0, 0, 0, FIXED_SAMPLE<<1);
    aInterleave(ptr++, AL_MAIN_L_OUT, AL_MAIN_R_OUT);
    aSetBuffer (ptr++, 0, 0, 0, FIXED_SAMPLE<<2);
    aSaveBuffer(ptr++, n_syn->sv_dramout);
#else
    n_aInterleave(ptr++);
    n_aSaveBuffer(ptr++, FIXED_SAMPLE<<2, 0, n_syn->sv_dramout);
#endif
    return ptr;
}