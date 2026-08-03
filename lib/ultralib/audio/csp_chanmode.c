#include <ultra64.h>
#include "n_synth.h"

void func_8025F5C0(N_ALCSPlayer *seqp, u8 chan) {
    ALEvent evt;

    evt.type = AL_SEQP_MIDI_EVT;
    evt.msg.midi.ticks = 0;
    evt.msg.midi.status = AL_MIDI_ChannelModeSelect;
    evt.msg.midi.byte1 = 0x7E;
    evt.msg.midi.byte2 = chan;
    alEvtqPostEvent(&seqp->evtq, &evt, 0);
}

void func_8025F570(N_ALCSPlayer *seqp, u8 chan) {
    ALEvent evt;

    evt.type = AL_SEQP_MIDI_EVT;
    evt.msg.midi.ticks = 0;
    evt.msg.midi.status = AL_MIDI_ChannelModeSelect;
    evt.msg.midi.byte1 = 0x7F;
    evt.msg.midi.byte2 = chan;
    alEvtqPostEvent(&seqp->evtq, &evt, 0);
}