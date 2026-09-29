// BanjoDecomp: core2/animationfile.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#include "assets.h"
#include "animation.h"
#include "bk_math.h"

extern f32 glspline_catmull_rom_interpolate(f32, s32, f32 *);
void func_8033AFB8(BoneTransformList *arg0, s32 arg1, f32 arg2[3][3]);

f32 D_803709E0[] = {
    0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f
};

// convert animation frame to time (?)
f32 animationfilebin_func_8033AA10(BKAnimationFileBin *this, s32 arg1) {
    if (arg1 == this->unk2) {
        return 0.999999f;
    }

    return (f32) (arg1 - this->unk0) / (f32) (this->unk2 - this->unk0);
}

void animationfilebin_getBoneTransformList(BKAnimationFileBin *this, f32 progress, BoneTransformList *bone_transform_list) {
    s32 bone_id;
    int i;
    f32 tmp_f22;
    BKAnimationFileElement *element;
    // [port] unk0_3 indexes 0-8 across rotation(0-2)/scale(3-5)/translation(6-8).
    // Original code used f32[3][3] with sp54[0][unk0_3] — UB when unk0_3 >= 3.
    // Use a flat array to avoid strict-aliasing/bounds optimization issues in Release.
    f32 sp54_flat[9];
    f32 (*sp54)[3] = (f32 (*)[3])sp54_flat;

    if (this == NULL) return; // [port] sentinel anim IDs (0xFFFE) produce NULL from animcommoncache_get
    tmp_f22 = animationfilebin_func_8033ABA0(this, progress);
//  element = this->elements;
    element = (BKAnimationFileElement *) this->elements;
    bone_id = 0;

    for (i = 0; i < this->element_count; i++) {
        if (element->bone_id != bone_id) {
            if (bone_id != 0) {
                func_8033AFB8(bone_transform_list, bone_id, sp54);
            }
            bone_id = element->bone_id;
            TUPLE_SET(sp54[0], 0.0f);
            TUPLE_SET(sp54[1], 1.0f);
            TUPLE_SET(sp54[2], 0.0f);
        }

//      sp54[0][element->unk0_3] = animationfilebin_func_8033AC38(this, element, tmp_f22);
        sp54_flat[element->unk0_3] = animationfilebin_func_8033AC38(this, element, tmp_f22);
        element = (BKAnimationFileElement *) (element->data + element->data_count);
    }

    func_8033AFB8(bone_transform_list, bone_id, sp54);
}

// convert time to animation frame (?)
f32 animationfilebin_func_8033ABA0(BKAnimationFileBin *this, f32 arg1) {
    return this->unk0 + arg1 * (this->unk2 - this->unk0);
}

f32 animationfilebin_func_8033ABCC(BKAnimationFileBin *this) {
    f32 tmp = animationfilebin_func_8033AC0C(this);
    return (tmp - 1.0) / tmp;
}

// get last animation frame
s32 animationfilebin_func_8033AC0C(BKAnimationFileBin *this) {
    return this->unk2;
}

// get first animation frame
s32 animationfilebin_func_8033AC14(BKAnimationFileBin *this) {
    return this->unk0;
}

// get animation frame count
s32 animationfilebin_func_8033AC1C(BKAnimationFileBin *this) {
    return this->unk2 - this->unk0 + 1;
}

s32 animationfilebin_getElementCount(BKAnimationFileBin *this) {
    return this->element_count;
}

f32 animationfilebin_func_8033AC38(BKAnimationFileBin *this, BKAnimationFileElement *element, f32 time) {
    BKAnimationFileElementData *last_anim, *first_anim, *i_anim;
    f32 temp_f12;
    f32 knot_list[4];
    u32 temp_t2;

    first_anim = element->data;

    if ((s32) time < first_anim->unk0_13) {
        knot_list[0] = knot_list[1] = D_803709E0[element->unk0_3];
        knot_list[2] = (f32) first_anim->unk2 / 64;
        knot_list[3] = ((first_anim->unk0_15 == 1) && (element->data_count >= 2)) ? (f32) (first_anim + 1)->unk2 / 64 : knot_list[2];
        return glspline_catmull_rom_interpolate((time - this->unk0)/(first_anim->unk0_13 - this->unk0), 4, knot_list);
    }

    last_anim = first_anim + element->data_count - 1;

    if ((s32) time >= (last_anim->unk0_13)) {
        knot_list[1] = (f32) last_anim->unk2 / 64;
        knot_list[0] =  ((last_anim->unk0_14 == 1) && (element->data_count >= 2)) ? (f32) (last_anim - 1)->unk2 / 64 : knot_list[1];
        knot_list[2] = knot_list[3] = knot_list[1];

        return glspline_catmull_rom_interpolate(time - last_anim->unk0_13, 4, knot_list);
    }

    i_anim = first_anim + 1;
    while (i_anim < last_anim) {
        i_anim = &first_anim[(last_anim - first_anim) / 2];
        if (i_anim->unk0_13 <= (s32) time) {
            first_anim = i_anim;
        } else {
            last_anim = i_anim;
        }

        i_anim = first_anim + 1;
    }
    
    knot_list[1] = (f32) first_anim->unk2 / 64;
    knot_list[2] = (f32) last_anim->unk2 / 64;
    temp_f12 = (time - first_anim->unk0_13) / (last_anim->unk0_13 - first_anim->unk0_13);

    if ((first_anim->unk0_14 == 0) && (last_anim->unk0_15 == 0)) {
        return knot_list[1] + ((knot_list[2] - knot_list[1]) * temp_f12);
    }
    
    knot_list[0] = ((first_anim->unk0_14 == 1) && ((first_anim - 1) >= &element->data[0])) ? (f32) (first_anim - 1)->unk2 / 64 : knot_list[1];
    knot_list[3] = ((last_anim->unk0_15 == 1) && ((last_anim + 1) < &element->data[element->data_count])) ? (f32) (last_anim + 1)->unk2 / 64 : knot_list[2];

    return glspline_catmull_rom_interpolate(temp_f12, 4, knot_list);
}

void func_8033AFB8(BoneTransformList *bone_transform_list, s32 bone_id, f32 arg2[3][3]){
    f32 sp18[4]; 
    func_80345CD4(sp18, arg2[0]);
    func_8033A8F0(bone_transform_list, bone_id, sp18);
    boneTransformList_setBoneScale(bone_transform_list, bone_id, arg2[1]);
    func_8033A968(bone_transform_list, bone_id, arg2[2]);
}
