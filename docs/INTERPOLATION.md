# Frame Interpolation: Architecture & Findings

## Overview

This document captures the findings from an extensive investigation into frame interpolation for Lighthouse (Banjo-Kazooie PC port). The game logic runs at 30fps; the display interpolates to 60fps or higher by generating sub-frames with replacement matrices.

## Current State

The FPS slider and Match Refresh Rate are hooked up in `Engine.cpp` via `CVAR_SETTING("InterpolationFPS")`, `CVAR_SETTING("MatchRefreshRate")`, and `CVAR_VSYNC_ENABLED`. The existing DL-scanning interpolation system (`FrameInterpolation.cpp`) works but has significant limitations.

## The Fundamental Problem

**Camera translation is baked into every modelview matrix.** In `func_80252AF0` (mlmtx.c), every object's modelview is computed as `translate(obj_pos - cam_pos) * rotate * scale`. The camera position is an inseparable part of each matrix.

This means:
- Interpolating a modelview matrix also interpolates the camera position (linear/chord path)
- The camera rotation lives in the projection (3 `guRotate` MUL commands)
- Interpolating camera rotation independently follows an angular/arc path
- The chord (translation) and arc (rotation) diverge during camera orbiting -> **visible ghosting/buzzing**

**This is the core architectural limitation.** Every approach tried hits this wall.

## Approaches Tried

### 1. DL Scanning with Scope Matching (Original System)

**How it works:** Scan the root display list for `G_MTX` commands, collect matrices, match between frames using `ScopeBegin/End` identity (actor pointer + array index) or global unscoped index.

**Problems:**
- Unscoped matrices (map, sky, particles) matched by sequential index — shifts when frustum culling changes the visible set -> ghosting
- Camera rotation in projection not interpolated -> 30fps camera snap
- Adding camera rotation interpolation causes arc-vs-chord mismatch with baked modelview -> buzzing
- The original scan broke at the sky's first projection MUL (`projCount > 1`) and collected zero scene matrices — interpolation was effectively a no-op

### 2. Camera Separation (Rendering Pipeline Change)

**How it works:** Emit camera translation as a separate base modelview in `viewport_setRenderPerspectiveMatrix`. Objects use `G_MTX_MUL` (world-space) instead of `G_MTX_LOAD` (camera-baked). Camera and objects interpolate independently.

**Problems:**
- Root matrix works correctly with G_MTX_MUL
- **Bone matrices cannot use G_MTX_MUL** — `mlMtx_push_multiplied_2(D_80383BF8, bone_anim)` computes `bone_anim × D_80383BF8` (bone BEFORE object). With MUL, the bone ends up AFTER the object on the RSP stack -> reversed multiplication order
- Bones MUST use G_MTX_LOAD with camera-baked D_80383BF8
- This creates a split: root is world-space (MUL), bones are camera-baked (LOAD) -> inconsistent interpolation -> ghosting

### 3. Operation Tree Recording (Sister Port Pattern)

**How it works:** Record every matrix operation (translate, rotate, scale) during the game tick with parameters. Build a tree using `OpenChild/CloseChild` for hierarchical matching. At interpolation time, replay operations with interpolated parameters.

**Implementation:** Completed and tested locally, uncommitted. Instrumented `mlmtx.c` (Identity, Translate, RotateYaw/Pitch/Roll, Scale, MatrixToMtx), `viewport.c` (GuRotate for camera), `render.c` (OpenChild/CloseChild per model draw, PushMultiplied for bones), `scenedraw.c` (scopes for sky/map/player/actors).

**Advantages over DL scanning:**
- Correct hierarchical matching (no index mismatch)
- Angle-aware rotation interpolation (LerpAngle with shortest-path wrapping)
- Separate recording of obj/cam positions in `func_80252AF0` via `CameraTranslate` op
- Sprite detection (no Identity -> skip interpolation for viewport-matrix-baked sprites)

**Remaining problem:** Even with separate obj/cam recording, interpolating the camera position produces the same chord-vs-arc mismatch with the non-interpolated camera rotation. Not interpolating the camera position causes the relative obj-cam distance to change on sub-frames when both move together (camera follows player).

## Key Technical Details

### Matrix Flow: Models vs Sprites

| Aspect | Models (`modelRender_draw`) | Sprites Type 1/2 (`func_80344138/424`) | Sprites Type 3 (`func_80344720`) |
|--------|---------------------------|---------------------------------------|----------------------------------|
| Init | `mlMtxIdent()` | `mlMtxSet(viewport_getMatrix())` | `mlMtxIdent()` |
| Camera rotation | NOT in matrix (in projection) | BAKED IN from viewport matrix | NOT in matrix |
| Position | `mlMtxTranslate(obj-cam)` via `func_80252AF0` | `func_80252330(x,y,z)` direct set | `func_80252330(x,y,z)` direct set |
| Rotation | `mlMtxRotYaw/Pitch/Roll` | Optional roll only | Full PYR |

### Why Sprites Buzz

Sprite types 1/2 use `mlMtxSet(viewport_getMatrix())` which copies the full viewport rotation matrix (camera yaw+pitch). Element-wise lerp of two different rotation matrices produces non-orthogonal intermediates -> visible buzzing. These sprites must be identified and skipped. Detection: they don't call `mlMtxIdent()`.

### Why Bones Can't Separate

The RSP matrix stack processes matrices in order: `vertex × modelview × projection`. Bone matrices are computed as `bone_anim * D_80383BF8` where D_80383BF8 is the object base. This multiplication places the bone transform BEFORE the object transform. With G_MTX_LOAD, the entire combined matrix replaces the stack — correct. With G_MTX_MUL, the bone would be placed AFTER whatever is on the stack (camera × object) -> `camera * object * bone * object` = doubled object transform.

### BK Angle Convention

BK rotation functions (`mlMtxRotYaw/Pitch/Roll`) take degrees and multiply by `BAD_DTOR = π/180` (standard conversion). The rotation functions use standard `sinf/cosf`. Replay functions must match the exact sign conventions:
- **Yaw:** `row0 = row0*cos - row2*sin`, `row2 = row0*sin + row2*cos`
- **Pitch:** `row1 = row1*cos + row2*sin`, `row2 = -row1*sin + row2*cos`
- **Roll:** `row0 = row0*cos + row1*sin`, `row1 = -row0*sin + row1*cos`

### DL Structure

```
[sky viewport_setRenderPerspectiveMatrix: LOAD proj + 3 MUL proj + LOAD mv]
[sky modelRender_draw: PUSH|LOAD mv + bone PUSH|LOAD mv + POP]
[main viewport_setRenderPerspectiveMatrix: LOAD proj + 3 MUL proj + LOAD mv]
[map modelRender_draw: PUSH|LOAD mv + ...]
[actor modelRender_draw: PUSH|LOAD mv + bone PUSH|LOAD mv + POP] -> Loop for actors, particles, effects etc
[HUD viewport: LOAD proj + LOAD mv]
```

## How Recomp (RT64) Solves This

BanjoRecomp uses `gEXSetViewMatrixFloat` to provide the camera translation as a separate view matrix to RT64. RT64 handles the camera/object separation internally at the GPU level:

1. Modelview matrices still have camera baked in (game code unchanged)
2. RT64 receives the explicit view matrix and can subtract it from modelviews
3. Each component interpolates independently with per-draw-call transform IDs
4. `G_EX_INTERPOLATE_SIMPLE` for boned models, `G_EX_INTERPOLATE_DECOMPOSE` for others

This requires renderer-level support that LUS's Fast3D interpreter doesn't have.

## Files Reference

| File | Role |
|------|------|
| `src/port/FrameInterpolation.cpp` | Core interpolation system |
| `src/port/FrameInterpolation.h` | Public API |
| `src/port/Engine.cpp` | `ProcessGfxCommands` orchestration, `GetInterpolationFPS` CVar hookup |
| `src/core1/mlmtx.c` | Matrix stack operations (`func_80252AF0` bakes camera) |
| `src/core1/viewport.c` | Camera rotation in projection (`guRotate` MULs) |
| `src/core2/model/render.c` | `modelRender_draw`, bone handlers (cmd0/2/5), D_80383BF8 |
| `src/core2/scenedraw.c` | Main scene draw function |
| `src/core2/sprite/render.c` | Sprite rendering (3 paths with different matrix init) |
| `src/core2/actor_cubepropsystem.c` | `__marker_draw` with ScopeBegin/End |
| `libultraship/src/fast/interpreter.cpp` | `AdjXForAspectRatio`, `GfxSpMatrix` replacement map |
