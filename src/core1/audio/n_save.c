
#include <ultra64.h>
#include "n_synth.h"

#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include "n_abi.h"
#include "abi.h"
#endif


Acmd *n_alSavePull(s32 sampleOffset, Acmd *p) {
  Acmd *ptr = p;

  ptr = n_alMainBusPull(sampleOffset, ptr);

#ifndef N_MICRO
  aSetBuffer(ptr++, 0, 0, 0, FIXED_SAMPLE << 1);
  #ifdef LIGHTHOUSE_P
  aInterleave(ptr++, AL_MAIN_L_OUT, AL_MAIN_R_OUT, 0, 0);
  #else
  aInterleave(ptr++, AL_MAIN_L_OUT, AL_MAIN_R_OUT);
  #endif
  aSetBuffer(ptr++, 0, 0, 0, FIXED_SAMPLE << 2);
  #ifdef LIGHTHOUSE_P
  aSaveBuffer(ptr++, n_syn->sv_dramout, 0, FIXED_SAMPLE << 1);
  #else
  aSaveBuffer(ptr++, n_syn->sv_dramout);
  #endif
#else
#include "n_save_add01.c_"
#endif
  return ptr;
}
