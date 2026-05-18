#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include <bk_math.h>

#define CORE2_C31A0_VEC_COUNT 0x21

void vec3fArray_clearValues(Vec3fArray *this) {
    f32(*iPtr)[3];

    for (iPtr = this->begin_ptr; iPtr < this->end_ptr; iPtr++) {
        (*iPtr)[0] = (*iPtr)[1] = (*iPtr)[2] = 0.0f;
    }
}

void vec3fArray_get_vec3f(Vec3fArray *this, s32 indx, f32 dst[3]) {
    TUPLE_COPY(dst, this->begin_ptr[indx])
}

void vec3fArray_get_vec3i(Vec3fArray *this, s32 indx, s32 dst[3]) {
    TUPLE_COPY(dst, this->begin_ptr[indx])
}

void vec3fArray_getDirectionVector(Vec3fArray *this, s32 indx1, s32 indx2, f32 dst[3]) {
    TUPLE_DIFF_COPY(dst, this->begin_ptr[indx2], this->begin_ptr[indx1])
    ml_vec3f_normalize(dst);
}

void vec3fArray_free(Vec3fArray *this) {
    bk_free(this);
}

Vec3fArray *vec3fArray_new(void) {
    Vec3fArray *this = (Vec3fArray *) bk_malloc(sizeof(Vec3fArray) + sizeof(f32[3]) * CORE2_C31A0_VEC_COUNT);
    this->begin_ptr = (f32(*)[3])((uintptr_t) this + sizeof(Vec3fArray));
    this->end_ptr = (f32(*)[3])((uintptr_t) this->begin_ptr + sizeof(f32[3]) * CORE2_C31A0_VEC_COUNT);
    vec3fArray_clearValues(this);
    return this;
}

void vec3fArray_set_vec3f(Vec3fArray *this, s32 indx, f32 arg2[3]) {
    TUPLE_COPY(this->begin_ptr[indx], arg2)
}

Vec3fArray *vec3fArray_defrag(Vec3fArray *this) {
    if (this) {
        this = (Vec3fArray *) defrag(this);
        this->begin_ptr = (f32(*)[3])(((uintptr_t) this + sizeof(Vec3fArray)));
        this->end_ptr = (f32(*)[3])((uintptr_t) this->begin_ptr + sizeof(f32[3]) * CORE2_C31A0_VEC_COUNT);
    }

    return this;
}
