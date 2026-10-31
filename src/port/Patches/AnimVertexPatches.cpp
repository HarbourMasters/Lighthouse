#include <cstring>
#include <memory>
#include <new>
#include <vector>

extern "C" {
#include <ultra64.h>
#include "functions.h"
#include "libultraship/libultra/gbi.h"
#include "port/Patches/Patches.h"
#include "RBB/ch/whistleswitch.h"
}

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Interpolation/FrameInterpolation.h"
#include "port/ShipInit.hpp"

namespace {

// A single anim-vertex model runs 500-750 vertices. This covers roughly 20-25
// of them on screen at once; past that a draw gets a block of its own, freed with the slot.
constexpr size_t kVerticesPerSlot = 16384;
constexpr int kSlots = 2;

Vtx* gArena[kSlots] = {};
std::vector<std::unique_ptr<Vtx[]>> gOverflow[kSlots];
size_t gUsed = 0;
int gSlot = 0;

BKVertexList* gTintedVertices = nullptr;

} // namespace

extern "C" void port_animVtx_beginTick(void) {
    gSlot = (gSlot + 1) % kSlots;
    gUsed = 0;
    gOverflow[gSlot].clear();
    if (gArena[gSlot] == nullptr) {
        gArena[gSlot] = new (std::nothrow) Vtx[kVerticesPerSlot];
    }
}

extern "C" void port_modelRender_snapshotAnimVertices(Gfx** gfx, void* vertices, int32_t count) {
    if (gfx == nullptr || vertices == nullptr || count <= 0) {
        return;
    }
    Vtx* arena = gArena[gSlot];
    Vtx* copy;
    if (arena != nullptr && gUsed + (size_t)count <= kVerticesPerSlot) {
        copy = arena + gUsed;
        gUsed += (size_t)count;
    } else {
        copy = gOverflow[gSlot].emplace_back(new Vtx[count]).get();
    }
    std::memcpy(copy, vertices, (size_t)count * sizeof(Vtx));

    // Hand the pose to interpolation. It owns the blend from here: this buffer
    // is private to this draw, so replay can rewrite it per sub-frame.
    FrameInterpolation_RecordAnimVertices(copy, vertices, count);
    gSPSegment((*gfx)++, 0x01, osVirtualToPhysical(copy));
}

static void MarkTintedWhistleSwitch(IEvent* event) {
    Actor* actor = ((OnActorDrawBegin*)event)->actor;
    auto* local = (ActorLocal_RBB_47D0*)&actor->local;
    // chRBBWhistleSwitch_draw's test for the tint
    if (actor->state == 2 && local->unk0 != 0) {
        gTintedVertices = local->unkC;
    }
}

// A pressed whistle switch tints one clone of its vertices in place each draw and frees it on release, while the
// render thread may still be drawing the last tick from it. Its model draws from a private copy instead.
static void RegisterWhistleSwitchVertices_Init() {
    COND_ID_HOOK(OnActorDrawBegin, ACTOR_1BF_WHISTLE_SWITCH_1, EVENT_PRIORITY_NORMAL, true, MarkTintedWhistleSwitch);
    COND_ID_HOOK(OnActorDrawBegin, ACTOR_1C0_WHISTLE_SWITCH_2, EVENT_PRIORITY_NORMAL, true, MarkTintedWhistleSwitch);
    COND_ID_HOOK(OnActorDrawBegin, ACTOR_1C1_WHISTLE_SWITCH_3, EVENT_PRIORITY_NORMAL, true, MarkTintedWhistleSwitch);
    COND_HOOK(OnModelRenderBegin, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        if (gTintedVertices != nullptr) {
            port_modelRender_snapshotAnimVertices(((OnModelRenderBegin*)event)->gfx,
                                                  vtxList_getVertices(gTintedVertices),
                                                  vtxList_getVtxCount(gTintedVertices));
            gTintedVertices = nullptr;
        }
    });
    // A culled model never reaches OnModelRenderBegin.
    COND_HOOK(OnActorDrawEnd, EVENT_PRIORITY_NORMAL, true, [](IEvent*) { gTintedVertices = nullptr; });
}

static RegisterShipInitFunc whistleSwitchVerticesInitFunc(RegisterWhistleSwitchVertices_Init);
