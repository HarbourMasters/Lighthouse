#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <ultratypes.h>
#include <core2/file.h>

#define CAMERA_NODE_FLAG_HITS 0x1 // collide the camera with level geometry as it moves (func_802BE60C)
#define CAMERA_NODE_FLAG_VFIX 0x2 // hold the vertical Y instead of tracking Banjo's height (Type 1 only)
#define CAMERA_NODE_FLAG_BEE  0x4 // whether this node still applies while the bee is flying

typedef struct {
    s32 type: 24;
    s32 valid: 8;
    uintptr_t data_ptr;
} CameraNode;

// Pivot camera
typedef struct {
    f32 position[3];
    f32 horizontalSpeed;
    f32 verticalSpeed;
    f32 rotation;
    f32 accelaration;
    f32 pitchYawRoll[3]; // 0 = pitch, 1 = yaw, 2 = roll
    s32 unknownFlag; // CAMERA_NODE_FLAG_*
} PivotCameraNode;

PivotCameraNode *cameraNodeType1_init();
void cameraNodeType1_free(PivotCameraNode *self);
void cameraNodeType1_fromFile(File *file_ptr, PivotCameraNode *self);
void cameraNodeType1_getPosition(PivotCameraNode *self, f32 position[3]);
void cameraNodeType1_getHorizontalAndVerticalSpeed(PivotCameraNode *self, f32 *horizontal_speed, f32 *vertical_speed);
void cameraNodeType1_getRotationAndAccelaration(PivotCameraNode *self, f32 *rotation, f32 *accelaration);
bool cameraNodeType1_getHits(PivotCameraNode *self);
bool cameraNodeType1_getBee(PivotCameraNode *self);
bool cameraNodeType1_getVFix(PivotCameraNode *self);

// Static camera
typedef struct {
    f32 position[3];
    f32 pitchYawRoll[3]; // 0 = pitch, 1 = yaw, 2 = roll
} StaticCameraNode;

StaticCameraNode *cameraNodeType2_init();
void cameraNodeType2_free(StaticCameraNode *self);
void cameraNodeType2_fromFile(File *file_ptr, StaticCameraNode *self);
void cameraNodeType2_getPosition(StaticCameraNode *self, f32 position[3]);
/*
 * Sets 0 to pitch, 1 to yaw and 2 to roll
 */
void cameraNodeType2_getPitchYawRoll(StaticCameraNode *self, f32 pitch_yaw_roll[3]);

// Zoom camera
typedef struct {
    f32 position[3];
    f32 horizontalSpeed;
    f32 verticalSpeed;
    f32 rotation;
    f32 accelaration;
    f32 closeDistance;
    f32 farDistance;
    f32 pitchYawRoll[3]; // 0 = pitch, 1 = yaw, 2 = roll
    s32 unknownFlag; // CAMERA_NODE_FLAG_HITS and _BEE only
} ZoomCameraNode;
ZoomCameraNode *cameraNodeType3_init();
void cameraNodeType3_free(ZoomCameraNode *self);
void cameraNodeType3_fromFile(File *file_ptr, ZoomCameraNode *self);
void cameraNodeType3_getPosition(ZoomCameraNode *self, f32 position[3]);
void cameraNodeType3_getHorizontalAndVerticalSpeed(ZoomCameraNode *self, f32 *horizontal_speed, f32 *vertical_speed);
void cameraNodeType3_getRotationAndAccelaration(ZoomCameraNode *self, f32 *rotation, f32 *accelaration);
/*
 * Sets 0 to pitch, 1 to yaw and 2 to roll
 */
void cameraNodeType3_getPositionWithPitchYawRoll(ZoomCameraNode *self, f32 pitch_yaw_roll[3]);
f32 cameraNodeType3_getCloseDistance(ZoomCameraNode *self);
f32 cameraNodeType3_getFarDistance(ZoomCameraNode *self);
bool cameraNodeType3_getBee(ZoomCameraNode *self);
bool cameraNodeType3_getHits(ZoomCameraNode *self);

typedef struct {
    s32 unknownFlag;
} RandomCameraNode;
RandomCameraNode *cameraNodeType4_init();
void cameraNodeType4_free(RandomCameraNode *self);
void cameraNodeType4_fromFile(File *file_ptr, RandomCameraNode *self);
#endif
