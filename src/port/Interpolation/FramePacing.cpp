#include "port/Engine.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <future>
#include <thread>
#include <unordered_map>
#include <vector>

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
bool sInterpolationRecorded = false;
std::vector<std::future<void>> sMapBuildFutures;
long long sLastSubFrameNs = 0;
long long sPassBudgetNs = 0;
long long sFrameLatchNs = 0;
int sFrameVis = 2;
unsigned sFrameViSerial = 0;

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
} // namespace

void GameEngine::RunCommands(Gfx* Commands, const std::vector<std::unordered_map<Mtx*, MtxF>>& mtx_replacements,
                             size_t frameCount) {
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetRawInstance()->GetWindow());
    if (wnd == nullptr) {
        return;
    }
    auto interpreter = wnd->GetInterpreterWeak().lock().get();
    wnd->HandleEvents();
    interpreter->mInterpolationIndex = 0;
    auto wndBase = Ship::Context::GetRawInstance()->GetWindow();
    const auto passT0 = Clock::now();
    for (size_t frameIdx = 0; frameIdx < frameCount; frameIdx++) {
        if (frameIdx >= 1 && frameIdx - 1 < sMapBuildFutures.size()) {
            sMapBuildFutures[frameIdx - 1].wait();
        }
        if (frameIdx > 0 && sLastSubFrameNs > 0 && (sPassBudgetNs - NsSince(passT0)) < sLastSubFrameNs) {
            break;
        }
        const auto& m = mtx_replacements[frameIdx];
        const float subframeBlend = (frameCount > 1) ? (float)(frameIdx + 1) / (float)frameCount : 1.0f;
        if (frameCount > 1) {
            FrameInterpolation_ApplyAnimVertices(subframeBlend);
        }
        Nametag::SetSubframeBlend(subframeBlend);
        bool isFinalFrame = (frameIdx == frameCount - 1);
        auto runT0 = Clock::now();
        auto gui = wndBase->GetGui();
        wndBase->GetMouseStateManager()->StartFrame();
        gui->StartDraw();
        interpreter->StartFrame();
        interpreter->Run(Commands, m);
        if (OS_ViBlackActive()) {
            interpreter->mGfxFrameBuffer = 0;
            auto rapi = interpreter->GetCurrentRenderingAPI();
            rapi->StartDrawToFramebuffer(0, 1.0f);
            rapi->ClearFramebuffer(true, false);
        }
        gui->EndDraw();
        sLastSubFrameNs = NsSince(runT0);
        interpreter->EndFrame();
        CALL_EVENT(FrameDrawEnd);
        interpreter->mInterpolationIndex++;
    }
    SyncAltAssets();
}

void GameEngine::SetInterpolationRecorded(bool recorded) {
    sInterpolationRecorded = recorded;
}

namespace {
struct SubframePacing {
    int subframes;
    int fps;
    int viPerTick;
};

int CurrentViPerTick() {
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

int EffectiveLogicFps() {
    int fps = 60 / CurrentViPerTick();
    return (fps < 1) ? 1 : fps;
}

int SubframesForTarget(int targetFps) {
    int subframes = targetFps / EffectiveLogicFps();
    return (subframes < 1) ? 1 : subframes;
}

SubframePacing ComputeSubframePacing() {
    int target_fps = (int)GameEngine::Instance->GetInterpolationFPS();
    int viPerTick = CurrentViPerTick();
    int subframesPerTick = SubframesForTarget(target_fps);

    if (!sInterpolationRecorded) {
        subframesPerTick = 1;
    }

    int fps = subframesPerTick * 60 / viPerTick;
    if (fps < 1) {
        fps = 1;
    }

    return { subframesPerTick, fps, viPerTick };
}
} // namespace

bool GameEngine::IsInterpolationEnabled() {
    return (int)GetInterpolationFPS() > EffectiveLogicFps();
}

int GameEngine::CurrentViPerTick() {
    return ::CurrentViPerTick();
}

void GameEngine::SetFrameTiming(long long latchNs, int viPerTick, unsigned viSerial) {
    sFrameViSerial = viSerial;
    sFrameLatchNs = latchNs;
    sFrameVis = viPerTick > 0 ? viPerTick : 2;
}

namespace {
long long SteadyNs() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count();
}

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
    std::future<void> prefetch;
    float prefetchT = -1.0f;
    const auto countBy = Clock::now() + std::chrono::milliseconds(3);
    while (port_getDemoViSerial() == sFrameViSerial && Clock::now() < countBy) {
        std::this_thread::yield();
    }
    const int vis = std::max(port_getDemoViCount(), 2);
    const long long tickNs = 1000000000LL * vis / 60;
    const long long presentNs = 1000000000LL / fps;
    const long long passStartNs = SteadyNs();
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
        const long long now = SteadyNs();
        if (prevRunNs != 0) {
            runIntervalNs = std::max(presentNs, now - prevRunNs);
        }
        prevRunNs = now;
        // Last when another draw after this one could not be finished by the release deadline.
        const bool last = count + 1 >= kMaxTimedSubframes || now + runIntervalNs + sLastSubFrameNs > releaseBy;
        float t = std::clamp((float)(now + presentNs - anchorNs) / (float)tickNs, 0.0f, 1.0f);
        auto& replacements = maps[count & 1];
        bool prefetched = false;
        if (prefetch.valid()) {
            prefetch.wait();
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
            auto* next = &maps[(count + 1) & 1];
            const float nextT = prefetchT;
            prefetch = std::async(std::launch::async, [nextT, next] { BuildReplacements(nextT, *next); });
        }
        FrameInterpolation_ApplyAnimVertices(t);
        Nametag::SetSubframeBlend(t);

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
        if (last) {
            port_releaseRcpTask();
        }
        interpreter->EndFrame();
        CALL_EVENT(FrameDrawEnd);
        interpreter->mInterpolationIndex++;
        count++;
        if (last) {
            break;
        }
    }
    if (prefetch.valid()) {
        prefetch.wait();
    }
    SyncAltAssets();
}
} // namespace

bool GameEngine::IsTimedPassActive() {
    return IsInterpolationEnabled();
}

void GameEngine::ProcessGfxCommands(Gfx* commands) {
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetRawInstance()->GetWindow());

    if (wnd == nullptr) {
        return;
    }

    // if(gEnableGammaBoost) {
    //     wnd->EnableSRGBMode();
    // }
    wnd->SetRendererUCode(UcodeHandlers::ucode_f3dex);

    static std::vector<std::unordered_map<Mtx*, MtxF>> mtx_replacements;

    if (sInterpolationRecorded && !GfxDebuggerIsDebugging()) {
        const int fps = (int)GetInterpolationFPS();
        if (fps > 60 / sFrameVis) {
            wnd->SetTargetFps(fps);
            wnd->SetMaximumFrameLatency(2);
            RunTimedPass(commands, fps);
            return;
        }
    }

    const SubframePacing pacing = ComputeSubframePacing();
    const int subframesPerTick = pacing.subframes;
    const int fps = pacing.fps;

    if ((int)mtx_replacements.size() < subframesPerTick) {
        mtx_replacements.resize(subframesPerTick);
    }
    size_t activeFrames = 0;
    sMapBuildFutures.clear();
    for (int i = 1; i <= subframesPerTick; i++) {
        if (i < subframesPerTick) {
            float t = (float)i / (float)subframesPerTick;
            if (i == 1) {
                FrameInterpolation_Interpolate(t, mtx_replacements[activeFrames]);
            } else {
                auto* map = &mtx_replacements[activeFrames];
                sMapBuildFutures.push_back(
                    std::async(std::launch::async, [t, map] { FrameInterpolation_Interpolate(t, *map); }));
            }
        } else {
            mtx_replacements[activeFrames].clear();
        }
        activeFrames++;
    }

    sPassBudgetNs = 1000000000LL * pacing.viPerTick / 60;

    if (wnd != nullptr) {
        wnd->SetTargetFps(fps);
        wnd->SetMaximumFrameLatency(2);
    }

    if (GfxDebuggerIsDebugging()) {
        if (mtx_replacements.empty()) {
            mtx_replacements.emplace_back();
        }
        mtx_replacements[0].clear();
        activeFrames = 1;
    }

    RunCommands(commands, mtx_replacements, activeFrames);

    for (auto& f : sMapBuildFutures) {
        if (f.valid()) {
            f.wait();
        }
    }
    sMapBuildFutures.clear();
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
