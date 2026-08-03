#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include <PR/libaudio.h>
// [port] N64 SDK audio library - stubbed for PC port
void func_8025F510(ALCSPlayer *seqp, u8 chan, u8 tempo)
{
    ALEvent       evt;

    evt.type            = AL_SEQP_MIDI_EVT;
    evt.msg.midi.ticks  = 0;
    evt.msg.midi.status = AL_MIDI_ControlChange | chan;
    evt.msg.midi.byte1  = 0x7D;
    evt.msg.midi.byte2  = tempo;
                    
    alEvtqPostEvent(&seqp->evtq, &evt, 0);
}