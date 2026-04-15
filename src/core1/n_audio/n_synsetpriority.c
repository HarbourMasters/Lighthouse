#include <ultra64.h>
#include <n_audio/PR/n_libaudio.h>

void n_alSynSetPriority(N_ALVoice *voice, s16 priority){
    voice->priority = priority;
}