#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_audio.h"
#else
#include "libaudio.h"
#endif

void alCSPSetVol(ALCSPlayer *seqp, s16 vol) {
  ALEvent evt;

  evt.type = AL_SEQP_VOL_EVT;
  evt.msg.spvol.vol = vol;

  alEvtqPostEvent(&seqp->evtq, &evt, 0);
}
