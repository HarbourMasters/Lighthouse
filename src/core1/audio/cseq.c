#include <ultra64.h>

#include "cseq.h"

static u32 cseq_readVarLen(ALCSeq *s, u32 track);
static u8 cseq_getTrackByte(ALCSeq *s, u32 track);
static u32 cseq_getTrackEvent(ALCSeq *seq, u32 track, ALEvent *event);

void alCSeqNew(ALCSeq *seq, u8 *ptr) {
  u32 i, tmpOff, flagTmp;

  /* load the seqence pointed to by ptr   */
  seq->base = (ALCMidiHdr *)ptr;
  seq->validTracks = 0;
  seq->lastDeltaTicks = 0;
  seq->lastTicks = 0;
  seq->deltaFlag = 1;

  for (i = 0; i < 16; i++) {
    seq->lastStatus[i] = 0;
    seq->curBUPtr[i] = 0;
    seq->curBULen[i] = 0;
    tmpOff = seq->base->trackOffset[i];
    if (tmpOff) /* if the track is valid */
    {
      flagTmp = 1 << i;
      seq->validTracks |= flagTmp;
      seq->curLoc[i] = (u8 *)((u32)ptr + tmpOff);
      seq->evtDeltaTicks[i] = cseq_readVarLen(seq, i);
      /*cseq_getTrackEvent(seq,i); prime the event buffers  */
    } else
      seq->curLoc[i] = 0;
  }

  seq->qnpt = 1.0 / (f32)seq->base->division;
}

void alCSeqNextEvent(ALCSeq *seq, ALEvent *evt) {
  u32 i;
  u32 firstTime = 0xFFFFFFFF;
  u32 firstTrack;
  u32 lastTicks = seq->lastDeltaTicks;

#ifdef _DEBUG
  /* sct 1/17/96 - Warn if we are beyond the end of sequence. */
  if (!seq->validTracks)
    __osError(ERR_ALSEQOVERRUN, 0);
#endif

  for (i = 0; i < 16; i++) {
    if ((seq->validTracks >> i) & 1) {
      if (seq->deltaFlag)
        seq->evtDeltaTicks[i] -= lastTicks;
      if (seq->evtDeltaTicks[i] < firstTime) {
        firstTime = seq->evtDeltaTicks[i];
        firstTrack = i;
      }
    }
  }

  cseq_getTrackEvent(seq, firstTrack, evt);

  evt->msg.midi.ticks = firstTime;
  seq->lastTicks += firstTime;
  seq->lastDeltaTicks = firstTime;
  if (evt->type != AL_TRACK_END)
    seq->evtDeltaTicks[firstTrack] += cseq_readVarLen(seq, firstTrack);
  seq->deltaFlag = 1;
}

/*
  Note: If there are no valid tracks (ie. all tracks have
  reached the end of their data stream), then return FALSE
  to indicate that there is no next event.
*/
char cseq_getNextDelta(ALCSeq *seq, s32 *pDeltaTicks) {
  u32 i;
  u32 firstTime = 0xFFFFFFFF;
  u32 lastTicks = seq->lastDeltaTicks;

  if (!seq->validTracks)
    return FALSE;

  for (i = 0; i < 16; i++) {
    if ((seq->validTracks >> i) & 1) {
      if (seq->deltaFlag)
        seq->evtDeltaTicks[i] -= lastTicks;

      if (seq->evtDeltaTicks[i] < firstTime)
        firstTime = seq->evtDeltaTicks[i];
    }
  }

  seq->deltaFlag = 0;
  *pDeltaTicks = firstTime;

  return TRUE;
}

/* only call alCSeqGetTrackEvent with a valid track !! */
static u32 cseq_getTrackEvent(ALCSeq *seq, u32 track, ALEvent *event) {
  u32 offset;
  u8 status, loopCt, curLpCt, *tmpPtr;

  status = cseq_getTrackByte(seq, track); /* read the status byte */

  if (status == AL_MIDI_Meta) /* running status not allowed on meta events!! */
  {
    u8 type = cseq_getTrackByte(seq, track);

    if (type == AL_MIDI_META_TEMPO) {
      event->type = AL_TEMPO_EVT;
      event->msg.tempo.status = status;
      event->msg.tempo.type = type;
      event->msg.tempo.byte1 = cseq_getTrackByte(seq, track);
      event->msg.tempo.byte2 = cseq_getTrackByte(seq, track);
      event->msg.tempo.byte3 = cseq_getTrackByte(seq, track);
      seq->lastStatus[track] = 0; /* lastStatus not supported after meta */
    } else if (type == AL_MIDI_META_EOT) {
      u32 flagMask;

      flagMask = 0x01 << track;
      seq->validTracks = seq->validTracks ^ flagMask;

      if (seq->validTracks) /* there is music left don't end */
        event->type = AL_TRACK_END;
      else /* no more music send AL_SEQ_END_EVT msg */
        event->type = AL_SEQ_END_EVT;
    } else if (type == AL_CMIDI_LOOPSTART_CODE) {
      status =
          cseq_getTrackByte(seq, track); /* get next two bytes, ignore them */
      status = cseq_getTrackByte(seq, track);
      seq->lastStatus[track] = 0;
      event->type = AL_CSP_LOOPSTART;
    } else if (type == AL_CMIDI_LOOPEND_CODE) {
      tmpPtr = seq->curLoc[track];
      loopCt = *tmpPtr++;
      curLpCt = *tmpPtr;
      if (curLpCt == 0) /* done looping */
      {
        *tmpPtr = loopCt;                /* reset current loop count */
        seq->curLoc[track] = tmpPtr + 5; /* move pointer to end of event */
      } else {
        if (curLpCt != 0xFF)     /* not a loop forever */
          *tmpPtr = curLpCt - 1; /* decrement current loop count */
        tmpPtr++;                /* get offset from end of event */
        offset = (*tmpPtr++) << 24;
        offset += (*tmpPtr++) << 16;
        offset += (*tmpPtr++) << 8;
        offset += *tmpPtr++;
        seq->curLoc[track] = tmpPtr - offset;
      }
      seq->lastStatus[track] = 0;
      event->type = AL_CSP_LOOPEND;
    }

#ifdef _DEBUG
    else
      __osError(ERR_ALSEQMETA, 1, type);
#endif

  } else {
    event->type = AL_SEQ_MIDI_EVT;
    if (status & 0x80) /* if high bit is set, then new status */
    {
      event->msg.midi.status = status;
      event->msg.midi.byte1 = cseq_getTrackByte(seq, track);
      seq->lastStatus[track] = status;
    } else /* running status */
    {
#ifdef _DEBUG
      if (seq->lastStatus[track] == 0)
        __osError(ERR_ALCSEQZEROSTATUS, 1, track);
#endif
      event->msg.midi.status = seq->lastStatus[track];
      event->msg.midi.byte1 = status;
    }

    if (((event->msg.midi.status & 0xf0) != AL_MIDI_ProgramChange) &&
        ((event->msg.midi.status & 0xf0) != AL_MIDI_ChannelPressure)) {
      event->msg.midi.byte2 = cseq_getTrackByte(seq, track);
      if ((event->msg.midi.status & 0xf0) == AL_MIDI_NoteOn) {
        event->msg.midi.duration = cseq_readVarLen(seq, track);
#ifdef _DEBUG
        if (event->msg.midi.byte2 == 0)
          __osError(ERR_ALCSEQZEROVEL, 1, track);
#endif
      }
    } else
      event->msg.midi.byte2 = 0;
  }
  return TRUE;
}

f32 alCSeqTicksToSec(ALCSeq *seq, s32 ticks, u32 tempo) {
  return ((f32)(((f32)(ticks) * (f32)(tempo)) /
                ((f32)(seq->base->division) * 1000000.0)));
}

u32 alCSeqSecToTicks(ALCSeq *seq, f32 sec, u32 tempo) {
  return (u32)(((sec * 1000000.0) * seq->base->division) / tempo);
}

s32 alCSeqGetTicks(ALCSeq *seq) { return seq->lastTicks; }

void alCSeqNewMarker(ALCSeq *seq, ALCSeqMarker *m, u32 ticks) {
  ALEvent evt;
  ALCSeq tempSeq;
  s32 i;

  alCSeqNew(&tempSeq, (u8 *)seq->base);

  do {
    m->validTracks = tempSeq.validTracks;
    m->lastTicks = tempSeq.lastTicks;
    m->lastDeltaTicks = tempSeq.lastDeltaTicks;

    for (i = 0; i < 16; i++) {
      m->curLoc[i] = tempSeq.curLoc[i];
      m->curBUPtr[i] = tempSeq.curBUPtr[i];
      m->curBULen[i] = tempSeq.curBULen[i];
      m->lastStatus[i] = tempSeq.lastStatus[i];
      m->evtDeltaTicks[i] = tempSeq.evtDeltaTicks[i];
    }

    alCSeqNextEvent(&tempSeq, &evt);

    if (evt.type == AL_SEQ_END_EVT)
      break;

  } while (tempSeq.lastTicks < ticks);
}

void alCSeqSetLoc(ALCSeq *seq, ALCSeqMarker *m) {
  s32 i;

  seq->validTracks = m->validTracks;
  seq->lastTicks = m->lastTicks;
  seq->lastDeltaTicks = m->lastDeltaTicks;

  for (i = 0; i < 16; i++) {
    seq->curLoc[i] = m->curLoc[i];
    seq->curBUPtr[i] = m->curBUPtr[i];
    seq->curBULen[i] = m->curBULen[i];
    seq->lastStatus[i] = m->lastStatus[i];
    seq->evtDeltaTicks[i] = m->evtDeltaTicks[i];
  }
}

void alCSeqGetLoc(ALCSeq *seq, ALCSeqMarker *m) {
  s32 i;

  m->validTracks = seq->validTracks;
  m->lastTicks = seq->lastTicks;
  m->lastDeltaTicks = seq->lastDeltaTicks;

  for (i = 0; i < 16; i++) {
    m->curLoc[i] = seq->curLoc[i];
    m->curBUPtr[i] = seq->curBUPtr[i];
    m->curBULen[i] = seq->curBULen[i];
    m->lastStatus[i] = seq->lastStatus[i];
    m->evtDeltaTicks[i] = seq->evtDeltaTicks[i];
  }
}

/* non-aligned byte reading routines */
static u8 cseq_getTrackByte(ALCSeq *seq, u32 track) {
  u8 theByte;

  if (seq->curBULen[track]) {
    theByte = *seq->curBUPtr[track];
    seq->curBUPtr[track]++;
    seq->curBULen[track]--;
  } else /* need to handle backup mode */
  {
    theByte = *seq->curLoc[track];
    seq->curLoc[track]++;
    if (theByte == AL_CMIDI_BLOCK_CODE) {
      u8 loBackUp, hiBackUp, theLen, nextByte;
      u32 backup;

      nextByte = *seq->curLoc[track];
      seq->curLoc[track]++;
      if (nextByte != AL_CMIDI_BLOCK_CODE) {
        /* if here, then got a backup section. get the amount of
           backup, and the len of the section. Subtract the amount of
           backup from the curLoc ptr, and subtract four more, since
           curLoc has been advanced by four while reading the codes. */
        hiBackUp = nextByte;
        loBackUp = *seq->curLoc[track];
        seq->curLoc[track]++;
        theLen = *seq->curLoc[track];
        seq->curLoc[track]++;
        backup = (u32)hiBackUp;
        backup = backup << 8;
        backup += loBackUp;
        seq->curBUPtr[track] = seq->curLoc[track] - (backup + 4);
        seq->curBULen[track] = (u32)theLen;

        /* now get the byte */
        theByte = *seq->curBUPtr[track];
        seq->curBUPtr[track]++;
        seq->curBULen[track]--;
      }
    }
  }

  return theByte;
}

static u32 cseq_readVarLen(ALCSeq *seq, u32 track) {
  u32 value;
  u32 c;

  value = (u32)cseq_getTrackByte(seq, track);
  if (value & 0x00000080) {
    value &= 0x7f;
    do {
      c = (u32)cseq_getTrackByte(seq, track);
      value = (value << 7) + (c & 0x7f);
    } while (c & 0x80);
  }
  return (value);
}
