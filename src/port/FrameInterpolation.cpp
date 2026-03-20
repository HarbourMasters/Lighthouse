#include "FrameInterpolation.h"

#include <vector>
#include <algorithm>
#include <cstring>
#include <cmath>

// [port] Display-list matrix scanner for 60fps frame interpolation.
// Scans ROOT DL only for G_MTX commands (no G_DL branches — those are OTR geometry).
// Matches matrices between frames by sequential index.

// F3DEX opcodes (as int8_t, matching interpreter dispatch)
static constexpr int8_t OP_MTX   = (int8_t)0x01;  // G_MTX
static constexpr int8_t OP_ENDDL = (int8_t)(-72); // G_ENDDL = G_IMMFIRST - 7

struct MatrixEntry {
    Mtx* ptr;    // Pointer to the Mtx in the display list
    MtxF value;  // Float copy of the matrix data (GBI_FLOATS=1: Mtx is MtxF)
};

static std::vector<MatrixEntry> s_currentFrame;
static std::vector<MatrixEntry> s_previousFrame;

// [port] Scan root DL for modelview matrices. Stops at 2nd projection matrix (HUD switch).
static void ScanDisplayList(Gfx* commands, std::vector<MatrixEntry>& out) {
    if (!commands) return;

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
                if (projCount > 1) break; // Stop at HUD/overlay projection switch
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

void FrameInterpolation_RecordFrame(Gfx* commands) {
    s_previousFrame.swap(s_currentFrame);
    s_currentFrame.clear();
    ScanDisplayList(commands, s_currentFrame);
}

std::unordered_map<Mtx*, MtxF> FrameInterpolation_Interpolate(float t) {
    std::unordered_map<Mtx*, MtxF> result;

    if (s_previousFrame.empty() || s_currentFrame.empty()) {
        return result; // First frame or no data — no interpolation
    }

    // Match by index; skip pairs with large translation deltas (culling can shift indices).
    size_t minCount = std::min(s_previousFrame.size(), s_currentFrame.size());
    if (minCount == 0) {
        return result;
    }

    result.reserve(minCount);

    for (size_t i = 0; i < minCount; i++) {
        const MtxF& prev = s_previousFrame[i].value;
        const MtxF& cur  = s_currentFrame[i].value;

        // Translation delta check — mismatched objects are hundreds of units apart.
        float dx = fabsf(cur.mf[3][0] - prev.mf[3][0]);
        float dy = fabsf(cur.mf[3][1] - prev.mf[3][1]);
        float dz = fabsf(cur.mf[3][2] - prev.mf[3][2]);
        if (dx > 100.0f || dy > 100.0f || dz > 100.0f) {
            continue; // Mismatch — skip, interpreter uses original matrix
        }

        MtxF interpolated;
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                interpolated.mf[r][c] = prev.mf[r][c] + (cur.mf[r][c] - prev.mf[r][c]) * t;
            }
        }

        result[s_currentFrame[i].ptr] = interpolated;
    }

    return result;
}
