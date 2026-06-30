// Shared right-stick orbit camera core. See OrbitCameraCore.h.

#include "port/Enhancements/Camera/OrbitCameraCore.h"

extern "C" {
// Vanilla camera / math entry points used to drive the orbit state.
void ncDynamicCamera_getPosition(float dst[3]);
void ncDynamicCamera_setPosition(float src[3]);
void ncDynamicCamera_setRotation(float src[3]);
int ncDynamicCamera_getState(void);
void ncDynamicCamera_setState(int state);

void func_802C02D4(float center[3]);                                                 // camera focus/orbit center
void func_80256E24(float dst[3], float pitch, float yaw, float x, float y, float z); // spherical -> offset
int func_8025801C(float vec[3], float* yaw);                                         // vector -> yaw (degrees)
void func_802BC434(float rotOut[3], float fromPos[3], float targetPos[3]);           // look-at rotation
int func_802BE60C(void);                                                             // swept camera collision + slide
float func_802BD8D4(void); // target orbit distance (zoom level)
float func_802BD51C(void); // target camera height

float ml_acosf(float x);
float mlNormalizeAngle(float deg);
float gu_sqrtf(float x);
float time_getDelta(void);
}

namespace {

// 1/sec; how fast distance chases the zoom-level target.
constexpr float kDistanceRate = 8.0f;
// 1/sec; vertical follow (flat-pitch mode only). Matched to the vanilla camera,
// whose Y lerp is target*dt*2, so the camera stays mostly put during a jump
// (Banjo rises in frame) instead of choppily chasing the parabola up and down.
constexpr float kHeightRate = 2.0f;
// 1/sec; how fast the look-at target's Y chases its raw value. The orbit center
// Y is derived from the floor height under the player, which snaps up in discrete
// steps on stairs; smoothing it de-steps the aim pitch the way the vanilla camera's
// rotation tracker (func_802BD904) does, without adding lag to the stick-driven yaw.
constexpr float kAimHeightRate = 8.0f;

float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

} // namespace

extern "C" void OrbitCamera_Capture(OrbitCamera* c) {
    float camPos[3];
    float center[3];
    ncDynamicCamera_getPosition(camPos);
    func_802C02D4(center);

    float diff[3] = { camPos[0] - center[0], camPos[1] - center[1], camPos[2] - center[2] };

    c->yaw = 0.0f;
    func_8025801C(diff, &c->yaw);

    if (c->allowPitch) {
        float dist = gu_sqrtf(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
        c->distance = dist;
        float sinPitch = clampf(-diff[1] / dist, -1.0f, 1.0f);
        float pitchMag = ml_acosf(sinPitch);
        c->pitch = clampf(sinPitch < 0.0f ? -pitchMag : pitchMag, c->minPitch, c->maxPitch);
    } else {
        c->distance = gu_sqrtf(diff[0] * diff[0] + diff[2] * diff[2]);
        c->height = camPos[1];
        c->pitch = 0.0f;
    }

    c->justEntered = 1;
    c->smoothValid = 0;
    c->aimValid = 0;
}

extern "C" void OrbitCamera_Enter(OrbitCamera* c) {
    OrbitCamera_Capture(c);
    c->active = 1;
    ncDynamicCamera_setState(c->stateId);
}

extern "C" void OrbitCamera_Exit(OrbitCamera* c) {
    c->active = 0;
    if (ncDynamicCamera_getState() == c->stateId) {
        ncDynamicCamera_setState(0xB); // hand back to the normal follow camera
    }
}

extern "C" void OrbitCamera_Update(OrbitCamera* c, float yawDelta, float pitchDelta) {
    float dt = time_getDelta();
    float center[3];
    func_802C02D4(center);

    c->yaw = mlNormalizeAngle(c->yaw + yawDelta);
    if (c->allowPitch) {
        c->pitch = clampf(c->pitch + pitchDelta, c->minPitch, c->maxPitch);
    }
    c->justEntered = 0;

    // Track the vanilla zoom-level distance so the normal zoom controls still take
    // effect while orbiting.
    float distTrack = clampf(kDistanceRate * dt, 0.0f, 1.0f);
    c->distance += (func_802BD8D4() - c->distance) * distTrack;

    float offset[3];
    func_80256E24(offset, c->allowPitch ? c->pitch : 0.0f, c->yaw, 0.0f, 0.0f, c->distance);

    float pos[3];
    pos[0] = center[0] + offset[0];
    pos[2] = center[2] + offset[2];
    if (c->allowPitch) {
        // Pitch carries the vertical offset.
        pos[1] = center[1] + offset[1];
    } else {
        // Flat orbit: height follows the vanilla target, more slowly than distance
        // so jumps read as natural rather than choppy.
        float heightTrack = clampf(kHeightRate * dt, 0.0f, 1.0f);
        c->height += (func_802BD51C() - c->height) * heightTrack;
        pos[1] = c->height;
    }
    ncDynamicCamera_setPosition(pos);

    // Resolve geometry.
    func_802BE60C();

    float resolved[3];
    ncDynamicCamera_getPosition(resolved);

    if (!c->smoothValid) {
        c->smoothPos[0] = resolved[0];
        c->smoothPos[1] = resolved[1];
        c->smoothPos[2] = resolved[2];
        c->smoothValid = 1;
    } else {
        // Smooth the resolved position to prevent hitching on edges.
        float f = clampf(c->smoothRate * dt, 0.0f, 1.0f);
        c->smoothPos[0] += (resolved[0] - c->smoothPos[0]) * f;
        c->smoothPos[1] += (resolved[1] - c->smoothPos[1]) * f;
        c->smoothPos[2] += (resolved[2] - c->smoothPos[2]) * f;
    }
    ncDynamicCamera_setPosition(c->smoothPos);

    // Aim back at the player from the smoothed position. Smooth only the target's
    // Y: the orbit center Y steps with the floor height under the player.
    // X/Z follow the player directly so horizontal aim stays responsive.
    if (!c->aimValid) {
        c->aimCenterY = center[1];
        c->aimValid = 1;
    } else {
        float aimTrack = clampf(kAimHeightRate * dt, 0.0f, 1.0f);
        c->aimCenterY += (center[1] - c->aimCenterY) * aimTrack;
    }
    float aimCenter[3] = { center[0], c->aimCenterY, center[2] };

    float rot[3];
    func_802BC434(rot, aimCenter, c->smoothPos);
    ncDynamicCamera_setRotation(rot);
}
