// BanjoDecomp: (port-specific, no decomp origin)
//
// GBI command interceptors that resolve OTR resource paths embedded in the display
// list. The decomp emits gSPDisplayList/gSPVertex/etc. with pointers that may be
// __OTR__ asset paths instead of raw RAM; these wrappers detect that, swap in the
// loaded resource via the ResourceManager, then forward to the real __gSP* GBI ops.

#include <assert.h>

#include "libultraship/libultra/gbi.h"
#include "libultraship/bridge/resourcebridge.h"
#include "src/port/ResourceHelpers.h"

int ResourceMgr_OTRSigCheck(char* imgData) {
    uintptr_t i = (uintptr_t)(imgData);

    // [port] Guard against small integers masquerading as pointers, which arise when
    // N64 code computes an address from a NULL-based buffer.
    if (i < 0x10000)
        return 0;

    // if ((i & 0xFF000000) != 0xAB000000 && (i & 0xFF000000) != 0xCD000000 && i != 0) {
    if (i != 0) {
        if (imgData[0] == '_' && imgData[1] == '_' && imgData[2] == 'O' && imgData[3] == 'T' && imgData[4] == 'R' &&
            imgData[5] == '_' && imgData[6] == '_')
            return 1;
    }

    return 0;
}

void gSPDisplayList(Gfx* pkt, Gfx* dl) {
    char* imgData = (char*)dl;

    if (ResourceMgr_OTRSigCheck(imgData) == 1) {

        // ResourceMgr_PushCurrentDirectory(imgData);
        // gsSPPushCD(pkt++, imgData);
        dl = ResourceMgr_LoadGfxByName(imgData);
    }

    __gSPDisplayList(pkt, dl);
}

void gSPDisplayListOffset(Gfx* pkt, Gfx* dl, int offset) {
    char* imgData = (char*)dl;

    if (ResourceMgr_OTRSigCheck(imgData) == 1)
        dl = ResourceMgr_LoadGfxByName(imgData);

    __gSPDisplayList(pkt, dl + offset);
}

// [port] `v` is a host pointer or an __OTR__ path. It is never a segmented address:
// callers that mean "segment N, offset off" use gSPVertexSeg.
void gSPVertex(Gfx* pkt, uintptr_t v, int n, int v0) {
    if (ResourceMgr_OTRSigCheck((char*)v) == 1)
        v = (uintptr_t)ResourceMgr_LoadVtxByName((char*)v);

    __gSPVertex(pkt, v, n, v0);
}

// [port] Emit a vertex command that references segment `seg` at byte offset `off`.
// Bit 0 is the tag Fast::Interpreter::SegAddr keys on to resolve against its segment
// table; host pointers are always even, so the two can never be confused.
void gSPVertexSeg(Gfx* pkt, u32 seg, uintptr_t off, int n, int v0) {
    assert(seg < 16);
    assert(off <= 0x00FFFFFE);

    __gSPVertex(pkt, ((uintptr_t)(seg & 0xF) << 24) | (off & 0x00FFFFFEu) | 1u, n, v0);
}

void gSPInvalidateTexCache(Gfx* pkt, uintptr_t texAddr) {
    char* imgData = (char*)texAddr;

    if (texAddr != 0 && ResourceMgr_OTRSigCheck(imgData)) {
        // Temporary solution to the mq/nonmq issue, this will be
        // handled better with LUS 1.0
        texAddr = (uintptr_t)ResourceMgr_LoadTexOrDListByName(imgData);
    }

    __gSPInvalidateTexCache(pkt, texAddr);
}

void gSPSegment(void* value, int segNum, uintptr_t target) {
    // [port] BK never passes OTR paths through gSPSegment — segment addresses are
    // always raw data pointers (vertices, textures, render mode tables). Skipping
    // the OTR signature check avoids reading from potentially-freed model blob memory.
    __gSPSegment(value, segNum, target);
}

void gSPSegmentLoadRes(void* value, int segNum, uintptr_t target) {
    char* imgData = (char*)target;

    int res = ResourceMgr_OTRSigCheck(imgData);

    if (res) {
        target = (uintptr_t)ResourceMgr_LoadTexOrDListByName(imgData);
    }

    __gSPSegment(value, segNum, target);
}

// The OtrSignatureCheck and gfx_check_image_signature in LUS have been patched to
// handle raw heap pointers without crashing (byte-by-byte check, address filtering).
// Raw sprite texture data embedded in BKSprite structures passes through as-is;
// OTR-tagged model textures (__OTR__ paths) are handled by LUS natively.
void port_gDPSetTextureImage(Gfx* pkt, int fmt, int siz, int width, const void* img) {
    Gfx* _g = (Gfx*)(pkt);
    _g->words.w0 = _SHIFTL(G_SETTIMG, 24, 8) | _SHIFTL(fmt, 21, 3) | _SHIFTL(siz, 19, 2) | _SHIFTL((width)-1, 0, 12);
    _g->words.w1 = (uintptr_t)(img);
}
