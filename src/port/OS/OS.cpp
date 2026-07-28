// This file should eventually go to LUS as the threading api

#include "OS.h"

#include <map>
#include <mutex>
#include <set>
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

} // namespace

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
    it->second.worker = std::thread(it->second.entry, it->second.arg);
    it->second.worker.detach();
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
