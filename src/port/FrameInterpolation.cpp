#include "FrameInterpolation.h"

#include <vector>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <unordered_map>

// Display-list matrix interpolation with identity-scoped matching.
//
// Matrices are extracted from the root DL by scanning for G_MTX commands.
// During the game tick, ScopeBegin/ScopeEnd calls record which Mtx buffer
// range belongs to each actor (keyed by ActorMarker pointer + array index).
// At interpolation time, matrices are grouped by scope and matched by
// identity + local index within scope. Unscoped matrices (sky, map, camera)
// are matched by global index among themselves.

// F3DEX opcodes (as int8_t, matching interpreter dispatch)
static constexpr int8_t OP_MTX = (int8_t)0x01;    // G_MTX
static constexpr int8_t OP_ENDDL = (int8_t)(-72); // G_ENDDL = G_IMMFIRST - 7

// ---------------------------------------------------------------------------
// Matrix scanning
// ---------------------------------------------------------------------------

struct MatrixEntry {
    Mtx* ptr;   // Pointer to the Mtx in the display list
    MtxF value; // Float copy of the matrix data (GBI_FLOATS=1: Mtx is MtxF)
};

static std::vector<MatrixEntry> s_currentFrame;
static std::vector<MatrixEntry> s_previousFrame;

// Scan root DL for modelview matrices. Stops at 2nd projection matrix (HUD switch).
static void ScanDisplayList(Gfx* commands, std::vector<MatrixEntry>& out) {
    if (!commands) {
        return;
    }

    Gfx* cmd = commands;
    int budget = 50000;
    int projCount = 0;

    while (budget-- > 0) {
        int8_t opcode = (int8_t)(cmd->words.w0 >> 24);

        if (opcode == OP_MTX) {
            // F3DEX params at bits [23:16]
            bool isProjection = ((cmd->words.w0 >> 16) & 0x01) != 0;

            if (isProjection) {
                projCount++;
                if (projCount > 1) {
                    break; // Stop at HUD/overlay projection switch
                }
            } else {
                uintptr_t addr = cmd->words.w1;
                if (addr != 0 && !(addr & 1)) {
                    MatrixEntry entry;
                    entry.ptr = (Mtx*)addr;
                    memcpy(&entry.value, entry.ptr, sizeof(MtxF));
                    out.push_back(entry);
                }
            }
        } else if (opcode == OP_ENDDL) {
            break;
        }

        cmd++;
    }
}

// ---------------------------------------------------------------------------
// Identity-scoped recording (side-channel populated during game tick)
// ---------------------------------------------------------------------------

struct ScopeKey {
    uintptr_t addr; // ActorMarker* as integer
    uint32_t idx;   // marker->actrArrayIdx (11-bit unique instance index)

    bool operator==(const ScopeKey& o) const {
        return addr == o.addr && idx == o.idx;
    }
};

struct ScopeKeyHash {
    size_t operator()(const ScopeKey& k) const {
        return std::hash<uintptr_t>()(k.addr) ^ (std::hash<uint32_t>()(k.idx) << 16);
    }
};

struct ScopeRecord {
    ScopeKey key;
    Mtx* mtx_start; // *mtx BEFORE actor's drawFunc
    Mtx* mtx_end;   // *mtx AFTER actor's drawFunc
};

static std::vector<ScopeRecord> s_currentScopes;
static std::vector<ScopeRecord> s_previousScopes;

static ScopeRecord s_pendingScope;
static bool s_scopeActive = false;

extern "C" void FrameInterpolation_ScopeBegin(void* marker_ptr, unsigned int array_idx, void* mtx_cursor) {
    s_pendingScope.key = { (uintptr_t)marker_ptr, (uint32_t)array_idx };
    s_pendingScope.mtx_start = (Mtx*)mtx_cursor;
    s_scopeActive = true;
}

extern "C" void FrameInterpolation_ScopeEnd(void* mtx_cursor) {
    if (!s_scopeActive) {
        return;
    }
    s_pendingScope.mtx_end = (Mtx*)mtx_cursor;
    s_currentScopes.push_back(s_pendingScope);
    s_scopeActive = false;
}

// ---------------------------------------------------------------------------
// Frame recording
// ---------------------------------------------------------------------------

void FrameInterpolation_RecordFrame(Gfx* commands) {
    s_previousFrame.swap(s_currentFrame);
    s_currentFrame.clear();

    s_previousScopes.swap(s_currentScopes);
    s_currentScopes.clear();

    ScanDisplayList(commands, s_currentFrame);
}

// ---------------------------------------------------------------------------
// Scoped interpolation
// ---------------------------------------------------------------------------

// Assign each MatrixEntry to a scope. Returns the scope index (into the scopes vector),
// or -1 if the matrix is unscoped. Also outputs the local index within the scope.
struct ScopeAssignment {
    int scope_idx;      // index into the scopes vector, or -1 for unscoped
    uint32_t local_idx; // index within scope (or global unscoped index)
};

static void AssignScopes(const std::vector<MatrixEntry>& matrices, const std::vector<ScopeRecord>& scopes,
                         std::vector<ScopeAssignment>& out) {
    out.resize(matrices.size());

    // Count local indices per scope
    std::vector<uint32_t> scopeLocalCounters(scopes.size(), 0);
    uint32_t unscopedCounter = 0;

    for (size_t i = 0; i < matrices.size(); i++) {
        Mtx* ptr = matrices[i].ptr;
        int found = -1;

        // Check if this matrix falls within any scope's range.
        // Scopes are typically <100, so linear scan is fine.
        for (size_t s = 0; s < scopes.size(); s++) {
            if (ptr >= scopes[s].mtx_start && ptr < scopes[s].mtx_end) {
                found = (int)s;
                break;
            }
        }

        if (found >= 0) {
            out[i].scope_idx = found;
            out[i].local_idx = scopeLocalCounters[found]++;
        } else {
            out[i].scope_idx = -1;
            out[i].local_idx = unscopedCounter++;
        }
    }
}

static bool LerpMatrix(const MtxF& prev, const MtxF& cur, float t, MtxF& out) {
    // Translation delta check — mismatched objects are hundreds of units apart.
    float dx = fabsf(cur.mf[3][0] - prev.mf[3][0]);
    float dy = fabsf(cur.mf[3][1] - prev.mf[3][1]);
    float dz = fabsf(cur.mf[3][2] - prev.mf[3][2]);
    if (dx > 100.0f || dy > 100.0f || dz > 100.0f) {
        return false;
    }

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            out.mf[r][c] = prev.mf[r][c] + (cur.mf[r][c] - prev.mf[r][c]) * t;
        }
    }
    return true;
}

std::unordered_map<Mtx*, MtxF> FrameInterpolation_Interpolate(float t) {
    std::unordered_map<Mtx*, MtxF> result;

    if (s_previousFrame.empty() || s_currentFrame.empty()) {
        return result;
    }

    result.reserve(s_currentFrame.size());

    // Assign matrices to scopes for both frames
    std::vector<ScopeAssignment> curAssign, prevAssign;
    AssignScopes(s_currentFrame, s_currentScopes, curAssign);
    AssignScopes(s_previousFrame, s_previousScopes, prevAssign);

    // Build lookup for previous frame: scope_key -> (local_idx -> matrix index)

    // Scoped previous: map from (scope_key, local_idx) -> previous frame index
    std::unordered_map<ScopeKey, std::unordered_map<uint32_t, size_t>, ScopeKeyHash> prevScoped;

    // Unscoped previous: map from unscoped local_idx -> previous frame index
    std::unordered_map<uint32_t, size_t> prevUnscoped;

    for (size_t i = 0; i < s_previousFrame.size(); i++) {
        if (prevAssign[i].scope_idx >= 0) {
            const ScopeKey& key = s_previousScopes[prevAssign[i].scope_idx].key;
            prevScoped[key][prevAssign[i].local_idx] = i;
        } else {
            prevUnscoped[prevAssign[i].local_idx] = i;
        }
    }

    // Match current frame matrices against previous frame
    for (size_t i = 0; i < s_currentFrame.size(); i++) {
        const MtxF& cur = s_currentFrame[i].value;
        const MtxF* prev = nullptr;

        if (curAssign[i].scope_idx >= 0) {
            // Scoped: look up by identity + local index
            const ScopeKey& key = s_currentScopes[curAssign[i].scope_idx].key;
            auto scopeIt = prevScoped.find(key);
            if (scopeIt != prevScoped.end()) {
                auto localIt = scopeIt->second.find(curAssign[i].local_idx);
                if (localIt != scopeIt->second.end()) {
                    prev = &s_previousFrame[localIt->second].value;
                }
            }
        } else {
            // Unscoped: match by global unscoped index
            auto it = prevUnscoped.find(curAssign[i].local_idx);
            if (it != prevUnscoped.end()) {
                prev = &s_previousFrame[it->second].value;
            }
        }

        if (prev) {
            MtxF interpolated;
            if (LerpMatrix(*prev, cur, t, interpolated)) {
                result[s_currentFrame[i].ptr] = interpolated;
            }
        }
    }

    return result;
}
