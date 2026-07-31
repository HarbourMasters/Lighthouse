#include "ThreadWatchdog.h"

#include <atomic>
#include <chrono>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <cstring>
#include <string>
#include <thread>

#include <SDL2/SDL_thread.h>

#include "port/OS/OS.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#define LH_WATCHDOG_STACKS 1
#endif

extern "C" {
#include "libultraship/libultra/types.h"
#include "libultraship/libultra/message.h"
u32 osDpGetStatus(void);
void* osViGetCurrentFramebuffer(void);
void* osViGetNextFramebuffer(void);
OSMesgQueue* thread5_getTaskQueue(void);
OSMesgQueue* thread5_getSyncQueue(void);
OSMesgQueue* pfsManager_getFrameMesgQ(void);
OSMesgQueue* pfsManager_getFrameReplyQ(void);
OSMesgQueue* pfsManager_getSiLockQueue(void);
int pfsManager_isBusy(void);
OSMesgQueue* audioManager_getFrameMesgQueue(void);
OSMesgQueue* audioManager_getReplyMesgQueue(void);
OSMesgQueue* baMotor_getRetraceQueue(void);
OSMesgQueue* viMgr_getRetraceQueue(void);
OSMesgQueue* viMgr_getFrameTokenQueue(void);
OSMesgQueue* viMgr_getTickRetraceQueue(void);
int port_audioHeld(void);
int port_renderServicePending(void);
// Repro context
int gsworld_getMap(void);
int gsworld_getExit(void);
int getGameMode(void);
extern char gBuildVersion[];
extern char gGitBranch[];
extern char gGitCommitHash[];
}

namespace {

using Clock = std::chrono::steady_clock;

constexpr const char* kThreadNames[WATCHDOG_NUM_THREADS] = {
    "vi-ticker", "main-loop", "game-tick", "thread5", "vimgr", "pfsmanager", "audio-mgr", "rumble",
};

// Blocking points in each loop that are not message-queue waits. A stall with
// no queue park is in one of these; naming them gives the reader somewhere to
// look when no symbols were available to resolve a stack.
constexpr const char* kThreadNonQueueWaits[WATCHDOG_NUM_THREADS] = {
    "sleep_until; OS_SendEventMesg -> sMesgMutex",
    "HandleEvents (SDL/window), ProcessGfxCommands (renderer present/DXGI wait), DrainRenderService callback, "
    "OS_SiService -> ControlDeck",
    "port_runOnRenderThread sSvcCv handoff; port_lockAudio (gAudioLock); ProcessGfxCommands std::async waits; "
    "map-load I/O",
    "sMesgMutex; a handler blocking in osSendMesg on a full queue",
    "sMesgMutex",
    "func_8024F450 -> D_802816E8 wait; port_shapeControllerInput; CALL_EVENT(OnControllerUpdate)",
    "port_lockAudio (gAudioLock) in audioManager_handleFrameMsg",
    "func_8024F450 -> D_802816E8 wait via __baMotor_startRumble/stopRumble/func_80250930; __osMotorAccess -> "
    "ControlDeck rumble",
};

// Entry point each heartbeat belongs to.
constexpr const char* kThreadLoops[WATCHDOG_NUM_THREADS] = {
    "osCreateViManager ticker (OS_VI.cpp)",
    "SDL_main service loop (Game.cpp)",
    "push_frame -> mainLoop (Game.cpp)",
    "thread5_entry (graphics_thread.c)",
    "viMgr_entry (vimgr.c)",
    "pfsManager_entry (pfsmanager.c)",
    "audioManagerThread_entry (audio_manager.c)",
    "rumbleThread_entry (bamotor.c)",
};

// The audio thread legitimately idles through demo audio holds, and the
// pfsmanager thread races the game tick for the same polling queue, so both
// get a longer leash than the frame-paced threads.
constexpr auto kStallAfter = std::chrono::seconds(5);
constexpr auto kRelaxedStallAfter = std::chrono::seconds(10);
constexpr auto kSampleEvery = std::chrono::milliseconds(500);
constexpr auto kRedumpEvery = std::chrono::seconds(10);

struct Heartbeat {
    std::atomic<uint64_t> count{ 0 };
    std::atomic<unsigned long> tid{ 0 }; // SDL thread id, recorded on first beat
    // Real (non-pseudo) thread handle, so the watcher can walk this thread's
    // stack when it stalls. Recorded on first beat alongside the tid.
    std::atomic<void*> handle{ nullptr };
    // Watcher-thread state
    uint64_t lastCount = 0;
    Clock::time_point lastBeat{};
    bool started = false;
};

Heartbeat sBeats[WATCHDOG_NUM_THREADS];
std::atomic<bool> sStalledFlags[WATCHDOG_NUM_THREADS] = {};
std::thread sWatcher;
std::atomic<bool> sWatcherRun{ false };

// Non-zero while a serviced thread is blocked on purpose.
std::atomic<int> sExpectedStallDepth{ 0 };
std::atomic<const char*> sExpectedStallReason{ nullptr };

std::string DescribeUnkFlag1(int32_t f) {
    switch (f) {
        case 0x04:
            return "AUDIO_TASK";
        case 0x10:
            return "NO_TASK";
        case 0x20:
            return "TASK_YIELDED";
        default:
            break;
    }
    if (f & 0x08) {
        return fmt::format("GFX_TASK|{:#x}", f & ~0x08);
    }
    return fmt::format("{:#x}?", f);
}

std::string HumanDuration(std::chrono::milliseconds ms) {
    if (ms.count() < 1000) {
        return fmt::format("{}ms", ms.count());
    }
    return fmt::format("{:.1f}s", ms.count() / 1000.0);
}

// A queue's name and its producers, so a blocked wait names the code to audit.
struct QueueInfo {
    bool known;
    const char* name;
    const char* fedBy;
};

QueueInfo DescribeQueue(OSMesgQueue* mq) {
    if (mq == thread5_getTaskQueue()) {
        return { true, "sThread5MesgQueue (graphics_thread.c)",
                 "thread5_sendTaskToQueue, osSetTimer cbs, viMgr_entry retrace, OS_SendEventMesg(SP/DP) from "
                 "ServiceRcp" };
    }
    if (mq == thread5_getSyncQueue()) {
        return { true, "sThread5SyncMesgQueue (graphics_thread.c)",
                 "thread5_handleSyncEvent/handleDPEvent/handleSPEvent/handleVIRetraceEvent; all need "
                 "sUnkFlag1==NO_TASK + gfx ring drained" };
    }
    if (mq == viMgr_getRetraceQueue()) {
        return { true, "sMesgQueue1 (vimgr.c)", "OS_VI ticker via OS_EVENT_VI" };
    }
    if (mq == viMgr_getFrameTokenQueue()) {
        return { true, "sMesgQueue2 (vimgr.c)",
                 "viMgr_func_8024BFAC <- thread5_handleDPEvent (needs sUnkFlag2 bit30), func_80253FE8" };
    }
    if (mq == viMgr_getTickRetraceQueue()) {
        return { true, "sMesgQueue3 (vimgr.c)", "viMgr_entry" };
    }
    if (mq == pfsManager_getFrameMesgQ()) {
        return { true, "pfsManagerContPollingMsqQ (pfsmanager.c)",
                 "OS_EVENT_SI <- OS_SiService (window thread) after osContStartReadData" };
    }
    if (mq == pfsManager_getFrameReplyQ()) {
        return { true, "pfsManagerContReplyMsgQ (pfsmanager.c)", "pfsManager_entry when !pfsManagerBusy" };
    }
    if (mq == pfsManager_getSiLockQueue()) {
        return { true, "D_802816E8 (pfsmanager.c)", "func_8024F4AC via func_8024F35C(0)" };
    }
    if (mq == baMotor_getRetraceQueue()) {
        return { true, "D_80282390 (bamotor.c)", "viMgr_entry retrace signal (NOBLOCK, so extras drop)" };
    }
    if (mq == audioManager_getFrameMesgQueue()) {
        return { true, "audioFrameMsgQ (audio_manager.c)",
                 "thread5_handleAudioTimerEvent (skipped while port_audioHeld)" };
    }
    if (mq == audioManager_getReplyMesgQueue()) {
        return { true, "audioReplyMsgQ (audio_manager.c)",
                 "thread5_handleSPEvent when sUnkFlag1==UNKFLAG1_AUDIO_TASK" };
    }
    return { false, "unmapped queue", "unknown" };
}

// Repo source root, derived from this file's own build-time path so the check
// holds wherever the repo was cloned. The PDB records paths from the same
// compiler invocation, so our translation units share this prefix while the
// CRT/STL ones (which also contain "\src\") do not.
const std::string& ProjectSourceRoot() {
    static const std::string root = [] {
        const std::string self = __FILE__;
        const size_t pos = self.rfind("src");
        // Only usable if __FILE__ came through absolute; otherwise mark nothing
        // rather than risk the false positives a looser match produces.
        if (pos == std::string::npos || self.find(':') == std::string::npos) {
            return std::string();
        }
        return self.substr(0, pos);
    }();
    return root;
}

// Stack of the stalled thread, symbolized against the shipped PDB. Nothing else
// in the report can be reconstructed after the fact: by the time a maintainer
// reads the log the process is gone, so capture it here rather than asking for
// a debugger they cannot attach.
//
// Addresses are collected while the target is suspended and symbolized only
// after it resumes, to keep the window in which it is held as narrow as
// possible. See the caller for the hang risk this still carries.
std::string CaptureStalledStack(int threadIdx) {
#if defined(LH_WATCHDOG_STACKS)
    HANDLE thread = (HANDLE)sBeats[threadIdx].handle.load(std::memory_order_relaxed);
    if (thread == nullptr) {
        return {};
    }

    HANDLE proc = GetCurrentProcess();
    static std::once_flag symOnce;
    std::call_once(symOnce, [proc] {
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        SymInitialize(proc, nullptr, TRUE);
    });

    if (SuspendThread(thread) == (DWORD)-1) {
        return {};
    }

    CONTEXT ctx = {};
    ctx.ContextFlags = CONTEXT_FULL;
    DWORD64 frames[32];
    int numFrames = 0;
    if (GetThreadContext(thread, &ctx)) {
        STACKFRAME64 sf = {};
#if defined(_M_X64)
        const DWORD machine = IMAGE_FILE_MACHINE_AMD64;
        sf.AddrPC.Offset = ctx.Rip;
        sf.AddrFrame.Offset = ctx.Rbp;
        sf.AddrStack.Offset = ctx.Rsp;
#elif defined(_M_ARM64)
        const DWORD machine = IMAGE_FILE_MACHINE_ARM64;
        sf.AddrPC.Offset = ctx.Pc;
        sf.AddrFrame.Offset = ctx.Fp;
        sf.AddrStack.Offset = ctx.Sp;
#else
        const DWORD machine = IMAGE_FILE_MACHINE_I386;
        sf.AddrPC.Offset = ctx.Eip;
        sf.AddrFrame.Offset = ctx.Ebp;
        sf.AddrStack.Offset = ctx.Esp;
#endif
        sf.AddrPC.Mode = sf.AddrFrame.Mode = sf.AddrStack.Mode = AddrModeFlat;
        // Addresses only while suspended; symbol lookup happens after resume.
        while (numFrames < 32 &&
               StackWalk64(machine, proc, thread, &sf, &ctx, nullptr, SymFunctionTableAccess64, SymGetModuleBase64,
                           nullptr) &&
               sf.AddrPC.Offset != 0) {
            frames[numFrames++] = sf.AddrPC.Offset;
        }
    }
    ResumeThread(thread);

    if (numFrames == 0) {
        return {};
    }

    const std::string& srcRoot = ProjectSourceRoot();
    std::string out = "Stack:\n";
    alignas(SYMBOL_INFO) char symBuf[sizeof(SYMBOL_INFO) + 256] = {};
    auto* sym = reinterpret_cast<SYMBOL_INFO*>(symBuf);
    sym->SizeOfStruct = sizeof(SYMBOL_INFO);
    sym->MaxNameLen = 255;
    for (int i = 0; i < numFrames; i++) {
        DWORD64 symDisp = 0;
        DWORD lineDisp = 0;
        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(line);
        if (SymFromAddr(proc, frames[i], &symDisp, sym)) {
            if (SymGetLineFromAddr64(proc, frames[i], &lineDisp, &line)) {
                // Only frames under this repo's source root are ours; CRT and
                // STL paths contain "\src\" as well and must not match.
                const bool ours = !srcRoot.empty() && _strnicmp(line.FileName, srcRoot.c_str(), srcRoot.size()) == 0;
                out += fmt::format("{} #{:02} {}+0x{:x} ({}:{})\n", ours ? ">" : " ", i, sym->Name, symDisp,
                                   line.FileName, line.LineNumber);
            } else {
                out += fmt::format("  #{:02} {}+0x{:x}\n", i, sym->Name, symDisp);
            }
        } else {
            // No symbols (plain Release with no PDB alongside): emit
            // module+offset, which stays resolvable later against a matching
            // build. A raw address is meaningless once ASLR is factored in.
            DWORD64 base = SymGetModuleBase64(proc, frames[i]);
            char modPath[MAX_PATH] = {};
            const char* modName = "?";
            if (base != 0 && GetModuleFileNameA((HMODULE)(uintptr_t)base, modPath, MAX_PATH) != 0) {
                const char* slash = strrchr(modPath, '\\');
                modName = slash != nullptr ? slash + 1 : modPath;
            }
            if (base != 0) {
                out += fmt::format("  #{:02} {}+0x{:x}\n", i, modName, frames[i] - base);
            } else {
                out += fmt::format("  #{:02} 0x{:x}\n", i, frames[i]);
            }
        }
    }
    return out;
#else
    (void)threadIdx;
    return {};
#endif
}

std::string BuildDump(const bool* stalled, Clock::time_point now, const std::string& stack) {
    OS_BlockedWait waits[16];
    const int numWaits = OS_MesgSnapshotBlockedWaits(waits, 16);
    auto waitFor = [&](int threadIdx) -> const OS_BlockedWait* {
        unsigned long tid = sBeats[threadIdx].tid.load(std::memory_order_relaxed);
        for (int i = 0; i < numWaits; i++) {
            if (waits[i].tid == tid) {
                return &waits[i];
            }
        }
        return nullptr;
    };
    auto ageOf = [&](int threadIdx) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - sBeats[threadIdx].lastBeat);
    };

    std::string out;

    // Downstream threads stall behind the one that actually broke, so the
    // longest-stalled thread is the best root-cause guess.
    int firstStalled = -1;
    int numStalled = 0;
    if (stalled != nullptr) {
        for (int i = 0; i < WATCHDOG_NUM_THREADS; i++) {
            if (!stalled[i]) {
                continue;
            }
            numStalled++;
            if (firstStalled < 0 || sBeats[i].lastBeat < sBeats[firstStalled].lastBeat) {
                firstStalled = i;
            }
        }
    }

    if (firstStalled < 0) {
        out += "===== Thread Watchdog: snapshot, no stall =====\n";
    } else {
        out += fmt::format("===== Thread Watchdog: STALL DETECTED =====\n"
                           "{} has not reported for {} (tid {}){}\n"
                           "In loop: {}\n",
                           kThreadNames[firstStalled], HumanDuration(ageOf(firstStalled)),
                           sBeats[firstStalled].tid.load(std::memory_order_relaxed),
                           numStalled > 1 ? fmt::format(", {} stalled total, this one first", numStalled) : "",
                           kThreadLoops[firstStalled]);
        if (const OS_BlockedWait* w = waitFor(firstStalled)) {
            QueueInfo qi = DescribeQueue(w->mq);
            out += fmt::format("Parked at: {} on {} {}/{}\n", w->isSend ? "osSendMesg (full)" : "osRecvMesg (empty)",
                               qi.name, w->mq->validCount, w->mq->msgCount);
            out += fmt::format("Producers: {}\n", qi.fedBy);
        } else if (!stack.empty()) {
            out += "Parked at: no queue wait; see Stack below\n";
        } else {
            out += fmt::format("Parked at: no queue wait, so loop/mutex/condvar. Candidates: {}\n",
                               kThreadNonQueueWaits[firstStalled]);
            if (firstStalled == WATCHDOG_GAME_TICK && port_renderServicePending()) {
                out += "Note: port_runOnRenderThread call is pending, so sSvcCv is the prime suspect\n";
            }
        }
    }

    for (int i = 0; i < WATCHDOG_NUM_THREADS; i++) {
        const Heartbeat& hb = sBeats[i];
        uint64_t count = hb.count.load(std::memory_order_relaxed);
        if (count == 0) {
            out += fmt::format("  {:<11} never started\n", kThreadNames[i]);
            continue;
        }
        std::string age = (hb.lastBeat == Clock::time_point{}) ? std::string("?") : HumanDuration(ageOf(i));
        std::string park;
        if (const OS_BlockedWait* w = waitFor(i)) {
            QueueInfo qi = DescribeQueue(w->mq);
            park =
                fmt::format("  {} {} {}/{}", w->isSend ? "send" : "recv", qi.name, w->mq->validCount, w->mq->msgCount);
        }
        out += fmt::format("  {:<11} {:<7} {:>6} tid {:<6} {} beats{}\n", kThreadNames[i],
                           (stalled != nullptr && stalled[i]) ? "STALLED" : "ok", age,
                           hb.tid.load(std::memory_order_relaxed), count, park);
    }

    Thread5WatchdogState t5;
    thread5_getWatchdogState(&t5);
    ViMgrWatchdogState vi;
    viMgr_getWatchdogState(&vi);
    const u32 dpStatus = osDpGetStatus();
    void* curFb = osViGetCurrentFramebuffer();
    void* nextFb = osViGetNextFramebuffer();

    out += fmt::format("Pipeline: sUnkFlag1={} sUnkFlag2={:#x}/saved={:#x} sSyncCounter={} task7={} "
                       "gfxRing={}->{} audioRing={}->{} taskQ={}/{} syncQ={}/{} FREEZE={} pendingSpTask={} "
                       "fb cur{}next{}\n",
                       DescribeUnkFlag1(t5.unkFlag1), t5.unkFlag2, t5.unkFlag2Saved, t5.syncCounter, t5.task7Handled,
                       t5.gfxActiveId, t5.gfxSelectedId, t5.audioActiveId, t5.audioSelectedId, t5.taskQueueCount,
                       t5.taskQueueCap, t5.syncQueueCount, t5.syncQueueCap, (dpStatus & 0x2) ? 1 : 0,
                       OS_SpPeekPendingTask() != nullptr ? "yes" : "no",
                       curFb == nextFb ? "==" : "!=", OS_ViBlackActive() ? " viBlack" : "");
    out += fmt::format("Frame pacing: D_802808D8={}{} frameTokenQ={} tickRetraceQ={} retraceQ={}\n", vi.retraceCount,
                       vi.retraceCount > 10 ? " (accumulating)" : "", vi.q2Count, vi.q3Count, vi.q1Count);

    OSMesgQueue* pfsPoll = pfsManager_getFrameMesgQ();
    OSMesgQueue* audioFrame = audioManager_getFrameMesgQueue();
    OSMesgQueue* audioReply = audioManager_getReplyMesgQueue();
    out += fmt::format("SI/audio: pfsBusy={} pollingQ={} audioFrameQ={} audioReplyQ={} audioHeld={} svcPending={}\n",
                       pfsManager_isBusy(), pfsPoll->validCount, audioFrame->validCount, audioReply->validCount,
                       port_audioHeld(), port_renderServicePending());
    out += fmt::format("Context: map={:#x} exit={:#x} gameMode={} build={} {}@{}", gsworld_getMap(), gsworld_getExit(),
                       getGameMode(), (char*)gBuildVersion, (char*)gGitBranch, (char*)gGitCommitHash);

    if (!stack.empty()) {
        out += "\n" + stack;
    }
    return out;
}

void Watcher() {
    auto lastSample = Clock::now();
    auto lastReport = Clock::time_point{};
    bool wasStalled = false;
    bool wasExpected = false;

    while (sWatcherRun.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(kSampleEvery);
        auto now = Clock::now();

        // A debugger pause (or system sleep) suspends every thread including
        // this one; a stale "last beat" then isn't a stall. Rebaseline.
        const bool suspended = (now - lastSample) > kSampleEvery * 4;
        lastSample = now;
        const bool expected = sExpectedStallDepth.load(std::memory_order_acquire) > 0;
        const bool rebaseline = suspended || expected || wasExpected;
        wasExpected = expected;

        bool stalled[WATCHDOG_NUM_THREADS] = {};
        bool anyStalled = false;
        for (int i = 0; i < WATCHDOG_NUM_THREADS; i++) {
            Heartbeat& hb = sBeats[i];
            uint64_t cur = hb.count.load(std::memory_order_relaxed);
            if (!hb.started) {
                if (cur == 0) {
                    continue; // thread not running yet; don't judge it
                }
                hb.started = true;
            }
            if (cur != hb.lastCount || rebaseline) {
                hb.lastCount = cur;
                hb.lastBeat = now;
                continue;
            }
            const auto limit =
                (i == WATCHDOG_AUDIO_MANAGER || i == WATCHDOG_PFSMANAGER) ? kRelaxedStallAfter : kStallAfter;
            if (now - hb.lastBeat > limit) {
                stalled[i] = true;
                anyStalled = true;
            }
        }
        for (int i = 0; i < WATCHDOG_NUM_THREADS; i++) {
            sStalledFlags[i].store(stalled[i], std::memory_order_relaxed);
        }

        if (!anyStalled) {
            wasStalled = false;
            continue;
        }
        // One dump when a stall appears, then a refresher while it persists.
        if (wasStalled && (now - lastReport) < kRedumpEvery) {
            continue;
        }
        const bool firstReportOfEpisode = !wasStalled;
        wasStalled = true;
        lastReport = now;

        int root = -1;
        for (int i = 0; i < WATCHDOG_NUM_THREADS; i++) {
            if (stalled[i] && (root < 0 || sBeats[i].lastBeat < sBeats[root].lastBeat)) {
                root = i;
            }
        }

        // The stack is folded into the report below so the diagnosis is one
        // block, but walking a thread means suspending it, which can hang if it
        // holds a DbgHelp or loader lock and would take this watcher with it.
        // Leave a breadcrumb first so a hang still names the stalled thread.
        std::string stack;
        if (firstReportOfEpisode && root >= 0) {
            SPDLOG_ERROR("[Watchdog] {} stalled, collecting state", kThreadNames[root]);
            stack = CaptureStalledStack(root);
        }
        SPDLOG_ERROR("{}", BuildDump(stalled, now, stack));
    }
}

} // namespace

extern "C" void ThreadWatchdog_Beat(WatchdogThread id) {
    Heartbeat& hb = sBeats[id];
    hb.count.fetch_add(1, std::memory_order_relaxed);
    if (hb.tid.load(std::memory_order_relaxed) == 0) {
#if defined(LH_WATCHDOG_STACKS)
        // GetCurrentThread is a pseudo-handle only valid on this thread, so
        // duplicate it into one the watcher can use. Leaked by design: it must
        // outlive the thread's own loop for the whole session.
        HANDLE dup = nullptr;
        DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &dup,
                        THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, 0);
        hb.handle.store(dup, std::memory_order_relaxed);
#endif
        hb.tid.store((unsigned long)SDL_ThreadID(), std::memory_order_relaxed);
    }
}

extern "C" void ThreadWatchdog_Start(void) {
    if (sWatcherRun.exchange(true)) {
        return;
    }
    sWatcher = std::thread(Watcher);
}

extern "C" void ThreadWatchdog_Stop(void) {
    if (!sWatcherRun.exchange(false)) {
        return;
    }
    if (sWatcher.joinable()) {
        sWatcher.join();
    }
}

extern "C" void ThreadWatchdog_DumpNow(void) {
    SPDLOG_INFO("{}", BuildDump(nullptr, Clock::now(), std::string()));
}

extern "C" int ThreadWatchdog_IsStalled(WatchdogThread id) {
    return sStalledFlags[id].load(std::memory_order_relaxed) ? 1 : 0;
}

extern "C" void ThreadWatchdog_BeginExpectedStall(const char* reason) {
    if (sExpectedStallDepth.fetch_add(1, std::memory_order_acq_rel) == 0) {
        sExpectedStallReason.store(reason, std::memory_order_relaxed);
        SPDLOG_DEBUG("[Watchdog] stall reporting suppressed: {}", reason ? reason : "?");
    }
}

extern "C" void ThreadWatchdog_EndExpectedStall(void) {
    const int prev = sExpectedStallDepth.fetch_sub(1, std::memory_order_acq_rel);
    if (prev <= 0) {
        // Unbalanced release would leave the counter negative and suppress
        // every future stall, which is worse than the false positive.
        sExpectedStallDepth.store(0, std::memory_order_release);
        SPDLOG_WARN("[Watchdog] ThreadWatchdog_EndExpectedStall without a matching Begin");
        return;
    }
    if (prev == 1) {
        const char* reason = sExpectedStallReason.load(std::memory_order_relaxed);
        SPDLOG_DEBUG("[Watchdog] stall reporting resumed: {}", reason ? reason : "?");
        sExpectedStallReason.store(nullptr, std::memory_order_relaxed);
    }
}
