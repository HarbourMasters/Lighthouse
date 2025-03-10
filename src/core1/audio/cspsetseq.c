#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_audio.h"
#else
#include "libaudio.h"
#endif

void alCSPSetSeq(ALCSPlayer *seqp, ALCSeq *seq) {
  ALEvent evt;

  evt.type = AL_SEQP_SEQ_EVT;
  evt.msg.spseq.seq = seq;

  alEvtqPostEvent(&seqp->evtq, &evt, 0);
}
