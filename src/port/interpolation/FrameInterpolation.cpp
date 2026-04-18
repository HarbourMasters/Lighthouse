#include "FrameInterpolation.h"

#include <vector>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <unordered_map>

#include <libultraship/libultra/gu.h>

// Double-buffered op recorder. Matrix primitives in core1/math/matrix_stack.c
// append into the current tree between StartRecord/StopRecord; Interpolate()
// pairs ops across the two trees by scope-path and lerps the inputs. Camera
// projection rotations and sprite inputs are angle-lerped (not matrix-lerped)
// to dodge the paper-fold artifact on fast spins. See docs/INTERPOLATION.md.

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

// Common ops fit inline; the heavy ones (Set/Mult/ToMtx/camera/sprite)
// stash their payload in per-tree side vectors and store an index here.
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
    // Skip the replacement and let the curr matrix ride. Set for sprites —
    // cube culling shuffles their order, so index pairing mismatches.
    bool noInterpolate;
};

// Raw sprite inputs for each draw, so replay can lerp them in natural
// spaces (angles for rotations, linear for pos/scale) and rebuild a fresh
// matrix that stays consistent with the angle-lerped projection. Which
// fields matter depends on kind — billboards use camYaw/camPitch, FULL
// uses rotation[], only BILLBOARD_ROLL cares about spriteRoll.
struct SpriteDrawData {
    void* dst;
    float camRelPos[3];
    float scale[3];
    float camYaw;
    float camPitch;
    float spriteRoll;
    float rotation[3];
    uint8_t kind;
    bool mirrored;
};

// Raw angles behind BK's three projection-rotation matrices, captured so
// replay can angle-lerp (shortest-path) and rebuild fresh matrices via
// guRotateF. Matrix-space lerp on these folds the camera during fast
// spins.
struct CameraProjRotData {
    void* rollMtx;
    void* pitchMtx;
    void* yawMtx;
    float rollDeg;
    float pitchDeg;
    float yawDeg;
};

struct FrameTree {
    std::vector<OpNode> ops;
    std::vector<MatrixSetData> sets;
    std::vector<MatrixMultData> mults;
    std::vector<ToMtxData> toMtxs;
    std::vector<CameraProjRotData> projRots;
    std::vector<SpriteDrawData> sprites;
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
        cameraPos[0] = cameraPos[1] = cameraPos[2] = 0.0f;
        hasCameraPos = false;
        valid = false;
    }
};

// gCurrent fills during recording, flips to gPrevious at StopRecord.
FrameTree gTreeA;
FrameTree gTreeB;
FrameTree* gCurrent = &gTreeA;
FrameTree* gPrevious = &gTreeB;

bool gRecording = false;
bool gShouldInterpolate = true;
int gNoInterpolateDepth = 0;

// Monotonic ids for heap pointers that get reused by the allocator. Keyed
// by raw pointer, valued by a 64-bit counter that never repeats.
std::unordered_map<const void*, uint64_t> gIdMap;
uint64_t gNextId = 1;

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
    // Reserve once — subsequent ticks reuse the existing capacity.
    gCurrent->ops.reserve(4096);
    gCurrent->sets.reserve(128);
    gCurrent->mults.reserve(256);
    gCurrent->toMtxs.reserve(512);
    gCurrent->projRots.reserve(4);
    gCurrent->sprites.reserve(256);
    gShouldInterpolate = true;
    gNoInterpolateDepth = 0;
    gRecording = true;
}

void FrameInterpolation_StopRecord(void) {
    gRecording = false;
    gCurrent->valid = gShouldInterpolate;

    // Swap buffers: what we just recorded becomes `previous`.
    FrameTree* tmp = gPrevious;
    gPrevious = gCurrent;
    gCurrent = tmp;
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
}

void FrameInterpolation_RecordCloseChild(void) {
    if (!gRecording) {
        return;
    }
    append(Op::CloseChild);
}

uintptr_t FrameInterpolation_Hash3(uint64_t a, uint64_t b, uint64_t c) {
    // Three distinct odd primes — each input dominates different hash bits,
    // so a, b, c are separable even when they share most of their bytes.
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
    gCurrent->toMtxs.emplace_back();
    ToMtxData& d = gCurrent->toMtxs.back();
    d.dst = dst;
    std::memcpy(d.src, src, sizeof(float) * 16);
    d.noInterpolate = (gNoInterpolateDepth > 0);
    append(Op::MatrixToMtx).side_index = static_cast<uint32_t>(gCurrent->toMtxs.size() - 1);
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
    d.kind = static_cast<uint8_t>(kind);
    d.mirrored = (mirrored != 0);
    append(Op::SpriteDraw).side_index = static_cast<uint32_t>(gCurrent->sprites.size() - 1);
}

void FrameInterpolation_DontInterpolateCamera(void) {
    // Drop the prev tree — next Interpolate() bails and replay uses curr
    // as-is. Matters because the prev tree's matrices belong to a scene
    // the camera just left.
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
    // Fall back to the raw pointer. Good enough unless the caller cares
    // about ABA — those should Register at alloc time.
    return reinterpret_cast<uintptr_t>(ptr);
}

void FrameInterpolation_UnregisterId(const void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    gIdMap.erase(ptr);
}

} // extern "C"

std::unordered_map<Mtx*, MtxF> FrameInterpolation_Interpolate(float t) {
    // Pairing is by scope-path, not flat index. Whenever the actor set
    // changes (frustum in/out, animation branch) a flat pair would shift
    // every subsequent matrix and smear bones across bodies. The path is
    // FNV-1a over the (OpenChild key, id) chain — same tuple across two
    // ticks always lands on the same signature, so we pair bone-to-bone
    // even when the surrounding draw list churns.
    //
    // Element-wise matrix lerp with an X/Y basis-dot guard handles the
    // >90° "paper fold" case. Sprites skip the lerp entirely (flagged
    // noInterpolate at record) because their DL order is cube-sorted.
    std::unordered_map<Mtx*, MtxF> replacements;

    if (!gPrevious->valid || !gShouldInterpolate) {
        return replacements;
    }

    // Cut detector. Big camera jump between ticks → teleport (warp,
    // fixed-cam snap, shot change, or any site we haven't hooked). Bail
    // rather than lerp the world between two scenes for one sub-frame.
    // 1000 units is way above normal motion (talon trot is ~50/tick).
    if (gPrevious->hasCameraPos && gCurrent->hasCameraPos) {
        float dx = gCurrent->cameraPos[0] - gPrevious->cameraPos[0];
        float dy = gCurrent->cameraPos[1] - gPrevious->cameraPos[1];
        float dz = gCurrent->cameraPos[2] - gPrevious->cameraPos[2];
        float distSq = dx * dx + dy * dy + dz * dz;
        constexpr float kCutDistSq = 1000.0f * 1000.0f;
        if (distSq > kCutDistSq) {
            return replacements;
        }
    }

    const std::vector<OpNode>& prevOps = gPrevious->ops;
    const std::vector<OpNode>& currOps = gCurrent->ops;
    const std::vector<ToMtxData>& prevToMtxs = gPrevious->toMtxs;
    const std::vector<ToMtxData>& currToMtxs = gCurrent->toMtxs;

    replacements.reserve(currToMtxs.size() + gCurrent->sprites.size() + gCurrent->projRots.size() * 3);
    const float w = 1.0f - t;

    // Build signature (pathHash * within-scope-index) → prev ToMtx index
    // by walking prev's ops with a scope stack.
    auto fnvMix = [](uint64_t h, uint64_t v) {
        h ^= v;
        h *= 0x100000001b3ULL;
        return h;
    };

    struct ScopeFrame {
        uint64_t pathHash;
        uint32_t toMtxIdx;
        uint32_t spriteIdx;
    };
    // Disambiguators so ToMtx index 0 and Sprite index 0 in the same
    // scope don't collide on a single signature.
    constexpr uint64_t kSigKindToMtx = 0x1ULL;
    constexpr uint64_t kSigKindSprite = 0x2ULL;

    auto buildSigMap = [&](const std::vector<OpNode>& ops, std::unordered_map<uint64_t, uint32_t>& outToMtx,
                           std::unordered_map<uint64_t, uint32_t>& outSprite) {
        std::vector<ScopeFrame> stack;
        stack.push_back({ 0xcbf29ce484222325ULL, 0, 0 });
        for (const OpNode& op : ops) {
            switch (op.op) {
                case Op::OpenChild: {
                    uint64_t h = stack.back().pathHash;
                    h = fnvMix(h, reinterpret_cast<uintptr_t>(op.open_child.key));
                    h = fnvMix(h, static_cast<uint64_t>(op.open_child.id));
                    stack.push_back({ h, 0, 0 });
                    break;
                }
                case Op::CloseChild:
                    if (stack.size() > 1) {
                        stack.pop_back();
                    }
                    break;
                case Op::MatrixToMtx: {
                    uint64_t sig = fnvMix(fnvMix(stack.back().pathHash, kSigKindToMtx), stack.back().toMtxIdx);
                    outToMtx.emplace(sig, op.side_index);
                    stack.back().toMtxIdx++;
                    break;
                }
                case Op::SpriteDraw: {
                    uint64_t sig = fnvMix(fnvMix(stack.back().pathHash, kSigKindSprite), stack.back().spriteIdx);
                    outSprite.emplace(sig, op.side_index);
                    stack.back().spriteIdx++;
                    break;
                }
                default:
                    break;
            }
        }
    };

    std::unordered_map<uint64_t, uint32_t> prevSig;
    std::unordered_map<uint64_t, uint32_t> prevSpriteSig;
    prevSig.reserve(prevToMtxs.size());
    prevSpriteSig.reserve(gPrevious->sprites.size());
    buildSigMap(prevOps, prevSig, prevSpriteSig);

    // Opposite-hemisphere basis check. If X or Y of prev/curr point apart
    // (dot < 0), the delta is >90° and a matrix lerp folds the character
    // inside-out. Snap to curr instead.
    auto shouldSnap = [](const float pa[4][4], const float ca[4][4]) {
        float dotX = pa[0][0] * ca[0][0] + pa[0][1] * ca[0][1] + pa[0][2] * ca[0][2];
        float dotY = pa[1][0] * ca[1][0] + pa[1][1] * ca[1][1] + pa[1][2] * ca[1][2];
        return dotX < 0.0f || dotY < 0.0f;
    };

    // Angle-shortest-path lerp (degrees).
    auto lerpAngleDegSP = [](float a, float b, float tt) {
        float d = b - a;
        d = std::fmod(d + 540.0f, 360.0f) - 180.0f;
        return a + d * tt;
    };

    // Build a sprite matrix from lerped inputs. Mirrors the decomp
    // composition for each kind so the result stays in sync with the
    // record site. Writes into replacements[dst].
    auto emitSprite = [&](const SpriteDrawData& L) {
        if (L.dst == nullptr) {
            return;
        }
        float m[4][4];
        std::memset(m, 0, sizeof(m));
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;

        // Rotations match the matrix_stack.c primitives — rows 0-2 only.
        auto rotYaw = [&](float deg) {
            if (deg == 0.0f) return;
            float rad = deg * 0.017453292519943295f;
            float c = std::cos(rad), sn = std::sin(rad);
            for (int i = 0; i < 3; i++) {
                float r0 = m[0][i], r2 = m[2][i];
                m[0][i] = r0 * c - r2 * sn;
                m[2][i] = r0 * sn + r2 * c;
            }
        };
        auto rotPitch = [&](float deg) {
            if (deg == 0.0f) return;
            float rad = deg * 0.017453292519943295f;
            float c = std::cos(rad), sn = std::sin(rad);
            for (int i = 0; i < 3; i++) {
                float r1 = m[1][i], r2 = m[2][i];
                m[1][i] = r1 * c + r2 * sn;
                m[2][i] = -r1 * sn + r2 * c;
            }
        };
        auto rotRoll = [&](float deg) {
            if (deg == 0.0f) return;
            float rad = deg * 0.017453292519943295f;
            float c = std::cos(rad), sn = std::sin(rad);
            for (int i = 0; i < 3; i++) {
                float r0 = m[0][i], r1 = m[1][i];
                m[0][i] = r0 * c + r1 * sn;
                m[1][i] = -r0 * sn + r1 * c;
            }
        };

        if (L.kind == FI_SPRITE_KIND_FULL) {
            // FULL path: translate first (func_80252330 stomps mf[3]),
            // then rotate — rotations only touch rows 0-2.
            m[3][0] = L.camRelPos[0];
            m[3][1] = L.camRelPos[1];
            m[3][2] = L.camRelPos[2];
            rotYaw(L.rotation[1]);
            rotPitch(L.rotation[0]);
            rotRoll(L.rotation[2]);
        } else {
            // Billboards: cam-aligned rotations, optional sprite roll,
            // then translate overwrites mf[3].
            rotYaw(L.camYaw);
            rotPitch(L.camPitch);
            if (L.kind == FI_SPRITE_KIND_BILLBOARD_ROLL) {
                rotRoll(L.spriteRoll);
            }
            m[3][0] = L.camRelPos[0];
            m[3][1] = L.camRelPos[1];
            m[3][2] = L.camRelPos[2];
        }

        // mlMtxScale_xyz: scale rows 0-2 only (row 3 / translation preserved).
        float sx = L.mirrored ? -L.scale[0] : L.scale[0];
        for (int i = 0; i < 3; i++) {
            m[0][i] *= sx;
            m[1][i] *= L.scale[1];
            m[2][i] *= L.scale[2];
        }

        MtxF out;
        std::memcpy(out.mf, m, sizeof(out.mf));
        replacements[reinterpret_cast<Mtx*>(L.dst)] = out;
    };

    // Walk curr's ops. Each ToMtx/Sprite builds its signature, looks up
    // the prev counterpart, and emits a lerped replacement.
    {
        std::vector<ScopeFrame> stack;
        stack.push_back({ 0xcbf29ce484222325ULL, 0, 0 });
        for (const OpNode& op : currOps) {
            switch (op.op) {
                case Op::OpenChild: {
                    uint64_t h = stack.back().pathHash;
                    h = fnvMix(h, reinterpret_cast<uintptr_t>(op.open_child.key));
                    h = fnvMix(h, static_cast<uint64_t>(op.open_child.id));
                    stack.push_back({ h, 0, 0 });
                    break;
                }
                case Op::CloseChild:
                    if (stack.size() > 1) {
                        stack.pop_back();
                    }
                    break;
                case Op::MatrixToMtx: {
                    uint64_t sig = fnvMix(fnvMix(stack.back().pathHash, kSigKindToMtx), stack.back().toMtxIdx);
                    stack.back().toMtxIdx++;

                    const ToMtxData& c = currToMtxs[op.side_index];
                    if (c.dst == nullptr || c.noInterpolate) {
                        break;
                    }
                    MtxF out;
                    auto it = prevSig.find(sig);
                    if (it == prevSig.end()) {
                        std::memcpy(out.mf, c.src, sizeof(out.mf));
                    } else {
                        const ToMtxData& p = prevToMtxs[it->second];
                        if (shouldSnap(p.src, c.src)) {
                            std::memcpy(out.mf, c.src, sizeof(out.mf));
                        } else {
                            for (int r = 0; r < 4; r++) {
                                for (int col = 0; col < 4; col++) {
                                    out.mf[r][col] = w * p.src[r][col] + t * c.src[r][col];
                                }
                            }
                        }
                    }
                    replacements[reinterpret_cast<Mtx*>(c.dst)] = out;
                    break;
                }
                case Op::SpriteDraw: {
                    uint64_t sig = fnvMix(fnvMix(stack.back().pathHash, kSigKindSprite), stack.back().spriteIdx);
                    stack.back().spriteIdx++;

                    const SpriteDrawData& c = gCurrent->sprites[op.side_index];
                    SpriteDrawData L = c;
                    auto it = prevSpriteSig.find(sig);
                    if (it == prevSpriteSig.end() || gPrevious->sprites[it->second].kind != c.kind) {
                        // No pair, or kind flipped — emit curr as-is. The
                        // rebuilt matrix still matches the curr projection.
                    } else {
                        const SpriteDrawData& p = gPrevious->sprites[it->second];
                        for (int k = 0; k < 3; k++) {
                            L.camRelPos[k] = w * p.camRelPos[k] + t * c.camRelPos[k];
                            L.scale[k] = w * p.scale[k] + t * c.scale[k];
                        }
                        // Angle-space so rotations stay in sync with the
                        // projection lerp, not short-cut through a chord.
                        L.camYaw = lerpAngleDegSP(p.camYaw, c.camYaw, t);
                        L.camPitch = lerpAngleDegSP(p.camPitch, c.camPitch, t);
                        L.spriteRoll = lerpAngleDegSP(p.spriteRoll, c.spriteRoll, t);
                        L.rotation[0] = lerpAngleDegSP(p.rotation[0], c.rotation[0], t);
                        L.rotation[1] = lerpAngleDegSP(p.rotation[1], c.rotation[1], t);
                        L.rotation[2] = lerpAngleDegSP(p.rotation[2], c.rotation[2], t);
                    }
                    emitSprite(L);
                    break;
                }
                default:
                    break;
            }
        }
    }

    // Projection rotations in angle space, then rebuild via guRotateF so
    // every sub-frame gets a clean rotation matrix — works for any delta.
    auto lerpAngleDeg = [](float a, float b, float tt) {
        float d = b - a;
        d = std::fmod(d + 540.0f, 360.0f) - 180.0f;
        return a + d * tt;
    };
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
                MtxF out;
                guRotateF(out.mf, deg, ax, ay, az);
                replacements[reinterpret_cast<Mtx*>(dst)] = out;
            };
            // Axes match viewport.c: roll Z-, pitch X+, yaw Y+.
            emitProj(c.rollMtx, lerpAngleDeg(p.rollDeg, c.rollDeg, t), 0.0f, 0.0f, -1.0f);
            emitProj(c.pitchMtx, lerpAngleDeg(p.pitchDeg, c.pitchDeg, t), 1.0f, 0.0f, 0.0f);
            emitProj(c.yawMtx, lerpAngleDeg(p.yawDeg, c.yawDeg, t), 0.0f, 1.0f, 0.0f);
        }
    }

    return replacements;
}
