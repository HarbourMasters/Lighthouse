#include <ultra64.h>
#include "2.0L/PR/libaudio.h"

void alSynDelete(ALSynth *drvr)
{
    drvr->head = 0;
}