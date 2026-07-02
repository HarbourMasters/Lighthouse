#ifndef PORT_PATCHES_H
#define PORT_PATCHES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Frame Pacing (FramePacingPatches.cpp)

int port_getDemoViCount(void);
void port_setDemoViCount(int viCount);
int port_getDemoDisplayViCount(int rawViCount);
int port_getCutsceneExtraVis(void);
int port_getInterpolationFpsCap(void);

// Localization (Localization.cpp)

int port_pauseMenuNeedsRefresh(void); // language or Return-to-Lair CVar changed while menu open
void port_pauseMenuRebuild(void);     // free + recreate + replay the main menu open
void port_setPrintScale(float scale);

// Framebuffer (FramebufferPatches.cpp)

void port_setViBlack(int active);
int port_isViBlack(void);
void port_freezeReadback(int freeze);
void port_requestReadback(void);
int port_consumeReadbackRequest(void);
int port_getPauseFramebufferId(void);
int port_capturePauseFramebuffer(void);
void port_getPauseFramebufferSize(int* w, int* h);
int port_pauseConsumeRecaptureRequest(void);
int port_shouldCaptureTransition(void);

int32_t port_getAuxGpuFbId(void);
void port_readAuxFbToCpu(void* gfx_ptr);
void port_patchPictureModel(void* model_bin, int32_t min_xy, int32_t max_xy, int32_t min_z, int32_t max_z,
                            uint32_t from);
int32_t port_getTransitionGpuFbId(void);
void port_captureTransitionFb(void* gfx_ptr);
void port_patchTransitionModel(void* model_bin);

// Sprite Display Cache (SpritePatches.cpp)

void port_spriteDisplayCache_clear(void);

// Save (SaveEnhancements.cpp)

void port_syncBottlesBonusIndex(void);

// Camera (CameraPatches.cpp)

void port_camera_applyWsYawFix(float rotation[3]);

// Input

float port_getRumbleScale(void);

// Gameplay

int port_scalePlayerDamage(int damage);

// Graphics (GraphicsPatches.cpp)

int port_getDrawDistanceLevel(void);
int port_shouldDisableLOD(void);
float port_drawDistanceMul(void);
void port_applyModelDrawDistanceCull(int* fadeFlag, float* cullMult, float* cullDist);
int port_spriteSizeCulled(float depth, float size, float baseThreshold, int disableFlag);
float port_hudOrthoShift(float refX);

// Mirror (MirrorPatches.cpp)

int port_mirror_active(void);
void port_mirror_beginScene(void);
void port_mirror_endScene(void);
void port_mirror_undoProjection(Gfx** gfx, Mtx** mtx);
void port_viewport_applyMirror(Gfx** gfx, Mtx** mtx);
void port_mirror_markCapture(void);
int port_mirror_shouldFlipPauseBg(void);

// Mirror per-model exclusion (counter-mirror text-bearing objects)
void port_mirror_setExclude(void);
void port_mirror_clearExclude(void);
int port_mirror_bakeCounterScale(void);
void port_mirror_patchTextActors(void);

// Volatile flag checks

int port_isInCharacterParade(void);

// Audio engine lock

void port_lockAudio(void);
void port_unlockAudio(void);
void port_audioIntMaskEnter(void);
void port_audioIntMaskExit(void);

// Attract-demo audio hold

void port_beginDemoAudioHold(void);

#ifdef __cplusplus
}
#endif

#endif
