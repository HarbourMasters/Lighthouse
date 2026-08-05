#include "Engine.h"
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <chrono>
#include <map>
#include <mutex>
#include <thread>

#include <fast/interpreter.h>
#include <libultraship.h>
#ifdef _WIN32
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#endif
#include <SDL2/SDL.h>

#include "DevTools/ThreadWatchdog.h"
#include "GameStatus.h"
#include "Interpolation/FrameInterpolation.h"
#include "Nametag/Nametag.h"
#include "Network/Anchor/Anchor.h"
#include "OS/OS.h"
#include "Patches/Patches.h"
#include "ShipUtils.h"
#include "ShipInit.hpp"
#include "src/port/Enhancements/Events/Hooks/Events.h"
#include "UI/LighthouseModMenuWindow.h"

extern "C" {
#include "enums.h"
#include "core1/core1.h"
#include "core1/main.h"
#include "core1/thread5.h"
void viMgr_entry(void* arg);
void thread5_entry(void* arg);
void audioManagerThread_entry(void* arg);
void core1_15B30_sendMesg3ToRenderThread(void);
OSMesgQueue* thread5_getTaskQueue(void);
OSMesgQueue* thread5_getSyncQueue(void);
}

// The game tick runs on its own thread and submits display lists through the
// decomp's thread5 queue; this thread stays behind as the RCP and event pump.
namespace {
std::atomic<bool> sGameThreadDone{ false };
std::thread sGameThread;
thread_local bool tIsGameThread = false;

// The interpolation pair a submitted list was built from, carried to whoever
// renders it. At most a couple are live at once.
struct InterpPair {
    int prev = -1;
    int curr = -1;
    bool should = false;
    uint64_t serial = 0;
};
std::mutex sInterpMutex;
std::map<void*, InterpPair> sTaskInterp;
uint64_t sInterpSerial = 0;
// Submissions of slack before an unrendered pair is assumed dropped. The ring is
// 4 slots, so by then its trees have been recycled regardless.
constexpr uint64_t kInterpStaleAfter = 4;

// Renderer calls made from tick code, run by the main loop between services.
std::mutex sSvcMutex;
std::condition_variable sSvcCv;
void (*sSvcFn)(void*) = nullptr;
void* sSvcArg = nullptr;
std::atomic<bool> sShutdownRequested{ false };

int sTitleMap = 0;

void DrainRenderService() {
    std::unique_lock<std::mutex> lock(sSvcMutex);
    if (sSvcFn != nullptr) {
        auto* fn = sSvcFn;
        void* arg = sSvcArg;
        lock.unlock();
        fn(arg);
        lock.lock();
        sSvcFn = nullptr;
        sSvcCv.notify_all();
    }
}

// False on the window thread, including everything that runs during init
// before the tick thread exists.
bool OnGameThread() {
    return tIsGameThread;
}
} // namespace

// A list is submitted while its tick is still recording, so the pair is
// captured here and travels with the task.
extern "C" void port_thread5_onSubmit(void* taskData) {
    if (!OnGameThread() || (uintptr_t)taskData < 100) {
        return;
    }
    struct ucode_task_data_s* task = (struct ucode_task_data_s*)taskData;
    if (task->task_type != UCODE_TASK_TYPE_F3DEX && task->task_type != UCODE_TASK_TYPE_L3DEX) {
        return;
    }
    InterpPair pair;
    FrameInterpolation_GetRecordingPair(&pair.prev, &pair.curr, &pair.should);
    FrameInterpolation_ClaimPair(pair.prev, pair.curr);
    FrameInterpolation_StopRecord();
    Nametag::SubmitFrame(task->data_ptr);
    std::lock_guard<std::mutex> lock(sInterpMutex);
    pair.serial = ++sInterpSerial;
    auto [it, inserted] = sTaskInterp.emplace(task->data_ptr, pair);
    if (!inserted) {
        FrameInterpolation_ReleasePair(it->second.prev, it->second.curr);
        it->second = pair;
    }

    // Anything still here after a full trip round the ring can no longer be
    // blended against a live tree, so its claim is only holding a slot hostage.
    // Dropping the entry leaves RenderTask with a -1 pair, which renders
    // uninterpolated rather than against a recycled tree.
    for (auto stale = sTaskInterp.begin(); stale != sTaskInterp.end();) {
        if (stale->second.serial + kInterpStaleAfter < pair.serial) {
            FrameInterpolation_ReleasePair(stale->second.prev, stale->second.curr);
            stale = sTaskInterp.erase(stale);
        } else {
            ++stale;
        }
    }
}

namespace {
void RenderTask(void* dlStart) {
    InterpPair pair;
    {
        std::lock_guard<std::mutex> lock(sInterpMutex);
        auto it = sTaskInterp.find(dlStart);
        if (it != sTaskInterp.end()) {
            pair = it->second;
            sTaskInterp.erase(it);
        }
    }
    FrameInterpolation_BeginRenderPass(pair.prev, pair.curr, pair.should);
    Nametag::BeginRenderPass(dlStart, pair.should);
    GameEngine::ProcessGfxCommands((Gfx*)dlStart);
    FrameInterpolation_ReleasePair(pair.prev, pair.curr);
}

// This thread plays the RCP: thread5 hands over a task,
// we run it and raise SP then DP.
int ServiceRcp() {
    OSTask* task = OS_SpTakePendingTask();
    if (task == nullptr) {
        return 0;
    }
    RenderTask(task->t.data_ptr);
    OS_SendEventMesg(OS_EVENT_SP);
    OS_SendEventMesg(OS_EVENT_DP);
    return 1;
}

// Called before core1_init, which is where these threads are created.
void EnableThread5() {
    OS_EnableThreadEntry((void*)thread5_entry);
    OS_SetQueueBlocking(thread5_getTaskQueue(), 1);
    OS_SetQueueBlocking(thread5_getSyncQueue(), 1);
    // The controller manager parks on its polling queue waiting for OS_EVENT_SI.
    OS_EnableThreadEntry((void*)pfsManager_entry);
    OS_SetQueueBlocking(pfsManager_getFrameMesgQ(), 1);
    OS_EnableThreadEntry((void*)audioManagerThread_entry);
    OS_SetQueueBlocking(audioManager_getFrameMesgQueue(), 1);
    OS_SetQueueBlocking(audioManager_getReplyMesgQueue(), 1);
}
} // namespace

// Drain submitted lists for safety.
static void RegisterThread5MapSync_Init() {
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        (void)event;
        port_pipelineSyncPoint();
    });
}

static RegisterShipInitFunc sThread5MapSyncInit(RegisterThread5MapSync_Init);

// Whether a tick-side renderer call is waiting on the window thread. The
// handshake below is a condvar rather than a message queue, so it is the one
// park the watchdog's blocked-wait registry cannot see.
extern "C" int port_renderServicePending(void) {
    return sSvcFn != nullptr;
}

// Renderer calls from tick code come through here; D3D11 hangs if they run
// off the window thread. Inline when there is no separate tick thread.
extern "C" void port_runOnRenderThread(void (*fn)(void*), void* arg) {
    if (!OnGameThread()) {
        fn(arg);
        return;
    }
    std::unique_lock<std::mutex> lock(sSvcMutex);
    auto done = [] { return sSvcFn == nullptr || sShutdownRequested.load(std::memory_order_acquire); };
    if (sShutdownRequested.load(std::memory_order_acquire)) {
        return;
    }
    sSvcCv.wait(lock, done);
    if (sShutdownRequested.load(std::memory_order_acquire)) {
        return;
    }
    sSvcFn = fn;
    sSvcArg = arg;
    sSvcCv.wait(lock, done);
}

// Barrier before the tick frees or reads memory an in-flight list references.
// The game's own EVENT_SYNC handshake is the RDP-done wait.
extern "C" void port_pipelineSyncPoint(void) {
    if (OnGameThread()) {
        core1_15B30_sendMesg3ToRenderThread();
    }
}

// Tracks whether mainLoop actually fed the renderer this iteration.
// BK's gameloop conditionally skips game_draw during scene transitions.
static bool sFrameRendered = false;

// The list itself reaches the renderer through thread5's task queue, submitted
// by core1_15B30_addF3DEXTaskData right after this call; all that is left here
// is noting that the tick drew.
extern "C" void Graphics_PushFrame(Gfx* data) {
    (void)data;
    sFrameRendered = true;
}

void push_frame() {
    static int sTitleCounter = 0;
    sFrameRendered = false;

    // The window thread keeps the progress modal alive while an inline mod
    // extraction runs; the tick just idles so the extractor gets the machine.
    if (IsInlineModExtractionBusy()) {
        SDL_Delay(16);
        return;
    }

    GameEngine::Instance->StartFrame();
    port_animVtx_beginTick();
    const bool recordInterpolation = GameEngine::IsInterpolationEnabled();
    if (recordInterpolation) {
        FrameInterpolation_StartRecord();
    }
    mainLoop();
    if (recordInterpolation) {
        FrameInterpolation_StopRecord();
    }
    if (sFrameRendered) {
        port_tickDemoAudioHold();
    }

    // Refresh window title stats once per second (every 30 game ticks). The
    // window belongs to the other thread, so hand the call over.
    if (++sTitleCounter >= 30) {
        sTitleCounter = 0;
        sTitleMap = gsworld_getMap();
        port_runOnRenderThread([](void*) { port_setWindowTitle(sTitleMap); }, nullptr);
    }

    if (!sFrameRendered) {
        SDL_Delay(33);
    }
}

/* Rename SDL_main to main for SDL compatibility */
#ifdef __GNUC__
#define SDL_main main
#endif

int SDL_main(int argc, char* argv[]) {
#ifdef _WIN32
    timeBeginPeriod(1);
#endif

    // Anchor relative paths to the executable instead of cwd
    // when SHIP_HOME is not in use
    std::error_code ec;
    const char* shipHome = std::getenv("SHIP_HOME");
    const char* appImage = std::getenv("APPIMAGE");
    if (shipHome != nullptr && shipHome[0] != '\0') {
        std::filesystem::current_path(shipHome, ec);
    } else if (appImage != nullptr && appImage[0] != '\0') {
        // Running from an AppImage: the executable lives in a read-only squashfs
        // mount under /tmp, so anchor to the .AppImage file's directory instead.
        std::filesystem::current_path(std::filesystem::path(appImage).parent_path(), ec);
    } else {
        std::string base = Ship::Context::GetAppBundlePath();
        if (!base.empty() && base != ".") {
            std::filesystem::current_path(base, ec);
        }
    }

    GameEngine::Create(argc, argv);
    // Both threads are created during core1_init, so allowlist them first.
    OS_EnableThreadEntry((void*)viMgr_entry);
    EnableThread5();
    core1_init();
    ThreadWatchdog_Start();

    sGameThread = std::thread([] {
        tIsGameThread = true;
        while (WindowIsRunning()) {
            ThreadWatchdog_Beat(WATCHDOG_GAME_TICK);
            push_frame();
        }
        sGameThreadDone.store(true);
    });
    while (WindowIsRunning() || !sGameThreadDone.load()) {
        ThreadWatchdog_Beat(WATCHDOG_MAIN_LOOP);
        port_noteMainLoopAlive();
        // Pump events every iteration: a task-starved pass must not starve
        // input and window messages.
        Ship::Context::GetRawInstance()->GetWindow()->HandleEvents();
        OS_SiService();
        if (IsInlineModExtractionBusy()) {
            GameEngine::Instance->RenderGuiFrame();
            SDL_Delay(16);
            continue;
        }
        DrainRenderService();
        if (!ServiceRcp()) {
            // The gui only draws inside serviced frames, so a stalled game
            // thread would freeze ImGui with it. Render gui-only frames during
            // a stall so the menu (and the watchdog dump) stays reachable.
            if (ThreadWatchdog_IsStalled(WATCHDOG_GAME_TICK)) {
                GameEngine::Instance->RenderGuiFrame();
                SDL_Delay(16);
                continue;
            }
            SDL_Delay(1);
        }
    }
    // Ask first, then release: a thread woken before the request is set would just
    // park again.
    OS_RequestThreadExit();
    {
        std::lock_guard<std::mutex> lock(sSvcMutex);
        sShutdownRequested.store(true, std::memory_order_release);
        sSvcFn = nullptr;
    }
    sSvcCv.notify_all();
    OS_BeginShutdown();

    if (sGameThread.joinable()) {
        sGameThread.join();
    }
    // Before Destroy: these threads draw and play audio through the engine.
    OS_JoinDecompThreads();
    ThreadWatchdog_Stop();
    OS_StopViTicker();
    OS_StopTimerWorker();
#ifdef USE_NETWORKING
    Anchor::GetInstance()->Disable();
    SDLNet_Quit();
#endif
#ifdef _WIN32
    timeEndPeriod(1);
#endif
    GameEngine::Instance->Destroy();
    GameEngine::RelaunchIfRequested(argc, argv);
    return 0;
}
