#pragma once

#include <cstdint>

// Adaptive interpolation FPS cap.
//
// Drawing each extra interpolated frame is expensive, so asking for too many
// in a busy scene can starve the game and make it stutter. This times how long
// drawing takes and lowers the FPS just enough to keep the game smooth, then
// raises it again once the scene clears.
//
// Call order (from the gfx pipeline):
//   AdaptiveFps_Configure(tickHz)              // once at startup
//   AdaptiveFps_SampleTick(logicNs)            // each tick, before Cap
//   AdaptiveFps_Cap(userTarget)                // pick the allowed FPS
//   AdaptiveFps_Sample(runNs)                  // after each frame is drawn
//
// Defaults assume the game runs at 30 fps native; pass a different rate to
// Configure() if not. On a 60 Hz display the result is either 60 or 30 with
// nothing in between (no whole number of frames fits), so it switches directly.

#ifdef __cplusplus
extern "C" {
#endif

// Set how many times per second the game updates. Also clears past
// measurements so a freshly loaded game starts fresh. Safe to call repeatedly.
void AdaptiveFps_Configure(uint32_t tickHz);

// Returns the highest FPS the current scene can sustain, never above what the
// user asked for nor below the game's update rate. Returns the user's value
// until it has measured at least once.
uint32_t AdaptiveFps_Cap(uint32_t userTarget);

// Report how long it took to draw one frame (nanoseconds), so the module can
// gauge how heavy the scene is.
void AdaptiveFps_Sample(long long runNs);

// Report how long the game spent updating this tick before drawing started.
// Adaptive FPS reserves that time so a busy scene can't make it schedule too
// many interpolated frames. Call once per tick, before Cap().
void AdaptiveFps_SampleTick(long long logicNs);

#ifdef __cplusplus
}
#endif
