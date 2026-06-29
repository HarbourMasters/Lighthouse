#include "VileHoles.h"

#include <stddef.h>

// Exact NodeProp coordinates captured from the map setup data (see VileHoles.h for layout).
const float gVileHolePositions[VILE_HOLE_COUNT][2] = {
    // Row z = 900
    { -398.0f, 898.0f }, // VILE_HOLE_XN398_Z898
    { 4.0f, 897.0f },    // VILE_HOLE_X4_Z897
    { 400.0f, 898.0f },  // VILE_HOLE_X400_Z898
    // Row z = 600
    { -593.0f, 593.0f }, // VILE_HOLE_XN593_Z593
    { -198.0f, 597.0f }, // VILE_HOLE_XN198_Z597
    { 200.0f, 596.0f },  // VILE_HOLE_X200_Z596
    { 599.0f, 598.0f },  // VILE_HOLE_X599_Z598
    // Row z = 300
    { -798.0f, 297.0f }, // VILE_HOLE_XN798_Z297
    { -395.0f, 298.0f }, // VILE_HOLE_XN395_Z298
    { 2.0f, 296.0f },    // VILE_HOLE_X2_Z296
    { 403.0f, 296.0f },  // VILE_HOLE_X403_Z296
    { 799.0f, 298.0f },  // VILE_HOLE_X799_Z298
    // Row z = 0
    { -596.0f, -2.0f }, // VILE_HOLE_XN596_ZN2
    { -197.0f, 0.0f },  // VILE_HOLE_XN197_Z0
    { 202.0f, 0.0f },   // VILE_HOLE_X202_Z0
    { 605.0f, -8.0f },  // VILE_HOLE_X605_ZN8
    // Row z = -300
    { -794.0f, -298.0f }, // VILE_HOLE_XN794_ZN298
    { -398.0f, -295.0f }, // VILE_HOLE_XN398_ZN295
    { 7.0f, -304.0f },    // VILE_HOLE_X7_ZN304
    { 403.0f, -294.0f },  // VILE_HOLE_X403_ZN294
    { 804.0f, -304.0f },  // VILE_HOLE_X804_ZN304
    // Row z = -600
    { -594.0f, -599.0f }, // VILE_HOLE_XN594_ZN599
    { -198.0f, -601.0f }, // VILE_HOLE_XN198_ZN601
    { 199.0f, -604.0f },  // VILE_HOLE_X199_ZN604
    { 603.0f, -604.0f },  // VILE_HOLE_X603_ZN604
};

// Adjacent holes are ~300 units apart, so anything within 100 units is unambiguous.
#define VILE_HOLE_MATCH_TOLERANCE_SQ (100.0f * 100.0f)

VileHoleId VileHoles_IdFromPosition(float x, float z) {
    VileHoleId best = VILE_HOLE_NONE;
    float bestDistSq = VILE_HOLE_MATCH_TOLERANCE_SQ;

    for (int32_t i = 0; i < VILE_HOLE_COUNT; i++) {
        float dx = gVileHolePositions[i][0] - x;
        float dz = gVileHolePositions[i][1] - z;
        float distSq = dx * dx + dz * dz;
        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            best = (VileHoleId)i;
        }
    }
    return best;
}

const float* VileHoles_GetPosition(VileHoleId id) {
    if (id <= VILE_HOLE_NONE || id >= VILE_HOLE_COUNT) {
        return NULL;
    }
    return gVileHolePositions[id];
}
