#include "port/Engine.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <libultraship/libultraship.h>
#include <fast/Fast3dWindow.h>
#include <fast/interpreter.h>

#include "FrameInterpolation.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Nametag/Nametag.h"
#include "port/OS/OS.h"
#include "port/Patches/Patches.h"
#include "port/UI/cvar_prefixes.h"

#define gVIsPerFrame 2 // 30 Hz

extern "C" bool prevAltAssets;
extern "C" void port_releaseRcpTask(void);

namespace {
long long sLastSubFrameNs = 0;
long long sFrameLatchNs = 0;
unsigned sFrameViSerial = 0;
bool sFrameTimed = false;

using Clock = std::chrono::steady_clock;
inline long long NsSince(Clock::time_point t0) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - t0).count();
}

void SyncAltAssets() {
    bool curAltAssets = CVarGetInteger(CVAR_SETTING("Mods.AlternateAssets"), 1);
    if (prevAltAssets != curAltAssets) {
        prevAltAssets = curAltAssets;
        Ship::Context::GetRawInstance()->GetResourceManager()->SetAltAssetsEnabled(curAltAssets);
        gfx_texture_cache_clear();
    }
}

// Draws one sub-frame without presenting it.
void DrawSubframe(Fast::Interpreter* interpreter, const std::shared_ptr<Ship::Window>& wndBase, Gfx* commands,
                  const std::unordered_map<Mtx*, MtxF>& replacements) {
    auto runT0 = Clock::now();
    auto gui = wndBase->GetGui();
    wndBase->GetMouseStateManager()->StartFrame();
    gui->StartDraw();
    interpreter->StartFrame();
    interpreter->Run(commands, replacements);
    if (OS_ViBlackActive()) {
        interpreter->mGfxFrameBuffer = 0;
        auto rapi = interpreter->GetCurrentRenderingAPI();
        rapi->StartDrawToFramebuffer(0, 1.0f);
        rapi->ClearFramebuffer(true, false);
    }
    gui->EndDraw();
    sLastSubFrameNs = NsSince(runT0);
}

void PresentSubframe(Fast::Interpreter* interpreter) {
    interpreter->EndFrame();
    CALL_EVENT(FrameDrawEnd);
    interpreter->mInterpolationIndex++;
}
} // namespace

void GameEngine::RunCommands(Gfx* Commands) {
    static const std::unordered_map<Mtx*, MtxF> kNoReplacements;
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetRawInstance()->GetWindow());
    if (wnd == nullptr) {
        return;
    }
    auto interpreter = wnd->GetInterpreterWeak().lock().get();
    wnd->HandleEvents();
    interpreter->mInterpolationIndex = 0;
    auto wndBase = Ship::Context::GetRawInstance()->GetWindow();
    Nametag::SetSubframeBlend(1.0f);
    if (wndBase->IsFrameReady()) {
        DrawSubframe(interpreter, wndBase, Commands, kNoReplacements);
        PresentSubframe(interpreter);
    }
    SyncAltAssets();
}

int GameEngine::CurrentViPerTick() {
    int viPerTick = port_getDemoViCount();
    if (viPerTick <= 0) {
        viPerTick = gVIsPerFrame + port_getCutsceneExtraVis();
    }
    if (viPerTick < gVIsPerFrame) {
        viPerTick = gVIsPerFrame;
    }
    // Clamp to 15 for demo playbacks.
    if (viPerTick > 15) {
        viPerTick = 15;
    }
    return viPerTick;
}

namespace {
int EffectiveLogicFps() {
    int fps = 60 / GameEngine::CurrentViPerTick();
    return (fps < 1) ? 1 : fps;
}

int SubframesForTarget(int targetFps) {
    int subframes = targetFps / EffectiveLogicFps();
    return (subframes < 1) ? 1 : subframes;
}
} // namespace

bool GameEngine::IsInterpolationEnabled() {
    return (int)GetInterpolationFPS() > EffectiveLogicFps();
}

// Decided once per list at submit, so the pickup in ServiceRcp and the pass that draws it agree.
bool GameEngine::WantsTimedPass(bool recorded, int viPerTick) {
    return recorded && !GfxDebuggerIsDebugging() && (int)GetInterpolationFPS() > 60 / viPerTick;
}

void GameEngine::SetFrameTiming(long long latchNs, unsigned viSerial, bool timed) {
    sFrameViSerial = viSerial;
    sFrameLatchNs = latchNs;
    sFrameTimed = timed;
}

namespace {
// Room left before the next swap has to latch, for the last draw and the game's swap behind it.
constexpr long long kReleaseMarginNs = 4000000;
constexpr int kMaxTimedSubframes = 32;
// How far a prefetched blend may be from the sub-frame's own before it is redone, as a share of the tick.
constexpr float kPrefetchSlack = 0.03f;

void BuildReplacements(float t, std::unordered_map<Mtx*, MtxF>& replacements) {
    if (t < 1.0f) {
        FrameInterpolation_Interpolate(t, replacements);
    } else {
        replacements.clear();
    }
}

class Prefetcher {
public:
    void Start(float t, std::unordered_map<Mtx*, MtxF>* out) {
        std::lock_guard<std::mutex> lock(mMutex);
        if (!mThread.joinable()) {
            mThread = std::thread([this] { Run(); });
        }
        mT = t;
        mOut = out;
        mBusy = true;
        mCv.notify_all();
    }

    void Wait() {
        std::unique_lock<std::mutex> lock(mMutex);
        mCv.wait(lock, [this] { return !mBusy; });
    }

private:
    void Run() {
        std::unique_lock<std::mutex> lock(mMutex);
        for (;;) {
            mCv.wait(lock, [this] { return mOut != nullptr; });
            auto* out = mOut;
            const float t = mT;
            mOut = nullptr;
            lock.unlock();
            BuildReplacements(t, *out);
            lock.lock();
            mBusy = false;
            mCv.notify_all();
        }
    }

    std::thread mThread;
    std::mutex mMutex;
    std::condition_variable mCv;
    std::unordered_map<Mtx*, MtxF>* mOut = nullptr;
    float mT = 0.0f;
    bool mBusy = false;
};

Prefetcher& GetPrefetcher() {
    static Prefetcher* sPrefetcher = new Prefetcher();
    return *sPrefetcher;
}

// Draws the frame at the window's rate until its successor is due to latch, blending each sub-frame to
// where its present lands between the previous frame and this one.
void RunTimedPass(Gfx* commands, int fps) {
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetRawInstance()->GetWindow());
    if (wnd == nullptr) {
        port_releaseRcpTask();
        return;
    }
    auto interpreter = wnd->GetInterpreterWeak().lock().get();
    auto wndBase = Ship::Context::GetRawInstance()->GetWindow();
    wnd->HandleEvents();
    interpreter->mInterpolationIndex = 0;
    static std::unordered_map<Mtx*, MtxF> maps[2];
    Prefetcher& prefetcher = GetPrefetcher();
    bool prefetching = false;
    float prefetchT = -1.0f;
    // The tick after this list's sets its VI count when it polls input, right after the submit, and that
    // count decides when this frame latches.
    const long long waitEndNs = OS_SteadyNs() + 3000000;
    while (port_getDemoViSerial() == sFrameViSerial && OS_SteadyNs() < waitEndNs) {
        port_serviceRenderRequests();
        port_waitDemoViSerial(sFrameViSerial, 250);
    }
    const int vis = std::max(port_getDemoViCount(), 2);
    const long long tickNs = 1000000000LL * vis / 60;
    const long long presentNs = 1000000000LL / fps;
    const long long passStartNs = OS_SteadyNs();
    long long anchorNs = sFrameLatchNs;
    if (anchorNs == 0 || std::llabs(passStartNs - anchorNs) > tickNs) {
        const long long latchNs = OS_ViLastLatchNs();
        anchorNs = (latchNs > 0 && passStartNs - latchNs < tickNs) ? latchNs : passStartNs;
    }
    const long long releaseBy = anchorNs + tickNs - kReleaseMarginNs;
    long long prevRunNs = 0;
    long long runIntervalNs = presentNs;
    int count = 0;

    for (;;) {
        const long long now = OS_SteadyNs();
        if (prevRunNs != 0) {
            runIntervalNs = std::max(presentNs, now - prevRunNs);
        }
        prevRunNs = now;
        // Last when another draw after this one could not be finished by the release deadline.
        const bool last = count + 1 >= kMaxTimedSubframes || now + runIntervalNs + sLastSubFrameNs > releaseBy;
        float t = std::clamp((float)(now + presentNs - anchorNs) / (float)tickNs, 0.0f, 1.0f);
        auto& replacements = maps[count & 1];
        bool prefetched = false;
        if (prefetching) {
            prefetcher.Wait();
            prefetching = false;
            if (std::fabs(prefetchT - t) <= kPrefetchSlack) {
                t = prefetchT;
                prefetched = true;
            }
        }
        if (!prefetched) {
            BuildReplacements(t, replacements);
        }
        if (!last) {
            prefetchT = std::clamp((float)(now + runIntervalNs + presentNs - anchorNs) / (float)tickNs, 0.0f, 1.0f);
            prefetcher.Start(prefetchT, &maps[(count + 1) & 1]);
            prefetching = true;
        }
        FrameInterpolation_ApplyAnimVertices(t);
        Nametag::SetSubframeBlend(t);
        DrawSubframe(interpreter, wndBase, commands, replacements);
        if (last) {
            port_releaseRcpTask();
        }
        PresentSubframe(interpreter);
        count++;
        if (last) {
            break;
        }
    }
    if (prefetching) {
        prefetcher.Wait();
    }
    SyncAltAssets();
}
} // namespace

void GameEngine::ProcessGfxCommands(Gfx* commands) {
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetRawInstance()->GetWindow());

    if (wnd == nullptr) {
        return;
    }

    // if(gEnableGammaBoost) {
    //     wnd->EnableSRGBMode();
    // }
    wnd->SetRendererUCode(UcodeHandlers::ucode_f3dex);

    if (sFrameTimed) {
        const int fps = (int)GetInterpolationFPS();
        wnd->SetTargetFps(fps);
        wnd->SetMaximumFrameLatency(2);
        RunTimedPass(commands, fps);
        return;
    }

    wnd->SetTargetFps(EffectiveLogicFps());
    wnd->SetMaximumFrameLatency(2);
    RunCommands(commands);
}

uint32_t GameEngine::GetInterpolationFPS() {
    if (CVarGetInteger(CVAR_SETTING("MatchRefreshRate"), 0)) {
        return Ship::Context::GetRawInstance()->GetWindow()->GetCurrentRefreshRate();

    } else if (CVarGetInteger(CVAR_VSYNC_ENABLED, 1) ||
               !Ship::Context::GetRawInstance()->GetWindow()->CanDisableVerticalSync()) {
        return std::min<uint32_t>(Ship::Context::GetRawInstance()->GetWindow()->GetCurrentRefreshRate(),
                                  CVarGetInteger(CVAR_SETTING("InterpolationFPS"), 60));
    }

    return CVarGetInteger(CVAR_SETTING("InterpolationFPS"), 30);
}

uint32_t GameEngine::GetInterpolationFrameCount() {
    return static_cast<uint32_t>(SubframesForTarget((int)GetInterpolationFPS()));
}

extern "C" uint32_t GameEngine_GetInterpolationFrameCount() {
    return GameEngine::GetInterpolationFrameCount();
}
