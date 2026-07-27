#include "FrameInterpolation.h"

#include <vector>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <unordered_map>

#include <libultraship/libultra/gu.h>

namespace {

struct ToMtxData {
    void* dst;
    float src[4][4];
    uint64_t pathSig;
    bool noInterpolate;
};

// Sprites keep their raw inputs instead of the finished matrix so replay can
// blend angles as angles; blending the matrix itself makes billboards shear
// while the camera swings around them.
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

struct CameraProjRotData {
    void* rollMtx;
    void* pitchMtx;
    void* yawMtx;
    float rollDeg;
    float pitchDeg;
    float yawDeg;
};

// Two entries pair up across ticks when they were recorded under the same
// scope path at the same index; that identity is hashed into pathSig as the
// game draws.
struct ScopeFrame {
    uint64_t pathHash;
    uint32_t toMtxIdx;
    uint32_t spriteIdx;
};

constexpr uint64_t kFnvSeed = 0xcbf29ce484222325ULL;
constexpr uint64_t kSigKindToMtx = 0x1ULL;
constexpr uint64_t kSigKindSprite = 0x2ULL;

inline uint64_t fnvMix(uint64_t h, uint64_t v) {
    h ^= v;
    h *= 0x100000001b3ULL;
    return h;
}

struct FrameTree {
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

std::unordered_map<const void*, uint64_t> gIdMap;
uint64_t gNextId = 1;

struct PairedToMtx {
    uint32_t currIdx;
    uint32_t prevIdx;
    bool snap;
};
struct PairedSprite {
    uint32_t currIdx;
    uint32_t prevIdx;
};

// Pairing happens once per tick, on the first Interpolate() call; later
// sub-frames only read the result. Engine.cpp builds those on worker threads,
// so after `built` goes true nothing in here may be mutated until the next
// StartRecord.
struct InterpolationCache {
    bool built = false;
    bool valid = false;
    std::vector<PairedToMtx> pairedToMtxs;
    std::vector<PairedSprite> pairedSprites;
    bool prevHasCameraPos = false;
    float prevCameraPos[3] = { 0.0f, 0.0f, 0.0f };

    void reset() {
        built = false;
        valid = false;
        pairedToMtxs.clear();
        pairedSprites.clear();
        prevHasCameraPos = false;
    }
};
InterpolationCache gCache;

} // namespace

extern "C" {

void FrameInterpolation_StartRecord(void) {
    gCurrent->reset();
    gCurrent->toMtxs.reserve(512);
    gCurrent->projRots.reserve(4);
    gCurrent->sprites.reserve(256);
    gCurrent->sigToToMtx.reserve(512);
    gCurrent->sigToSprite.reserve(64);
    gCurrent->scopeStack.push_back({ kFnvSeed, 0, 0 });
    gShouldInterpolate = true;
    gNoInterpolateDepth = 0;
    gRecording = true;
    // The old cache indexes into the tree this tick is about to overwrite.
    gCache.built = false;
    gCache.valid = false;
}

void FrameInterpolation_StopRecord(void) {
    gRecording = false;
    gCurrent->valid = gShouldInterpolate;
    std::swap(gCurrent, gPrevious);
}

void FrameInterpolation_ShouldInterpolateFrame(bool shouldInterpolate) {
    gShouldInterpolate = shouldInterpolate;
}

void FrameInterpolation_RecordOpenChild(const void* key, uintptr_t id) {
    if (!gRecording) {
        return;
    }
    uint64_t h = gCurrent->scopeStack.back().pathHash;
    h = fnvMix(h, reinterpret_cast<uintptr_t>(key));
    h = fnvMix(h, static_cast<uint64_t>(id));
    gCurrent->scopeStack.push_back({ h, 0, 0 });
}

void FrameInterpolation_RecordCloseChild(void) {
    if (!gRecording) {
        return;
    }
    if (gCurrent->scopeStack.size() > 1) {
        gCurrent->scopeStack.pop_back();
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
}

void FrameInterpolation_DontInterpolateCamera(void) {
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
    if (!gPrevious->valid) {
        return;
    }

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

    // Whatever stays out of the cache falls back to the interpreter decoding
    // the Mtx bytes the game itself wrote, which already hold the correct
    // end-of-tick matrix. Entries with no previous partner, or whose
    // matrix didn't change, can simply be dropped here.
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

    // Sprites whose inputs didn't move at all are dropped for the same
    // reason; the rest get their matrix rebuilt every sub-frame so billboards
    // keep facing the blended camera.
    for (uint32_t i = 0; i < currSprites.size(); i++) {
        const SpriteDrawData& c = currSprites[i];
        auto it = prevSpriteMap.find(c.pathSig);
        if (it == prevSpriteMap.end() || prevSprites[it->second].kind != c.kind) {
            gCache.pairedSprites.push_back({ i, UINT32_MAX });
            continue;
        }
        const SpriteDrawData& p = prevSprites[it->second];
        if (std::memcmp(p.camRelPos, c.camRelPos, sizeof(c.camRelPos)) == 0 &&
            std::memcmp(p.scale, c.scale, sizeof(c.scale)) == 0 && p.camYaw == c.camYaw && p.camPitch == c.camPitch &&
            p.spriteRoll == c.spriteRoll && std::memcmp(p.rotation, c.rotation, sizeof(c.rotation)) == 0 &&
            p.mirrored == c.mirrored) {
            continue;
        }
        gCache.pairedSprites.push_back({ i, it->second });
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

    // The rotation lambdas only touch rows 0-2, which is why FULL can
    // translate first while billboards translate last.
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

void FrameInterpolation_Interpolate(float t, std::unordered_map<Mtx*, MtxF>& replacements) {
    replacements.clear();

    if (!gShouldInterpolate) {
        return;
    }
    if (!gCache.built) {
        BuildInterpolationCache();
    }
    if (!gCache.valid) {
        return;
    }

    // A large camera jump means a warp or camera cut; blending across it
    // would draw the world halfway between two scenes for a frame.
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
            // Axis conventions per viewport.c: roll about -Z, pitch +X, yaw +Y.
            emitProj(c.rollMtx, lerpAngleDegSP(p.rollDeg, c.rollDeg, t), 0.0f, 0.0f, -1.0f);
            emitProj(c.pitchMtx, lerpAngleDegSP(p.pitchDeg, c.pitchDeg, t), 1.0f, 0.0f, 0.0f);
            emitProj(c.yawMtx, lerpAngleDegSP(p.yawDeg, c.yawDeg, t), 0.0f, 1.0f, 0.0f);
        }
    }
}
