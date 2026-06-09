#include "FrameInterpolation.h"

#include <vector>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <unordered_map>

#include <libultraship/libultra/gu.h>

// Double-buffered op recorder. Matrix primitives in core1/math/matrix_stack.c
// append into gCurrent between StartRecord/StopRecord; Interpolate(t) pairs
// ops across two consecutive ticks (gPrevious vs gCurrent) by scope-path and
// lerps the inputs. Camera projection rotations and sprite inputs go through
// angle-space lerp (not matrix-lerp) to avoid the paper-fold artifact on fast
// spins. See docs/INTERPOLATION.md for the full design.

namespace {

enum class Op : uint8_t {
    OpenChild,
    CloseChild,
    Marker,

    MatrixIdent,
    MatrixTranslate,
    MatrixRotYaw,
    MatrixRotPitch,
    MatrixRotRoll,
    MatrixScale,
    MatrixSet,
    MatrixMult,
    MatrixToMtx,

    CameraProjectionRotation,
    SpriteDraw,
};

// Heavy ops (Set/Mult/ToMtx/Camera/Sprite) stash their payload in per-tree
// side vectors and reference it via side_index.
struct OpNode {
    Op op;
    union {
        struct {
            float x, y, z;
        } vec3;
        struct {
            float degrees;
        } rotate;
        struct {
            const void* key;
            uintptr_t id;
        } open_child;
        struct {
            const char* file;
            int line;
        } marker;
        uint32_t side_index;
    };

    OpNode() : op(Op::Marker), side_index(0) {
    }
};

struct MatrixSetData {
    float m[4][4];
};

struct MatrixMultData {
    float l[4][4];
    float r[4][4];
};

struct ToMtxData {
    void* dst;
    float src[4][4];
    // Cross-tick pairing key, built at record time from the live FNV scope
    // hash; same scope path on both ticks → same sig → O(1) pair lookup.
    uint64_t pathSig;
    // Sprites flag this so the lerp loop leaves their dst alone — cube
    // culling reorders sprites between ticks so index pairing mismatches.
    bool noInterpolate;
};

// Sprites record raw inputs (angles for rotations, linear for pos/scale)
// rather than the final matrix, so replay can lerp in natural spaces and
// rebuild a matrix that stays consistent with the angle-lerped projection.
// Which fields matter depends on kind: billboards use camYaw/camPitch,
// FULL uses rotation[], only BILLBOARD_ROLL cares about spriteRoll.
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
};

// We record the three Euler angles behind BK's projection rotation rather
// than the matrices, so replay can shortest-path angle-lerp and rebuild via
// guRotateF — element-wise lerp on these matrices folds during fast spins.
struct CameraProjRotData {
    void* rollMtx;
    void* pitchMtx;
    void* yawMtx;
    float rollDeg;
    float pitchDeg;
    float yawDeg;
};

struct ScopeFrame {
    uint64_t pathHash;
    uint32_t toMtxIdx;
    uint32_t spriteIdx;
};

constexpr uint64_t kFnvSeed = 0xcbf29ce484222325ULL;
// Disambiguators so a ToMtx and a Sprite at the same in-scope index don't
// collide on a single signature.
constexpr uint64_t kSigKindToMtx = 0x1ULL;
constexpr uint64_t kSigKindSprite = 0x2ULL;

inline uint64_t fnvMix(uint64_t h, uint64_t v) {
    h ^= v;
    h *= 0x100000001b3ULL;
    return h;
}

// Sig maps are populated live during recording. After StopRecord swaps
// trees, gPrevious already has its maps and BuildInterpolationCache can
// pair entries with O(1) lookups instead of re-walking the prev tree.
struct FrameTree {
    std::vector<OpNode> ops;
    std::vector<MatrixSetData> sets;
    std::vector<MatrixMultData> mults;
    std::vector<ToMtxData> toMtxs;
    std::vector<CameraProjRotData> projRots;
    std::vector<SpriteDrawData> sprites;
    std::unordered_map<uint64_t, uint32_t> sigToToMtx;
    std::unordered_map<uint64_t, uint32_t> sigToSprite;
    std::vector<ScopeFrame> scopeStack;
    float cameraPos[3] = { 0.0f, 0.0f, 0.0f };
    bool hasCameraPos = false;
    bool valid = false;

    void reset() {
        ops.clear();
        sets.clear();
        mults.clear();
        toMtxs.clear();
        projRots.clear();
        sprites.clear();
        sigToToMtx.clear();
        sigToSprite.clear();
        scopeStack.clear();
        cameraPos[0] = cameraPos[1] = cameraPos[2] = 0.0f;
        hasCameraPos = false;
        valid = false;
    }
};

FrameTree gTreeA;
FrameTree gTreeB;
FrameTree* gCurrent = &gTreeA;
FrameTree* gPrevious = &gTreeB;

bool gRecording = false;
bool gShouldInterpolate = true;
int gNoInterpolateDepth = 0;

// Monotonic ids for heap pointers that get reused by the allocator. Without
// these, a freed object's address gets reused by the next allocation and
// the new one inherits the dead one's matrix pairing.
std::unordered_map<const void*, uint64_t> gIdMap;
uint64_t gNextId = 1;

// Per-tick cache built lazily on the first Interpolate(t) call and reused
// for every sub-frame within the tick. Stores only (curr, prev) pairs whose
// lerped output differs from c.src — no-prev and bit-identical entries are
// skipped because the interpreter's fallback path decodes the DL bytes that
// c.src was already encoded into, producing the same matrix without a map
// lookup. Shrinking the map this way speeds up every interpreter find()
// across the whole DL on top of cheaper inserts.
struct PairedToMtx {
    uint32_t currIdx;
    uint32_t prevIdx;
    // Hemisphere check is stable across t for a (prev, curr) pair, so we
    // do it once at cache build instead of 6 fmuls + cmps per sub-frame.
    bool snap;
};
struct PairedSprite {
    uint32_t currIdx;
    uint32_t prevIdx;
};
struct InterpolationCache {
    bool valid = false;
    std::vector<PairedToMtx> pairedToMtxs;
    std::vector<PairedSprite> pairedSprites;
    bool prevHasCameraPos = false;
    float prevCameraPos[3] = { 0.0f, 0.0f, 0.0f };

    void reset() {
        valid = false;
        pairedToMtxs.clear();
        pairedSprites.clear();
        prevHasCameraPos = false;
    }
};
InterpolationCache gCache;

OpNode& append(Op op) {
    gCurrent->ops.emplace_back();
    OpNode& n = gCurrent->ops.back();
    n.op = op;
    return n;
}

} // namespace

extern "C" {

void FrameInterpolation_StartRecord(void) {
    gCurrent->reset();
    // Reserve once; subsequent ticks reuse the existing capacity.
    gCurrent->ops.reserve(4096);
    gCurrent->sets.reserve(128);
    gCurrent->mults.reserve(256);
    gCurrent->toMtxs.reserve(512);
    gCurrent->projRots.reserve(4);
    gCurrent->sprites.reserve(256);
    gCurrent->sigToToMtx.reserve(512);
    gCurrent->sigToSprite.reserve(64);
    gCurrent->scopeStack.push_back({ kFnvSeed, 0, 0 });
    gShouldInterpolate = true;
    gNoInterpolateDepth = 0;
    gRecording = true;
    // The cache built during the prior render pass references trees that
    // are about to be reused for this tick.
    gCache.valid = false;
}

void FrameInterpolation_StopRecord(void) {
    gRecording = false;
    gCurrent->valid = gShouldInterpolate;
    // What we just recorded becomes `previous` for the next tick.
    std::swap(gCurrent, gPrevious);
}

void FrameInterpolation_ShouldInterpolateFrame(bool shouldInterpolate) {
    gShouldInterpolate = shouldInterpolate;
}

void FrameInterpolation_RecordOpenChild(const void* key, uintptr_t id) {
    if (!gRecording) {
        return;
    }
    OpNode& n = append(Op::OpenChild);
    n.open_child.key = key;
    n.open_child.id = id;
    uint64_t h = gCurrent->scopeStack.back().pathHash;
    h = fnvMix(h, reinterpret_cast<uintptr_t>(key));
    h = fnvMix(h, static_cast<uint64_t>(id));
    gCurrent->scopeStack.push_back({ h, 0, 0 });
}

void FrameInterpolation_RecordCloseChild(void) {
    if (!gRecording) {
        return;
    }
    append(Op::CloseChild);
    if (gCurrent->scopeStack.size() > 1) {
        gCurrent->scopeStack.pop_back();
    }
}

uintptr_t FrameInterpolation_Hash3(uint64_t a, uint64_t b, uint64_t c) {
    // Three distinct odd primes keep the three inputs separable even when
    // they share most of their bytes.
    uint64_t h = a * 0x9E3779B97F4A7C15ULL;
    h ^= b * 0x100000001b3ULL;
    h ^= c * 0xbf58476d1ce4e5b9ULL;
    return static_cast<uintptr_t>(h);
}

void FrameInterpolation_RecordOpenChildHash3(const char* key, uint64_t a, uint64_t b, uint64_t c) {
    FrameInterpolation_RecordOpenChild(key, FrameInterpolation_Hash3(a, b, c));
}

void FrameInterpolation_RecordMarker(const char* file, int line) {
    if (!gRecording) {
        return;
    }
    OpNode& n = append(Op::Marker);
    n.marker.file = file;
    n.marker.line = line;
}

void FrameInterpolation_RecordMatrixIdent(void) {
    if (!gRecording) {
        return;
    }
    append(Op::MatrixIdent);
}

void FrameInterpolation_RecordMatrixTranslate(float x, float y, float z) {
    if (!gRecording) {
        return;
    }
    OpNode& n = append(Op::MatrixTranslate);
    n.vec3.x = x;
    n.vec3.y = y;
    n.vec3.z = z;
}

void FrameInterpolation_RecordMatrixRotYaw(float degrees) {
    if (!gRecording) {
        return;
    }
    append(Op::MatrixRotYaw).rotate.degrees = degrees;
}

void FrameInterpolation_RecordMatrixRotPitch(float degrees) {
    if (!gRecording) {
        return;
    }
    append(Op::MatrixRotPitch).rotate.degrees = degrees;
}

void FrameInterpolation_RecordMatrixRotRoll(float degrees) {
    if (!gRecording) {
        return;
    }
    append(Op::MatrixRotRoll).rotate.degrees = degrees;
}

void FrameInterpolation_RecordMatrixScale(float x, float y, float z) {
    if (!gRecording) {
        return;
    }
    OpNode& n = append(Op::MatrixScale);
    n.vec3.x = x;
    n.vec3.y = y;
    n.vec3.z = z;
}

void FrameInterpolation_RecordMatrixSet(const float src[4][4]) {
    if (!gRecording) {
        return;
    }
    gCurrent->sets.emplace_back();
    std::memcpy(gCurrent->sets.back().m, src, sizeof(float) * 16);
    append(Op::MatrixSet).side_index = static_cast<uint32_t>(gCurrent->sets.size() - 1);
}

void FrameInterpolation_RecordMatrixMult(const float l[4][4], const float r[4][4]) {
    if (!gRecording) {
        return;
    }
    gCurrent->mults.emplace_back();
    MatrixMultData& d = gCurrent->mults.back();
    std::memcpy(d.l, l, sizeof(float) * 16);
    std::memcpy(d.r, r, sizeof(float) * 16);
    append(Op::MatrixMult).side_index = static_cast<uint32_t>(gCurrent->mults.size() - 1);
}

void FrameInterpolation_RecordMatrixToMtx(void* dst, const float src[4][4]) {
    if (!gRecording) {
        return;
    }
    ScopeFrame& scope = gCurrent->scopeStack.back();
    uint64_t sig = fnvMix(fnvMix(scope.pathHash, kSigKindToMtx), scope.toMtxIdx);
    scope.toMtxIdx++;

    uint32_t idx = static_cast<uint32_t>(gCurrent->toMtxs.size());
    gCurrent->toMtxs.emplace_back();
    ToMtxData& d = gCurrent->toMtxs.back();
    d.dst = dst;
    std::memcpy(d.src, src, sizeof(float) * 16);
    d.pathSig = sig;
    d.noInterpolate = (gNoInterpolateDepth > 0);
    gCurrent->sigToToMtx.emplace(sig, idx);
    append(Op::MatrixToMtx).side_index = idx;
}

void FrameInterpolation_RecordCameraProjectionRotation(void* rollMtx, float rollDeg, void* pitchMtx, float pitchDeg,
                                                       void* yawMtx, float yawDeg) {
    if (!gRecording) {
        return;
    }
    gCurrent->projRots.emplace_back();
    CameraProjRotData& d = gCurrent->projRots.back();
    d.rollMtx = rollMtx;
    d.pitchMtx = pitchMtx;
    d.yawMtx = yawMtx;
    d.rollDeg = rollDeg;
    d.pitchDeg = pitchDeg;
    d.yawDeg = yawDeg;
    append(Op::CameraProjectionRotation).side_index = static_cast<uint32_t>(gCurrent->projRots.size() - 1);
}

void FrameInterpolation_RecordCameraPosition(const float pos[3]) {
    if (!gRecording || pos == nullptr) {
        return;
    }
    gCurrent->cameraPos[0] = pos[0];
    gCurrent->cameraPos[1] = pos[1];
    gCurrent->cameraPos[2] = pos[2];
    gCurrent->hasCameraPos = true;
}

void FrameInterpolation_NoInterpolatePush(void) {
    gNoInterpolateDepth++;
}

void FrameInterpolation_NoInterpolatePop(void) {
    if (gNoInterpolateDepth > 0) {
        gNoInterpolateDepth--;
    }
}

void FrameInterpolation_RecordSpriteDraw(int kind, void* dst, const float camRelPos[3], const float scale[3],
                                         float camYaw, float camPitch, float spriteRoll, const float rotation[3],
                                         int mirrored) {
    if (!gRecording) {
        return;
    }
    ScopeFrame& scope = gCurrent->scopeStack.back();
    uint64_t sig = fnvMix(fnvMix(scope.pathHash, kSigKindSprite), scope.spriteIdx);
    scope.spriteIdx++;

    uint32_t idx = static_cast<uint32_t>(gCurrent->sprites.size());
    gCurrent->sprites.emplace_back();
    SpriteDrawData& d = gCurrent->sprites.back();
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
    gCurrent->sigToSprite.emplace(sig, idx);
    append(Op::SpriteDraw).side_index = idx;
}

void FrameInterpolation_DontInterpolateCamera(void) {
    // Prev's matrices belong to a scene the camera just left; dropping the
    // tree makes the next Interpolate() bail and replay uses curr as-is.
    if (gPrevious != nullptr) {
        gPrevious->valid = false;
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
    // Fallback for callers that don't Register; ABA-prone.
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

// Snap to curr when prev/curr point apart on X or Y (>90° rotation):
// element-wise lerp on those folds the character inside-out.
inline bool shouldSnap(const float pa[4][4], const float ca[4][4]) {
    float dotX = pa[0][0] * ca[0][0] + pa[0][1] * ca[0][1] + pa[0][2] * ca[0][2];
    float dotY = pa[1][0] * ca[1][0] + pa[1][1] * ca[1][1] + pa[1][2] * ca[1][2];
    return dotX < 0.0f || dotY < 0.0f;
}

// Shortest-path angle lerp in degrees.
inline float lerpAngleDegSP(float a, float b, float tt) {
    float d = b - a;
    d = std::fmod(d + 540.0f, 360.0f) - 180.0f;
    return a + d * tt;
}

void BuildInterpolationCache() {
    gCache.reset();
    if (!gPrevious->valid) {
        return;
    }

    // Snapshot prev's camera pos so Interpolate's cut detector reads only
    // gCache for tick-stable state.
    gCache.prevHasCameraPos = gPrevious->hasCameraPos;
    if (gCache.prevHasCameraPos) {
        gCache.prevCameraPos[0] = gPrevious->cameraPos[0];
        gCache.prevCameraPos[1] = gPrevious->cameraPos[1];
        gCache.prevCameraPos[2] = gPrevious->cameraPos[2];
    }

    const std::vector<ToMtxData>& currToMtxs = gCurrent->toMtxs;
    const std::vector<ToMtxData>& prevToMtxs = gPrevious->toMtxs;
    const std::vector<SpriteDrawData>& currSprites = gCurrent->sprites;
    const std::vector<SpriteDrawData>& prevSprites = gPrevious->sprites;
    const std::unordered_map<uint64_t, uint32_t>& prevSigMap = gPrevious->sigToToMtx;
    const std::unordered_map<uint64_t, uint32_t>& prevSpriteMap = gPrevious->sigToSprite;

    gCache.pairedToMtxs.reserve(currToMtxs.size());
    gCache.pairedSprites.reserve(currSprites.size());

    // Only pair ToMtxs whose replacement actually differs from the curr
    // matrix. The interpreter's fallback path decodes the DL bytes at
    // c.dst, which the game wrote from the same float matrix that's now
    // c.src — so unpaired AND bit-identical entries decode to the same
    // result without a map lookup. Skipping them shrinks the replacement
    // map and speeds up every interpreter find() across the whole DL.
    for (uint32_t i = 0; i < currToMtxs.size(); i++) {
        const ToMtxData& c = currToMtxs[i];
        if (c.dst == nullptr || c.noInterpolate) {
            continue;
        }
        auto it = prevSigMap.find(c.pathSig);
        if (it == prevSigMap.end()) {
            continue;
        }
        const ToMtxData& p = prevToMtxs[it->second];
        if (std::memcmp(p.src, c.src, sizeof(c.src)) == 0) {
            continue;
        }
        gCache.pairedToMtxs.push_back({ i, it->second, shouldSnap(p.src, c.src) });
    }

    // Sprites always pair (or get a sentinel prev-idx) because the matrix
    // must be rebuilt every sub-frame anyway — billboard rotations follow
    // the lerped projection.
    for (uint32_t i = 0; i < currSprites.size(); i++) {
        const SpriteDrawData& c = currSprites[i];
        auto it = prevSpriteMap.find(c.pathSig);
        if (it == prevSpriteMap.end() || prevSprites[it->second].kind != c.kind) {
            gCache.pairedSprites.push_back({ i, UINT32_MAX });
        } else {
            gCache.pairedSprites.push_back({ i, it->second });
        }
    }

    gCache.valid = true;
}

// Build a sprite modelview from lerped inputs. Mirrors the decomp
// composition for each kind so the replay matrix stays consistent with
// the record-site one.
void emitSprite(const SpriteDrawData& L, std::unordered_map<Mtx*, MtxF>& replacements) {
    if (L.dst == nullptr) {
        return;
    }
    float m[4][4];
    std::memset(m, 0, sizeof(m));
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;

    // Rotations match the matrix_stack.c primitives — rows 0-2 only.
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
        // FULL: translate first (the game's func_80252330 stomps mf[3]),
        // then rotate — rotations only touch rows 0-2.
        m[3][0] = L.camRelPos[0];
        m[3][1] = L.camRelPos[1];
        m[3][2] = L.camRelPos[2];
        rotYaw(L.rotation[1]);
        rotPitch(L.rotation[0]);
        rotRoll(L.rotation[2]);
    } else {
        // Billboard: cam-aligned rotations, optional sprite roll, then
        // translate overwrites mf[3].
        rotYaw(L.camYaw);
        rotPitch(L.camPitch);
        if (L.kind == FI_SPRITE_KIND_BILLBOARD_ROLL) {
            rotRoll(L.spriteRoll);
        }
        m[3][0] = L.camRelPos[0];
        m[3][1] = L.camRelPos[1];
        m[3][2] = L.camRelPos[2];
    }

    // mlMtxScale_xyz: rows 0-2 only, translation preserved.
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

void FrameInterpolation_Interpolate(float t, std::unordered_map<Mtx*, MtxF>& replacements) {
    // Cache is built lazily by the first sub-frame call per render pass and
    // reused; StartRecord clears it for the next tick. Caller owns
    // `replacements` so its bucket array persists across sub-frames.
    replacements.clear();

    if (!gShouldInterpolate) {
        return;
    }
    if (!gCache.valid) {
        BuildInterpolationCache();
        if (!gCache.valid) {
            return;
        }
    }

    // Cut detector: a >1000-unit camera jump means a teleport (warp,
    // fixed-cam snap). Lerping across that places the world between two
    // scenes for one sub-frame.
    if (gCache.prevHasCameraPos && gCurrent->hasCameraPos) {
        float dx = gCurrent->cameraPos[0] - gCache.prevCameraPos[0];
        float dy = gCurrent->cameraPos[1] - gCache.prevCameraPos[1];
        float dz = gCurrent->cameraPos[2] - gCache.prevCameraPos[2];
        constexpr float kCutDistSq = 1000.0f * 1000.0f;
        if (dx * dx + dy * dy + dz * dz > kCutDistSq) {
            return;
        }
    }

    const std::vector<ToMtxData>& prevToMtxs = gPrevious->toMtxs;
    const std::vector<ToMtxData>& currToMtxs = gCurrent->toMtxs;
    const std::vector<SpriteDrawData>& prevSpritesData = gPrevious->sprites;
    const std::vector<SpriteDrawData>& currSpritesData = gCurrent->sprites;

    replacements.reserve(gCache.pairedToMtxs.size() + gCache.pairedSprites.size() + gCurrent->projRots.size() * 3);
    const float w = 1.0f - t;

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

    // Even unpaired sprites need a per-sub-frame matrix rebuild because
    // their billboard rotations follow the lerped projection.
    for (const PairedSprite& pair : gCache.pairedSprites) {
        const SpriteDrawData& c = currSpritesData[pair.currIdx];
        SpriteDrawData L = c;
        if (pair.prevIdx != UINT32_MAX) {
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

    const std::vector<CameraProjRotData>& prevProj = gPrevious->projRots;
    const std::vector<CameraProjRotData>& currProj = gCurrent->projRots;
    if (prevProj.size() == currProj.size()) {
        for (size_t i = 0; i < currProj.size(); i++) {
            const CameraProjRotData& p = prevProj[i];
            const CameraProjRotData& c = currProj[i];
            auto emitProj = [&](void* dst, float deg, float ax, float ay, float az) {
                if (dst == nullptr) {
                    return;
                }
                MtxF& out = replacements[reinterpret_cast<Mtx*>(dst)];
                guRotateF(out.mf, deg, ax, ay, az);
            };
            // Axes match viewport.c: roll Z-, pitch X+, yaw Y+.
            emitProj(c.rollMtx, lerpAngleDegSP(p.rollDeg, c.rollDeg, t), 0.0f, 0.0f, -1.0f);
            emitProj(c.pitchMtx, lerpAngleDegSP(p.pitchDeg, c.pitchDeg, t), 1.0f, 0.0f, 0.0f);
            emitProj(c.yawMtx, lerpAngleDegSP(p.yawDeg, c.yawDeg, t), 0.0f, 1.0f, 0.0f);
        }
    }
}
