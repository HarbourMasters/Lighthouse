#include "synthInternals.h"
#include <ultra64.h>

Acmd *alSavePull(void *filter, s16 *outp, s32 outCount, s32 sampleOffset,
                 Acmd *p)

{
  Acmd *ptr = p;
  ALSave *f = (ALSave *)filter;
  ALFilter *source = f->filter.source;

  ptr = (*source->handler)(source, outp, outCount, sampleOffset, ptr);

  aSetBuffer(ptr++, 0, 0, 0, outCount << 1);
  #ifdef LIGHTHOUSE_P
  ainterleave(ptr++, AL_MAIN_L_OUT, AL_MAIN_R_OUT, 0, 0);
  #else
  aInterleave(ptr++, AL_MAIN_L_OUT, AL_MAIN_R_OUT);
  #endif
  aSetBuffer(ptr++, 0, 0, 0, outCount << 2);
  #ifdef LIGHTHOUSE_P
  aSaveBuffer(ptr++, f->dramout, 0, outCount << 1);
  #else
  aSaveBuffer(ptr++, f->dramout);
  #endif
  return ptr;
}

s32 alSaveParam(void *filter, s32 paramID, void *param) {
  ALSave *a = (ALSave *)filter;
  ALFilter *f = (ALFilter *)filter;
  s32 pp = (s32)param;

  switch (paramID) {
  case (AL_FILTER_SET_SOURCE):
    f->source = (ALFilter *)param;
    break;

  case (AL_FILTER_SET_DRAM):
    a->dramout = pp;
    break;

  default:
    break;
  }
  return 0;
}
