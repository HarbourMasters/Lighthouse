#pragma once

#include <cstdint>

// Drop-in adaptive interpolation FPS cap
//
// Libultraship re-runs the entire DL once per sub-frame. On busy scenes
// that's ~93% of per-tick wall-clock, so naively running N sub-frames
// per game tick at MRR can blow the tick budget and stall the sim.
// This module measures per-sub-frame render cost and clamps the user's
// requested interpolation FPS so all sub-frames fit within a configurable
// fraction of one game tick.
//
// Integration (per port, called from your gfx pipeline):
//   AdaptiveFps_Configure(tickHz)              // once at startup
//   uint32_t fps = AdaptiveFps_Cap(userTarget) // before picking sub-frame count
//   AdaptiveFps_Sample(runNs)                  // after each interpreter->Run
//
// Defaults assume a 30 Hz game tick. Override Configure() if your port
// runs the simulation at a different rate (e.g. 60 Hz).
//
// Behaviour across refresh rates:
//   The cap returns the largest FPS the hardware can sustain, clipped by
//   userTarget on top and by tickHz on the bottom. Higher MRR just
//   raises the upper bound — a 144 Hz user with a strong GPU sees 144 in
//   steady scenes and (say) 90 in busy ones; a 60 Hz user with the same
//   GPU sees 60 in steady scenes and 30 in busy ones. The math doesn't
//   know about MRR; it only knows the per-sub-frame cost.
//
//   On 60 Hz displays the floor (= tickHz, 30 by default) means the cap
//   snaps 60 -> 30 with no intermediate stop. This is correct (there's
//   no valid integer sub-frame count between them) but feels more
//   abrupt than the smooth slide higher-MRR users experience.

#ifdef __cplusplus
extern "C" {
#endif

// Set the game tick rate (Hz). Also resets the EMA so a freshly-loaded
// port starts measuring from scratch. Safe to call multiple times.
void AdaptiveFps_Configure(uint32_t tickHz);

// Returns the largest FPS the measured render cost can sustain, clamped
// to userTarget on top and to tickHz on the bottom. Returns userTarget
// unchanged until at least one full sample window has elapsed.
uint32_t AdaptiveFps_Cap(uint32_t userTarget);

// Feed one per-sub-frame render duration in nanoseconds. The EMA updates
// once per sample window (~1 second of accumulated samples).
void AdaptiveFps_Sample(long long runNs);

#ifdef __cplusplus
}
#endif
