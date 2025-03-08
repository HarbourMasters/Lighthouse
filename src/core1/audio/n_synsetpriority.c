
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_audio.h"
#else
#include "n_libaudio.h"
#endif

void n_alSynSetPriority(N_ALVoice *voice, s16 priority) {
  voice->priority = priority;
}
