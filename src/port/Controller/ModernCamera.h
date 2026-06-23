#ifndef MODERN_CAMERA_H
#define MODERN_CAMERA_H

#define MODERN_ORBIT_CAM_STATE 0x15

#ifdef __cplusplus
extern "C" {
#endif

int port_modernCamera_handleYaw(void);
void port_modernCamera_update(void);
void port_modernCamera_handleZoom(void);
int port_camera_suppressVanillaZoom(void);

#ifdef __cplusplus
}
#endif

#endif // MODERN_CAMERA_H
