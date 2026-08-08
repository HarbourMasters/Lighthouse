// [port] Audio engine synchronization: a single recursive lock serializing the free-running
// audio worker (GameEngine::HandleAudioThread, advancing the synth in n_alAudioFrame) against the
// game thread's SFX/music writes.
//
// osSetIntMask lives here, not with the libultra OS stubs, because in this port it's no longer an
// interrupt primitive — its only callers are the N64 audio files, which bracket every
// thread-shared section with osSetIntMask(OS_IM_NONE)/restore. Routing those brackets to this
// lock protects all of them with no edits to the audio code.

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>

extern "C" {
#include <libultra/exception.h> // OSIntMask, OS_IM_NONE
}

namespace {
std::recursive_mutex gAudioLock;
thread_local int gAudioMaskDepth = 0;
} // namespace

// Window-freeze audio hold
namespace {
constexpr int64_t kCushionMs = 100;
std::atomic<int64_t> sAllowedUntilMs{ 0 };

int64_t NowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}
} // namespace

extern "C" void port_noteMainLoopAlive(void) {
    sAllowedUntilMs.store(NowMs() + kCushionMs, std::memory_order_relaxed);
}

extern "C" int port_audioStallHold(void) {
    const int64_t allowedUntil = sAllowedUntilMs.load(std::memory_order_relaxed);
    return allowedUntil != 0 && NowMs() > allowedUntil;
}

extern "C" void port_lockAudio(void) {
    gAudioLock.lock();
}

extern "C" void port_unlockAudio(void) {
    gAudioLock.unlock();
}

extern "C" void port_audioIntMaskEnter(void) {
    gAudioLock.lock();
    gAudioMaskDepth++;
}

extern "C" void port_audioIntMaskExit(void) {
    if (gAudioMaskDepth > 0) {
        gAudioMaskDepth--;
        gAudioLock.unlock();
    }
}

// OS_IM_NONE enters a critical section, a restored mask exits it. Returns 0 so the saved value
// the caller passes to the paired restore is non-OS_IM_NONE.
extern "C" OSIntMask osSetIntMask(OSIntMask mask) {
    if (mask == OS_IM_NONE) {
        port_audioIntMaskEnter();
    } else {
        port_audioIntMaskExit();
    }
    return 0;
}
