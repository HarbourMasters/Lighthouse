#include "n_synth.h"
#include <ultra64.h>

void n_alSeqPlayerSetChannelPressure(N_ALSeqPlayer *arg0, u8 arg1) {
  ALEvent evt;

  evt.type = AL_SEQP_MIDI_EVT;
  evt.msg.midi.ticks = 0;
  evt.msg.midi.status = AL_MIDI_ChannelModeSelect;
  evt.msg.midi.byte1 = 0x7E;
  evt.msg.midi.byte2 = arg1;
  alEvtqPostEvent(&arg0->evtq, &evt, 0);
}

void n_alSeqPlayerSetChannelMode(N_ALSeqPlayer *arg0, u8 arg1) {
  ALEvent evt;

  evt.type = AL_SEQP_MIDI_EVT;
  evt.msg.midi.ticks = 0;
  evt.msg.midi.status = AL_MIDI_ChannelModeSelect;
  evt.msg.midi.byte1 = 0x7F;
  evt.msg.midi.byte2 = arg1;
  alEvtqPostEvent(&arg0->evtq, &evt, 0);
}
