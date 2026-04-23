#pragma once

// Portable nametag overlay. Draws world-space labels via an ImGui foreground
// draw list; port-specific wiring lives in a companion bindings file.

#include <cstdint>

namespace Nametag {

// World -> native-framebuffer-pixel projector. Writes (screen[0], screen[1])
// and returns true if the point is in front of the camera.
using ProjectFn = bool (*)(float pos[3], float* screen);

void SetProjectFn(ProjectFn fn);
void SetNativeFramebufferSize(const int* width, const int* height);
void RegisterOverlay();

void Clear();
void Push(float x, float y, float z, const char* label);

} // namespace Nametag