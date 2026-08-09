// BanjoDecomp: core2/code_630D0.c
#include <ultra64.h>
#include "functions.h"
#include "port/ShipUtils.h" // BK_LOG_*, port_shapeControllerInput
#include "variables.h"
#include "animation.h"

void *defrag(void *);

void animMtxList_setBoneless(AnimMtxList **this_ptr, BKAnimationList *anim_list){
    AnimMtxList * this;
    MtxF *end_ptr;
    MtxF *i_ptr;

    this = *this_ptr;
    if(this->capacity < anim_list->count){
        this = (AnimMtxList *)bk_realloc(this, anim_list->count * sizeof(Mtx) + sizeof(AnimMtxList));
        this->capacity = anim_list->count;
        (*this_ptr) = this;
    }

    this->size = anim_list->count;
    end_ptr = (MtxF *)(this->size*sizeof(MtxF) + (uintptr_t)this +sizeof(AnimMtxList));
    mlMtxIdent();
    for(i_ptr = this->data; i_ptr < end_ptr; i_ptr++){
        mlMtxGet(i_ptr);
    }
}

MtxF *animMtxList_get(AnimMtxList *this, s32 arg1){
    if (arg1 == -1){
        return &this->default_matrix;
    }
    // [port] bounds check - on N64 this never happens but corrupt data can cause OOB
    if (arg1 < 0 || arg1 >= this->size) {
        BK_LOG_WARN("animMtxList_get: OOB index %d (size=%d)", arg1, this->size);
        return &this->default_matrix; // fallback to identity
    }
    return &this->data[arg1];

}

void animMtxList_free(AnimMtxList *this){
    bk_free(this);
}

AnimMtxList *animMtxList_new(void){
    AnimMtxList *this = bk_malloc(sizeof(AnimMtxList));
    this->size = 0;
    this->capacity = 0;
    mlMtxIdent();
    mlMtxGet(&this->default_matrix);
    return this;
}

s32 animMtxList_getLength(AnimMtxList* this){
    if(this)
        return this->size;
    return 1;
}

void func_8033A5B8(BoneTransformList *this, s32 bone_id, f32 arg2[4], f32 scale[3], f32 arg4[3]);

void animMtxList_setBoned(AnimMtxList **this_ptr, BKAnimationList *anim_list, BoneTransformList *arg2){
    AnimMtxList * this;
    MtxF *start_ptr;
    MtxF *end_ptr;
    MtxF *i_ptr;
    BKAnimation *s0;
    f32 tmp_f0;
    f32 sp88[3][3];
    f32 sp74[4];
    f32 sp68[3];
    f32 sp5C[3];

    //resize animation matrices
    this = *this_ptr;
    if(this->capacity < anim_list->count){
        this = (AnimMtxList *)bk_realloc(this, anim_list->count * sizeof(Mtx) + sizeof(AnimMtxList));
        this->capacity = anim_list->count;
        (*this_ptr) = this;
    }

    this->size = anim_list->count;
    start_ptr = this->data;
    end_ptr = &start_ptr[this->size];
    s0 = anim_list->animations;

    for(i_ptr = start_ptr; i_ptr < end_ptr; s0++, i_ptr++){
        func_8033A5B8(arg2, s0->bone_id, sp74, sp68, sp5C);

        if(s0->mtx_id == -1)
            mlMtxIdent();
        else if(s0->mtx_id + 1 != i_ptr - start_ptr)
            mlMtxSet(&start_ptr[s0->mtx_id]);
        tmp_f0 = anim_list->unk0;
        mlMtxTranslate(s0->translation[0] + tmp_f0*sp5C[0], s0->translation[1] + tmp_f0*sp5C[1], s0->translation[2] + tmp_f0*sp5C[2] );

        if(!vec4f_isZero(sp74)){
            func_80345274(sp74, sp88);
            func_802515D4(sp88);
        }

        mlMtxScale_xyz(sp68[0], sp68[1], sp68[2]);
        mlMtxTranslate(-s0->translation[0], -s0->translation[1], -s0->translation[2]);
        mlMtxGet(i_ptr);

    }
}

AnimMtxList *animMtxList_defrag(AnimMtxList *this){
    return defrag(this);
}
