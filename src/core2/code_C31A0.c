#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#define CORE2_C31A0_VEC_COUNT 0x21

void func_8034A130(struct5Bs *vthis) {
    f32(*iPtr)[3];

    for (iPtr = vthis->unk0; iPtr < vthis->unk4; iPtr++) {
        (*iPtr)[0] = (*iPtr)[1] = (*iPtr)[2] = 0.0f;
    }
}

void func_8034A174(struct5Bs *vthis, s32 indx, f32 dst[3]) {
    TUPLE_COPY(dst, vthis->unk0[indx])
}

void func_8034A1B4(struct5Bs *vthis, s32 indx, s32 dst[3]) {
    TUPLE_COPY(dst, vthis->unk0[indx])
}

void func_8034A214(struct5Bs *vthis, s32 indx1, s32 indx2, f32 dst[3]) {
    TUPLE_DIFF_COPY(dst, vthis->unk0[indx2], vthis->unk0[indx1])
    ml_vec3f_normalize(dst);
}

void func_8034A2A8(struct5Bs *vthis) {
    bk_free(vthis);
}

struct5Bs *func_8034A2C8(void) {
    struct5Bs *vthis = (struct5Bs *) heap_malloc(sizeof(struct5Bs) + sizeof(f32[3]) * CORE2_C31A0_VEC_COUNT);
    vthis->unk0 = (f32(*)[3])((s32) vthis + sizeof(struct5Bs));
    vthis->unk4 = (f32(*)[3])((s32) vthis->unk0 + sizeof(f32[3]) * CORE2_C31A0_VEC_COUNT);
    func_8034A130(vthis);
    return vthis;
}

void func_8034A308(struct5Bs *vthis, s32 indx, f32 arg2[3]) {
    TUPLE_COPY(vthis->unk0[indx], arg2)
}

struct5Bs *func_8034A348(struct5Bs *vthis) {
    if (vthis) {
        vthis = (struct5Bs *) defrag(vthis);
        vthis->unk0 = (f32(*)[3])(((s32) vthis + sizeof(struct5Bs)));
        vthis->unk4 = (f32(*)[3])((s32) vthis->unk0 + sizeof(f32[3]) * CORE2_C31A0_VEC_COUNT);
    }

    return vthis;
}
