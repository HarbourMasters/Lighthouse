// This file should eventually go to LUS as the RCP api

#include "OS.h"

#include <atomic>

extern "C" {
#include <libultra/rdp.h>
#include "libultraship/libultra/types.h"
#include "libultraship/libultra/sptask.h"
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

// Task submission.
//
// There is no RSP, so this only hands the task over; whoever drains it decides
// what running one means. For the graphics task that has to be the window
// thread, since it ends up in the renderer.
//
// One slot is enough: the decomp starts a task and waits for its SP event
// before starting the next, so a second can never be pending.
namespace {
std::atomic<OSTask*> sPendingTask{ nullptr };
std::atomic<bool> sYieldPending{ false };  // yield requested, not yet observed
std::atomic<bool> sResumePending{ false }; // next gfx StartGo is the resume
} // namespace

extern "C" void osSpTaskYield(void) {
    sYieldPending.store(true, std::memory_order_release);
    OS_JamEventMesg(OS_EVENT_SP);
}

extern "C" OSYieldResult osSpTaskYielded(OSTask* task) {
    (void)task;
    if (sYieldPending.exchange(false, std::memory_order_acq_rel)) {
        sResumePending.store(true, std::memory_order_release);
        return 1;
    }
    return 0;
}

extern "C" void osSpTaskLoad(OSTask* task) {
    (void)task; // no DMEM to load; StartGo carries the pointer
}

extern "C" void osSpTaskStartGo(OSTask* task) {
    if (task->t.type == M_AUDTASK) {
        OS_JamEventMesg(OS_EVENT_SP);
        return;
    }

    if (sResumePending.exchange(false, std::memory_order_acq_rel)) {
        return;
    }
    sPendingTask.store(task, std::memory_order_release);
}

extern "C" OSTask* OS_SpTakePendingTask(void) {
    return sPendingTask.exchange(nullptr, std::memory_order_acq_rel);
}

extern "C" OSTask* OS_SpPeekPendingTask(void) {
    return sPendingTask.load(std::memory_order_acquire);
}
