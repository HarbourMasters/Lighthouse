#include <ultra64.h>
#include "2.0L/PR/libaudio.h"

void alCSPStop(ALCSPlayer *seqp)
{
    ALEvent     evt;

    evt.type = AL_SEQP_STOPPING_EVT;                    
    alEvtqPostEvent(&seqp->evtq, &evt, 0);
}