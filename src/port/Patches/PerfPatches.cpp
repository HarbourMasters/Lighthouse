#include <algorithm>
#include <cmath>
#include <cstring>

#include <spdlog/spdlog.h>

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Patches/Patches.h"
#include "port/ShipInit.hpp"

extern "C" {
#include <ultra64.h>
#include "functions.h"
#include "core2/lighting.h"

extern s32 D_80383450[CUBE_SORT_SCRATCH_SIZE];

// core1 helpers not exposed through functions.h.
extern void func_80256E24(f32 dst[3], f32 theta, f32 phi, f32 x, f32 y, f32 z);
extern void ml_vec3f_normalize(f32 vec[3]);
extern void viewport_getPosition_vec3f(f32 dst[3]);
extern void viewport_getRotation_vec3f(f32 dst[3]);
}

static const f32 kFrustumZX = 45.168514251708984f; // must match viewport.c
static const f32 kFrustumZY = 34.20201110839844f;
static constexpr float kDrawDistCullPadDeg = 25.0f;
static f32 sPadFrustumX = 89.21774f;
static f32 sPadFrustumY = 93.9692611694336f;
static f32 sPadPlanes[4][4];
static bool sPadPlanesDirty = true;

// TODO: Find better place for these two warn functions
// Capped so a bad frame can't flood the log; one print is enough to investigate.
extern "C" void port_warnPropNotInCube(int32_t index, int32_t propCnt) {
    static int32_t sReported = 0;
    if (sReported < 20) {
        sReported++;
        SPDLOG_WARN("cube_removeProp: prop not in cube (index {}, cnt {})", index, propCnt);
    }
}

// Fires only for a cube the old :5 unk0_4 would have silently corrupted, so if this
// never prints, no shipped map reaches the limit and the widening is forward-looking.
extern "C" void port_warnNodePropSplit(int32_t splitIndex, int32_t nodeCnt) {
    static int32_t sReported = 0;
    if (sReported < 20) {
        sReported++;
        SPDLOG_WARN("cube node-prop split {} of {} exceeds the old 31 limit", splitIndex, nodeCnt);
    }
}

// __cube_sort re-sorts every visible cube's props each tick, but only the camera
// moves, so the order rarely changes. A stable insertion sort skims an already
// sorted array in one pass and produces the same ordering as the vanilla sort.
static void RegisterCubeSort_Init() {
    COND_VB_SHOULD(VB_CUBE_PROP_SORT, EVENT_PRIORITY_NORMAL, true, {
        Cube* cube = va_arg(args, Cube*);
        s32 n = cube->prop2Cnt;
        for (s32 i = 1; i < n; i++) {
            if (D_80383450[i] <= D_80383450[i - 1]) {
                continue;
            }
            s32 key = D_80383450[i];
            Prop tmp;
            std::memcpy(&tmp, &cube->prop2Ptr[i], sizeof(Prop));
            s32 j = i - 1;
            while (j >= 0 && D_80383450[j] < key) {
                D_80383450[j + 1] = D_80383450[j];
                std::memcpy(&cube->prop2Ptr[j + 1], &cube->prop2Ptr[j], sizeof(Prop));
                j--;
            }
            D_80383450[j + 1] = key;
            std::memcpy(&cube->prop2Ptr[j + 1], &tmp, sizeof(Prop));
        }
        *should = false;
    });
}

// Same vertex recolor loop as gclights_recolor_vertices, but out-of-range lights
// are rejected with squared distances so only vertices inside a fade band pay a sqrt.
static void RegisterGcLights_Init() {
    COND_VB_SHOULD(VB_GCLIGHTS_RECOLOR, EVENT_PRIORITY_NORMAL, true, {
        Vtx* vtx = va_arg(args, Vtx*);
        Vtx* vtxEnd = va_arg(args, Vtx*);
        Vtx* refVtx = va_arg(args, Vtx*);
        Lighting** lightsBegin = va_arg(args, Lighting**);
        Lighting** lightsEnd = va_arg(args, Lighting**);

        // Active-light list caps at 16 entries (NUM_LIGHTING_ELEM).
        f32 minSq[16];
        f32 maxSq[16];
        s32 nLights = (s32)(lightsEnd - lightsBegin);
        for (s32 li = 0; li < nLights; li++) {
            minSq[li] = lightsBegin[li]->fade_radius_min * lightsBegin[li]->fade_radius_min;
            maxSq[li] = lightsBegin[li]->fade_radius_max * lightsBegin[li]->fade_radius_max;
        }

        for (; vtx < vtxEnd; vtx++, refVtx++) {
            f32 mod[3];
            mod[0] = mod[1] = mod[2] = 0.0f;
            for (s32 li = 0; li < nLights; li++) {
                const Lighting* light = lightsBegin[li];
                f32 dx = light->positionCopy[0] - (f32)refVtx->v.ob[0];
                f32 dy = light->positionCopy[1] - (f32)refVtx->v.ob[1];
                f32 dz = light->positionCopy[2] - (f32)refVtx->v.ob[2];
                f32 distSq = dx * dx + dy * dy + dz * dz;
                if (maxSq[li] <= distSq) {
                    continue;
                }
                if (distSq <= minSq[li]) {
                    mod[0] += light->red;
                    mod[1] += light->green;
                    mod[2] += light->blue;
                } else {
                    f32 dist = sqrtf(distSq);
                    f32 fade =
                        1.0f - ((dist - light->fade_radius_min) / (light->fade_radius_max - light->fade_radius_min));
                    mod[0] += fade * light->red;
                    mod[1] += fade * light->green;
                    mod[2] += fade * light->blue;
                }
            }
            vtx->v.cn[0] = (s8)((refVtx->v.cn[0] * mod[0]) / 256.0) & 0xFF;
            vtx->v.cn[1] = (s8)((refVtx->v.cn[1] * mod[1]) / 256.0) & 0xFF;
            vtx->v.cn[2] = (s8)((refVtx->v.cn[2] * mod[2]) / 256.0) & 0xFF;
        }
        *should = false;
    });
}

// Same rotations as mlMtx_rotate_yaw_deg/mlMtx_rotate_pitch_deg, but every
// snowflake shares one camera rotation, so cache the sin/cos instead of
// recomputing four of them per flake.
static void ApplySnowCameraRotation(const f32 camRot[3]) {
    constexpr double kBadDtor = 3.141592654 / 180.0;
    static f32 sYawDeg;
    static f32 sPitchDeg;
    static f32 sSinYaw, sCosYaw;
    static f32 sSinPitch, sCosPitch;
    static bool sHaveTrig = false;

    f32 pitchDeg = camRot[0];
    f32 yawDeg = camRot[1];
    if (!sHaveTrig || yawDeg != sYawDeg || pitchDeg != sPitchDeg) {
        f32 yawRad = (f32)(yawDeg * kBadDtor);
        f32 pitchRad = (f32)(pitchDeg * kBadDtor);
        sSinYaw = sinf(yawRad);
        sCosYaw = cosf(yawRad);
        sSinPitch = sinf(pitchRad);
        sCosPitch = cosf(pitchRad);
        sYawDeg = yawDeg;
        sPitchDeg = pitchDeg;
        sHaveTrig = true;
    }

    MtxF* m = mlMtx_get_stack_pointer();
    if (yawDeg != 0.0f) {
        for (s32 i = 0; i < 3; i++) {
            f32 r0 = m->mf[0][i];
            f32 r2 = m->mf[2][i];
            m->mf[0][i] = r0 * sCosYaw - r2 * sSinYaw;
            m->mf[2][i] = r0 * sSinYaw + r2 * sCosYaw;
        }
    }
    if (pitchDeg != 0.0f) {
        for (s32 i = 0; i < 3; i++) {
            f32 r1 = m->mf[1][i];
            f32 r2 = m->mf[2][i];
            m->mf[1][i] = r1 * sCosPitch + r2 * sSinPitch;
            m->mf[2][i] = r1 * -sSinPitch + r2 * sCosPitch;
        }
    }
}

static void RegisterSnowTrig_Init() {
    COND_VB_SHOULD(VB_SNOW_CAMERA_ROTATION, EVENT_PRIORITY_NORMAL, true, {
        ApplySnowCameraRotation(va_arg(args, f32*));
        *should = false;
    });
}

static void RebuildPaddedPlanes() {
    f32 rot[3];
    f32 pos[3];
    viewport_getRotation_vec3f(rot);
    viewport_getPosition_vec3f(pos);

    // Components are cotangent-form (component/Z = cot(half angle)), so
    // widening the frustum means shrinking the lateral component.
    const f32 kDegToRad = 3.14159265f / 180.0f;
    const f32 kMaxHalfAngle = 88.0f * kDegToRad;
    f32 halfX = atanf(kFrustumZX / sPadFrustumX) + kDrawDistCullPadDeg * kDegToRad;
    f32 halfY = atanf(kFrustumZY / sPadFrustumY) + kDrawDistCullPadDeg * kDegToRad;
    halfX = std::min(halfX, kMaxHalfAngle);
    halfY = std::min(halfY, kMaxHalfAngle);
    f32 px = kFrustumZX / tanf(halfX);
    f32 py = kFrustumZY / tanf(halfY);

    func_80256E24(sPadPlanes[0], rot[0], rot[1], -px, 0.0f, kFrustumZX);
    func_80256E24(sPadPlanes[1], rot[0], rot[1], px, 0.0f, kFrustumZX);
    func_80256E24(sPadPlanes[2], rot[0], rot[1], 0.0f, py, kFrustumZY);
    func_80256E24(sPadPlanes[3], rot[0], rot[1], 0.0f, -py, kFrustumZY);
    for (int i = 0; i < 4; i++) {
        ml_vec3f_normalize(sPadPlanes[i]);
        sPadPlanes[i][3] = -(pos[0] * sPadPlanes[i][0] + pos[1] * sPadPlanes[i][1] + pos[2] * sPadPlanes[i][2]);
    }
    sPadPlanesDirty = false;
}

// Culled when all 8 corners are outside one plane.
static bool PaddedBoxCulled(const f32 boxMin[3], const f32 boxMax[3]) {
    for (int p = 0; p < 4; p++) {
        bool allOutside = true;
        for (int c = 0; c < 8 && allOutside; c++) {
            f32 x = (c & 1) ? boxMax[0] : boxMin[0];
            f32 y = (c & 2) ? boxMax[1] : boxMin[1];
            f32 z = (c & 4) ? boxMax[2] : boxMin[2];
            if (sPadPlanes[p][0] * x + sPadPlanes[p][1] * y + sPadPlanes[p][2] * z + sPadPlanes[p][3] < 0.0f) {
                allOutside = false;
            }
        }
        if (allOutside) {
            return true;
        }
    }
    return false;
}

static void RegisterDrawDistCull_Init() {
    COND_HOOK(ViewportFrustumUpdate, EVENT_PRIORITY_LOW, true, [](IEvent* event) {
        auto* ev = (ViewportFrustumUpdate*)event;
        sPadFrustumX = *ev->frustumX;
        sPadFrustumY = *ev->frustumY;
        sPadPlanesDirty = true;
    });
    COND_VB_SHOULD(VB_DRAWDIST_BOX_CULL, EVENT_PRIORITY_NORMAL, true, {
        f32* boxMin = va_arg(args, f32*);
        f32* boxMax = va_arg(args, f32*);
        if (sPadPlanesDirty) {
            RebuildPaddedPlanes();
        }
        if (PaddedBoxCulled(boxMin, boxMax)) {
            *should = false;
        }
    });
}

static RegisterShipInitFunc sCubeSortInit(RegisterCubeSort_Init);
static RegisterShipInitFunc sGcLightsInit(RegisterGcLights_Init);
static RegisterShipInitFunc sSnowTrigInit(RegisterSnowTrig_Init);
static RegisterShipInitFunc sDrawDistCullInit(RegisterDrawDistCull_Init);