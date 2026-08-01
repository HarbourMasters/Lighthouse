#include <cstring>
#include <new>
#include <spdlog/spdlog.h>

extern "C" {
#include <ultra64.h>
#include "functions.h"
#include "libultraship/libultra/gbi.h"
#include "port/Patches/Patches.h"
}

#include "port/Interpolation/FrameInterpolation.h"

namespace {

// A single anim-vertex model runs 500-750 vertices. This covers roughly 20-25
// of them on screen at once; past that a draw falls back to the shared buffer,
// which is exactly the behaviour that existed before this file.
constexpr size_t kVerticesPerSlot = 16384;
constexpr int kSlots = 2;

Vtx* gArena[kSlots] = {};
size_t gUsed = 0;
int gSlot = 0;
bool gWarnedExhausted = false;

} // namespace

extern "C" void port_animVtx_beginTick(void) {
    gSlot = (gSlot + 1) % kSlots;
    gUsed = 0;
    if (gArena[gSlot] == nullptr) {
        gArena[gSlot] = new (std::nothrow) Vtx[kVerticesPerSlot];
    }
}

extern "C" void port_modelRender_snapshotAnimVertices(Gfx** gfx, void* vertices, int32_t count) {
    if (gfx == nullptr || vertices == nullptr || count <= 0) {
        return;
    }
    Vtx* arena = gArena[gSlot];
    if (arena == nullptr || gUsed + (size_t)count > kVerticesPerSlot) {
        return;
    }

    Vtx* copy = arena + gUsed;
    gUsed += (size_t)count;
    std::memcpy(copy, vertices, (size_t)count * sizeof(Vtx));

    // Hand the pose to interpolation. It owns the blend from here: this buffer
    // is private to this draw, so replay can rewrite it per sub-frame.
    FrameInterpolation_RecordAnimVertices(copy, vertices, count);
    gSPSegment((*gfx)++, 0x01, osVirtualToPhysical(copy));
}
