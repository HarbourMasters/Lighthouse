// This file should eventually replace libultraship's os_vi.cpp

#include "OS.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "port/DevTools/ThreadWatchdog.h"

extern "C" {
#include "libultraship/libultra/types.h"
#include "libultraship/libultra/vi.h"
}

// The VI: retrace source and framebuffer registers.
//
// LUS drives retraces off an SDL timer at 16ms, which is 62.5Hz rather than 60,
// and only if SDL_INIT_TIMER were up. It never is, so retraces never fire at
// all. Game speed is meant to come off this cadence, so it needs to be a real
// 60Hz on an absolute schedule instead of an interval that drifts.
//
// The framebuffer registers are no-ops in LUS: swap does nothing and the
// getters return null. The decomp waits for a swap to latch and compares the
// current buffer against what it last saw, so they have to hold real values.

namespace {

std::thread sTicker;
std::atomic<bool> sTickerRun{ false };

std::atomic<void*> sNextFramebuffer{ nullptr };
std::atomic<void*> sCurrentFramebuffer{ nullptr };
std::atomic<bool> sBlack{ false };
std::atomic<long long> sLatchNs{ 0 };
std::atomic<long long> sRetraceNs{ 0 }; // when the latest retrace was scheduled
std::atomic<long long> sSwapNs{ 0 };
constexpr long long kViNs = 16666667;

long long SteadyNs() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

} // namespace

extern "C" void osCreateViManager(OSPri pri) {
    (void)pri;
    if (sTickerRun.exchange(true)) {
        return;
    }
    sTicker = std::thread([] {
        constexpr std::chrono::nanoseconds kVi(16666667); // NTSC 60Hz
        auto next = std::chrono::steady_clock::now() + kVi;
        while (sTickerRun.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_until(next);
            sRetraceNs.store(std::chrono::duration_cast<std::chrono::nanoseconds>(next.time_since_epoch()).count(),
                             std::memory_order_release);
            next += kVi;
            // If the process was suspended (debugger, sleep), resync rather
            // than firing a burst of catch-up retraces.
            auto now = std::chrono::steady_clock::now();
            if (now - next > std::chrono::milliseconds(100)) {
                next = now + kVi;
            }
            // A retrace latches whatever swap armed, then raises VI.
            void* armed = sNextFramebuffer.load(std::memory_order_acquire);
            if (sCurrentFramebuffer.exchange(armed, std::memory_order_acq_rel) != armed) {
                sLatchNs.store(SteadyNs(), std::memory_order_release);
            }
            ThreadWatchdog_Beat(WATCHDOG_VI_TICKER);
            OS_SendEventMesg(OS_EVENT_VI);
        }
    });
}

extern "C" void OS_StopViTicker(void) {
    if (!sTickerRun.exchange(false)) {
        return;
    }
    if (sTicker.joinable()) {
        sTicker.join();
    }
}

extern "C" void osViSetEvent(OSMesgQueue* queue, OSMesg mesg, u32 retraceCount) {
    (void)retraceCount;
    osSetEventMesg(OS_EVENT_VI, queue, mesg);
}

extern "C" void osViSwapBuffer(void* framebuffer) {
    sSwapNs.store(SteadyNs(), std::memory_order_release);
    sNextFramebuffer.store(framebuffer, std::memory_order_release);
}

extern "C" void* osViGetNextFramebuffer(void) {
    return sNextFramebuffer.load(std::memory_order_acquire);
}

extern "C" void* osViGetCurrentFramebuffer(void) {
    return sCurrentFramebuffer.load(std::memory_order_acquire);
}

// When the last swap latched, which is the earliest a new frame can start drawing.
extern "C" long long OS_ViLastLatchNs(void) {
    return sLatchNs.load(std::memory_order_acquire);
}

extern "C" long long OS_ViLastSwapNs(void) {
    return sSwapNs.load(std::memory_order_acquire);
}

// The first retrace after t on the ticker's schedule, which is where a swap made at t latches.
extern "C" long long OS_ViNextRetraceAfterNs(long long t) {
    const long long last = sRetraceNs.load(std::memory_order_acquire);
    if (last == 0 || t == 0) {
        return 0;
    }
    long long d = t - last;
    long long k = d >= 0 ? d / kViNs + 1 : -((-d - 1) / kViNs);
    return last + k * kViNs;
}

extern "C" void osViSetMode(OSViMode* mode) {
    (void)mode;
}

extern "C" void osViSetSpecialFeatures(u32 features) {
    (void)features;
}

extern "C" void osViBlack(u8 active) {
    sBlack.store(active != 0, std::memory_order_release);
}

extern "C" int OS_ViBlackActive(void) {
    return sBlack.load(std::memory_order_acquire) ? 1 : 0;
}

extern "C" void osViSetXScale(f32 scale) {
    (void)scale;
}

extern "C" void osViSetYScale(f32 scale) {
    (void)scale;
}
