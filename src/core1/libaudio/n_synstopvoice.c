#include <ultra64.h>
#include "n_synth.h"
#include "2.0L/PR/ultraerror.h"
//#include "2.0L/PR/os_error.h"

void    n_alSynStopVoice( N_ALVoice *v)
{
    ALParam  *update;
    
    if (v->pvoice) {
        
        update = __n_allocParam();
        ALFailIf(update == 0, ERR_ALSYN_NO_UPDATE);

        update->delta  = n_syn->paramSamples + v->pvoice->offset;
        update->type   = AL_FILTER_STOP_VOICE;
        update->next   = 0;

        n_alEnvmixerParam(v->pvoice, AL_FILTER_ADD_UPDATE, update);        
    }
}