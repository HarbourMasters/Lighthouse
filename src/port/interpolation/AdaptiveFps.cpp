#include "AdaptiveFps.h"

#include <algorithm>
#include <chrono>

namespace {

using Clock = std::chrono::steady_clock;

// Tuning knobs

// Fraction of the tick budget we allow once the peak is trusted. Raise
// toward 1.0 if the cap clamps below your refresh rate in scenes that
// look fine. Lower if you see audio stutter or the game itself slows.
constexpr double kSafetySteady = 0.95;

// Fraction used while the cap is reacting to a transition. Lower if
// transitions into busy areas still pile up; raise if the cap drops
// further than necessary at the start of every busy scene.
constexpr double kSafetyRising = 0.75;

// Multiplier on the running peak that defines a "transition". Lower
// (e.g. 1.10) makes the cap more reactive at the cost of tripping on
// normal noise; raise (e.g. 1.30) to ignore minor spikes and only
// react to large jumps.
constexpr double kRisingThreshold = 1.20;

// How many windows to stay in the rising-safety mode after a trip.
// Raise if the safety factor visibly oscillates in noisy steady scenes;
// lower if recovery to full speed feels sluggish after a transition.
constexpr int kRisingHoldWindows = 3;

// Sample window length. Shorter = the cap recovers faster after a busy
// area passes but is noisier; longer = smoother but slower to react.
constexpr auto kSampleWindow = std::chrono::milliseconds(200);

// EMA weight on the latest window's mean. 1.0 = no smoothing; 0.0 =
// EMA never updates. Affects the floor of the cap calculation.
constexpr double kEmaAlpha = 0.5;

// Multiplier on the peak that defines a single-sample "spike". Lower
// makes the cap react harder to outliers; raise to ignore them.
constexpr double kSpikeFactor = 1.25;

// How much weight a spike sample contributes to the peak (rest stays
// from the previous peak). 1.0 = spike replaces peak instantly (twitchy
// to one-off outliers); 0.0 = ignore spikes (defeats the purpose). 0.5
// is a balance: sustained transitions still escalate over a few samples.
constexpr double kSpikeBlend = 0.5;

// Per-window decay of the peak toward the window's max. 0 = peak sticks
// forever; 1 = peak collapses to the latest window. Raise for snappier
// recovery from one-off spikes; lower if scenes with intermittent heavy
// frames cause the cap to bounce up too eagerly.
constexpr double kPeakDecay = 0.40;

// Set this to the default game logic tick rate.
constexpr uint32_t kDefaultTickHz = 30;

struct State {
    uint32_t tickHz = kDefaultTickHz;
    double tickBudgetUs = 1'000'000.0 / kDefaultTickHz;
    double emaPerSubFrameUs = 0.0;  // EMA of the per-window mean.
    double peakPerSubFrameUs = 0.0; // Cap-driving busiest recent sub-frame.
    long long winRunNs = 0;         // Rolling window accumulators.
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
    s.winRunNs = 0;
    s.winSubFrames = 0;
    s.winMaxUs = 0.0;
    s.winStart = Clock::now();
    s.safety = kSafetySteady;
    s.risingHoldRemaining = 0;
}

uint32_t AdaptiveFps_Cap(uint32_t userTarget) {
    auto& s = state();
    // Cap on the peak rather than the mean: we render every sub-frame we
    // commit to (no frameskip), so the budget has to fit the busiest one.
    double costUs = std::max(s.peakPerSubFrameUs, s.emaPerSubFrameUs);
    if (costUs <= 0.0) {
        return userTarget;
    }
    double maxSubPerTick = (s.tickBudgetUs * s.safety) / costUs;
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
    double sampleUs = (double)runNs / 1000.0;

    s.winRunNs += runNs;
    s.winSubFrames++;
    if (sampleUs > s.winMaxUs) {
        s.winMaxUs = sampleUs;
    }

    // Hot reaction: a single sub-frame far busier than the running peak
    // immediately raises the peak (blended) and arms the rising-safety
    // hold so the next Cap() call already reflects the new reality.
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

    // Window close: refresh the EMA, decide whether the scene is still
    // transitioning, and decay the peak toward the latest window's max.
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
