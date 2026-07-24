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
extern float D_8037D948[3]; // dynamicCamera.c collision anchor: the point the camera stays visible from
float func_802BD8D4(void);  // target orbit distance (zoom level)
float func_802BD51C(void);  // target camera height

float ml_acosf(float x);
float mlNormalizeAngle(float deg);
float mlDiffDegF(float a, float b); // shortest signed a-b in degrees
float gu_sqrtf(float x);
float time_getDelta(void);
}

namespace {

constexpr float kDistanceRate = 8.0f;
constexpr float kReturnRateFactor = 0.2f; // collision offset relaxes this much slower than it deepens
constexpr float kVertRate = 0.8f;         // height + aim Y follow rate; low to filter floor-height churn
constexpr float kRotRate = 12.0f;         // look-at damping, mirroring the vanilla follow camera

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
    c->rotValid = 0;
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

    // The collision resolve can trap the camera far from the player; snap home when the gap runs away.
    bool driftEscape = false;
    float zoomTarget = func_802BD8D4();
    if (c->smoothValid && zoomTarget > 0.0f) {
        float rgx = center[0] - c->smoothPos[0];
        float rgz = center[2] - c->smoothPos[2];
        float limit = zoomTarget * 2.0f;
        if ((rgx * rgx + rgz * rgz) > (limit * limit)) {
            c->distance = zoomTarget;
            c->smoothValid = 0;
            driftEscape = true;
        }
    }

    // Track the vanilla zoom-level distance so zoom controls still apply while orbiting.
    float distTrack = clampf(kDistanceRate * dt, 0.0f, 1.0f);
    c->distance += (func_802BD8D4() - c->distance) * distTrack;

    float offset[3];
    func_80256E24(offset, c->allowPitch ? c->pitch : 0.0f, c->yaw, 0.0f, 0.0f, c->distance);

    float pos[3];
    pos[0] = center[0] + offset[0];
    pos[2] = center[2] + offset[2];
    if (c->allowPitch) {
        pos[1] = center[1] + offset[1]; // pitch carries the vertical offset
    } else {
        // Flat orbit: height low-passes the vanilla target, filtering plank/step floor churn.
        c->height += (func_802BD51C() - c->height) * clampf(kVertRate * dt, 0.0f, 1.0f);
        pos[1] = c->height;
    }
    ncDynamicCamera_setPosition(pos);

    // Resolve geometry; on a drift escape, anchor at the player so the resolve pulls back in.
    if (driftEscape) {
        D_8037D948[0] = center[0];
        D_8037D948[1] = center[1];
        D_8037D948[2] = center[2];
    }
    func_802BE60C();

    float resolved[3];
    ncDynamicCamera_getPosition(resolved);

    // pos already follows the player smoothly; the shake near thin/translucent geometry lives in the
    // collision correction, so smooth only that (damps every direction with no added follow lag).
    float corr[3] = { resolved[0] - pos[0], resolved[1] - pos[1], resolved[2] - pos[2] };
    if (!c->smoothValid) {
        c->collisionOffset[0] = corr[0];
        c->collisionOffset[1] = corr[1];
        c->collisionOffset[2] = corr[2];
        c->smoothValid = 1;
    } else {
        // Deepen the pull-in fast (never clip through geometry), relax slowly so a flickering
        // collision settles at its deep value instead of oscillating.
        float newSq = corr[0] * corr[0] + corr[1] * corr[1] + corr[2] * corr[2];
        float curSq = c->collisionOffset[0] * c->collisionOffset[0] + c->collisionOffset[1] * c->collisionOffset[1] +
                      c->collisionOffset[2] * c->collisionOffset[2];
        float rate = (newSq > curSq) ? c->smoothRate : c->smoothRate * kReturnRateFactor;
        float f = clampf(rate * dt, 0.0f, 1.0f);
        c->collisionOffset[0] += (corr[0] - c->collisionOffset[0]) * f;
        c->collisionOffset[1] += (corr[1] - c->collisionOffset[1]) * f;
        c->collisionOffset[2] += (corr[2] - c->collisionOffset[2]) * f;
    }
    c->smoothPos[0] = pos[0] + c->collisionOffset[0];
    c->smoothPos[1] = pos[1] + c->collisionOffset[1];
    c->smoothPos[2] = pos[2] + c->collisionOffset[2];
    ncDynamicCamera_setPosition(c->smoothPos);

    // Aim at the player; smooth only the target Y (X/Z follow directly for responsive horizontal aim).
    if (!c->aimValid) {
        c->aimCenterY = center[1];
        c->aimValid = 1;
    } else {
        c->aimCenterY += (center[1] - c->aimCenterY) * clampf(kVertRate * dt, 0.0f, 1.0f);
    }
    float aimCenter[3] = { center[0], c->aimCenterY, center[2] };

    float rot[3];
    func_802BC434(rot, aimCenter, c->smoothPos);

    // Damp the look-at instead of snapping to it, like the vanilla camera; filters residual aim wobble.
    if (!c->rotValid) {
        c->smoothRot[0] = rot[0];
        c->smoothRot[1] = rot[1];
        c->smoothRot[2] = rot[2];
        c->rotValid = 1;
    } else {
        float rf = clampf(kRotRate * dt, 0.0f, 1.0f);
        c->smoothRot[0] = mlNormalizeAngle(c->smoothRot[0] + mlDiffDegF(rot[0], c->smoothRot[0]) * rf);
        c->smoothRot[1] = mlNormalizeAngle(c->smoothRot[1] + mlDiffDegF(rot[1], c->smoothRot[1]) * rf);
        c->smoothRot[2] = rot[2];
    }
    ncDynamicCamera_setRotation(c->smoothRot);
}
