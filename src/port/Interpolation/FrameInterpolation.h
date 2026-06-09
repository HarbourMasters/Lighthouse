#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
#include <unordered_map>
#include <fast/types.h>

// Caller owns and reuses `out` across sub-frames; we clear entries but the
// bucket array persists.
void FrameInterpolation_Interpolate(float t, std::unordered_map<Mtx*, MtxF>& out);

extern "C" {
#endif

// Frame boundary.
void FrameInterpolation_StartRecord(void);
void FrameInterpolation_StopRecord(void);
void FrameInterpolation_ShouldInterpolateFrame(bool shouldInterpolate);

// Hierarchical scope (cross-tick pairing identity).
void FrameInterpolation_RecordOpenChild(const void* key, uintptr_t id);
void FrameInterpolation_RecordCloseChild(void);
void FrameInterpolation_RecordMarker(const char* file, int line);

// Mix three scalar fields into a stable scope id when no single field is
// unique on its own (e.g. modelId+pos, particle spawn randomness).
uintptr_t FrameInterpolation_Hash3(uint64_t a, uint64_t b, uint64_t c);
void FrameInterpolation_RecordOpenChildHash3(const char* key, uint64_t a, uint64_t b, uint64_t c);

// Safe float->u32 for feeding into Hash3.
static inline uint64_t FrameInterpolation_FloatBits(float f) {
    uint32_t u;
    memcpy(&u, &f, sizeof(u));
    return (uint64_t)u;
}

// Low-level matrix primitives (from src/core1/math/matrix_stack.c).
void FrameInterpolation_RecordMatrixIdent(void);
void FrameInterpolation_RecordMatrixTranslate(float x, float y, float z);
void FrameInterpolation_RecordMatrixRotYaw(float degrees);
void FrameInterpolation_RecordMatrixRotPitch(float degrees);
void FrameInterpolation_RecordMatrixRotRoll(float degrees);
void FrameInterpolation_RecordMatrixScale(float x, float y, float z);
void FrameInterpolation_RecordMatrixSet(const float src[4][4]);
void FrameInterpolation_RecordMatrixMult(const float l[4][4], const float r[4][4]);
void FrameInterpolation_RecordMatrixToMtx(void* dst, const float src[4][4]);

// BK puts camera rotation in projection, not modelview. We record the
// three Euler angles so replay can shortest-path angle-lerp and rebuild
// matrices via guRotateF — element-wise matrix lerp on these goes
// degenerate on fast spins (talon trot, snap-turns).
void FrameInterpolation_RecordCameraProjectionRotation(void* rollMtx, float rollDeg, void* pitchMtx, float pitchDeg,
                                                       void* yawMtx, float yawDeg);

// Cut-detect backstop: Interpolate() bails if camera position jumps more
// than ~1000 units between ticks, in case a teleport site doesn't call
// DontInterpolateCamera directly.
void FrameInterpolation_RecordCameraPosition(const float pos[3]);

// Flag the ToMtx inside this bracket as "use curr as-is, no lerp".
// Sprite modelviews need this — cube culling reorders them across ticks,
// so flat-index pairing would produce one-frame teleports.
void FrameInterpolation_NoInterpolatePush(void);
void FrameInterpolation_NoInterpolatePop(void);

// kind matches the three sprite paths in sprite/render.c:
//   BILLBOARD       — func_80344138. camYaw/camPitch, no spriteRoll.
//   BILLBOARD_ROLL  — func_80344424. camYaw/camPitch + spriteRoll forward.
//   FULL            — func_80344720. rotation[] drives orientation.
// Caller must NoInterpolatePush/Pop around the following mlMtxApply so
// replay doesn't write two competing matrices for the same Mtx*.
#define FI_SPRITE_KIND_BILLBOARD 0
#define FI_SPRITE_KIND_BILLBOARD_ROLL 1
#define FI_SPRITE_KIND_FULL 2
void FrameInterpolation_RecordSpriteDraw(int kind, void* dst, const float camRelPos[3], const float scale[3],
                                         float camYaw, float camPitch, float spriteRoll, const float rotation[3],
                                         int mirrored);

// Drop the prev tree at known camera cuts (map load, camera type change).
void FrameInterpolation_DontInterpolateCamera(void);

// Stable cross-tick identity for short-lived heap objects. Without it, a
// freed emitter's address gets reused by the next allocation and the new
// object inherits the dead one's matrix pairing (ghost on first sub-frame
// after respawn). Register at alloc, Unregister at free.
uintptr_t FrameInterpolation_RegisterId(const void* ptr);
uintptr_t FrameInterpolation_GetId(const void* ptr);
void FrameInterpolation_UnregisterId(const void* ptr);

#ifdef __cplusplus
}
#endif
