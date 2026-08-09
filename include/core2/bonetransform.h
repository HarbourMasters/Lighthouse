#ifndef _BONE_TRANSFORMATION_H_
#define _BONE_TRANSFORMATION_H_
#include <ultratypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    f32 unk0[4];
    f32 scale[3];
    f32 unk1C[3];
}BoneTransform;

typedef struct bone_transform_list_s{
    BoneTransform *ptr;
    s32 count;
}BoneTransformList;

BoneTransformList *boneTransformList_new(void);
BoneTransformList *boneTransformList_defrag(BoneTransformList *self);
void boneTransformList_free(BoneTransformList *self);
#ifdef __cplusplus
}
#endif

#endif
