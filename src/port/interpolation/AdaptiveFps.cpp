#include "AdaptiveFps.h"

#include <algorithm>
#include <chrono>

namespace {

using Clock = std::chrono::steady_clock;

constexpr double kBudgetSafety = 0.80; // fraction of a tick we allow for render
constexpr double kEmaAlpha = 0.5;      // weight on the latest 1s window
constexpr uint32_t kDefaultTickHz = 30;

struct State {
    uint32_t tickHz = kDefaultTickHz;
    double tickBudgetUs = 1'000'000.0 / kDefaultTickHz;
    double emaPerSubFrameUs = 0.0;
    long long winRunNs = 0;
    int winSubFrames = 0;
    Clock::time_point winStart = Clock::now();
};

State& state() {
    static State s;
    return s;
}

} // namespace

extern "C" {

void AdaptiveFps_Configure(uint32_t tickHz) {
    if (tickHz == 0) {
        tickHz = kDefaultTickHz;
    }
    auto& s = state();
    s.tickHz = tickHz;
    s.tickBudgetUs = 1'000'000.0 / tickHz;
    s.emaPerSubFrameUs = 0.0;
    s.winRunNs = 0;
    s.winSubFrames = 0;
    s.winStart = Clock::now();
}

uint32_t AdaptiveFps_Cap(uint32_t userTarget) {
    auto& s = state();
    if (s.emaPerSubFrameUs <= 0.0) {
        return userTarget;
    }
    double maxSubPerTick = (s.tickBudgetUs * kBudgetSafety) / s.emaPerSubFrameUs;
    if (maxSubPerTick < 1.0) {
        return s.tickHz;
    }
    uint32_t maxFps = (uint32_t)(maxSubPerTick * s.tickHz);
    if (maxFps < s.tickHz) {
        maxFps = s.tickHz;
    }
    return std::min(userTarget, maxFps);
}

void AdaptiveFps_Sample(long long runNs) {
    auto& s = state();
    s.winRunNs += runNs;
    s.winSubFrames++;
    auto now = Clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - s.winStart).count() < 1) {
        return;
    }
    double sample = (double)s.winRunNs / s.winSubFrames / 1000.0;
    s.emaPerSubFrameUs =
        s.emaPerSubFrameUs == 0.0 ? sample : kEmaAlpha * sample + (1.0 - kEmaAlpha) * s.emaPerSubFrameUs;
    s.winRunNs = 0;
    s.winSubFrames = 0;
    s.winStart = now;
}

} // extern "C"
