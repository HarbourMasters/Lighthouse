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

// Framebuffer (FramebufferPatches.cpp)

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

// Save (SaveEnhancements.cpp)

void port_syncBottlesBonusIndex(void);

// Camera (CameraPatches.cpp)

void port_camera_applyWsYawFix(float rotation[3]);

// Input

float port_getRumbleScale(void);

// Graphics (GraphicsPatches.cpp)

int port_getDrawDistanceLevel(void);
int port_shouldForceHighPolyBanjo(void);

// Fixes (GameFixes.cpp)

int port_fixMumboTokenId(int ret, int pos[3], int map_id);
int port_shouldAllowAllHoneycombExtensions(void);

#ifdef __cplusplus
}
#endif

#endif
