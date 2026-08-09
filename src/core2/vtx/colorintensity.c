// BanjoDecomp: core2/code_C8230.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "model.h"


void func_8034F1C0(s32 arg0, BKModelVtxRef *ref, Vtx *dst, void *arg3) {
    Struct75s *data = (Struct75s *)arg3;
    s32 temp_f4;
    s32 i;

    for(i = 0; i < 3; i++){
        temp_f4 = (s32) (ref->v.v.cn[i] + data->unk0 * 50.0f);
        dst->v.cn[i] = (temp_f4 < 0xFF) ? temp_f4 : 0xFF;
    }
}

void func_8034F248(Struct75s arg0){}

void func_8034F250(Struct75s *arg0, s32 arg1, s32 arg2, s32 arg3){
    arg0->unk0 = 0.0f;
}

void func_8034F268(Struct75s *arg0, BKModel *arg1, s32 arg2) {
    void *temp_v0;

    temp_v0 = func_8034C448(arg2 - 0x64);
    if (temp_v0 != NULL) {
        arg0->unk0 = func_8034F560(temp_v0) / 255.0;
    } else {
        arg0->unk0 = 0.0f;
    }
    model_transformMesh(arg1, arg2, func_8034F1C0, (void *) arg0);
}
