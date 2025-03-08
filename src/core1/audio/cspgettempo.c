#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_audio.h"
#else
#include "libaudio.h"
#endif

s32 alCSPGetTempo(ALCSPlayer *seqp) {
  if (seqp->target == NULL)
    return 0;
  return seqp->uspt / seqp->target->qnpt;
}
