#ifndef BANJO_KAZOOIE_CORE2_ANIMATIONFILE_H
#define BANJO_KAZOOIE_CORE2_ANIMATIONFILE_H

#include <ultra64.h>
#include "core2/bonetransform.h"

typedef struct bk_animation_file_element_data_s {
    u16 unk0_15 : 1;
    u16 unk0_14 : 1;
    u16 unk0_13 : 14;
    s16 unk2;
} BKAnimationFileElementData;

typedef struct bk_animation_file_element_s {
    u16 bone_id : 12;
    u16 unk0_3 : 4;
    s16 data_count;
    BKAnimationFileElementData data[];
} BKAnimationFileElement;

typedef struct bk_animation_file_bin_s { 
    s16 unk0; // first animation frame (?)
    s16 unk2; // last animation frame (?)
    s16 element_count;
    u16 pad;
//  BKAnimationFileElement elements[];
    u16 elements[];
} BKAnimationFileBin;

f32 animationfilebin_func_8033AA10(BKAnimationFileBin *self, s32 arg1);
void animationfilebin_getBoneTransformList(BKAnimationFileBin *self, f32 progress, BoneTransformList *bone_transform_list);
f32 animationfilebin_func_8033ABA0(BKAnimationFileBin *self, f32 arg1);
f32 animationfilebin_func_8033ABCC(BKAnimationFileBin *self);
s32 animationfilebin_func_8033AC0C(BKAnimationFileBin *self);
s32 animationfilebin_func_8033AC14(BKAnimationFileBin *self);
s32 animationfilebin_func_8033AC1C(BKAnimationFileBin *self);
s32 animationfilebin_getElementCount(BKAnimationFileBin *self);
f32 animationfilebin_func_8033AC38(BKAnimationFileBin *self, BKAnimationFileElement *element, f32 time);

#endif
