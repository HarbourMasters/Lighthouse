#include "AdaptiveFps.h"

#include <algorithm>
#include <chrono>

namespace {

using Clock = std::chrono::steady_clock;

// Tuning knobs

// How much of each update's time we'll spend drawing once things are steady.
// Raise toward 1.0 if FPS dips in scenes that look fine; lower if it stutters.
constexpr double kSafetySteady = 0.95;

// Same, but used the moment a scene gets busier (more cautious). Lower if busy
// scenes still hitch; raise if FPS drops more than needed when they start.
constexpr double kSafetyRising = 0.75;

// How much busier a scene must get to count as "getting busy". Lower reacts
// sooner but trips on normal noise; raise to ignore small bumps.
constexpr double kRisingThreshold = 1.20;

// How long to stay cautious after a scene gets busy. Raise if FPS wobbles in
// steady scenes; lower if it feels slow to speed back up.
constexpr int kRisingHoldWindows = 3;

// How often measurements are averaged. Shorter reacts quicker but is jumpier;
// longer is steadier but slower to react.
constexpr auto kSampleWindow = std::chrono::milliseconds(200);

// How much each new measurement counts vs. past ones when averaging.
// 1.0 = only the latest; 0.0 = never changes.
constexpr double kEmaAlpha = 0.5;

// How much busier one frame must be than usual to count as a spike. Lower
// reacts harder to one-off heavy frames; raise to ignore them.
constexpr double kSpikeFactor = 1.25;

// How strongly a spike pulls the estimate up. 1.0 = snap to it instantly;
// 0.0 = ignore spikes. 0.5 lets a sustained busy scene ramp up over a few frames.
constexpr double kSpikeBlend = 0.5;

// How quickly the busy estimate eases back down each averaging period. 0 =
// never; 1 = drop to the latest right away. Higher recovers FPS faster.
constexpr double kPeakDecay = 0.40;

// How fast the reserved game-update time follows the latest tick: jump up
// quickly when a scene gets busy, ease back down slowly when it calms.
constexpr double kLogicRiseAlpha = 0.5;
constexpr double kLogicFallAlpha = 0.1;

// Default game update rate (times per second) if none is given.
constexpr uint32_t kDefaultTickHz = 30;

struct State {
    uint32_t tickHz = kDefaultTickHz;
    double tickBudgetUs = 1'000'000.0 / kDefaultTickHz;
    double emaPerSubFrameUs = 0.0;  // Average time to draw one frame.
    double peakPerSubFrameUs = 0.0; // Busiest recent frame; the cap is based on this.
    double envLogicUs = 0.0;        // Reserved game-update time per tick.
    long long winRunNs = 0;         // Totals for the current measurement period.
    int winSubFrames = 0;
    double winMaxUs = 0.0;
    Clock::time_point winStart = Clock::now();
    double safety = kSafetySteady;
    int risingHoldRemaining = 0;
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
    s.peakPerSubFrameUs = 0.0;
    s.envLogicUs = 0.0;
    s.winRunNs = 0;
    s.winSubFrames = 0;
    s.winMaxUs = 0.0;
    s.winStart = Clock::now();
    s.safety = kSafetySteady;
    s.risingHoldRemaining = 0;
}

uint32_t AdaptiveFps_Cap(uint32_t userTarget) {
    auto& s = state();
    // Base the cap on the busiest recent frame, not the average: every frame we
    // promise gets drawn, so the time has to fit the worst one.
    double costUs = std::max(s.peakPerSubFrameUs, s.emaPerSubFrameUs);
    if (costUs <= 0.0) {
        return userTarget;
    }
    // Set aside the time the game spent updating, then share what's left among
    // interpolated frames so a busy scene can't overshoot the tick's budget.
    double renderBudgetUs = s.tickBudgetUs - s.envLogicUs;
    if (renderBudgetUs <= 0.0) {
        return s.tickHz;
    }
    double maxSubPerTick = (renderBudgetUs * s.safety) / costUs;
    if (maxSubPerTick < 1.0) {
        return s.tickHz;
    }
    uint32_t maxFps = (uint32_t)(maxSubPerTick * s.tickHz);
    if (maxFps < s.tickHz) {
        maxFps = s.tickHz;
    }
    return std::min(userTarget, maxFps);
}

void AdaptiveFps_SampleTick(long long logicNs) {
    auto& s = state();
    double us = (double)logicNs / 1000.0;
    if (us < 0.0) {
        return;
    }
    if (s.envLogicUs == 0.0) {
        s.envLogicUs = us;
    } else {
        // Rise fast when the scene gets busier, fall slow when it calms.
        double alpha = us > s.envLogicUs ? kLogicRiseAlpha : kLogicFallAlpha;
        s.envLogicUs = alpha * us + (1.0 - alpha) * s.envLogicUs;
    }
}

void AdaptiveFps_Sample(long long runNs) {
    auto& s = state();
    double sampleUs = (double)runNs / 1000.0;

    s.winRunNs += runNs;
    s.winSubFrames++;
    if (sampleUs > s.winMaxUs) {
        s.winMaxUs = sampleUs;
    }

    // If one frame is suddenly much busier than usual, bump the estimate up
    // right away and stay cautious, so the next cap already reflects it.
    if (s.peakPerSubFrameUs == 0.0) {
        s.peakPerSubFrameUs = sampleUs;
        s.safety = kSafetyRising;
        s.risingHoldRemaining = kRisingHoldWindows;
    } else if (sampleUs > s.peakPerSubFrameUs * kSpikeFactor) {
        s.peakPerSubFrameUs = kSpikeBlend * sampleUs + (1.0 - kSpikeBlend) * s.peakPerSubFrameUs;
        s.safety = kSafetyRising;
        s.risingHoldRemaining = kRisingHoldWindows;
    }

    auto now = Clock::now();
    if (now - s.winStart < kSampleWindow) {
        return;
    }

    // End of a measurement period: refresh the average, decide whether the
    // scene is still getting busier, and ease the busy estimate back down.
    double mean = (double)s.winRunNs / s.winSubFrames / 1000.0;
    s.emaPerSubFrameUs = s.emaPerSubFrameUs == 0.0 ? mean : kEmaAlpha * mean + (1.0 - kEmaAlpha) * s.emaPerSubFrameUs;

    bool rising = s.peakPerSubFrameUs > 0.0 && s.winMaxUs > s.peakPerSubFrameUs * kRisingThreshold;
    if (rising) {
        s.risingHoldRemaining = kRisingHoldWindows;
    } else if (s.risingHoldRemaining > 0) {
        s.risingHoldRemaining--;
    }
    s.safety = s.risingHoldRemaining > 0 ? kSafetyRising : kSafetySteady;

    s.peakPerSubFrameUs = std::max(s.winMaxUs, s.peakPerSubFrameUs * (1.0 - kPeakDecay) + s.winMaxUs * kPeakDecay);

    s.winRunNs = 0;
    s.winSubFrames = 0;
    s.winMaxUs = 0.0;
    s.winStart = now;
}

} // extern "C"
