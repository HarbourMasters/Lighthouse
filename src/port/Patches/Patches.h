#ifndef PORT_PATCHES_H
#define PORT_PATCHES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Threaded rendering (Game.cpp)

// Carries the interpolation pair with a submitted display list.
void Graphics_OnThread5Submit(void* taskData);
// Runs fn on the window thread when the tick is on its own thread.
void Graphics_RunOnRenderThread(void (*fn)(void*), void* arg);
void port_serviceRenderRequests(void);
// Waits for in-flight display lists before freeing or reading what they use.
void Graphics_PipelineSyncPoint(void);

// Frame Pacing (FramePacingPatches.cpp)

int FramePacingPatches_GetDemoViCount(void);
void FramePacingPatches_SetDemoViCount(int viCount);
unsigned port_getDemoViSerial(void);
int port_waitDemoViSerial(unsigned seen, int timeoutUs);
int FramePacingPatches_GetDemoDisplayViCount(int rawViCount);
void FramePacingPatches_TickCutsceneStutter(void);
int FramePacingPatches_GetCutsceneExtraVis(void);

// Localization (Localization.cpp)

void Localization_SetPrintScale(float scale);

// Dialog font (AltDialogFont.cpp, SpritePatches.cpp)

void port_dialogFontHd_rebuild(void);               // rebuild the HD glyph textures for the current base
void port_refreshDialogFontGlyphCount(void);        // re-read the reachable glyph count from the active font
int SpritePatches_DialogFontUsesShiftedCodes(void); // format codes shifted past the extended (PAL) glyph range

// Framebuffer (FramebufferPatches.cpp, TransitionPatches.cpp)

void Framebuffer_FreezeReadback(int freeze);
void Framebuffer_RequestReadback(void);
int Framebuffer_ConsumeReadbackRequest(void);
void Framebuffer_DrawPauseBackground(Gfx** gfx, int capture);
int Framebuffer_ConsumePauseRecapture(void);
int TransitionPatches_ShouldCapture(void);

void Framebuffer_PatchPictureModel(void* model_bin, int32_t min_xy, int32_t max_xy, int32_t min_z, int32_t max_z,
                                   uint32_t from);
int32_t port_getTransitionGpuFbId(void);
void Framebuffer_CaptureTransition(void* gfx_ptr);
void Framebuffer_PatchTransitionModel(void* model_bin);

// Sprite Display Cache (SpritePatches.cpp)

void SpritePatches_ClearDisplayCache(void);

// Camera (CameraPatches.cpp)

float port_wsCameraYawScale(void);
float port_wsCameraPitchScale(void);

// Input

float Ship_GetRumbleScale(void);

// Graphics (GraphicsPatches.cpp)

int GraphicsPatches_GetDrawDistanceSetting(void); // configured multiplier; safe during map load
int GraphicsPatches_GetDrawDistanceLevel(void);   // render-time multiplier; clamped to 1x outside normal gameplay
int GraphicsPatches_IsLodDisabled(void);
float GraphicsPatches_GetDrawDistanceMultiplier(void);
void GraphicsPatches_ApplyModelDrawDistanceCull(bool* fadeFlag, float* cullMult, float* cullDist);
int GraphicsPatches_IsSpriteSizeCulled(float depth, float size, float baseThreshold, int disableFlag);
float GraphicsPatches_GetHudOrthoShift(float refX);
void GraphicsPatches_ResetModelTLUT(Gfx** gfx);

// Vertex-animated models (AnimVertexPatches.cpp)

// Recycles the per-draw vertex arena; once per tick, before the display list is built.
void port_animVtx_beginTick(void);
// Copies the just-posed vertices somewhere only this draw points at, and repoints
// segment 0x01 at the copy.
void AnimVertexPatches_SnapshotVertices(Gfx** gfx, void* vertices, int32_t count);

// Volatile flag checks

int port_isInCharacterParade(void);

// Audio engine lock

void AudioSync_Lock(void);
void AudioSync_Unlock(void);
void port_audioIntMaskEnter(void);
void port_audioIntMaskExit(void);

// Volume sliders (AudioVolume.cpp)

float AudioVolume_GetMusic(void);
float AudioVolume_GetSfx(void);

// Note doors (NoteDoorNumbers.cpp)

void port_enableNoteDoorComposer(void);
int NoteDoorNumbers_GetCost(int doorIdx);

// Attract-demo audio hold

void GameEngine_BeginDemoAudioHold(void);
void port_tickDemoAudioHold(void);
int GameEngine_IsAudioHeld(void);
void port_noteMainLoopAlive(void);
int AudioSync_IsStallHeld(void);
int32_t OS_AudioPumpShouldWait(void);

int32_t port_breakable_isBroken(int32_t map, int32_t markerId, int32_t x, int32_t y, int32_t z);

void port_remoteCarry_setCarried(uint32_t clientId, int32_t markerId, float offset[3], float yawOffset);

// Connected, in a real room, syncing world state.
int32_t port_anchor_isWorldSyncActive(void);

// The puzzle mirrors below are home-scoped.
void port_puzzleStep_orBits(int32_t puzzleId, int32_t bits);
int32_t port_puzzleStep_get(int32_t puzzleId);
int32_t port_puzzleStep_getForMap(int32_t map, int32_t puzzleId);
int32_t port_puzzleStep_getForLevel(int32_t levelId, int32_t puzzleId);

void port_puzzlePos_mark(int32_t puzzleId, int32_t x, int32_t y, int32_t z);
int32_t port_puzzlePos_isMarked(int32_t puzzleId, int32_t x, int32_t y, int32_t z);

void port_puzzleCount_add(int32_t counterId, int32_t delta);
int32_t port_puzzleCount_get(int32_t counterId);

// Rate-limited: a marker reached cube_removeProp with a propPtr outside its cube.
void PerfPatches_WarnPropNotInCube(int32_t index, int32_t propCnt);

// Rate-limited: a cube's node-prop split index passed 31, where the old :5 field wrapped.
void PerfPatches_WarnNodePropSplit(int32_t splitIndex, int32_t nodeCnt);

#ifdef __cplusplus
}
#endif

#endif
