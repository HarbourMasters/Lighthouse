#include <ultra64.h>
#include <PR/libaudio.h>

void alSynDelete(ALSynth *drvr)
{
    drvr->head = 0;
}