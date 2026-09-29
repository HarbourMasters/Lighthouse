// BanjoDecomp: core2/propModelList.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#define PROP_MODEL_COUNT 0x2A2
#define PROP_SPRITE_COUNT 0x168


extern void *func_80255888(void *level);

typedef struct{
    BKModelBin *unk0;
    s32 timestamp;
    f32 scale;
} PropModelData;

typedef struct{
    BKSprite *unk0;
    BKSpriteDisplayData *display;
    s32 timestamp;
    f32 scale;
} PropSpriteData;

BKModelBin *propModelList_getModel(s32 level);

/* .data */
s32 D_8036B800 = 0;

/* .bss */
PropModelData *sPropModelList; //prop models ???
PropSpriteData *sPropSpriteList; //prop_sprites ???

BKSpriteDisplayData *propModelList_getSpriteDisplayList(s32 level);


void propModelList_drawModel(Gfx **gfx, Mtx **mtx, Vtx **vtx, f32 position[3], f32 arg4[3], f32 arg5, s32 modelId, Cube* rgb_remove_red){
    BKModelBin * sp2C;
    
    sp2C = propModelList_getModel(modelId);
    modelRender_func_8033A244(3700.0f);
    modelRender_func_8033A28C(1);
    modelRender_setDepthMode(MODEL_RENDER_DEPTH_FULL);
    modelRender_func_8033A1FC();
    modelRender_draw(gfx, mtx, position, arg4, arg5, NULL, sp2C);
}

void propModelList_drawSprite(Gfx **gfx, Mtx **mtx, Vtx **Vtx, f32 position[3], f32 arg4, s32 arg5, Cube *modelId, s32 rgb_remove_red, s32 rgb_remove_green, s32 rgb_remove_blue, s32 mirrored, s32 frame) {
    f32 sp2C[3];
    BKSpriteDisplayData *sprite;

    sprite = propModelList_getSpriteDisplayList(arg5);
    sp2C[0] = arg4;
    sp2C[1] = arg4;
    sp2C[2] = arg4;
    codeAEDA0_setPrimaryColorRGB(0xFF - (rgb_remove_red * 0x10), 0xFF - (rgb_remove_green * 0x10), 0xFF - (rgb_remove_blue * 0x10));
    if (codeBD100_getSpriteType(sprite) & 0xB00) {
        codeAEDA0_setSpriteDrawMode(0xB);
    } else {
        codeAEDA0_setSpriteDrawMode(0xE);
    }
    codeAEDA0_drawSprite(gfx);
    func_80344138(sprite, frame, mirrored, position, sp2C, gfx, mtx);
    codeAEDA0_postDrawSprite(gfx);
}

BKModelBin *propModelList_getModel(s32 level){
    if(sPropModelList[level].unk0 == NULL){
        sPropModelList[level].unk0 = assetcache_get(0x2d1 + level);
    }
    sPropModelList[level].timestamp = globalTimer_getTime();
    return sPropModelList[level].unk0;
}

BKModelBin *propModelList_getModelIfActive(s32 level){
    return sPropModelList[level].unk0;
}

BKSpriteDisplayData *propModelList_getSpriteDisplayList(s32 level)
{
    
    if (((PropSpriteData *)((uintptr_t)sPropSpriteList + level*sizeof(PropSpriteData)))->unk0 == 0){
        ((PropSpriteData *)((uintptr_t)sPropSpriteList + level*sizeof(PropSpriteData)))->unk0 = codeB3A80_getSprite(level + 0x572, &((PropSpriteData *)((uintptr_t)sPropSpriteList + level*sizeof(PropSpriteData)))->display);
    }
    sPropSpriteList[level].timestamp = globalTimer_getTime();
    return sPropSpriteList[level].display;
}

BKSprite *propModelList_getSprite(s32 level){
    propModelList_getSpriteDisplayList(level);
    return sPropSpriteList[level].unk0;
}

f32 propModelList_getScale(Prop *level){
    if(level->isModelProp){
        ModelProp* ModelProp = &level->modelProp;
        if (sPropModelList == NULL || level->spriteProp.spriteId >= 0x2A2) {
            return 0.0f;
        }
        return sPropModelList[level->spriteProp.spriteId].scale;
    }
    else{//L8030A65C
        SpriteProp *spriteProp = &level->spriteProp;
        if (sPropSpriteList == NULL || spriteProp->spriteId >= 0x168) {
            return 0.0f;
        }
        return sPropSpriteList[spriteProp->spriteId].scale;
    }
}

void propModelList_setScale(Prop *level, f32 arg1){
    if(level->isModelProp){
        ModelProp* ModelProp = &level->modelProp;
        if (sPropModelList == NULL || level->spriteProp.spriteId >= 0x2A2) {
            return;
        }
        sPropModelList[level->spriteProp.spriteId].scale = (f32)ModelProp->scale*arg1/100.0f;
    }
    else{//L8030A65C
        SpriteProp *spriteProp = &level->spriteProp;
        if (sPropSpriteList == NULL || spriteProp->spriteId >= 0x168) {
            return;
        }
        sPropSpriteList[spriteProp->spriteId].scale = (f32)spriteProp->scale*arg1/100.0f;
    }
}

void propModelList_free(void){//clear
    PropModelData* iPtr;
    PropSpriteData* jPtr;

    for(iPtr = sPropModelList; iPtr < &sPropModelList[PROP_MODEL_COUNT]; iPtr++){
        if(iPtr->unk0){
            assetcache_release(iPtr->unk0);
        }
    }
    for(jPtr = sPropSpriteList; jPtr < &sPropSpriteList[PROP_SPRITE_COUNT]; jPtr++){
        if(jPtr->unk0){
            codeB3A80_releaseSprite(&jPtr->unk0, &jPtr->display);
        }
    }
    bk_free(sPropModelList);
    sPropModelList = NULL;
    bk_free(sPropSpriteList);
    sPropSpriteList = NULL;
}

void propModelList_init(void){//init
    PropModelData* iPtr;
    PropSpriteData* jPtr;

    sPropModelList = (PropModelData *)bk_malloc(PROP_MODEL_COUNT * sizeof(PropModelData));
    sPropSpriteList = (PropSpriteData *)bk_malloc(PROP_SPRITE_COUNT * sizeof(PropSpriteData));
    D_8036B800 = 0;
    for(iPtr = sPropModelList; iPtr < &sPropModelList[PROP_MODEL_COUNT]; iPtr++){
        iPtr->unk0 = NULL;
        iPtr->scale = 0.0f;
    }
    for(jPtr = sPropSpriteList; jPtr < &sPropSpriteList[PROP_SPRITE_COUNT]; jPtr++){
        jPtr->unk0 = NULL;
        jPtr->scale = 0.0f;
    }
}

void propModelList_flush(s32 level) {
    static s32 D_8036B804 = 0;
    static s32 D_8036B808 = 0;
    s32 oldest_active_time;
    s32 var_s0;
    PropModelData *model_entry;
    PropSpriteData *sprite_entry;

    oldest_active_time = globalTimer_getTime() - func_80255B08(level);
    for(var_s0 = 0;
        (sPropModelList != NULL) && (var_s0 < ((level == 1) ? 0x28 : PROP_MODEL_COUNT - 1));
        var_s0++, D_8036B804 = (D_8036B804 >= PROP_MODEL_COUNT - 1)? 0: D_8036B804 + 1)
    {
        model_entry = (PropModelData*)((uintptr_t)sPropModelList + sizeof(PropModelData)*D_8036B804);
        if ((model_entry->unk0 != 0) && ((model_entry->timestamp < oldest_active_time) || (level == 3))){
            assetcache_release(model_entry->unk0);
            model_entry->unk0 = 0;
            if( (level != 1) && (func_80254BC4(1))){
                return;
            }
        }
    }

    for(var_s0 = 0;
        (sPropSpriteList != NULL) && (var_s0 < ((level == 1) ? 0x28 : PROP_SPRITE_COUNT - 1));
        var_s0++, D_8036B808 = (D_8036B808 >= PROP_SPRITE_COUNT - 1)? 0: D_8036B808 + 1)
    {
        sprite_entry = (PropSpriteData*)((uintptr_t)sPropSpriteList + sizeof(PropSpriteData)*D_8036B808);
        if ((sprite_entry->unk0 != 0) &&
            ((sprite_entry->timestamp < oldest_active_time) ||
            (level == 3)))
        {
            codeB3A80_releaseSprite(&sprite_entry->unk0, &sprite_entry->display);
            if( (level != 1) && (func_80254BC4(1))){
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

    sPropSpriteList = (PropSpriteData *) defrag(sPropSpriteList);
    sPropModelList = (PropModelData *) defrag(sPropModelList);
#if 0
    if (!func_802559A0() && !func_80255AE4() && sPropModelList != NULL) {
        for(phi_s2 = 0x14; (phi_s2 != 0) && !func_80255AE4(); phi_s2--){
            D_8036B800++;
            if (D_8036B800 >= PROP_MODEL_COUNT) {
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
    s32 model_list_index;
    s32 temp_t7;
    PropSpriteData *sprite_entry;
    s32 phi_s2;
    PropModelData *model_entry;

    for(sprite_entry = sPropSpriteList; sprite_entry < sPropSpriteList + 360; sprite_entry++){
        if (sprite_entry->unk0 != NULL) {
            temp_t7 = sprite_entry - sPropSpriteList;
            codeB3A80_releaseSprite(&sprite_entry->unk0, &sprite_entry->display);
            // [port] original used hardcoded +4 byte offset for unk4 — wrong on 64-bit where pointers are 8 bytes
            sprite_entry->unk0 = codeB3A80_getSprite(temp_t7 + 0x572, &sprite_entry->display);
        }
    }
    
    for(model_entry = sPropModelList; model_entry < sPropModelList + 674; model_entry++){
        if(model_entry->unk0 != NULL){
            model_list_index = model_entry - sPropModelList;
            assetcache_release(model_entry->unk0);
            sPropModelList[model_list_index].unk0 = (BKModelBin *) assetcache_get(model_list_index + 0x2D1);

        }
    }
}
