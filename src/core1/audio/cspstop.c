#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_audio.h"
#else
#include "libaudio.h"
#endif

void alCSPStop(ALCSPlayer *seqp) {
  ALEvent evt;

  evt.type = AL_SEQP_STOPPING_EVT;
  alEvtqPostEvent(&seqp->evtq, &evt, 0);
}
