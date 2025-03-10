#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include "pc_audio.h"
#endif


void alSynDelete(ALSynth *drvr) { drvr->head = 0; }
