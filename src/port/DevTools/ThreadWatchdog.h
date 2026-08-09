#ifndef PORT_THREAD_WATCHDOG_H
#define PORT_THREAD_WATCHDOG_H

#include <stdint.h>

// Heartbeat watchdog over the revived decomp threads.
//
// Every serviced loop beats once per iteration and a watcher thread samples the
// counters. When a thread that has already proven alive stops beating while the
// wall clock keeps moving, the watcher logs the pipeline state: thread5's flag
// words and task rings, every queue depth, who is parked where, and the stalled
// thread's stack.

#ifdef __cplusplus
extern "C" {
#endif

typedef enum WatchdogThread {
    WATCHDOG_VI_TICKER = 0, // retrace source (OS_VI.cpp); everything below is paced by it
    WATCHDOG_MAIN_LOOP,     // window thread: event pump + RCP service (Game.cpp)
    WATCHDOG_GAME_TICK,     // push_frame loop (Game.cpp)
    WATCHDOG_THREAD5,       // decomp graphics thread (graphics_thread.c)
    WATCHDOG_VIMGR,         // decomp VI manager thread (vimgr.c)
    WATCHDOG_PFSMANAGER,    // controller thread (pfsmanager.c)
    WATCHDOG_AUDIO_MANAGER, // audio thread (audio_manager.c); idles during demo audio holds
    WATCHDOG_RUMBLE,        // motor thread (bamotor.c); one beat per retrace signal
    WATCHDOG_NUM_THREADS
} WatchdogThread;

// Called once per loop iteration; one relaxed atomic increment.
void ThreadWatchdog_Beat(WatchdogThread id);

// Started once the decomp threads exist, stopped before shutdown teardown.
void ThreadWatchdog_Start(void);
void ThreadWatchdog_Stop(void);

// Log the pipeline state immediately, stalled or not.
void ThreadWatchdog_DumpNow(void);

// Bracket a section that blocks a serviced thread on purpose.
void ThreadWatchdog_BeginExpectedStall(const char* reason);
void ThreadWatchdog_EndExpectedStall(void);

// Whether the watcher currently considers this thread stalled. The gui only
// draws inside serviced frames, so the main loop uses this to keep drawing
// while the tick is down instead of freezing ImGui along with it.
int ThreadWatchdog_IsStalled(WatchdogThread id);

// Diagnostic snapshots served by the decomp thread owners; plain structs so
// the C side fills them without knowing about the watchdog's internals.
typedef struct Thread5WatchdogState {
    int32_t unkFlag1; // 0x04 audio task, 0x08 gfx task, 0x10 idle, 0x20 yielded
    int32_t unkFlag2;
    int32_t unkFlag2Saved;
    int32_t syncCounter;
    int32_t task7Handled;
    int32_t gfxActiveId;
    int32_t gfxSelectedId;
    int32_t audioActiveId;
    int32_t audioSelectedId;
    int32_t taskQueueCount;
    int32_t taskQueueCap;
    int32_t syncQueueCount;
    int32_t syncQueueCap;
} Thread5WatchdogState;
void thread5_getWatchdogState(Thread5WatchdogState* out);

typedef struct ViMgrWatchdogState {
    int32_t retraceCount; // D_802808D8: retraces seen since the tick last consumed them
    int32_t q1Count;      // retrace event queue (viMgr_entry parks here)
    int32_t q2Count;      // frame token queue (the tick parks here)
    int32_t q3Count;      // tick retrace queue
} ViMgrWatchdogState;
void viMgr_getWatchdogState(ViMgrWatchdogState* out);

#ifdef __cplusplus
}

namespace Lighthouse {
class ExpectedStall {
public:
    explicit ExpectedStall(const char* reason) {
        ThreadWatchdog_BeginExpectedStall(reason);
    }
    ~ExpectedStall() {
        ThreadWatchdog_EndExpectedStall();
    }
    ExpectedStall(const ExpectedStall&) = delete;
    ExpectedStall& operator=(const ExpectedStall&) = delete;
};
} // namespace Lighthouse
#endif

#endif // PORT_THREAD_WATCHDOG_H
