#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_audio.h"
#else
#include "libaudio.h"
#endif


void n_alCSPSetTempoTransition(ALCSPlayer *seqp, f32 arg1, f32 arg2) {
  ALEvent evt;

  evt.type = AL_UNK18_EVT; // event type not listed;
  evt.msg.unk18.unk0 = arg1;
  evt.msg.unk18.unk4 = arg2;
  alEvtqPostEvent(&seqp->evtq, &evt, 0);
}
