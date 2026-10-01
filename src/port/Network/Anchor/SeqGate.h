#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Monotonic sequence gate for one packet stream: stamps outgoing packets and
// rejects incoming ones that aren't newer than the last accepted.
typedef struct {
    uint32_t outgoing;
    uint32_t lastAccepted;
} SeqGate;

static inline uint32_t SeqGate_Next(SeqGate* gate) {
    return ++gate->outgoing;
}

static inline bool SeqGate_Accept(SeqGate* gate, uint32_t seq) {
    if (seq <= gate->lastAccepted) {
        return false;
    }
    gate->lastAccepted = seq;
    return true;
}

static inline void SeqGate_Reset(SeqGate* gate) {
    gate->outgoing = 0;
    gate->lastAccepted = 0;
}

extern SeqGate gVileSeq;
extern SeqGate gFightSeq;

#ifdef __cplusplus
}
#endif
