#include <ultra64.h>
#include <2.0L/PR/n_libaudio.h>

void n_alSynSetPriority(N_ALVoice *voice, s16 priority){
    voice->priority = priority;
}