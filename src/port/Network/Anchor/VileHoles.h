#pragma once
#include <stdint.h>

// Yumblie/grumblie hole positions in Mr. Vile's chamber (MAP_10_BGS_MR_VILE).
//
// The 25 holes are fixed NodeProps in the map setup data, laid out in a checkerboard grid
// (rows roughly at z = 900/600/300/0/-300/-600, columns every 400 units, alternating
// offset). All holes sit at y = -100, so only (x, z) is stored. Positions are identical
// on every client, so VileHoleId is a stable cross-client hole identifier for Anchor
// minigame sync packets.
//
// Enum names encode the exact whole-number coordinate (N = negative):
// VILE_HOLE_XN798_Z297 is the hole at (-798, 297) on the xz plane.

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VileHoleId {
    VILE_HOLE_NONE = -1,
    // Row z = 900
    VILE_HOLE_XN398_Z898,
    VILE_HOLE_X4_Z897,
    VILE_HOLE_X400_Z898,
    // Row z = 600
    VILE_HOLE_XN593_Z593,
    VILE_HOLE_XN198_Z597,
    VILE_HOLE_X200_Z596,
    VILE_HOLE_X599_Z598,
    // Row z = 300
    VILE_HOLE_XN798_Z297,
    VILE_HOLE_XN395_Z298,
    VILE_HOLE_X2_Z296,
    VILE_HOLE_X403_Z296,
    VILE_HOLE_X799_Z298,
    // Row z = 0
    VILE_HOLE_XN596_ZN2,
    VILE_HOLE_XN197_Z0,
    VILE_HOLE_X202_Z0,
    VILE_HOLE_X605_ZN8,
    // Row z = -300
    VILE_HOLE_XN794_ZN298,
    VILE_HOLE_XN398_ZN295,
    VILE_HOLE_X7_ZN304,
    VILE_HOLE_X403_ZN294,
    VILE_HOLE_X804_ZN304,
    // Row z = -600
    VILE_HOLE_XN594_ZN599,
    VILE_HOLE_XN198_ZN601,
    VILE_HOLE_X199_ZN604,
    VILE_HOLE_X603_ZN604,
    VILE_HOLE_COUNT
} VileHoleId;

// Hole (x, z) positions indexed by VileHoleId. All holes are at y = -100.
extern const float gVileHolePositions[VILE_HOLE_COUNT][2];

// Returns the id of the hole nearest to (x, z), or VILE_HOLE_NONE if nothing is within
// matching tolerance.
VileHoleId VileHoles_IdFromPosition(float x, float z);
// Returns the hole's float[2] (x, z) position, or NULL for an invalid id.
const float* VileHoles_GetPosition(VileHoleId id);

#ifdef __cplusplus
}
#endif
