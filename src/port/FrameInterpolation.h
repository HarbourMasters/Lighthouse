#pragma once

#ifdef __cplusplus
#include <unordered_map>
#include <libultraship/libultra/gbi.h>
#include <fast/types.h>

void FrameInterpolation_RecordFrame(Gfx* commands);
std::unordered_map<Mtx*, MtxF> FrameInterpolation_Interpolate(float t);

extern "C" {
#endif

// Identity-scoped interpolation: call before/after an actor's draw function
// to record which Mtx buffer range belongs to that actor. Parameters use void* to
// stay C-compatible (marker_ptr = ActorMarker*, mtx_cursor = current *mtx value).
void FrameInterpolation_ScopeBegin(void* marker_ptr, unsigned int array_idx, void* mtx_cursor);
void FrameInterpolation_ScopeEnd(void* mtx_cursor);

#ifdef __cplusplus
}
#endif
