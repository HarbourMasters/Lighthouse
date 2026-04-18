#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
#include <unordered_map>
#include <fast/types.h>

std::unordered_map<Mtx*, MtxF> FrameInterpolation_Interpolate(float t);

extern "C" {
#endif

// Frame boundary
void FrameInterpolation_StartRecord(void);
void FrameInterpolation_StopRecord(void);
void FrameInterpolation_ShouldInterpolateFrame(bool shouldInterpolate);

// Hierarchical scope (identity for cross-frame matching)
void FrameInterpolation_RecordOpenChild(const void* key, uintptr_t id);
void FrameInterpolation_RecordCloseChild(void);
void FrameInterpolation_RecordMarker(const char* file, int line);

// Mix three scalar fields into a stable scope id. Use for scope keys made
// from multiple fields (modelId+pos, particle spawn randomness, etc.)
// where no single field is unique on its own.
uintptr_t FrameInterpolation_Hash3(uint64_t a, uint64_t b, uint64_t c);
void FrameInterpolation_RecordOpenChildHash3(const char* key, uint64_t a, uint64_t b, uint64_t c);

// Safe float->u32 for feeding into the hash above.
static inline uint64_t FrameInterpolation_FloatBits(float f) {
    uint32_t u;
    memcpy(&u, &f, sizeof(u));
    return (uint64_t)u;
}

// Low-level matrix primitive ops (called from src/core1/math/matrix_stack.c)
void FrameInterpolation_RecordMatrixIdent(void);
void FrameInterpolation_RecordMatrixTranslate(float x, float y, float z);
void FrameInterpolation_RecordMatrixRotYaw(float degrees);
void FrameInterpolation_RecordMatrixRotPitch(float degrees);
void FrameInterpolation_RecordMatrixRotRoll(float degrees);
void FrameInterpolation_RecordMatrixScale(float x, float y, float z);
void FrameInterpolation_RecordMatrixSet(const float src[4][4]);
void FrameInterpolation_RecordMatrixMult(const float l[4][4], const float r[4][4]);
void FrameInterpolation_RecordMatrixToMtx(void* dst, const float src[4][4]);

// Capture the three Euler angles that build BK's projection rotation (BK
// puts camera rotation in projection, not modelview). Recording the angles
// instead of the matrices lets replay angle-lerp with shortest-path wrap
// and rebuild clean rotation matrices — element-wise matrix lerp on these
// goes degenerate on fast spins (talon trot, snap-turns).
void FrameInterpolation_RecordCameraProjectionRotation(void* rollMtx, float rollDeg, void* pitchMtx, float pitchDeg,
                                                       void* yawMtx, float yawDeg);

// Capture camera world-space position once per tick. Interpolate() uses it
// as a cut detector: a large frame-to-frame jump means the camera
// teleported (warp, fixed-cam snap, etc.) and lerping across it would
// briefly place the world between two scenes. Backstop for anything that
// teleports the camera without calling DontInterpolateCamera directly.
void FrameInterpolation_RecordCameraPosition(const float pos[3]);

// Flag ToMtx ops inside this bracket as "don't lerp, use curr as-is".
// Sprite modelviews need this — cube culling reorders them across frames
// so flat-index pairing mismatches and produces one-frame teleports.
void FrameInterpolation_NoInterpolatePush(void);
void FrameInterpolation_NoInterpolatePop(void);

// Record a sprite's raw inputs so replay can lerp in natural spaces
// (angles for rotations, linear for pos/scale) and rebuild a clean matrix.
// Needed because billboards buzz when their modelview is frozen while the
// projection is angle-lerped.
//
// kind matches the three sprite paths in sprite/render.c:
//   BILLBOARD       — func_80344138. camYaw/camPitch, no spriteRoll.
//   BILLBOARD_ROLL  — func_80344424. camYaw/camPitch + spriteRoll forward.
//   FULL            — func_80344720. rotation[] drives orientation.
//
// The caller must NoInterpolatePush/Pop around the following mlMtxApply
// so replay doesn't write two competing matrices for the same Mtx*.
#define FI_SPRITE_KIND_BILLBOARD 0
#define FI_SPRITE_KIND_BILLBOARD_ROLL 1
#define FI_SPRITE_KIND_FULL 2
void FrameInterpolation_RecordSpriteDraw(int kind, void* dst, const float camRelPos[3], const float scale[3],
                                         float camYaw, float camPitch, float spriteRoll, const float rotation[3],
                                         int mirrored);

// Drop the prev tree so the next Interpolate() returns empty. Call at
// known camera cuts (map load, camera type change, etc.).
void FrameInterpolation_DontInterpolateCamera(void);

// Stable cross-tick identity for short-lived heap objects. Without this,
// a freed emitter's address gets reused by the next one and the new
// object inherits the dead one's matrix pairing — visible as a ghost on
// the first sub-frame after respawn. Register at alloc, Unregister at
// free; Get returns the registered id or the pointer value as a fallback.
uintptr_t FrameInterpolation_RegisterId(const void* ptr);
uintptr_t FrameInterpolation_GetId(const void* ptr);
void FrameInterpolation_UnregisterId(const void* ptr);

#ifdef __cplusplus
}
#endif
