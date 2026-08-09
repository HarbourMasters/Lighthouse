// This file should eventually replace libultraship's os_mesg.cpp

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>

#include <SDL2/SDL_thread.h>

#include "OS.h"

extern "C" {
#include "libultraship/libultra/message.h"
#include "libultraship/libultra/internal.h"

// Event -> (queue, message) routing table.
// Override from lus os_vi.cpp.
__OSEventState __osEventStateTab[OS_NUM_EVENTS] = { 0 };
}

// Message queues that can actually block.
//
// LUS's queues never do: osRecvMesg on an empty queue just returns -1. Every
// decomp thread is a loop parked on a queue, so they'd spin instead of sleeping
// and nobody would ever get woken.
//
// The queue fields themselves stay as they were, since decomp code reads them
// directly (pfsmanager watches validCount); only the waiting is ours, a
// condvar pair per queue where libultra used priority-ordered thread lists.
// Blocking is opt-in per queue.

namespace {

struct QueueSync {
    std::condition_variable notEmpty;
    std::condition_variable notFull;
    bool blockingEnabled = false;
};

std::mutex sMesgMutex;
std::map<OSMesgQueue*, QueueSync> sQueueSync;

QueueSync& SyncFor(OSMesgQueue* mq) {
    return sQueueSync[mq];
}

// Blocked-wait registry for the watchdog. Slots are atomics so a stall dump
// can read them without taking sMesgMutex (which a deadlocked thread may
// implicate); marking and clearing happen under sMesgMutex on the wait path.
constexpr int kMaxBlockedWaits = 16;
struct BlockedWaitSlot {
    std::atomic<unsigned long> tid{ 0 };
    std::atomic<OSMesgQueue*> mq{ nullptr };
    std::atomic<int> isSend{ 0 };
};
BlockedWaitSlot sBlockedWaits[kMaxBlockedWaits];

int MarkBlockedWait(OSMesgQueue* mq, int isSend) {
    unsigned long tid = (unsigned long)SDL_ThreadID();
    for (int i = 0; i < kMaxBlockedWaits; i++) {
        unsigned long expected = 0;
        if (sBlockedWaits[i].tid.compare_exchange_strong(expected, tid, std::memory_order_acq_rel)) {
            sBlockedWaits[i].mq.store(mq, std::memory_order_release);
            sBlockedWaits[i].isSend.store(isSend, std::memory_order_release);
            return i;
        }
    }
    return -1;
}

void ClearBlockedWait(int slot) {
    if (slot >= 0) {
        sBlockedWaits[slot].mq.store(nullptr, std::memory_order_release);
        sBlockedWaits[slot].tid.store(0, std::memory_order_release);
    }
}

} // namespace

extern "C" {

void osCreateMesgQueue(OSMesgQueue* mq, OSMesg* msgBuf, int32_t count) {
    std::lock_guard<std::mutex> lock(sMesgMutex);
    mq->mtqueue = nullptr;
    mq->fullqueue = nullptr;
    mq->validCount = 0;
    mq->first = 0;
    mq->msgCount = count;
    mq->msg = msgBuf;
    sQueueSync[mq];
}

int32_t osSendMesg(OSMesgQueue* mq, OSMesg msg, int32_t flag) {
    std::unique_lock<std::mutex> lock(sMesgMutex);
    QueueSync& sync = SyncFor(mq);
    int waitSlot = -1;
    while (mq->validCount >= mq->msgCount) {
        if (flag != OS_MESG_BLOCK || !sync.blockingEnabled) {
            ClearBlockedWait(waitSlot);
            return -1;
        }
        if (waitSlot < 0) {
            waitSlot = MarkBlockedWait(mq, 1);
        }
        sync.notFull.wait(lock);
    }
    ClearBlockedWait(waitSlot);
    s32 last = (mq->first + mq->validCount) % mq->msgCount;
    mq->msg[last] = msg;
    mq->validCount++;
    sync.notEmpty.notify_one();
    return 0;
}

// LUS bails when the queue is empty, which has it backwards.
// A jam should fail when there's no room, same as a send.
int32_t osJamMesg(OSMesgQueue* mq, OSMesg msg, int32_t flag) {
    std::unique_lock<std::mutex> lock(sMesgMutex);
    QueueSync& sync = SyncFor(mq);
    int waitSlot = -1;
    while (mq->validCount >= mq->msgCount) {
        if (flag != OS_MESG_BLOCK || !sync.blockingEnabled) {
            ClearBlockedWait(waitSlot);
            return -1;
        }
        if (waitSlot < 0) {
            waitSlot = MarkBlockedWait(mq, 1);
        }
        sync.notFull.wait(lock);
    }
    ClearBlockedWait(waitSlot);
    mq->first = (mq->first + mq->msgCount - 1) % mq->msgCount;
    mq->msg[mq->first] = msg;
    mq->validCount++;
    sync.notEmpty.notify_one();
    return 0;
}

int32_t osRecvMesg(OSMesgQueue* mq, OSMesg* msg, int32_t flag) {
    std::unique_lock<std::mutex> lock(sMesgMutex);
    QueueSync& sync = SyncFor(mq);
    int waitSlot = -1;
    while (mq->validCount == 0) {
        if (flag != OS_MESG_BLOCK || !sync.blockingEnabled) {
            ClearBlockedWait(waitSlot);
            return -1;
        }
        if (waitSlot < 0) {
            waitSlot = MarkBlockedWait(mq, 0);
        }
        sync.notEmpty.wait(lock);
    }
    ClearBlockedWait(waitSlot);
    if (msg != nullptr) {
        *msg = mq->msg[mq->first];
    }
    mq->first = (mq->first + 1) % mq->msgCount;
    mq->validCount--;
    sync.notFull.notify_one();
    return 0;
}

void osSetEventMesg(OSEvent event, OSMesgQueue* mq, OSMesg msg) {
    std::lock_guard<std::mutex> lock(sMesgMutex);
    if (event < OS_NUM_EVENTS) {
        __osEventStateTab[event].queue = mq;
        __osEventStateTab[event].msg = msg;
    }
}

void OS_SetQueueBlocking(OSMesgQueue* mq, int enabled) {
    std::lock_guard<std::mutex> lock(sMesgMutex);
    SyncFor(mq).blockingEnabled = (enabled != 0);
}

// Called once the window closes, before the tick is joined.
void OS_BeginShutdown(void) {
    std::lock_guard<std::mutex> lock(sMesgMutex);
    for (auto& [mq, sync] : sQueueSync) {
        (void)mq;
        sync.blockingEnabled = false;
        sync.notEmpty.notify_all();
        sync.notFull.notify_all();
    }
}

// Lock-free on purpose: the caller is a watchdog inspecting a possible
// deadlock, so it must not touch sMesgMutex. Entries can be mid-update;
// they are diagnostics, not decisions.
int OS_MesgSnapshotBlockedWaits(OS_BlockedWait* out, int max) {
    int n = 0;
    for (int i = 0; i < kMaxBlockedWaits && n < max; i++) {
        unsigned long tid = sBlockedWaits[i].tid.load(std::memory_order_acquire);
        OSMesgQueue* mq = sBlockedWaits[i].mq.load(std::memory_order_acquire);
        if (tid == 0 || mq == nullptr) {
            continue;
        }
        out[n].tid = tid;
        out[n].mq = mq;
        out[n].isSend = sBlockedWaits[i].isSend.load(std::memory_order_acquire);
        n++;
    }
    return n;
}

// Raise a hardware event. Nothing interrupts on PC, so whoever stands in for a
// piece of hardware calls this when the real one would have fired.
void OS_SendEventMesg(OSEvent event) {
    OSMesgQueue* mq = nullptr;
    OSMesg msg = {};
    {
        std::lock_guard<std::mutex> lock(sMesgMutex);
        if (event >= OS_NUM_EVENTS || __osEventStateTab[event].queue == nullptr) {
            return;
        }
        mq = __osEventStateTab[event].queue;
        msg = __osEventStateTab[event].msg;
    }
    osSendMesg(mq, msg, OS_MESG_NOBLOCK);
}

// Same, but the message goes to the front of the queue. An interrupt landed
// ahead of whatever the recipient had queued; a hardware stand-in whose event
// must be seen before pending messages jams instead of sending.
void OS_JamEventMesg(OSEvent event) {
    OSMesgQueue* mq = nullptr;
    OSMesg msg = {};
    {
        std::lock_guard<std::mutex> lock(sMesgMutex);
        if (event >= OS_NUM_EVENTS || __osEventStateTab[event].queue == nullptr) {
            return;
        }
        mq = __osEventStateTab[event].queue;
        msg = __osEventStateTab[event].msg;
    }
    osJamMesg(mq, msg, OS_MESG_NOBLOCK);
}

} // extern "C"
