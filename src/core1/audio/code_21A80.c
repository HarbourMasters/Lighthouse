#include "functions.h"

#include "n_synth.h"
#include "variables.h"
#include <ultra64.h>

u8 n_alCSPGetChannelState(N_ALCSPlayer *seqp, u8 chan) {
  return seqp->chanState[chan].unkA;
}
