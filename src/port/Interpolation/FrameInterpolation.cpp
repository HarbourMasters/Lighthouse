#include "FrameInterpolation.h"

#include <atomic>
#include <spdlog/spdlog.h>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <unordered_map>

#include <libultraship/libultra/gu.h>
#include <libultraship/libultra/gbi.h>

namespace {

constexpr uint64_t kFnvSeed = 0xcbf29ce484222325ULL;
constexpr uint64_t kSigKindToMtx = 0x1ULL;
constexpr uint64_t kSigKindSprite = 0x2ULL;
constexpr uint64_t kSigKindAnimVtx = 0x3ULL;
constexpr uint64_t kSigKindProjRot = 0x4ULL;
constexpr float kAnimVtxSnapRatio = 0.5f;
constexpr float kSpriteSnapDistSq = 300.0f * 300.0f;
constexpr int kRingSlots = 4;
constexpr uint32_t kAmbiguousSig = UINT32_MAX;

struct ToMtxData {
    void* dst;
    float src[4][4];
    uint64_t pathSig;
    bool noInterpolate;
    bool camRelFixup;
    bool ambiguousSig;
};

struct AnimVtxData {
    void* dst;
    uint32_t count;
    uint64_t pathSig;
    bool ambiguousSig;
    std::vector<int16_t> pos;
};

struct SpriteDrawData {
    void* dst;
    float camRelPos[3];
    float scale[3];
    float camYaw;
    float camPitch;
    float spriteRoll;
    float rotation[3];
    uint64_t pathSig;
    uint8_t kind;
    bool mirrored;
    bool ambiguousSig;
};

struct CameraProjRotData {
    void* rollMtx;
    void* pitchMtx;
    void* yawMtx;
    float rollDeg;
    float pitchDeg;
    float yawDeg;
    uint64_t pathSig;
    bool ambiguousSig;
};

struct ScopeFrame {
    uint64_t pathHash;
    uint32_t toMtxIdx;
    uint32_t spriteIdx;
    uint32_t animVtxIdx;
    uint32_t projRotIdx;
    const char* key;
    uintptr_t id;
};

struct FrameTree {
    std::vector<ToMtxData> toMtxs;
    std::vector<CameraProjRotData> projRots;
    std::vector<SpriteDrawData> sprites;
    std::vector<AnimVtxData> animVtxs;
    std::unordered_map<uint64_t, uint32_t> sigToToMtx;
    std::unordered_map<uint64_t, uint32_t> sigToSprite;
    std::unordered_map<uint64_t, uint32_t> sigToAnimVtx;
    std::unordered_map<uint64_t, uint32_t> sigToProjRot;
    std::vector<ScopeFrame> scopeStack;
    float cameraPos[3] = { 0.0f, 0.0f, 0.0f };
    bool hasCameraPos = false;
    bool valid = false;
    uint32_t sigCollisions = 0;
    std::atomic<uint32_t> generation{ 0 };

    void reset() {
        generation.fetch_add(1, std::memory_order_release);
        toMtxs.clear();
        projRots.clear();
        sprites.clear();
        animVtxs.clear();
        sigToToMtx.clear();
        sigToSprite.clear();
        sigToAnimVtx.clear();
        sigToProjRot.clear();
        scopeStack.clear();
        cameraPos[0] = cameraPos[1] = cameraPos[2] = 0.0f;
        hasCameraPos = false;
        valid = false;
        sigCollisions = 0;
    }
};

struct PairedToMtx {
    uint32_t currIdx;
    uint32_t prevIdx;
    bool snap;
};

struct PairedSprite {
    uint32_t currIdx;
    uint32_t prevIdx;
    bool snap;
};

struct PairedAnimVtx {
    uint32_t currIdx;
    uint32_t prevIdx;
    bool snap;
};

struct PairedProjRot {
    uint32_t currIdx;
    uint32_t prevIdx;
};

struct InterpolationCache {
    bool built = false;
    bool valid = false;
    std::vector<PairedToMtx> pairedToMtxs;
    std::vector<PairedSprite> pairedSprites;
    std::vector<PairedAnimVtx> pairedAnimVtxs;
    std::vector<PairedProjRot> pairedProjRots;
    std::vector<uint32_t> camRelFixups;
    float cameraDelta[3] = { 0.0f, 0.0f, 0.0f };
    bool hasCameraDelta = false;
    bool prevHasCameraPos = false;
    float prevCameraPos[3] = { 0.0f, 0.0f, 0.0f };

    void reset() {
        built = false;
        valid = false;
        pairedToMtxs.clear();
        pairedSprites.clear();
        pairedAnimVtxs.clear();
        pairedProjRots.clear();
        camRelFixups.clear();
        hasCameraDelta = false;
        prevHasCameraPos = false;
    }
};

// Ring of trees
FrameTree gRing[kRingSlots];
std::atomic<int> gSlotClaims[kRingSlots] = {};
int gRecordSlot = 0;
int gLastRecordedSlot = -1;
FrameTree* gRecord = &gRing[0];
FrameTree* gRenderPrev = nullptr;
FrameTree* gRenderCurr = nullptr;
bool gRenderShould = false;

bool gRecording = false;
bool gShouldInterpolate = true;
int gNoInterpolateDepth = 0;
int gCameraRelativeDepth = 0;

std::unordered_map<const void*, uint64_t> gIdMap;
uint64_t gNextId = 1;

InterpolationCache gCache;

// First scope identity to collide this tick, reported once by StopRecord so a
// busy frame cannot flood the log. Cleared per tick in StartRecord; nullptr
// means nothing has collided yet.
const char* gCollidedScopeKey = nullptr;
uintptr_t gCollidedScopeId = 0;
const char* gCollidedParentKey = nullptr;
uintptr_t gCollidedParentId = 0;

inline uint64_t fnvMix(uint64_t h, uint64_t v) {
    h ^= v;
    h *= 0x100000001b3ULL;
    return h;
}

template <typename Map, typename Vec>
void recordSigCollision(Map& sigMap, Vec& entries, uint64_t sig, uint32_t idx, uint32_t& collisionCount,
                        const std::vector<ScopeFrame>& stack) {
    const ScopeFrame& scope = stack.back();
    auto [it, inserted] = sigMap.emplace(sig, idx);
    if (inserted) {
        return;
    }
    if (it->second < entries.size()) {
        entries[it->second].ambiguousSig = true;
    }
    entries[idx].ambiguousSig = true;
    it->second = kAmbiguousSig;
    collisionCount++;
    if (gCollidedScopeKey == nullptr) {
        gCollidedScopeKey = scope.key;
        gCollidedScopeId = scope.id;
        if (stack.size() >= 2) {
            const ScopeFrame& parent = stack[stack.size() - 2];
            gCollidedParentKey = parent.key;
            gCollidedParentId = parent.id;
        }
    }
}

} // namespace

extern "C" {

void FrameInterpolation_StartRecord(void) {
    for (int i = 0; i < kRingSlots; i++) {
        gRecordSlot = (gRecordSlot + 1) % kRingSlots;
        if (gSlotClaims[gRecordSlot].load(std::memory_order_acquire) == 0 && gRecordSlot != gLastRecordedSlot) {
            break;
        }
    }
    if (gSlotClaims[gRecordSlot].load(std::memory_order_acquire) != 0) {
        // Should be impossible, but if it ever prints, a claim leaked.
        SPDLOG_WARN("interp record forced into claimed slot {} claims=[{},{},{},{}]", gRecordSlot,
                    gSlotClaims[0].load(), gSlotClaims[1].load(), gSlotClaims[2].load(), gSlotClaims[3].load());
    }
    gRecord = &gRing[gRecordSlot];
    gCollidedScopeKey = nullptr;
    gCollidedScopeId = 0;
    gCollidedParentKey = nullptr;
    gCollidedParentId = 0;
    gRecord->reset();
    gRecord->toMtxs.reserve(512);
    gRecord->projRots.reserve(16);
    gRecord->sprites.reserve(256);
    gRecord->sigToToMtx.reserve(512);
    gRecord->sigToSprite.reserve(64);
    gRecord->sigToProjRot.reserve(16);
    gRecord->scopeStack.push_back({ kFnvSeed, 0, 0, 0, 0, "root", 0 });
    gShouldInterpolate = true;
    gNoInterpolateDepth = 0;
    gCameraRelativeDepth = 0;
    gRecording = true;
}

// Idempotent: a threaded submitter has to close the session early, before the
// tree goes to another thread, so the tick's own call then finds it closed.
void FrameInterpolation_StopRecord(void) {
    if (!gRecording) {
        return;
    }
    gRecording = false;
    gRecord->valid = gShouldInterpolate;
    gLastRecordedSlot = gRecordSlot;

    // One line per affected tick, not per collision: a scope whose id repeats
    // usually repeats for every instance behind it.
    if (gRecord->sigCollisions != 0) {
        static uint32_t sReported = 0;
        if (sReported < 20) {
            sReported++;
            SPDLOG_WARN("interp scope id collided {}x this tick under \"{}\" id={:#x} > \"{}\" id={:#x} "
                        "({} matrices, {} sprites)",
                        gRecord->sigCollisions, gCollidedParentKey != nullptr ? gCollidedParentKey : "?",
                        gCollidedParentId, gCollidedScopeKey != nullptr ? gCollidedScopeKey : "?", gCollidedScopeId,
                        gRecord->toMtxs.size(), gRecord->sprites.size());
        }
    }
}

// The pair for the tick being recorded right now, grabbed when a display list
// is handed off so it renders against the trees it was built from.
void FrameInterpolation_GetRecordingPair(int* prevSlot, int* currSlot, bool* shouldInterpolate) {
    if (!gRecording) {
        *prevSlot = -1;
        *currSlot = -1;
        *shouldInterpolate = false;
        return;
    }
    *prevSlot = gLastRecordedSlot;
    *currSlot = gRecordSlot;
    *shouldInterpolate = gShouldInterpolate;
}

// Claim and release bracket a pair's render lifetime: claimed on the tick side
// at submission, released once the render (and its workers) are done reading.
void FrameInterpolation_ClaimPair(int prevSlot, int currSlot) {
    if (prevSlot >= 0 && prevSlot < kRingSlots) {
        gSlotClaims[prevSlot].fetch_add(1, std::memory_order_release);
    }
    if (currSlot >= 0 && currSlot < kRingSlots) {
        gSlotClaims[currSlot].fetch_add(1, std::memory_order_release);
    }
}

void FrameInterpolation_ReleasePair(int prevSlot, int currSlot) {
    if (prevSlot >= 0 && prevSlot < kRingSlots) {
        gSlotClaims[prevSlot].fetch_sub(1, std::memory_order_release);
    }
    if (currSlot >= 0 && currSlot < kRingSlots) {
        gSlotClaims[currSlot].fetch_sub(1, std::memory_order_release);
    }
}

// Pins the trees this pass blends and drops the previous pass's pairing cache.
void FrameInterpolation_BeginRenderPass(int prevSlot, int currSlot, bool shouldInterpolate) {
    gRenderPrev = (prevSlot >= 0 && prevSlot < kRingSlots) ? &gRing[prevSlot] : nullptr;
    gRenderCurr = (currSlot >= 0 && currSlot < kRingSlots) ? &gRing[currSlot] : nullptr;
    gRenderShould = shouldInterpolate && gRenderCurr != nullptr && gRenderPrev != nullptr;
    gCache.built = false;
    gCache.valid = false;
}

void FrameInterpolation_ShouldInterpolateFrame(bool shouldInterpolate) {
    gShouldInterpolate = shouldInterpolate;
}

void FrameInterpolation_RecordOpenChild(const void* key, uintptr_t id) {
    if (!gRecording) {
        return;
    }
    uint64_t h = gRecord->scopeStack.back().pathHash;
    h = fnvMix(h, reinterpret_cast<uintptr_t>(key));
    h = fnvMix(h, static_cast<uint64_t>(id));
    gRecord->scopeStack.push_back({ h, 0, 0, 0, 0, static_cast<const char*>(key), id });
}

void FrameInterpolation_RecordCloseChild(void) {
    if (!gRecording) {
        return;
    }
    if (gRecord->scopeStack.size() > 1) {
        gRecord->scopeStack.pop_back();
    }
}

uintptr_t FrameInterpolation_Hash3(uint64_t a, uint64_t b, uint64_t c) {
    uint64_t h = a * 0x9E3779B97F4A7C15ULL;
    h ^= b * 0x100000001b3ULL;
    h ^= c * 0xbf58476d1ce4e5b9ULL;
    return static_cast<uintptr_t>(h);
}

void FrameInterpolation_RecordOpenChildHash3(const char* key, uint64_t a, uint64_t b, uint64_t c) {
    FrameInterpolation_RecordOpenChild(key, FrameInterpolation_Hash3(a, b, c));
}

void FrameInterpolation_RecordMatrixToMtx(void* dst, const float src[4][4]) {
    if (!gRecording) {
        return;
    }
    ScopeFrame& scope = gRecord->scopeStack.back();
    uint64_t sig = fnvMix(fnvMix(scope.pathHash, kSigKindToMtx), scope.toMtxIdx);
    scope.toMtxIdx++;

    uint32_t idx = static_cast<uint32_t>(gRecord->toMtxs.size());
    gRecord->toMtxs.emplace_back();
    ToMtxData& d = gRecord->toMtxs.back();
    d.dst = dst;
    std::memcpy(d.src, src, sizeof(float) * 16);
    d.pathSig = sig;
    d.noInterpolate = (gNoInterpolateDepth > 0 || gCameraRelativeDepth > 0);
    d.camRelFixup = (gCameraRelativeDepth > 0);
    d.ambiguousSig = false;
    recordSigCollision(gRecord->sigToToMtx, gRecord->toMtxs, sig, idx, gRecord->sigCollisions, gRecord->scopeStack);
}

void FrameInterpolation_RecordCameraProjectionRotation(void* rollMtx, float rollDeg, void* pitchMtx, float pitchDeg,
                                                       void* yawMtx, float yawDeg) {
    if (!gRecording) {
        return;
    }

    ScopeFrame& scope = gRecord->scopeStack.back();
    uint64_t sig = fnvMix(fnvMix(scope.pathHash, kSigKindProjRot), scope.projRotIdx);
    scope.projRotIdx++;

    uint32_t idx = static_cast<uint32_t>(gRecord->projRots.size());
    gRecord->projRots.emplace_back();
    CameraProjRotData& d = gRecord->projRots.back();
    d.rollMtx = rollMtx;
    d.pitchMtx = pitchMtx;
    d.yawMtx = yawMtx;
    d.rollDeg = rollDeg;
    d.pitchDeg = pitchDeg;
    d.yawDeg = yawDeg;
    d.pathSig = sig;
    d.ambiguousSig = false;
    recordSigCollision(gRecord->sigToProjRot, gRecord->projRots, sig, idx, gRecord->sigCollisions, gRecord->scopeStack);
}

void FrameInterpolation_RecordCameraPosition(const float pos[3]) {
    if (!gRecording || pos == nullptr) {
        return;
    }
    gRecord->cameraPos[0] = pos[0];
    gRecord->cameraPos[1] = pos[1];
    gRecord->cameraPos[2] = pos[2];
    gRecord->hasCameraPos = true;
}

void FrameInterpolation_NoInterpolatePush(void) {
    gNoInterpolateDepth++;
}

void FrameInterpolation_NoInterpolatePop(void) {
    if (gNoInterpolateDepth > 0) {
        gNoInterpolateDepth--;
    }
}

void FrameInterpolation_CameraRelativePush(void) {
    gCameraRelativeDepth++;
}

void FrameInterpolation_CameraRelativePop(void) {
    if (gCameraRelativeDepth > 0) {
        gCameraRelativeDepth--;
    }
}

void FrameInterpolation_RecordSpriteDraw(int kind, void* dst, const float camRelPos[3], const float scale[3],
                                         float camYaw, float camPitch, float spriteRoll, const float rotation[3],
                                         int mirrored) {
    if (!gRecording) {
        return;
    }
    ScopeFrame& scope = gRecord->scopeStack.back();
    uint64_t sig = fnvMix(fnvMix(scope.pathHash, kSigKindSprite), scope.spriteIdx);
    scope.spriteIdx++;

    uint32_t idx = static_cast<uint32_t>(gRecord->sprites.size());
    gRecord->sprites.emplace_back();
    SpriteDrawData& d = gRecord->sprites.back();
    d.dst = dst;
    std::memcpy(d.camRelPos, camRelPos, sizeof(float) * 3);
    std::memcpy(d.scale, scale, sizeof(float) * 3);
    d.camYaw = camYaw;
    d.camPitch = camPitch;
    d.spriteRoll = spriteRoll;
    if (rotation != nullptr) {
        std::memcpy(d.rotation, rotation, sizeof(float) * 3);
    } else {
        d.rotation[0] = d.rotation[1] = d.rotation[2] = 0.0f;
    }
    d.pathSig = sig;
    d.kind = static_cast<uint8_t>(kind);
    d.mirrored = (mirrored != 0);
    d.ambiguousSig = false;
    recordSigCollision(gRecord->sigToSprite, gRecord->sprites, sig, idx, gRecord->sigCollisions, gRecord->scopeStack);
}

void FrameInterpolation_RecordAnimVertices(void* dst, const void* vertices, int32_t count) {
    if (!gRecording || dst == nullptr || vertices == nullptr || count <= 0) {
        return;
    }
    ScopeFrame& scope = gRecord->scopeStack.back();
    uint64_t sig = fnvMix(fnvMix(scope.pathHash, kSigKindAnimVtx), scope.animVtxIdx);
    scope.animVtxIdx++;

    uint32_t idx = static_cast<uint32_t>(gRecord->animVtxs.size());
    gRecord->animVtxs.emplace_back();
    AnimVtxData& d = gRecord->animVtxs.back();
    d.dst = dst;
    d.count = static_cast<uint32_t>(count);
    d.pathSig = sig;
    d.ambiguousSig = false;
    d.pos.resize(static_cast<size_t>(count) * 3);

    // Only ob[] moves; everything else in the Vtx (uv, colour/normal) is what
    // the tick's own copy already put in dst.
    const Vtx* src = static_cast<const Vtx*>(vertices);
    for (int32_t i = 0; i < count; i++) {
        d.pos[i * 3 + 0] = src[i].v.ob[0];
        d.pos[i * 3 + 1] = src[i].v.ob[1];
        d.pos[i * 3 + 2] = src[i].v.ob[2];
    }
    recordSigCollision(gRecord->sigToAnimVtx, gRecord->animVtxs, sig, idx, gRecord->sigCollisions, gRecord->scopeStack);
}

void FrameInterpolation_DontInterpolateCamera(void) {
    if (gLastRecordedSlot >= 0) {
        gRing[gLastRecordedSlot].valid = false;
    }
}

uintptr_t FrameInterpolation_RegisterId(const void* ptr) {
    if (ptr == nullptr) {
        return 0;
    }
    uint64_t id = gNextId++;
    gIdMap[ptr] = id;
    return static_cast<uintptr_t>(id);
}

uintptr_t FrameInterpolation_GetId(const void* ptr) {
    if (ptr == nullptr) {
        return 0;
    }
    auto it = gIdMap.find(ptr);
    if (it != gIdMap.end()) {
        return static_cast<uintptr_t>(it->second);
    }
    return reinterpret_cast<uintptr_t>(ptr);
}

void FrameInterpolation_UnregisterId(const void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    gIdMap.erase(ptr);
}

} // extern "C"

namespace {

// Blending across a >90 degree turn drags the model through itself for a frame;
// snap to the new orientation instead.
inline bool shouldSnap(const float pa[4][4], const float ca[4][4]) {
    float dotX = pa[0][0] * ca[0][0] + pa[0][1] * ca[0][1] + pa[0][2] * ca[0][2];
    float dotY = pa[1][0] * ca[1][0] + pa[1][1] * ca[1][1] + pa[1][2] * ca[1][2];
    return dotX < 0.0f || dotY < 0.0f;
}

inline float lerpAngleDegSP(float a, float b, float tt) {
    float d = b - a;
    d = std::fmod(d + 540.0f, 360.0f) - 180.0f;
    return a + d * tt;
}

void BuildInterpolationCache() {
    gCache.reset();
    gCache.built = true;
    if (gRenderPrev == nullptr || gRenderCurr == nullptr || !gRenderPrev->valid) {
        return;
    }
    const uint32_t prevGen = gRenderPrev->generation.load(std::memory_order_acquire);
    const uint32_t currGen = gRenderCurr->generation.load(std::memory_order_acquire);

    gCache.prevHasCameraPos = gRenderPrev->hasCameraPos;
    if (gCache.prevHasCameraPos) {
        gCache.prevCameraPos[0] = gRenderPrev->cameraPos[0];
        gCache.prevCameraPos[1] = gRenderPrev->cameraPos[1];
        gCache.prevCameraPos[2] = gRenderPrev->cameraPos[2];
    }

    // Same two positions the cut heuristic below compares, so a fixup can never
    // shift further than the cut threshold before Interpolate() bails outright.
    gCache.hasCameraDelta = false;
    if (gRenderPrev->hasCameraPos && gRenderCurr->hasCameraPos) {
        for (int k = 0; k < 3; k++) {
            gCache.cameraDelta[k] = gRenderCurr->cameraPos[k] - gRenderPrev->cameraPos[k];
            gCache.hasCameraDelta |= (gCache.cameraDelta[k] != 0.0f);
        }
    }

    const std::vector<ToMtxData>& currToMtxs = gRenderCurr->toMtxs;
    const std::vector<ToMtxData>& prevToMtxs = gRenderPrev->toMtxs;
    const std::vector<SpriteDrawData>& currSprites = gRenderCurr->sprites;
    const std::vector<SpriteDrawData>& prevSprites = gRenderPrev->sprites;
    const std::unordered_map<uint64_t, uint32_t>& prevSigMap = gRenderPrev->sigToToMtx;
    const std::unordered_map<uint64_t, uint32_t>& prevSpriteMap = gRenderPrev->sigToSprite;

    gCache.pairedToMtxs.reserve(currToMtxs.size());
    gCache.pairedSprites.reserve(currSprites.size());

    // Camera/viewport projection rotations, matched by path signature. Anything
    // without a partner this tick keeps the matrix the game already wrote, which
    // is this tick's own end state.
    const std::vector<CameraProjRotData>& currProjRots = gRenderCurr->projRots;
    const std::vector<CameraProjRotData>& prevProjRots = gRenderPrev->projRots;
    gCache.pairedProjRots.reserve(currProjRots.size());
    for (uint32_t i = 0; i < currProjRots.size(); i++) {
        const CameraProjRotData& c = currProjRots[i];
        if (c.ambiguousSig) {
            continue;
        }
        auto it = gRenderPrev->sigToProjRot.find(c.pathSig);
        if (it == gRenderPrev->sigToProjRot.end() || it->second >= prevProjRots.size()) {
            continue;
        }
        if (prevProjRots[it->second].ambiguousSig) {
            continue;
        }
        gCache.pairedProjRots.push_back({ i, it->second });
    }

    // Whatever stays out of the cache falls back to the interpreter decoding
    // the Mtx bytes the game itself wrote, which already hold the correct
    // end-of-tick matrix. Entries with no previous partner, or whose
    // matrix didn't change, can simply be dropped here.
    for (uint32_t i = 0; i < currToMtxs.size(); i++) {
        const ToMtxData& c = currToMtxs[i];
        if (c.dst == nullptr) {
            continue;
        }
        // Camera-relative fixups skip pairing entirely.
        if (c.camRelFixup) {
            if (gCache.hasCameraDelta) {
                gCache.camRelFixups.push_back(i);
            }
            continue;
        }
        if (c.noInterpolate || c.ambiguousSig) {
            continue;
        }
        auto it = prevSigMap.find(c.pathSig);
        if (it == prevSigMap.end() || it->second >= prevToMtxs.size()) {
            continue;
        }
        const ToMtxData& p = prevToMtxs[it->second];
        if (std::memcmp(p.src, c.src, sizeof(c.src)) == 0) {
            continue;
        }
        gCache.pairedToMtxs.push_back({ i, it->second, shouldSnap(p.src, c.src) });
    }

    // Sprites whose inputs didn't move at all are dropped for the same
    // reason; the rest get their matrix rebuilt every sub-frame so billboards
    // keep facing the blended camera.
    for (uint32_t i = 0; i < currSprites.size(); i++) {
        const SpriteDrawData& c = currSprites[i];
        if (c.ambiguousSig) {
            continue;
        }
        auto it = prevSpriteMap.find(c.pathSig);
        if (it == prevSpriteMap.end() || it->second >= prevSprites.size() || prevSprites[it->second].kind != c.kind) {
            gCache.pairedSprites.push_back({ i, UINT32_MAX, false });
            continue;
        }
        const SpriteDrawData& p = prevSprites[it->second];
        if (std::memcmp(p.camRelPos, c.camRelPos, sizeof(c.camRelPos)) == 0 &&
            std::memcmp(p.scale, c.scale, sizeof(c.scale)) == 0 && p.camYaw == c.camYaw && p.camPitch == c.camPitch &&
            p.spriteRoll == c.spriteRoll && std::memcmp(p.rotation, c.rotation, sizeof(c.rotation)) == 0 &&
            p.mirrored == c.mirrored) {
            continue;
        }
        bool snap = false;
        if (gRenderPrev->hasCameraPos && gRenderCurr->hasCameraPos) {
            float d[3];
            for (int k = 0; k < 3; k++) {
                d[k] = (c.camRelPos[k] + gRenderCurr->cameraPos[k]) - (p.camRelPos[k] + gRenderPrev->cameraPos[k]);
            }
            snap = (d[0] * d[0] + d[1] * d[1] + d[2] * d[2]) > kSpriteSnapDistSq;
        }
        gCache.pairedSprites.push_back({ i, it->second, snap });
    }

    // Same pairing as the sprite path. A vertex blend can't go degenerate the way
    // a matrix lerp does, but blending across an animation cut still drags the
    // model through itself, so measure the jump against the model's own size.
    const std::vector<AnimVtxData>& currAnimVtxs = gRenderCurr->animVtxs;
    const std::vector<AnimVtxData>& prevAnimVtxs = gRenderPrev->animVtxs;
    const std::unordered_map<uint64_t, uint32_t>& prevAnimVtxMap = gRenderPrev->sigToAnimVtx;
    gCache.pairedAnimVtxs.reserve(currAnimVtxs.size());
    for (uint32_t i = 0; i < currAnimVtxs.size(); i++) {
        const AnimVtxData& c = currAnimVtxs[i];
        if (c.dst == nullptr || c.ambiguousSig) {
            continue;
        }
        auto it = prevAnimVtxMap.find(c.pathSig);
        if (it == prevAnimVtxMap.end() || it->second >= prevAnimVtxs.size()) {
            continue;
        }
        const AnimVtxData& p = prevAnimVtxs[it->second];
        // A different vertex count means the sig landed on a different model.
        if (p.count != c.count || p.pos.size() != c.pos.size()) {
            continue;
        }
        if (p.pos == c.pos) {
            continue; // pose held still; dst already has it
        }
        int32_t maxExtent = 0;
        int32_t maxDelta = 0;
        for (size_t k = 0; k < c.pos.size(); k++) {
            int32_t a = std::abs(static_cast<int32_t>(c.pos[k]));
            int32_t d = std::abs(static_cast<int32_t>(c.pos[k]) - static_cast<int32_t>(p.pos[k]));
            if (a > maxExtent) {
                maxExtent = a;
            }
            if (d > maxDelta) {
                maxDelta = d;
            }
        }
        bool snap = maxExtent > 0 && static_cast<float>(maxDelta) > kAnimVtxSnapRatio * static_cast<float>(maxExtent);
        gCache.pairedAnimVtxs.push_back({ i, it->second, snap });
    }

    // A tree recycled while we walked it means every index above is suspect.
    // Drop the pass to uninterpolated and say which slot collided.
    if (gRenderPrev->generation.load(std::memory_order_acquire) != prevGen ||
        gRenderCurr->generation.load(std::memory_order_acquire) != currGen) {
        SPDLOG_WARN("interp tree recycled mid-pass: prev={} curr={} record={} last={} claims=[{},{},{},{}]",
                    (int)(gRenderPrev - gRing), (int)(gRenderCurr - gRing), gRecordSlot, gLastRecordedSlot,
                    gSlotClaims[0].load(), gSlotClaims[1].load(), gSlotClaims[2].load(), gSlotClaims[3].load());
        gCache.reset();
        gCache.built = true;
        return;
    }
    gCache.valid = true;
}

// Rebuilds the sprite modelview the same way sprite/render.c composes it, so
// a replayed matrix can't drift from what the game would have written.
void emitSprite(const SpriteDrawData& L, std::unordered_map<Mtx*, MtxF>& replacements) {
    if (L.dst == nullptr) {
        return;
    }
    float m[4][4];
    std::memset(m, 0, sizeof(m));
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;

    auto rotYaw = [&](float deg) {
        if (deg == 0.0f)
            return;
        float rad = deg * 0.017453292519943295f;
        float c = std::cos(rad), sn = std::sin(rad);
        for (int i = 0; i < 3; i++) {
            float r0 = m[0][i], r2 = m[2][i];
            m[0][i] = r0 * c - r2 * sn;
            m[2][i] = r0 * sn + r2 * c;
        }
    };
    auto rotPitch = [&](float deg) {
        if (deg == 0.0f)
            return;
        float rad = deg * 0.017453292519943295f;
        float c = std::cos(rad), sn = std::sin(rad);
        for (int i = 0; i < 3; i++) {
            float r1 = m[1][i], r2 = m[2][i];
            m[1][i] = r1 * c + r2 * sn;
            m[2][i] = -r1 * sn + r2 * c;
        }
    };
    auto rotRoll = [&](float deg) {
        if (deg == 0.0f)
            return;
        float rad = deg * 0.017453292519943295f;
        float c = std::cos(rad), sn = std::sin(rad);
        for (int i = 0; i < 3; i++) {
            float r0 = m[0][i], r1 = m[1][i];
            m[0][i] = r0 * c + r1 * sn;
            m[1][i] = -r0 * sn + r1 * c;
        }
    };

    if (L.kind == FI_SPRITE_KIND_FULL) {
        m[3][0] = L.camRelPos[0];
        m[3][1] = L.camRelPos[1];
        m[3][2] = L.camRelPos[2];
        rotYaw(L.rotation[1]);
        rotPitch(L.rotation[0]);
        rotRoll(L.rotation[2]);
    } else {
        rotYaw(L.camYaw);
        rotPitch(L.camPitch);
        if (L.kind == FI_SPRITE_KIND_BILLBOARD_ROLL) {
            rotRoll(L.spriteRoll);
        }
        m[3][0] = L.camRelPos[0];
        m[3][1] = L.camRelPos[1];
        m[3][2] = L.camRelPos[2];
    }

    float sx = L.mirrored ? -L.scale[0] : L.scale[0];
    for (int i = 0; i < 3; i++) {
        m[0][i] *= sx;
        m[1][i] *= L.scale[1];
        m[2][i] *= L.scale[2];
    }

    MtxF& out = replacements[reinterpret_cast<Mtx*>(L.dst)];
    std::memcpy(out.mf, m, sizeof(out.mf));
}

} // namespace

void FrameInterpolation_ApplyAnimVertices(float t) {
    if (!gRenderShould) {
        return;
    }
    if (!gCache.built) {
        BuildInterpolationCache();
    }
    if (!gCache.valid) {
        return;
    }
    if (gCache.prevHasCameraPos && gRenderCurr->hasCameraPos) {
        float dx = gRenderCurr->cameraPos[0] - gCache.prevCameraPos[0];
        float dy = gRenderCurr->cameraPos[1] - gCache.prevCameraPos[1];
        float dz = gRenderCurr->cameraPos[2] - gCache.prevCameraPos[2];
        constexpr float kCutDistSq = 1000.0f * 1000.0f;
        if (dx * dx + dy * dy + dz * dz > kCutDistSq) {
            return;
        }
    }

    const std::vector<AnimVtxData>& currAnimVtxs = gRenderCurr->animVtxs;
    const std::vector<AnimVtxData>& prevAnimVtxs = gRenderPrev->animVtxs;
    const float w = 1.0f - t;

    for (const PairedAnimVtx& pair : gCache.pairedAnimVtxs) {
        const AnimVtxData& c = currAnimVtxs[pair.currIdx];
        if (pair.snap) {
            continue; // never written, so dst still holds this tick's pose
        }
        const AnimVtxData& p = prevAnimVtxs[pair.prevIdx];
        Vtx* dst = reinterpret_cast<Vtx*>(c.dst);
        for (uint32_t i = 0; i < c.count; i++) {
            for (int k = 0; k < 3; k++) {
                float blended = w * static_cast<float>(p.pos[i * 3 + k]) + t * static_cast<float>(c.pos[i * 3 + k]);
                dst[i].v.ob[k] = static_cast<int16_t>(std::lrintf(blended));
            }
        }
    }
}

void FrameInterpolation_Interpolate(float t, std::unordered_map<Mtx*, MtxF>& replacements) {
    replacements.clear();

    if (!gRenderShould) {
        return;
    }
    if (!gCache.built) {
        BuildInterpolationCache();
    }
    if (!gCache.valid) {
        return;
    }

    // A large camera jump means a warp or camera cut.
    if (gCache.prevHasCameraPos && gRenderCurr->hasCameraPos) {
        float dx = gRenderCurr->cameraPos[0] - gCache.prevCameraPos[0];
        float dy = gRenderCurr->cameraPos[1] - gCache.prevCameraPos[1];
        float dz = gRenderCurr->cameraPos[2] - gCache.prevCameraPos[2];
        constexpr float kCutDistSq = 1000.0f * 1000.0f;
        if (dx * dx + dy * dy + dz * dz > kCutDistSq) {
            return;
        }
    }

    const std::vector<ToMtxData>& prevToMtxs = gRenderPrev->toMtxs;
    const std::vector<ToMtxData>& currToMtxs = gRenderCurr->toMtxs;
    const std::vector<SpriteDrawData>& prevSpritesData = gRenderPrev->sprites;
    const std::vector<SpriteDrawData>& currSpritesData = gRenderCurr->sprites;

    replacements.reserve(gCache.pairedToMtxs.size() + gCache.pairedSprites.size() + gCache.camRelFixups.size() +
                         gRenderCurr->projRots.size() * 3);
    const float w = 1.0f - t;

    // Re-aim the frozen matrix at the sub-frame's camera.
    for (uint32_t idx : gCache.camRelFixups) {
        const ToMtxData& c = currToMtxs[idx];
        MtxF& out = replacements[reinterpret_cast<Mtx*>(c.dst)];
        std::memcpy(out.mf, c.src, sizeof(out.mf));
        for (int k = 0; k < 3; k++) {
            out.mf[3][k] += w * gCache.cameraDelta[k];
        }
    }

    for (const PairedToMtx& pair : gCache.pairedToMtxs) {
        const ToMtxData& c = currToMtxs[pair.currIdx];
        const ToMtxData& p = prevToMtxs[pair.prevIdx];
        MtxF& out = replacements[reinterpret_cast<Mtx*>(c.dst)];
        if (pair.snap) {
            std::memcpy(out.mf, c.src, sizeof(out.mf));
        } else {
            for (int r = 0; r < 4; r++) {
                for (int col = 0; col < 4; col++) {
                    out.mf[r][col] = w * p.src[r][col] + t * c.src[r][col];
                }
            }
        }
    }

    for (const PairedSprite& pair : gCache.pairedSprites) {
        const SpriteDrawData& c = currSpritesData[pair.currIdx];
        SpriteDrawData L = c;
        if (pair.prevIdx != UINT32_MAX && !pair.snap) {
            const SpriteDrawData& p = prevSpritesData[pair.prevIdx];
            for (int k = 0; k < 3; k++) {
                L.camRelPos[k] = w * p.camRelPos[k] + t * c.camRelPos[k];
                L.scale[k] = w * p.scale[k] + t * c.scale[k];
            }
            L.camYaw = lerpAngleDegSP(p.camYaw, c.camYaw, t);
            L.camPitch = lerpAngleDegSP(p.camPitch, c.camPitch, t);
            L.spriteRoll = lerpAngleDegSP(p.spriteRoll, c.spriteRoll, t);
            L.rotation[0] = lerpAngleDegSP(p.rotation[0], c.rotation[0], t);
            L.rotation[1] = lerpAngleDegSP(p.rotation[1], c.rotation[1], t);
            L.rotation[2] = lerpAngleDegSP(p.rotation[2], c.rotation[2], t);
        }
        emitSprite(L, replacements);
    }

    const std::vector<CameraProjRotData>& prevProj = gRenderPrev->projRots;
    const std::vector<CameraProjRotData>& currProj = gRenderCurr->projRots;
    for (const PairedProjRot& pair : gCache.pairedProjRots) {
        const CameraProjRotData& c = currProj[pair.currIdx];
        const CameraProjRotData& p = prevProj[pair.prevIdx];
        auto emitProj = [&](void* dst, float deg, float ax, float ay, float az) {
            if (dst == nullptr) {
                return;
            }
            MtxF& out = replacements[reinterpret_cast<Mtx*>(dst)];
            guRotateF(out.mf, deg, ax, ay, az);
        };
        // Axis conventions per viewport.c: roll about -Z, pitch +X, yaw +Y.
        emitProj(c.rollMtx, lerpAngleDegSP(p.rollDeg, c.rollDeg, t), 0.0f, 0.0f, -1.0f);
        emitProj(c.pitchMtx, lerpAngleDegSP(p.pitchDeg, c.pitchDeg, t), 1.0f, 0.0f, 0.0f);
        emitProj(c.yawMtx, lerpAngleDegSP(p.yawDeg, c.yawDeg, t), 0.0f, 1.0f, 0.0f);
    }
}
