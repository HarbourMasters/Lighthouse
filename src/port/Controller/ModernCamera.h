#ifndef MODERN_CAMERA_H
#define MODERN_CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

int port_modernCamera_handleYaw(void);
void port_modernCamera_handleZoom(void);
int port_camera_suppressVanillaZoom(void);

#ifdef __cplusplus
}
#endif

#endif // MODERN_CAMERA_H
