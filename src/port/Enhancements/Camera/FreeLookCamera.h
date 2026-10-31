#ifndef PORT_FREELOOK_CAMERA_H
#define PORT_FREELOOK_CAMERA_H

// Free Look Camera
//
// The right stick orbits the camera freely around the player. Uses dynamicCamA
// as a base, but is driven continuously from the analog stick instead of
// snapping in fixed 45 degree increments.
//
// It runs as a new dynamic camera state (FREELOOK_CAM_STATE). VB_CAMERA_SWIVEL
// decides when to enter, hold, or hand the frame back; OnDynamicCameraUpdate
// moves the camera each frame.
//
// Behavior: controls yaw + pitch and holds its angle indefinitely. Distance
// tracks the vanilla zoom level (shared with the modern-scheme camera), so the
// zoom controls re-zoom in place. Returns to the normal camera when the player
// rotates or enters first-person via a C-button camera control.

#define FREELOOK_CAM_STATE 0x14

#ifdef __cplusplus
extern "C" {
#endif

int port_freeLook_isEnabled(void);

#ifdef __cplusplus
}
#endif

#endif
