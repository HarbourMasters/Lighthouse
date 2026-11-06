#ifndef PORT_PATCHES_H
#define PORT_PATCHES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Threaded rendering (Game.cpp)

// Carries the interpolation pair with a submitted display list.
void port_thread5_onSubmit(void* taskData);
// Runs fn on the window thread when the tick is on its own thread.
void port_runOnRenderThread(void (*fn)(void*), void* arg);
void port_serviceRenderRequests(void);
// Waits for in-flight display lists before freeing or reading what they use.
void port_pipelineSyncPoint(void);

// Frame Pacing (FramePacingPatches.cpp)

int port_getDemoViCount(void);
void port_setDemoViCount(int viCount);
unsigned port_getDemoViSerial(void);
int port_waitDemoViSerial(unsigned seen, int timeoutUs);
int port_getDemoDisplayViCount(int rawViCount);
void port_tickCutsceneStutter(void);
int port_getCutsceneExtraVis(void);

// Localization (Localization.cpp)

void port_setPrintScale(float scale);

// Dialog font (AltDialogFont.cpp, SpritePatches.cpp)

void port_dialogFontHd_rebuild(void);        // rebuild the HD glyph textures for the current base
void port_refreshDialogFontGlyphCount(void); // re-read the reachable glyph count from the active font
int port_dialogFontUsesShiftedCodes(void);   // format codes shifted past the extended (PAL) glyph range

// Framebuffer (FramebufferPatches.cpp)

void port_freezeReadback(int freeze);
void port_requestReadback(void);
int port_consumeReadbackRequest(void);
void Framebuffer_DrawPauseBackground(Gfx** gfx, int capture);
int port_pauseConsumeRecaptureRequest(void);
int port_shouldCaptureTransition(void);

void port_patchPictureModel(void* model_bin, int32_t min_xy, int32_t max_xy, int32_t min_z, int32_t max_z,
                            uint32_t from);
int32_t port_getTransitionGpuFbId(void);
void port_captureTransitionFb(void* gfx_ptr);
void port_patchTransitionModel(void* model_bin);

// Sprite Display Cache (SpritePatches.cpp)

void port_spriteDisplayCache_clear(void);

// Camera (CameraPatches.cpp)

float port_wsCameraYawScale(void);
float port_wsCameraPitchScale(void);

// Input

float port_getRumbleScale(void);

// Graphics (GraphicsPatches.cpp)

int port_getDrawDistanceSetting(void); // configured multiplier; safe during map load
int port_getDrawDistanceLevel(void);   // render-time multiplier; clamped to 1x outside normal gameplay
int port_shouldDisableLOD(void);
float port_drawDistanceMul(void);
void port_applyModelDrawDistanceCull(bool* fadeFlag, float* cullMult, float* cullDist);
int port_spriteSizeCulled(float depth, float size, float baseThreshold, int disableFlag);
float port_hudOrthoShift(float refX);
void port_modelRenderResetTLUT(Gfx** gfx);

// Vertex-animated models (AnimVertexPatches.cpp)

// Recycles the per-draw vertex arena; once per tick, before the display list is built.
void port_animVtx_beginTick(void);
// Copies the just-posed vertices somewhere only this draw points at, and repoints
// segment 0x01 at the copy.
void port_modelRender_snapshotAnimVertices(Gfx** gfx, void* vertices, int32_t count);

// Volatile flag checks

int port_isInCharacterParade(void);

// Audio engine lock

void port_lockAudio(void);
void port_unlockAudio(void);
void port_audioIntMaskEnter(void);
void port_audioIntMaskExit(void);

// Volume sliders (AudioVolume.cpp)

float port_getMusicVolume(void);
float port_getSfxVolume(void);

// Note doors (NoteDoorNumbers.cpp)

void port_enableNoteDoorComposer(void);

// Attract-demo audio hold

void port_beginDemoAudioHold(void);
void port_tickDemoAudioHold(void);
int port_audioHeld(void);
void port_noteMainLoopAlive(void);
int port_audioStallHold(void);
int32_t port_audioPumpShouldWait(void);

int32_t port_breakable_isBroken(int32_t map, int32_t markerId, int32_t x, int32_t y, int32_t z);

int32_t port_cutsceneWarp_getReturnMap(void);

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

void port_fpTwinkly_release(void);

void port_puzzleCount_add(int32_t counterId, int32_t delta);
int32_t port_puzzleCount_get(int32_t counterId);

// Rate-limited: a marker reached cube_removeProp with a propPtr outside its cube.
void port_warnPropNotInCube(int32_t index, int32_t propCnt);

// Rate-limited: a cube's node-prop split index passed 31, where the old :5 field wrapped.
void port_warnNodePropSplit(int32_t splitIndex, int32_t nodeCnt);

#ifdef __cplusplus
}
#endif

#endif
