#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_audio.h"
#else
#include "libaudio.h"
#endif

void alCSPSetBank(ALCSPlayer *seqp, ALBank *b) {
  ALEvent evt;

  evt.type = AL_SEQP_BANK_EVT;
  evt.msg.spbank.bank = b;

  alEvtqPostEvent(&seqp->evtq, &evt, 0);
}
