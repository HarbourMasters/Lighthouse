// This file should eventually go to LUS as the timer api

#include "OS.h"

#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>

extern "C" {
#include "libultraship/libultra/time.h"
}

// Deadline timers. LUS's osSetTimer returns 0 without arming anything and it
// has no osStopTimer at all, so a decomp timer never fired. Thread5 uses one
// to pace controller reads.
//
// Countdowns are in 46.875MHz ticks; 1e9/46875000 is exactly 64/3, so
// nanoseconds are ticks * 64 / 3 with no rounding.
namespace {

struct Armed {
    std::chrono::steady_clock::time_point deadline;
    std::chrono::nanoseconds interval;
    OSMesgQueue* mq = nullptr;
    OSMesg msg = {};
};

std::mutex sMutex;
std::map<OSTimer*, Armed> sTimers;
std::condition_variable sCv;
std::thread sWorker;
bool sWorkerStarted = false;
bool sStop = false;

// One worker serves every timer, waking for whichever is due first and
// recomputing whenever the set changes under it.
void Worker() {
    std::unique_lock<std::mutex> lock(sMutex);
    for (;;) {
        if (sStop) {
            return;
        }
        OSTimer* key = nullptr;
        std::chrono::steady_clock::time_point deadline{};
        for (auto& [t, armed] : sTimers) {
            if (key == nullptr || armed.deadline < deadline) {
                key = t;
                deadline = armed.deadline;
            }
        }
        if (key == nullptr) {
            sCv.wait(lock);
            continue;
        }
        if (sCv.wait_until(lock, deadline) != std::cv_status::timeout) {
            continue; // re-armed or stopped while waiting
        }
        if (sStop) {
            return;
        }
        auto it = sTimers.find(key);
        if (it == sTimers.end() || it->second.deadline != deadline) {
            continue; // this one changed under the wait
        }
        OSMesgQueue* mq = it->second.mq;
        OSMesg msg = it->second.msg;
        if (it->second.interval.count() > 0) {
            it->second.deadline += it->second.interval;
        } else {
            sTimers.erase(it);
        }
        lock.unlock();
        osSendMesg(mq, msg, OS_MESG_NOBLOCK);
        lock.lock();
    }
}

} // namespace

extern "C" int osSetTimer(OSTimer* t, OSTime countdown, OSTime interval, OSMesgQueue* mq, OSMesg msg) {
    std::lock_guard<std::mutex> lock(sMutex);
    if (sStop) {
        return 0;
    }
    if (!sWorkerStarted) {
        sWorkerStarted = true;
        sWorker = std::thread(Worker);
    }
    Armed armed;
    armed.deadline = std::chrono::steady_clock::now() + std::chrono::nanoseconds(countdown * 64 / 3);
    armed.interval = std::chrono::nanoseconds(interval * 64 / 3);
    armed.mq = mq;
    armed.msg = msg;
    sTimers[t] = armed;
    sCv.notify_all();
    return 0;
}

extern "C" int osStopTimer(OSTimer* t) {
    std::lock_guard<std::mutex> lock(sMutex);
    sTimers.erase(t);
    sCv.notify_all();
    return 0;
}

extern "C" void OS_StopTimerWorker(void) {
    {
        std::lock_guard<std::mutex> lock(sMutex);
        if (!sWorkerStarted || sStop) {
            return;
        }
        sStop = true;
        sTimers.clear();
    }
    sCv.notify_all();
    if (sWorker.joinable()) {
        sWorker.join();
    }
}
