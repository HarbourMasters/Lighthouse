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
double port_getTargetFrameTime(void);
int port_getCutsceneExtraVis(void);
void port_resetCutsceneTimings(void);

// Framebuffer (FramebufferPatches.cpp)
// Functions using Gfx* or BKModelBin* are only callable from C translation
// units that include <ultra64.h>. Declare with void* here to avoid pulling
// N64 headers (which define macros incompatible with C++).

void port_setViBlack(int active);
int port_isViBlack(void);
void port_freezeReadback(int freeze);
void port_requestReadback(void);
int port_consumeReadbackRequest(void);
int port_getPauseFramebufferId(void);
int port_shouldCaptureTransition(void);

int32_t port_getAuxGpuFbId(void);
void port_readAuxFbToCpu(void* gfx_ptr);
void port_patchPictureModel(void* model_bin, int32_t min_xy, int32_t max_xy, int32_t min_z, int32_t max_z,
                            uint32_t from);
int32_t port_getTransitionGpuFbId(void);
void port_readTransitionFbToCpu(void* gfx_ptr);
void port_patchTransitionModel(void* model_bin);

// Sprite Display Cache (SpritePatches.cpp)

void port_spriteDisplayCache_clear(void);

// Save (SaveManager.cpp / SavePatches.cpp)

void port_restoreFileEnhancementData(int eepromSlot);
void port_onFileLoaded(int gamenum, int filenum);
void port_syncBottlesBonusIndex(void);

// Camera (CameraPatches.cpp)

int32_t port_camera_isCutsceneAspectActive(void);
void port_camera_updateCutsceneAspect(int32_t mapId);
void port_camera_setStaticNode(int32_t nodeIndex);
void port_camera_clearStaticNode(void);
void port_camera_applyWsYawFix(float rotation[3]);

// Input

float port_getRumbleScale(void);

#ifdef __cplusplus
}
#endif

#endif
