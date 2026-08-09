#pragma once

// Portable nametag overlay. Draws world-space labels via an ImGui foreground
// draw list; port-specific wiring lives in a companion bindings file.

#include <cstdint>

namespace Nametag {

// World -> native-framebuffer-pixel projector. Writes (screen[0], screen[1])
// and returns true if the point is in front of the camera.
using ProjectFn = bool (*)(float pos[3], float* screen);
using DistanceFn = float (*)(float pos[3]);

void SetProjectFn(ProjectFn fn);
void SetDistanceFn(DistanceFn fn);
void SetNativeFramebufferSize(const int* width, const int* height);
void RegisterOverlay();
void BeginDraw();
void Push(uint32_t id, float x, float y, float z, const char* label, float alpha = 1.0f);
float FadeForDistance(float x, float y, float z, float maxDistance);
void SetSubframeBlend(float blend);
void SubmitFrame(const void* key);
void BeginRenderPass(const void* key, bool interpolate);

} // namespace Nametag