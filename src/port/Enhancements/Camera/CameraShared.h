#pragma once

extern "C" {
// Math helpers
void func_80256E24(float dst[3], float pitch, float yaw, float x, float y, float z);
void ml_vec3f_add(float dst[3], float a[3], float b[3]);
void ml_vec3f_clear(float dst[3]);
float mlNormalizeAngle(float deg);

float time_getDelta(void);

void controller_getRightStick(int controller_index, float dst[2]);

int bainput_should_rotate_camera_left(void);
int bainput_should_rotate_camera_right(void);
int bainput_should_look_first_person_camera(void);

int ncDynamicCamera_getState(void);
void ncDynamicCamera_setState(int state);
void ncDynamicCamera_getPosition(float dst[3]);
void ncDynamicCamera_setRotation(float src[3]);

// Vanilla follow-camera machinery
extern float D_8037DB70;    // dynamicCamB.c: orbit yaw, degrees
extern float D_8037D9C8[3]; // dynamicCamera.c: rotation spring velocity

void func_802C0150(int mode);        // select the focus target
void func_802C0370(void);            // record the pre-move position (anti-flip guard)
void func_802C0394(float target[3]); // record the ideal target (anti-flip guard)
void func_802C03BC(void);            // undo a collision resolve that crossed over
void func_802C0490(float dst[3]);    // focus/orbit center
void func_802C04B0(void);            // re-derive D_8037DB70 from the live camera

float func_802BD8D4(void);                           // target orbit distance (zoom level)
void func_802BE190(float target[3]);                 // position spring toward target
void func_802BE230(float gain, float damp);          // position spring tuning
int func_802BE60C(void);                             // swept camera collision + slide
int func_802BC84C(int mode);                         // occluded-too-long recovery
void func_802BE6FC(float rotOut[3], float focus[3]); // look-at from the live position
}

namespace PortCamera {

inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
inline bool ManualCameraControl() {
    return bainput_should_rotate_camera_left() || bainput_should_rotate_camera_right() ||
           bainput_should_look_first_person_camera();
}
inline float SmoothTowards(float& current, bool& valid, float target, float rate, float dt) {
    if (!valid) {
        current = target;
        valid = true;
    } else {
        current += (target - current) * clampf(rate * dt, 0.0f, 1.0f);
    }
    return current;
}

} // namespace PortCamera
