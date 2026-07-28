// This file should eventually replace libultraship's os_mesg.cpp

#include <condition_variable>
#include <map>
#include <mutex>

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
    while (mq->validCount >= mq->msgCount) {
        if (flag != OS_MESG_BLOCK || !sync.blockingEnabled) {
            return -1;
        }
        sync.notFull.wait(lock);
    }
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
    while (mq->validCount >= mq->msgCount) {
        if (flag != OS_MESG_BLOCK || !sync.blockingEnabled) {
            return -1;
        }
        sync.notFull.wait(lock);
    }
    mq->first = (mq->first + mq->msgCount - 1) % mq->msgCount;
    mq->msg[mq->first] = msg;
    mq->validCount++;
    sync.notEmpty.notify_one();
    return 0;
}

int32_t osRecvMesg(OSMesgQueue* mq, OSMesg* msg, int32_t flag) {
    std::unique_lock<std::mutex> lock(sMesgMutex);
    QueueSync& sync = SyncFor(mq);
    while (mq->validCount == 0) {
        if (flag != OS_MESG_BLOCK || !sync.blockingEnabled) {
            return -1;
        }
        sync.notEmpty.wait(lock);
    }
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

} // extern "C"
