#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_audio.h"
#else
#include "libaudio.h"
#endif

void alCSPPlay(ALCSPlayer *seqp) {
  ALEvent evt;

  evt.type = AL_SEQP_PLAY_EVT;
  alEvtqPostEvent(&seqp->evtq, &evt, 0);
}
