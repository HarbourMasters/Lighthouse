#ifndef __cseq__
#define __cseq__

#ifdef LIGHTHOUSE_P
#include "compat.h"
#else
#include "libaudio.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

char cseq_getNextDelta(ALCSeq *seq, s32 *pDeltaTicks);

#ifdef __cplusplus
}
#endif

#endif /* __cseq__ */
