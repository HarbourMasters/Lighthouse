// BanjoDecomp: core2/code_336F0.c
#include <ultra64.h>

#include <core2/file.h>

#include "functions.h"
#include "variables.h"
#include <core2/camera.h>


static void __cameraNodeType1_setPosition(PivotCameraNode *this, f32 position[3]);
static void __cameraNodeType1_func_802BA808(PivotCameraNode *this, f32 arg1[3]);
static void __cameraNodeType1_setHorizontalAndVerticalSpeed(PivotCameraNode *this, f32 horizontal_speed, f32 vertical_speed);
static void __cameraNodeType1_setRotationAndAccelaration(PivotCameraNode *this, f32 rotation, f32 accelaration);
static void __cameraNodeType1_setHits(PivotCameraNode *this, s32 arg1);
static void __cameraNodeType1_setBee(PivotCameraNode *this, s32 arg1);
static void __cameraNodeType1_setVFix(PivotCameraNode *this, s32 arg1);

#define PIVOT_CAMERA_POSITION_INDICATOR          0x01
#define PIVOT_CAMERA_HORI_VERT_SPEED_INDICATOR   0x02
#define PIVOT_CAMERA_TURN_SPEED_INDICATOR        0x03
#define PIVOT_CAMERA_ROTATION_INDICATOR          0x04
#define PIVOT_CAMERA_UNK_5_INDICATOR             0x05
#define PIVOT_CAMERA_END_INDICATOR               0x00

static void __cameraNodeType1_setFlag(PivotCameraNode *this, s32 arg1, s32 arg2) {
    if(arg1) {
        this->unknownFlag |= arg2;
    } else{
        this->unknownFlag &= ~arg2;
    }
}

static bool __cameraNodeType1_getFlag(PivotCameraNode *this, s32 arg1) {
    if (this->unknownFlag & arg1) 
        return true;

    return false;
}

PivotCameraNode *cameraNodeType1_init() {
    PivotCameraNode *this;
    f32 sp20[3];

    this = (PivotCameraNode *)bk_malloc(sizeof(PivotCameraNode));
    ml_vec3f_clear(sp20);
    __cameraNodeType1_setPosition(this, sp20);
    __cameraNodeType1_func_802BA808(this, sp20);
    __cameraNodeType1_setHorizontalAndVerticalSpeed(this, 0.7f, 2.33f);
    __cameraNodeType1_setRotationAndAccelaration(this, 4.0f, 16.0f);
    __cameraNodeType1_setHits(this, 0);
    __cameraNodeType1_setVFix(this, 1);
    __cameraNodeType1_setBee(this, 0);
    return this;
}

void cameraNodeType1_free(PivotCameraNode *this) {
    bk_free(this);
}

void cameraNodeType1_getPosition(PivotCameraNode *this, f32 position[3]) {
    ml_vec3f_copy(position, this->position);
}

static void __cameraNodeType1_setPosition(PivotCameraNode *this, f32 position[3]) {
    ml_vec3f_copy(this->position, position);
}

void code336F0_func_802BA7D8(PivotCameraNode *this, f32 arg1[3]) {
    ml_vec3f_add(arg1, this->position, this->pitchYawRoll);
}

static void __cameraNodeType1_func_802BA808(PivotCameraNode *this, f32 arg1[3]) {
    ml_vec3f_diff_copy(this->pitchYawRoll, arg1, this->position);
}

void cameraNodeType1_getHorizontalAndVerticalSpeed(PivotCameraNode *this, f32 *horizontal_speed, f32 *vertical_speed) {
    *horizontal_speed = this->horizontalSpeed;
    *vertical_speed = this->verticalSpeed;
}

static void __cameraNodeType1_setHorizontalAndVerticalSpeed(PivotCameraNode *this, f32 horizontal_speed, f32 vertical_speed) {
    this->horizontalSpeed = horizontal_speed;
    this->verticalSpeed = vertical_speed;
}

void cameraNodeType1_getRotationAndAccelaration(PivotCameraNode *this, f32 *rotation, f32 *accelaration) {
    *rotation = this->rotation;
    *accelaration = this->accelaration;
}

static void __cameraNodeType1_setRotationAndAccelaration(PivotCameraNode *this, f32 rotation, f32 accelaration) {
    this->rotation = rotation;
    this->accelaration = accelaration;
}

bool cameraNodeType1_getHits(PivotCameraNode *this) {
    return __cameraNodeType1_getFlag(this, CAMERA_NODE_FLAG_HITS);
}

bool cameraNodeType1_getBee(PivotCameraNode *this) {
    return __cameraNodeType1_getFlag(this, CAMERA_NODE_FLAG_BEE);
}

bool cameraNodeType1_getVFix(PivotCameraNode *this) {
    return __cameraNodeType1_getFlag(this, CAMERA_NODE_FLAG_VFIX);
}

static void __cameraNodeType1_setHits(PivotCameraNode *this, s32 arg1) {
    __cameraNodeType1_setFlag(this, arg1, CAMERA_NODE_FLAG_HITS);
}

static void __cameraNodeType1_setBee(PivotCameraNode *this, s32 arg1) {
    __cameraNodeType1_setFlag(this, arg1, CAMERA_NODE_FLAG_BEE);
}

static void __cameraNodeType1_setVFix(PivotCameraNode *this, s32 arg1) {
    __cameraNodeType1_setFlag(this, arg1, CAMERA_NODE_FLAG_VFIX);
}

void cameraNodeType1_fromFile(File *file_ptr, PivotCameraNode *this) {
    while(!file_isNextByteExpected(file_ptr, PIVOT_CAMERA_END_INDICATOR)) {
        if(!file_getNFloats_ifExpected(file_ptr, PIVOT_CAMERA_POSITION_INDICATOR, this->position, 3)) {
            if(file_isNextByteExpected(file_ptr, PIVOT_CAMERA_HORI_VERT_SPEED_INDICATOR)) {
                file_getFloat(file_ptr, &this->horizontalSpeed);
                file_getFloat(file_ptr, &this->verticalSpeed);
            }
            else if(file_isNextByteExpected(file_ptr, PIVOT_CAMERA_TURN_SPEED_INDICATOR)) {
                file_getFloat(file_ptr, &this->rotation);
                file_getFloat(file_ptr, &this->accelaration);
            }
            else if(!file_getNFloats_ifExpected(file_ptr, PIVOT_CAMERA_ROTATION_INDICATOR, this->pitchYawRoll, 3)) {
                file_getWord_ifExpected(file_ptr, PIVOT_CAMERA_UNK_5_INDICATOR, &this->unknownFlag);
            }
        }
    }
}
