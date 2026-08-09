// This file should eventually go to LUS as the threading api

#include "OS.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <set>
#include <spdlog/spdlog.h>
#include <thread>

namespace {

struct OsThreadState {
    void (*entry)(void*) = nullptr;
    void* arg = nullptr;
    OSPri pri = 0;
    OSId id = 0;
    std::thread worker;
    bool started = false;
};

std::mutex sTableMutex;
std::map<OSThread*, OsThreadState> sThreads;
std::set<void*> sEnabledEntries;

bool ThreadEnabled(void* entry) {
    return sEnabledEntries.count(entry) != 0;
}

std::atomic<bool> sExitRequested{ false };

// Counted rather than joined blind: a thread stuck somewhere other than its queue
// wait would never see the request, and join() cannot time out. Waiting on the
// count lets us give up on a straggler instead of hanging the process on it.
std::mutex sExitMutex;
std::condition_variable sExitCv;
int sLiveThreads = 0;

} // namespace

extern "C" void OS_RequestThreadExit(void) {
    sExitRequested.store(true, std::memory_order_release);
}

extern "C" int OS_ThreadShouldExit(void) {
    return sExitRequested.load(std::memory_order_acquire) ? 1 : 0;
}

extern "C" void OS_JoinDecompThreads(void) {
    bool allReturned;
    {
        std::unique_lock<std::mutex> lock(sExitMutex);
        allReturned = sExitCv.wait_for(lock, std::chrono::seconds(2), [] { return sLiveThreads == 0; });
    }

    std::lock_guard<std::mutex> lock(sTableMutex);
    for (auto& [thread, st] : sThreads) {
        (void)thread;
        if (!st.worker.joinable()) {
            continue;
        }
        if (allReturned) {
            st.worker.join();
        } else {
            // Nothing safe left to do: it is still inside game code, so it cannot be
            // joined without hanging and cannot be killed without leaving locks held.
            // Detaching restores the pre-existing behaviour for that one thread.
            st.worker.detach();
        }
    }
    if (!allReturned) {
        SPDLOG_WARN("{} decomp thread(s) did not reach their exit check; detaching", sLiveThreads);
    }
}

extern "C" void OS_EnableThreadEntry(void* entry) {
    std::lock_guard<std::mutex> lock(sTableMutex);
    sEnabledEntries.insert(entry);
}

extern "C" void OS_CreateThread(OSThread* thread, OSId id, void* entry, void* arg, void* sp, OSPri p) {
    (void)sp; // N64 stack pointers are meaningless here
    std::lock_guard<std::mutex> lock(sTableMutex);
    OsThreadState& st = sThreads[thread];
    st.entry = (void (*)(void*))entry;
    st.arg = arg;
    st.pri = p;
    st.id = id;
    st.started = false;
}

extern "C" void OS_StartThread(OSThread* thread) {
    std::lock_guard<std::mutex> lock(sTableMutex);
    auto it = sThreads.find(thread);
    if (it == sThreads.end() || it->second.started || it->second.entry == nullptr) {
        return;
    }
    if (!ThreadEnabled((void*)it->second.entry)) {
        return;
    }
    it->second.started = true;
    void (*entry)(void*) = it->second.entry;
    void* arg = it->second.arg;
    {
        std::lock_guard<std::mutex> exitLock(sExitMutex);
        sLiveThreads++;
    }
    // Kept joinable: shutdown waits for these to leave their loops before the
    // engine they draw and play audio through is destroyed.
    it->second.worker = std::thread([entry, arg]() {
        entry(arg);
        {
            std::lock_guard<std::mutex> exitLock(sExitMutex);
            sLiveThreads--;
        }
        sExitCv.notify_all();
    });
}

extern "C" void OS_StopThread(OSThread* thread) {
    // N64 semantics are preemptive suspension, which std::thread cannot do.
    // Revived consumers must exit cooperatively; Stop just joins if running.
    OS_DestroyThread(thread);
}

extern "C" void OS_DestroyThread(OSThread* thread) {
    std::thread worker;
    {
        std::lock_guard<std::mutex> lock(sTableMutex);
        auto it = sThreads.find(thread);
        if (it == sThreads.end()) {
            return;
        }
        worker = std::move(it->second.worker);
        sThreads.erase(it);
    }
    if (worker.joinable()) {
        worker.join();
    }
}

// Priority is recorded but not applied: the N64 scheduler was cooperative and
// priority-ordered, while these are preemptive OS threads. What the game needs
// from priority is queue ordering and blocking behavior, which the queues
// provide; mapping it onto OS thread priorities would only add scheduler noise.
extern "C" void OS_SetThreadPri(OSThread* thread, OSPri p) {
    std::lock_guard<std::mutex> lock(sTableMutex);
    auto it = sThreads.find(thread);
    if (it != sThreads.end()) {
        it->second.pri = p;
    }
}
