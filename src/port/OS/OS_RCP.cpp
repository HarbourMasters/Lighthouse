// This file should eventually go to LUS as the RCP api

#include <atomic>

extern "C" {
#include <libultra/rdp.h>
#include "libultraship/libultra/types.h"
}

// The RDP command unit's status register.
//
// Nothing on PC obeys FREEZE, since there is no RDP to stall, but thread5 runs
// its pipeline off that bit: it freezes after handing a frame over and clears
// it on the next retrace, which is what keeps one frame in flight. Holding the
// bit as real state lets that code run as written instead of being replaced.

namespace {
std::atomic<uint32_t> sStatus{ 0 };
}

extern "C" u32 osDpGetStatus(void) {
    return sStatus.load(std::memory_order_acquire);
}

// Writes are set/clear command pairs, mirrored into the bits reads return.
extern "C" void osDpSetStatus(u32 data) {
    uint32_t set = 0;
    uint32_t clr = 0;
    if (data & DPC_SET_FREEZE) {
        set |= DPC_STATUS_FREEZE;
    }
    if (data & DPC_CLR_FREEZE) {
        clr |= DPC_STATUS_FREEZE;
    }
    if (data & DPC_SET_FLUSH) {
        set |= DPC_STATUS_FLUSH;
    }
    if (data & DPC_CLR_FLUSH) {
        clr |= DPC_STATUS_FLUSH;
    }
    if (set != 0) {
        sStatus.fetch_or(set, std::memory_order_acq_rel);
    }
    if (clr != 0) {
        sStatus.fetch_and(~clr, std::memory_order_acq_rel);
    }
}
