#ifndef MODERN_CAMERA_H
#define MODERN_CAMERA_H

#define MODERN_ORBIT_CAM_STATE 0x15

#ifdef __cplusplus
extern "C" {
#endif

void ModernCamera_Update(void);
float ModernCamera_GetInvertXSign(void);
float ModernCamera_GetInvertYSign(void);
float ModernCamera_GetFirstPersonInvertXSign(void);

#ifdef __cplusplus
}
#endif

#endif // MODERN_CAMERA_H
