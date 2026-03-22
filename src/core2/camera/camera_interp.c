#include <ultra64.h>
#include "functions.h"
#include "variables.h"


extern f32 func_8033A244(f32);
extern void *func_80255888(void *arg0); // [port] missing prototype — returns void*, implicit int truncates on 64-bit

typedef struct{
    BKModelBin *unk0;
    s32 unk4;
    f32 unk8;
}struct_7AF80_0;

typedef struct{
    BKSprite *unk0;
    BKSpriteDisplayData *unk4;
    s32 unk8;
    f32 unkC;
}struct_7AF80_1;

BKModelBin *propModelList_getModel(s32 arg0);

/* .data */
s32 D_8036B800 = 0;

/* .bss */
struct_7AF80_0 *sPropModelList; //prop models ???
struct_7AF80_1 *sPropSpriteList; //prop_sprites ???

BKSpriteDisplayData *propModelList_getSpriteDisplayList(s32 arg0);


void propModelList_drawModel(Gfx **gfx, Mtx **mtx, Vtx **vtx, f32 arg3[3], f32 arg4[3], f32 arg5, s32 arg6, Cube* arg7){
    BKModelBin * sp2C;
    
    sp2C = propModelList_getModel(arg6);
    func_8033A244(3700.0f);
    func_8033A28C(1);
    modelRender_setDepthMode(MODEL_RENDER_DEPTH_FULL);
    func_8033A1FC();
    modelRender_draw(gfx, mtx, arg3, arg4, arg5, NULL, sp2C);
}

void propModelList_drawSprite(Gfx **gfx, Mtx **mtx, Vtx **Vtx, f32 arg3[3], f32 arg4, s32 arg5, Cube *arg6, s32 arg7, s32 arg8, s32 arg9, s32 argA, s32 argB) {
    f32 sp2C[3];
    BKSpriteDisplayData *sp28;

    sp28 = propModelList_getSpriteDisplayList(arg5);
    sp2C[0] = arg4;
    sp2C[1] = arg4;
    sp2C[2] = arg4;
    func_80338338(0xFF - (arg7 * 0x10), 0xFF - (arg8 * 0x10), 0xFF - (arg9 * 0x10));
    if (func_80344C20(sp28) & 0xB00) {
        func_803382E4(0xB);
    } else {
        func_803382E4(0xE);
    }
    func_80335D30(gfx);
    func_80344138(sp28, argB, argA, arg3, sp2C, gfx, mtx);
    func_8033687C(gfx);
}

BKModelBin *propModelList_getModel(s32 arg0){
    if(sPropModelList[arg0].unk0 == NULL){
        sPropModelList[arg0].unk0 = assetcache_get(0x2d1 + arg0);
    }
    sPropModelList[arg0].unk4 = globalTimer_getTime();
    return sPropModelList[arg0].unk0;
}

BKModelBin *propModelList_getModelIfActive(s32 arg0){
    return sPropModelList[arg0].unk0;
}

BKSpriteDisplayData *propModelList_getSpriteDisplayList(s32 arg0)
{
    
    if (((struct_7AF80_1 *)((uintptr_t)sPropSpriteList + arg0*sizeof(struct_7AF80_1)))->unk0 == 0){
        ((struct_7AF80_1 *)((uintptr_t)sPropSpriteList + arg0*sizeof(struct_7AF80_1)))->unk0 = func_8033B6C4(arg0 + 0x572, &((struct_7AF80_1 *)((uintptr_t)sPropSpriteList + arg0*sizeof(struct_7AF80_1)))->unk4);
    }
    sPropSpriteList[arg0].unk8 = globalTimer_getTime();
    return sPropSpriteList[arg0].unk4;
}

BKSprite *propModelList_getSprite(s32 arg0){
    propModelList_getSpriteDisplayList(arg0);
    return sPropSpriteList[arg0].unk0;
}

f32 propModelList_getScale(Prop *arg0){
    if(arg0->unk8_1){
        ModelProp* ModelProp = &arg0->modelProp;
        return sPropModelList[arg0->spriteProp.unk0_31].unk8;
    }
    else{//L8030A65C
        SpriteProp *spriteProp = &arg0->spriteProp;
        return sPropSpriteList[spriteProp->unk0_31].unkC;
    }
}

void propModelList_setScale(Prop *arg0, f32 arg1){
    if(arg0->unk8_1){
        ModelProp* ModelProp = &arg0->modelProp;
        sPropModelList[arg0->spriteProp.unk0_31].unk8 = (f32)ModelProp->unkA*arg1/100.0f;
    }
    else{//L8030A65C
        SpriteProp *spriteProp = &arg0->spriteProp;
        sPropSpriteList[spriteProp->unk0_31].unkC = (f32)spriteProp->unk0_9*arg1/100.0f;
    }
}

void propModelList_free(void){//clear
    struct_7AF80_0* iPtr;
    struct_7AF80_1* jPtr;

    for(iPtr = sPropModelList; iPtr < &sPropModelList[0x2A2]; iPtr++){
        if(iPtr->unk0){
            assetcache_release(iPtr->unk0);
        }
    }
    for(jPtr = sPropSpriteList; jPtr < &sPropSpriteList[0x168]; jPtr++){
        if(jPtr->unk0){
            func_8033B338((void **)&jPtr->unk0, &jPtr->unk4); // [port]
        }
    }
    bk_free(sPropModelList);
    sPropModelList = NULL;
    bk_free(sPropSpriteList);
    sPropSpriteList = NULL;
}

void propModelList_init(void){//init
    struct_7AF80_0* iPtr;
    struct_7AF80_1* jPtr;

    sPropModelList = (struct_7AF80_0 *)bk_malloc(0x2A2 * sizeof(struct_7AF80_0));
    sPropSpriteList = (struct_7AF80_1 *)bk_malloc(0x168 * sizeof(struct_7AF80_1));
    D_8036B800 = 0;
    for(iPtr = sPropModelList; iPtr < &sPropModelList[0x2A2]; iPtr++){
        iPtr->unk0 = NULL;
        iPtr->unk8 = 0.0f;
    }
    for(jPtr = sPropSpriteList; jPtr < &sPropSpriteList[0x168]; jPtr++){
        jPtr->unk0 = NULL;
        jPtr->unkC = 0.0f;
    }
}

void propModelList_flush(s32 arg0) {
    static s32 D_8036B804 = 0;
    static s32 D_8036B808 = 0;
    s32 temp_s3;
    s32 var_s0;
    struct_7AF80_0 *sp3C;
    struct_7AF80_1 *temp_a0_2;

    temp_s3 = globalTimer_getTime() - func_80255B08(arg0);
    for(var_s0 = 0; (sPropModelList != NULL) && (var_s0 < ((arg0 == 1) ? 0x28 : 0x2A1)); var_s0++, D_8036B804 = (D_8036B804 >= 0x2A1)? 0: D_8036B804 + 1){
        sp3C = (struct_7AF80_0*)((uintptr_t)sPropModelList + sizeof(struct_7AF80_0)*D_8036B804);
        if ((sp3C->unk0 != 0) && ((sp3C->unk4 < temp_s3) || (arg0 == 3))){
            assetcache_release(sp3C->unk0);
            sp3C->unk0 = 0;
            if( (arg0 != 1) && (func_80254BC4(1))){
                return;
            }
        }
    }

    for(var_s0 = 0; (sPropSpriteList != NULL) && (var_s0 < ((arg0 == 1) ? 0x28 : 0x167)); var_s0++, D_8036B808 = (D_8036B808 >= 0x167)? 0: D_8036B808 + 1){
        temp_a0_2 = (struct_7AF80_1*)((uintptr_t)sPropSpriteList + sizeof(struct_7AF80_1)*D_8036B808);
        if ((temp_a0_2->unk0 != 0) && ((temp_a0_2->unk8 < temp_s3) || (arg0 == 3))){
            func_8033B338((void **)&temp_a0_2->unk0, &temp_a0_2->unk4); // [port]
            if( (arg0 != 1) && (func_80254BC4(1))){
                return;
            }
        }
    }
}

void propModelList_defrag(void) {
    // [port] N64 heap defrag — func_802546E4/func_80255888 assume bk_malloc HeapHeaders.
    // Assets now come from the resource manager; defragging them would corrupt memory.
    BKModelBin *temp_a0;
    s32 phi_s2;

    sPropSpriteList = (struct_7AF80_1 *) defrag(sPropSpriteList);
    sPropModelList = (struct_7AF80_0 *) defrag(sPropModelList);
#if 0
    if (!func_802559A0() && !func_80255AE4() && sPropModelList != NULL) {
        for(phi_s2 = 0x14; (phi_s2 != 0) && !func_80255AE4(); phi_s2--){
            D_8036B800++;
            if (D_8036B800 >= 0x2A2) {
                D_8036B800 = 0;
            }
            temp_a0 = sPropModelList[D_8036B800].unk0;
            if (temp_a0 != NULL && (func_802546E4(temp_a0) < 0x2AF8)) {
                sPropModelList[D_8036B800].unk0 = func_80255888(sPropModelList[D_8036B800].unk0);
            }
        }
    }
#endif
}

void propModelList_refresh(void) {
    s32 temp_lo;
    s32 temp_t7;
    struct_7AF80_1 *phi_s0;
    s32 phi_s2;
    struct_7AF80_0 *phi_s0_2;

    for(phi_s0 = sPropSpriteList; phi_s0 < sPropSpriteList + 360; phi_s0++){
        if (phi_s0->unk0 != NULL) {
            temp_t7 = phi_s0 - sPropSpriteList;
            func_8033B338((void **)&phi_s0->unk0, &phi_s0->unk4); // [port]
            // [port] original used hardcoded +4 byte offset for unk4 — wrong on 64-bit where pointers are 8 bytes
            phi_s0->unk0 = func_8033B6C4(temp_t7 + 0x572, &phi_s0->unk4);
        }
    }
    
    for(phi_s0_2 = sPropModelList; phi_s0_2 < sPropModelList + 674; phi_s0_2++){
        if(phi_s0_2->unk0 != NULL){
            temp_lo = phi_s0_2 - sPropModelList;
            assetcache_release(phi_s0_2->unk0);
            sPropModelList[temp_lo].unk0 = (BKModelBin *) assetcache_get(temp_lo + 0x2D1);

        }
    }
}
