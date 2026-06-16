#ifndef PORT_FREELOOK_CAMERA_H
#define PORT_FREELOOK_CAMERA_H

// Free Look Camera
//
// The right stick orbits the camera freely around the player. Uses dynamicCamA
// as a base, but is driven continuously from the analog stick instead of
// snapping in fixed 45 degree increments.
//
// It runs as a new dynamic camera state (FREELOOK_CAM_STATE). ba_camera's
// rotate handler calls port_freeLook_handle() to decide when to enter, hold, or
// hand the frame back; dynamicCamera's update loop calls
// port_freeLookCamera_update() to actually move the camera each frame.
//
// Behavior: controls yaw + pitch, holds its angle indefinitely, and returns
// to the normal camera as soon as the player uses a C-button camera control.

#define FREELOOK_CAM_STATE 0x14

#ifdef __cplusplus
extern "C" {
#endif

int port_freeLook_isEnabled(void);
int port_freeLook_handle(void);
void port_freeLookCamera_update(void);

#ifdef __cplusplus
}
#endif

#endif
