// BanjoDecomp: core2/code_A5BC0.c
#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include "enums.h"
#include "actor.h"

#include <core2/file.h>
#include "port/Patches/Patches.h"
#include "port/Interpolation/FrameInterpolation.h"

extern int ResourceMgr_IsModelAsset(uint32_t assetId);

#define AssetCacheSize 0x3D5

// [port] Expand Marker Pool for Rando Allocation
#define MARKER_POOL_SIZE ACTOR_POOL_SIZE
#define MARKER_BITMAP_BYTES (MARKER_POOL_SIZE / 8)

extern bool func_802E74A0(f32[3], f32, f32[3], f32[3]);
extern BKCollisionTriangle *func_802E9118(BKCollisionList * collision_list, BKVertexList *vtx_list, f32 arg2[3], f32 arg3[3], f32 arg4, f32 arg5[3], f32 arg6[3], f32 arg7, f32 arg8[3], s32 arg9, s32 argA);
extern f32 vtxList_getGlobalNorm(BKVertexList *);
extern void spawnQueue_func_802C39D4(void);
extern bool func_80340020(Struct83s *, f32[3], f32[3], f32, f32 *, BKVertexList *, f32[3], f32[3]);
extern void func_80340200(Struct83s *, f32[3], f32[3], f32, f32[3], s16[3], BKVertexList *, f32[3]);
extern BKCollisionTriangle *func_802E9DD8(BKCollisionList *collisionList, BKVertexList *vtxList, f32 arg2[3], f32 *arg3, f32 arg4, f32 arg5[3], f32 arg6, f32 arg7[3], s32 arg8);
extern s32 bkmodelunk14list_func_802EBAE0(BKModelUnk14List *arg0, f32 position[3], f32 rotation[3], f32 scale, f32 arg4[3], AnimMtxList *arg5, f32 arg6[3], f32 arg7, f32 arg8[3]);
extern BKCollisionTriangle *collisionList_func_802E805C(BKCollisionList *, BKVertexList *, f32[3], f32[3], f32, f32[3], f32[3], f32[3], u32);
extern NodeProp *func_803080C8(s32 arg0);
extern Cube *func_80308224(void);
extern Cube *func_8030364C(void);
extern Cube *cubeList_GetCubeAtPosition_s32(s32 position[3]);

extern f32 propModelList_getScale(Prop *);
extern void propModelList_setScale(Prop *, f32);

Prop *func_80303F7C(s32, f32, s32, s32);
s32 func_803058C0(f32);
void func_80305CD8(s32, s32);
void code_A5BC0_initCubePropActorProp(Cube*);
ActorMarker * func_80332A60(void);
extern void func_8032F3D4(s32 [3], ActorMarker *, s32);
extern void propModelList_drawSprite(Gfx **, Mtx **, Vtx **, f32[3], f32, s32, Cube*,s32 ,s32, s32, s32, s32);  
extern void propModelList_drawModel(Gfx **, Mtx **, Vtx **, f32[3], f32[3], f32, s32, Cube*);
s32 func_8032D9C0(Cube*, Prop*);
void func_8032F21C(Cube *cube, s32 position[3], ActorMarker *marker, bool arg3);
void func_80332B2C(ActorMarker * arg0);
BKSprite *propModelList_getSprite(s32 arg0);

typedef union{
    struct{
        u32 pad31: 27;
        u32 unk4: 1;
        u32 pad3: 1;
        u32 unk2: 1;
        u32 unk1: 1;
        u32 unk0: 1;
    };
    u32 word;
} tmp_bitfield;

typedef bool( *Method_Core2_A5BC0)(NodeProp *, s32);

s32 func_80330974(ActorMarker *marker, f32 arg1[3], f32 arg2, f32 arg3[3], s32 arg4_unused);
BKCollisionTriangle *func_80320DB0(f32[3], f32, f32[3], u32);
BKModelBin *func_80330DE4(ActorMarker *this);

extern ActorProp * func_80320EB0(ActorMarker *, f32, s32);
extern int func_80320ED8(ActorMarker *, f32, s32);
f32 func_8033229C(ActorMarker *marker);
s32 func_803327A8(s32 arg0);
void func_8032CD60(Prop *);
f32 modelRender_func_8033A244(f32);
void func_8032F64C(f32 *pos, ActorMarker * marker);
BKSprite *func_80330F50(ActorMarker * marker);

/* .data */
s32 D_8036E7B0 = 0;
u8 pad_8036E7B4[0xC] = {0}; //PROBABLY INDICATES FILE BREAK

ModelCache *modelCache = NULL; //D_8036E7C0 //model pointer array pointer
u8 *D_8036E7C4 = NULL;
ActorMarker *D_8036E7C8 = NULL;

enum Prop1Category {
    PROP_1_CATEGORY_0_UNK,
    PROP_1_CATEGORY_1_UNK,
    PROP_1_CATEGORY_2_WARP_OR_TRIGGER,
    PROP_1_CATEGORY_3_CAMERA_CONTROLLER,
    PROP_1_CATEGORY_4_UNK,
    PROP_1_CATEGORY_5_UNK,
    PROP_1_CATEGORY_6_ACTOR,
    PROP_1_CATEGORY_7_ENEMY_BOUNDARY,
    PROP_1_CATEGORY_8_PATH,
    PROP_1_CATEGORY_9_CAMERA_TRIGGER,
    PROP_1_CATEGORY_A_FLAG
};

#define CUBE_PROP_1_INDICATOR                     0x0A
#define CUBE_PROP_1_OTHER_INDICATOR               0x06
#define CUBE_DIVIDER_INDICATOR                    0x01
#define CUBE_PROP_1_LIST_START_INDICATOR          0x0B
#define CUBE_PROP_1_OTHER_LIST_START_INDICATOR    0x07
#define CUBE_PROP_1_LIST_END_INDICATOR            0x08
#define CUBE_PROP_2_LIST_START_INDICATOR          0x09

/* .bss */
s32 D_803833F0[3];
s32 D_803833FC;
s32 D_80383400;
Cube *D_80383404;
void *D_80383408;
s32 D_8038340C;

//BREAK???
f32 D_80383410[3];
ActorMarker *D_8038341C;
BKCollisionTriangle *D_80383420;
u8  D_80383428[MARKER_BITMAP_BYTES];
s32 D_80383444;
int D_80383448;
s32 D_80383450[CUBE_SORT_SCRATCH_SIZE];
bk_vector(ActorMarker *) *D_80383550;
bk_vector(ActorMarker *) *D_80383554;
Method_Core2_A5BC0 D_80383558;
s32 D_8038355C;

/* .code */
// This function sorts a cube's props based on distance
void __cube_sort(Cube *cube, bool global) {
    s32 ref_position[3];
    Prop *var_v1;
    Prop *start_prop;
    s32 temp_a2;
    Prop *var_t1;
    Prop * var_a3;
    Prop * var_t0;
    s32 i;
    Prop *new_var;

    // [port] Sorting is only draw order; skip rather than overrun the scratch.
    if (cube->prop2Cnt > CUBE_SORT_SCRATCH_SIZE) {
        return;
    }

    if (cube->prop2Cnt >= 2) {
        if (global == 0) {
            viewport_getPosition_vec3w(ref_position); //distance from viewport
        } else {
            ref_position[0] = 0;
            ref_position[1] = 0;
            ref_position[2] = 0;
        }

        //calculate prop distances
        new_var = var_v1 = cube->prop2Ptr;
        for(i = 0; i < cube->prop2Cnt; var_v1++, i++){
            D_80383450[i] =  (var_v1->actorProp.x - ref_position[0])*(var_v1->actorProp.x - ref_position[0]) 
                               +  (var_v1->actorProp.y - ref_position[1])* (var_v1->actorProp.y - ref_position[1])
                               +  (var_v1->actorProp.z - ref_position[2])* (var_v1->actorProp.z - ref_position[2]);
        }

        //sort prop list
        // [port] Cancelled by PerfPatches.cpp, which sorts the same distances
        // with a stable insertion sort instead of this exchange sort.
        if (EventSystem_Should(VB_CUBE_PROP_SORT, true, cube)) {
        start_prop = cube->prop2Ptr;
        var_t0 = cube->prop2Ptr + (cube->prop2Cnt - 1);   
        do {
            new_var = start_prop;
            var_t1 = var_t0;
            start_prop = NULL;
            var_v1 = new_var;
            i = (new_var - cube->prop2Ptr);
            while(var_v1 < var_t1){
                if(D_80383450[i] < D_80383450[i + 1]){
                    var_t0 = var_v1 + 1;
                    if (start_prop != 0) {
                        var_t0 = var_v1;
                    } else {
                        start_prop = (var_v1 == cube->prop2Ptr) ? var_v1 : var_v1 - 1;
                    }
                    
                    //swap_distances
                    temp_a2 = D_80383450[i];
                    D_80383450[i] = D_80383450[i + 1];
                    D_80383450[i + 1] = temp_a2;

                    //swap_props
                    // [port] N64 swapped 3 s32 words (12 bytes = sizeof(Prop) on N64).
                    // On 64-bit, sizeof(Prop) = 16 due to pointer-sized marker field.
                    // Use a full struct swap instead of raw word swap.
                    {
                        Prop tmp_prop;
                        memcpy(&tmp_prop, &var_v1[0], sizeof(Prop));
                        memcpy(&var_v1[0], &var_v1[1], sizeof(Prop));
                        memcpy(&var_v1[1], &tmp_prop, sizeof(Prop));
                    }
                }

                var_v1++;
                i++;
            }
        } while (start_prop != NULL);
        }
        code_A5BC0_initCubePropActorProp(cube);
    }
}

void func_8032CD60(Prop *prop) {
    BKSprite *var_v0;
    s32 sp48;
    s32 sp44;
    s32 sp40;
    s32 sp3C;
    s32 sp38;
    bool sp34;
    s32 sp30;
    s32 sp2C;
    bool var_t5;
    s32 var_v1;

    // [port] Use struct access instead of raw BE bit manipulation for cross-platform.
    // Original: ((u16*)prop)[5] & 1 → markerFlag, ((u16*)prop)[0] >> 4 → unk0_31
    if (prop->markerFlag && prop->actorProp.marker == NULL) return;
    var_v0 = prop->markerFlag ? func_80330F50(prop->actorProp.marker)
           : propModelList_getSprite(prop->spriteProp.spriteId);
    if ((var_v0 != NULL) && ((var_v0->unkC.bit27 != 0))) {
       sp48 = var_v0->unkC.bit31;
       sp44 = var_v0->unkC.bit27;
       sp40 = var_v0->unkC.bit24;
       sp3C = var_v0->unkC.bit22;
        sp38 =  var_v0->frameCnt;
        // [port] Guard: speed=0 or frameCnt<=0 would cause division by zero.
        // On N64 this was UB too, so these sprites were never animated.
        if (sp48 == 0 || sp38 <= 0) return;
        sp34 = (sp44 == 1) || (sp44 == 2U);
        sp30 = (sp44 == 3) ? sp38 : (sp38 - sp34)*2;
        if (sp30 == 0) return; // [port] prevent div-by-zero

        // [port] Replace raw BE bit ops with struct access:
        // ((u16*)prop)[5] bits 10:6 → spriteProp.unk8_10
        // ((u16*)prop)[1] bit 1 → spriteProp.unk0_1
        sp2C = (s32)((prop->spriteProp.unk8_10 * sp30) / 32);
        var_v1 = (((globalTimer_getTime() % (sp30 * sp48)) / sp48) + sp2C) % sp30; // [port] globalTimer_getTime(void) — MIPS ignored extra args, removed for C compliance
        var_t5 = 0;
        switch (sp40) {                          /* irregular */
            default:
                sp40 = 0;
                break;
            case 1:
                sp40 = (prop->spriteProp.unk8_10 & 2) ? 1 : 0;
                break;
            case 2:
                sp40 = 1;
                break;
            case 3:
                sp40 = prop->spriteProp.isMirrored;
                break;
        }

        switch(sp44){
            case 4:
                var_t5 = sp38 <= var_v1;
            case 1:
                sp3C = sp38 <= var_v1;
                break;
            case 2:
                
                var_t5 = sp38 <= var_v1;
            
            default:
                
                // [port] Replace raw BE bit ops with struct access
                switch (sp3C) {                      /* switch 1; irregular */
                    case 1:                                 /* switch 1 */
                         sp3C = (prop->spriteProp.unk8_10 & 1) ? 1 : 0;
                        break;
                    case 2:                                 /* switch 1 */
                        sp3C = 1;
                        break;
                    default:                                /* switch 1 */
                        if (prop->markerFlag) {
                            sp3C = prop->unk8_5;
                        } else {
                            sp3C = prop->spriteProp.isMirrored;
                        }
                      if (1) break;
                }
                break;

        }

        if ((sp3C ^ sp40 ^ var_t5)) {
            var_v1 = sp30 - var_v1;
        }

        var_v1 += (sp34) ? sp40 : -sp40;
        var_v1 = (var_v1 < 0) ? var_v1 +sp38 : var_v1 % sp38;
        prop->spriteProp.frame = var_v1;
        if (prop->markerFlag) {
            prop->spriteProp.unk8_5 = sp3C;
        }
        else{
            prop->spriteProp.isMirrored = sp3C;
        }
    }
}

void cube_sortAbsolute(Cube *cube){
    if(cube->prop2Cnt >= 2)
        __cube_sort(cube, 1);
}

void cube_sortRelative(Cube *cube){
    if(cube->prop2Cnt >= 2)
        __cube_sort(cube, 0);
}

static void __marker_draw(ActorMarker *this, Gfx **gfx, Mtx **mtx, Vtx **vtx){
    Actor *actor;
    u32 draw_dist;
    f32 draw_dist_f;
    f32 percentage;

    FrameInterpolation_RecordOpenChildHash3("marker", (uintptr_t)this, (uintptr_t)this->actrArrayIdx, 0);

    if(!this->unk3E_0){
        this->drawFunc(this, gfx, mtx, vtx);
        FrameInterpolation_RecordCloseChild();
        return;
    }
    actor =  marker_getActor(this);
    modelRender_func_8033A28C(actor->unk58_2);
    if( actor->unk58_2 && !this->unk40_23 && !this->unk40_21 && !D_8036E7B0){
        modelRender_func_8033A244(3700.0f);
    }

    if(actor->unk124_7 && !actor->despawn_flag && actor->unk58_0){
        draw_dist = actor->actor_info->draw_distance;
        if(draw_dist != 0){
            percentage = (f32)draw_dist*(1/(f64)0x400);
        }
        else if(this->unk40_21){
            percentage = 2.0f;
        }
        else{
            percentage = 1.0f;
        }
        modelRender_func_8033A280(percentage);
        this->drawFunc(this, gfx, mtx, vtx);
    }//L8032D300
    modelRender_func_8033A244(30000.0f);
    modelRender_func_8033A280(1.0f);

    FrameInterpolation_RecordCloseChild();
}

void func_8032D330(){
    D_80383550 = bk_vector_new(sizeof(ActorMarker *),2);
    D_80383554 = bk_vector_new(sizeof(ActorMarker *),2);
}

void func_8032D36C(void){
    bk_vector_free(D_80383550);
    D_80383550 = NULL;
    bk_vector_free(D_80383554);
    D_80383554 = NULL;
}

void func_8032D3A8(void){
    bk_vector_clear(D_80383550);
    bk_vector_clear(D_80383554);
}

void core2_A5BC0_drawUnknownMarkers(Gfx **gfx, Mtx **mtx, Vtx **vtx) { // further investigation needed what's drawn here
    int i;

    for (i = 0; i < bk_vector_size(D_80383550); i++) {
       __marker_draw(*(ActorMarker **) bk_vector_at(D_80383550, i), gfx, mtx, vtx);
    }
}

void core2_A5BC0_drawScreenOverlayMarkers(Gfx **gfx, Mtx **mtx, Vtx **vtx) {
    int i;

    for (i = 0; i < bk_vector_size(D_80383554); i++) {
       __marker_draw(*(ActorMarker **) bk_vector_at(D_80383554, i), gfx, mtx, vtx);
    }
}

void func_8032D510(Cube *cube, Gfx **gfx, Mtx **mtx, Vtx **vtx){
    Prop *iProp;
    int i;
    f32 sp94[3];
    f32 sp88[3];
    tmp_bitfield tmp_v0;
    int iOffset;
    ActorMarker **markerPtr;

    if(cube->prop2Cnt == 0 ) return;

    __cube_sort(cube, 0);
    iOffset = 0;
    for(i = 0; i < cube->prop2Cnt; i++){//L8032D5A0

        iProp = &cube->prop2Ptr[i];

        if(!iProp->unk8_4){

        }else{
            // [port] Original condition: if(!iProp->unk8_1) — only animates sprite props.
            // Actor/marker props with sprite assets (e.g., mumbo tokens) can have unk8_1=1
            // in the N64 setup data but still need sprite frame animation.
            // Call func_8032CD60 for sprites OR for actor sprites with sprite assets.
            if(!iProp->unk8_1
                || (iProp->markerFlag && iProp->actorProp.marker != NULL
                    && iProp->actorProp.marker->modelId != 0
                    && !ResourceMgr_IsModelAsset(iProp->actorProp.marker->modelId))){
                func_8032CD60(iProp);
            }
            if(iProp->markerFlag){//actorProp;
                // [port] marker is NULL until actor spawns
                if(iProp->actorProp.marker != NULL){
                    // [port] Fire one tick event per static prop for port-side features (e.g. nametags).
                    f32 propPos[3] = { (f32)iProp->actorProp.x, (f32)iProp->actorProp.y, (f32)iProp->actorProp.z };
                    CALL_EVENT(OnPropTick, iProp->actorProp.marker, propPos);
                    if(iProp->actorProp.marker->unk40_22){
                        markerPtr = (ActorMarker **)bk_vector_pushBackNew(&D_80383550);
                        *markerPtr = iProp->actorProp.marker;
                    }
                    else if(iProp->actorProp.marker->unk40_19){
                        markerPtr = (ActorMarker **)bk_vector_pushBackNew(&D_80383554);
                        *markerPtr = iProp->actorProp.marker;
                    }
                    else{
                        __marker_draw(iProp->actorProp.marker, gfx, mtx, vtx);
                    }//L8032D62C
                }
            }
            else{//L8032D640
                sp94[0] = (f32)iProp->modelProp.unk4[0];
                sp94[1] = (f32)iProp->modelProp.unk4[1];
                sp94[2] = (f32)iProp->modelProp.unk4[2];
                if(iProp->unk8_1){
                    // [port] __cube_sort reorders prop2Ptr per frame, so
                    // slot index is useless for pairing. Hash (modelId, pos)
                    // — those are fixed by the map's prop data.
                    FrameInterpolation_RecordOpenChildHash3("prop_model",
                        iProp->modelProp.modelId,
                        ((uint64_t)(u16)iProp->modelProp.unk4[0] << 32) | (u16)iProp->modelProp.unk4[1],
                        (u16)iProp->modelProp.unk4[2]);
                    sp88[0] = 0.0f;
                    sp88[1] = (f32)((s32)iProp->modelProp.yaw*2);
                    sp88[2] = (f32)((s32)iProp->modelProp.roll*2);
                    propModelList_drawModel(gfx, mtx, vtx,
                        sp94, sp88, (f32)iProp->modelProp.scale/100.0,
                        iProp->modelProp.modelId, cube
                    );
                    FrameInterpolation_RecordCloseChild();
                }
                else{//L8032D72C
                    // [port] Same story as prop_model above.
                    FrameInterpolation_RecordOpenChildHash3("prop_sprite",
                        iProp->spriteProp.spriteId,
                        ((uint64_t)(u16)iProp->spriteProp.unk4[0] << 32) | (u16)iProp->spriteProp.unk4[1],
                        (u16)iProp->spriteProp.unk4[2]);
                    // [port] Fire one tick event per static sprite prop. Asset id
                    // is the raw sprite index offset (+ 0x572 is the asset base).
                    CALL_EVENT(OnSpritePropTick, (s32)iProp->spriteProp.spriteId + 0x572, sp94);
                    propModelList_drawSprite( gfx, mtx, vtx,
                        sp94, (f32)iProp->spriteProp.scale/100.0, iProp->spriteProp.spriteId, cube,
                        iProp->spriteProp.rgb_remove_red, iProp->spriteProp.rgb_remove_green, iProp->spriteProp.rgb_remove_blue,
                        iProp->spriteProp.isMirrored, iProp->spriteProp.frame
                    );
                    FrameInterpolation_RecordCloseChild();
                }
            }//L8032D7C4
        }
    }//L8032D7D4
}

Prop *__codeA5BC0_initProp2Ptr(Cube *cube) {
    Prop *sp1C;

    if (cube->prop2Ptr != NULL) {
        cube->prop2Cnt++;
        cube->prop2Ptr = bk_realloc(cube->prop2Ptr, cube->prop2Cnt * sizeof(Prop));
    } else {
        cube->prop2Cnt = 1;
        cube->prop2Ptr = bk_malloc(sizeof(Prop));
    }
    // port - Rando manipulation of Props leads to rare errors with prop2Ptr, this ensures prop2Ptr is allocated each time if NULL
    if (cube->prop2Ptr == NULL) {
        cube->prop2Ptr = bk_malloc(sizeof(Prop));
        sp1C = cube->prop2Ptr;
    }
    else {
        sp1C = &cube->prop2Ptr[cube->prop2Cnt - 1];
    }
    sp1C->markerFlag = false;
    code_A5BC0_initCubePropActorProp(cube);
    return sp1C;
}

NodeProp *__codeA5BC0_pad_func_8032D8F0(Cube *cube) {
    if (cube->prop1Ptr != 0) {
        cube->prop1Cnt++;
        cube->prop1Ptr = bk_realloc(cube->prop1Ptr, cube->prop1Cnt * sizeof(NodeProp));
    } else {
        cube->prop1Cnt = 1;
        cube->prop1Ptr = bk_malloc(sizeof(NodeProp));
    }
    return &cube->prop1Ptr[cube->prop1Cnt - 1];
}

//cube_removeProp
s32 func_8032D9C0(Cube *cube, Prop* prop){
    s32 sp24;
    s32 tmp;

    sp24 = 0;
    if(cube != NULL && cube->prop2Cnt != 0 && prop != NULL && cube->prop2Ptr != NULL){
        ptrdiff_t index = prop - cube->prop2Ptr;
        // [port] A stale propPtr makes elementsAfter enormous; drop the removal instead.
        if (index < 0 || index >= (ptrdiff_t)cube->prop2Cnt) {
            port_warnPropNotInCube((s32)index, (s32)cube->prop2Cnt);
            return sp24;
        }

        sp24 = prop->unk8_1;
        if(func_80305D14()){
            func_80305CD8(func_803058C0(prop->unk4[1]), -1);
        }

        // port - Rando manipulation of prop2Ptr results in mismatched data, this corrects that.
        ptrdiff_t elementsAfter = (cube->prop2Cnt - 1) - index;
        if (elementsAfter > 0) {
            memmove(prop, prop + 1, elementsAfter * sizeof(Prop));
        }
        cube->prop2Cnt--;
        if(cube->prop2Cnt){
            cube->prop2Ptr = bk_realloc(cube->prop2Ptr, cube->prop2Cnt*sizeof(Prop));
            code_A5BC0_initCubePropActorProp(cube);
        }else{
            bk_free(cube->prop2Ptr);
            cube->prop2Ptr = NULL;
        }
        return sp24;
    }
    return 0;
}

void func_8032DB2C(Cube *cube, NodeProp *arg1) {
    s32 sp24;

    if(cube->prop1Cnt == 0)
        return;

    sp24 = arg1 - cube->prop1Ptr;
    if (sp24 < cube->prop1Cnt - 1) {
        memcpy(arg1, arg1 + 1, (uintptr_t)&cube->prop1Ptr[cube->prop1Cnt] - (uintptr_t)arg1 - sizeof(NodeProp));
    }
    if (sp24 < cube->unk0_4) {
        cube->unk0_4--;
    }

    cube->prop1Cnt--;
    if (cube->prop1Cnt != 0) {
        cube->prop1Ptr = bk_realloc(cube->prop1Ptr, cube->prop1Cnt * sizeof(NodeProp));
    }
    else{
        bk_free(cube->prop1Ptr);
        cube->prop1Ptr = NULL;
        cube->unk0_4 = 0;
    }
}

void func_8032DC70(s32 arg0) {
    NodeProp *sp1C;
    s32 temp_v0;

    sp1C = func_803080C8(arg0);
    if (sp1C != NULL) {
        func_8032DB2C(func_80308224(), sp1C);
    }
}

ActorMarker *func_8032DCAC(void){
    return D_8038341C;
}

SpriteProp *func_8032DCB8(Cube *cube) {
    SpriteProp *sp1C;

    sp1C = (SpriteProp *)__codeA5BC0_initProp2Ptr(cube);
    sp1C->isActorProp = false;
    sp1C->isModelProp = false;
    sp1C->frame = 0;
    sp1C->isMirrored = 0;
    sp1C->unk8_10 = randf() * 32.0f;
    sp1C->unk8_3 = false;
    sp1C->isCollisionResolved = false;
    sp1C->isNotFeatherEggOrNote = true;
    return sp1C;
}

void func_8032DDD8(Cube *cube) {
    Prop *temp_v0;

    temp_v0 = __codeA5BC0_initProp2Ptr(cube);
    temp_v0->markerFlag = false;
    temp_v0->unk8_1 = true;
    temp_v0->unk8_5 = false;
    temp_v0->unk8_3 = false;
    temp_v0->unk8_2 = false;
    temp_v0->unk8_4 = true;
}


void func_8032DE2C(ModelProp *model_prop, enum asset_e sprite_id){
    model_prop->modelId = sprite_id - 0x2d1;
}

void func_8032DE48(ModelProp *model_prop, enum asset_e *model_id_ptr){\
    *model_id_ptr = ((*(u32*)model_prop) >> 0x14) + 0x2d1;
}

void func_8032DE5C(SpriteProp *sprite_prop, enum asset_e sprite_id){
    sprite_prop->spriteId = sprite_id - 0x572;
}

void func_8032DE78(SpriteProp *sprite_prop, enum asset_e *sprite_id_ptr){
    *sprite_id_ptr = sprite_prop->spriteId + 0x572;
}

void func_8032DE8C(SpriteProp *sprite_prop, s32 *arg1){
    *arg1 = sprite_prop->scale;
}

void func_8032DEA0(SpriteProp *sprite_prop, s32 *arg1, s32 *arg2, s32 *arg3){
    *arg1 = sprite_prop->rgb_remove_red;
    *arg2 = sprite_prop->rgb_remove_green;
    *arg3 = sprite_prop->rgb_remove_blue;
}

void func_8032DECC(SpriteProp *sprite_prop, s32 *arg1){
    *arg1 = ((*(u32*)sprite_prop) << 0x1e) >> 0x1f;//sprite_prop->pad0_0;
}

void func_8032DEE0(SpriteProp *sprite_prop, s32 arg1){
    sprite_prop->scale = arg1;
}

void func_8032DEFC(ModelProp *prop_prop, s32 arg1){
    prop_prop->scale = arg1;
}

void func_8032DF04(ModelProp *prop_prop, s32 *arg1){
    *arg1 = prop_prop->scale;
}

void func_8032DF10(SpriteProp *sprite_prop, bool *arg1){
    *arg1 = sprite_prop->unk0_19;
}

void func_8032DF24(SpriteProp *sprite_prop, bool arg1){
    sprite_prop->isMirrored = arg1;
}

void func_8032DF40(ModelProp *prop_prop, s32 arg1, s32 arg2){
    prop_prop->yaw = arg1;
    prop_prop->roll = arg2;
}

void func_8032DF4C(ModelProp *prop_prop, s32 *arg1, s32 *arg2){
    *arg1 = prop_prop->yaw;
    *arg2 = prop_prop->roll;
}

void func_8032DF60(SpriteProp *sprite_prop, s32 arg1, s32 arg2, s32 arg3){
    sprite_prop->rgb_remove_red = arg1;
    sprite_prop->rgb_remove_green = arg2;
    sprite_prop->rgb_remove_blue = arg3;
}

void func_8032DFA0(SpriteProp *sprite_prop, bool arg1){
    sprite_prop->unk0_19 = arg1;
}

void func_8032DFBC(NodeProp *node, s32 src[3]){
    node->x = src[0];
    node->y = src[1];
    node->z = src[2];
}

void func_8032DFD8(NodeProp *node, s32 dst[3]){
    dst[0] = node->x;
    dst[1] = node->y;
    dst[2] = node->z;
}

void func_8032DFF4(Prop *prop, s32 src[3]){
    prop->unk4[0] = src[0];
    prop->unk4[1] = src[1];
    prop->unk4[2] = src[2];
}

void codeA5BC0_getActorPosition(ActorProp *prop, s32 dst[3]){
    dst[0] = prop->x;
    dst[1] = prop->y;
    dst[2] = prop->z;
}

NodeProp *codeA5BC0_getPropNodeAtIndex(Cube *cube, s32 prop_index) {
    if ((prop_index < 0) || (prop_index >= cube->prop1Cnt)) {
        return NULL;
    }
    return &cube->prop1Ptr[prop_index];
}

void func_8032E070(void){
    D_8038341C = NULL;
}

void cube_free(Cube *cube){
    Prop *iProp;

    if(cube->prop2Ptr){
        for(iProp = cube->prop2Ptr; iProp < cube->prop2Ptr +cube->prop2Cnt; iProp++){
            if(iProp->markerFlag){
                func_80332B2C(iProp->actorProp.marker);
            }
        }
        bk_free(cube->prop2Ptr);
        cube->prop2Ptr = NULL;
    }
    if(cube->prop1Ptr){
        bk_free(cube->prop1Ptr);
        cube->prop1Ptr = NULL;
    }
    cube->prop2Cnt = 0;
    cube->prop1Cnt = 0;
    cube->unk0_4 = 0;
}

bool __codeA5BC0_pad_func_8032E178(Cube *arg0, s32 *arg1, s32 arg2) {
    NodeProp *last_node_ptr;
    NodeProp *node_ptr;

    if ((arg0 != NULL) && (arg0->prop1Cnt != 0)) {
        node_ptr = arg0->prop1Ptr;
        last_node_ptr = arg0->prop1Ptr + arg0->prop1Cnt;
        while (node_ptr < last_node_ptr) {
            if( ((node_ptr->bit0 == true)
                    || ((node_ptr->bit0 == false) && (node_ptr->unk10_6 == true))
                ) 
                && (node_ptr->bit6 == PROP_1_CATEGORY_6_ACTOR) 
                && (arg2 == node_ptr->unk8)
            ) {
                *arg1 = node_ptr->radius;
                return true;
            }
            node_ptr++;
        }
    }
    return false;
}

NodeProp *cube_findNodePropByActorId(Cube *cube, enum actor_e actor_id) {
    NodeProp *i_ptr;

    if (cube != NULL && cube->prop1Cnt != 0){
        for(i_ptr = cube->prop1Ptr; i_ptr < cube->prop1Ptr + cube->prop1Cnt; i_ptr++){
            if( ( (i_ptr->bit0 == true) 
                  || ( (i_ptr->bit0 == false) && (i_ptr->unk10_6 == true))
                )
                && (i_ptr->bit6 == PROP_1_CATEGORY_6_ACTOR) 
                && (actor_id == i_ptr->unk8)
            ) {
                return i_ptr;
            }
        }
    }
    return NULL;
}

bool func_8032E2D4(Cube *arg0, s32 arg1[3], s32 arg2) {
    NodeProp * temp_a2;
    NodeProp *var_v1;

    if ((arg0 != NULL) && (arg0->prop1Cnt != 0)) {
        var_v1 = arg0->prop1Ptr;
        temp_a2 = arg0->prop1Ptr + arg0->prop1Cnt;
        while (var_v1 < temp_a2) {
            if( ((var_v1->bit0 == true)
                    || ((var_v1->bit0 == false) && (var_v1->unk10_6 == true))
                ) 
                && (var_v1->bit6 == PROP_1_CATEGORY_6_ACTOR) 
                && (arg2 == var_v1->unk8)
            ) {
                arg1[0] = var_v1->x;
                arg1[1] = var_v1->y;
                arg1[2] = var_v1->z;
                return true;
            }
            var_v1++;
        }
    }
    return false;
}

bool func_8032E398(Cube *cube, bool (*arg1)(NodeProp *), bool (*arg2)(Prop *)) {
    NodeProp *var_s0;
    Prop *var_s0_2;
    NodeProp *var_s1;
    Prop *var_s1_2;

    if ((cube->prop1Cnt != 0) && (arg1 != NULL)) {
        var_s0 = cube->prop1Ptr;
        var_s1 = cube->prop1Ptr + cube->prop1Cnt;
        while (var_s0 < var_s1) {
            if (!arg1(var_s0)) {
                return false;
            }
            var_s0++;
        }
    }
    if ((cube->prop2Cnt != 0) && (arg2 != NULL)) {
        var_s0_2 = cube->prop2Ptr;
        var_s1_2 = cube->prop2Ptr + cube->prop2Cnt;
        while(var_s0_2 < var_s1_2) {
            if (!arg2(var_s0_2)) {
                return false;
            }
            var_s0_2++;
        }
    }
    return true;
}

/* places up to `node_list_capacity` NodeProp pointers in `node_list`
 * from `cube` for any actors with id's in `actor_id_list`. Return number
 * nodeprops found.
 */
s32 func_8032E49C(Cube *cube, enum actor_e *actor_id_list, NodeProp **node_list, s32 node_list_capacity) {
    enum actor_e *i_actor;
    s32 found_cnt;
    NodeProp * end_node;
    NodeProp *i_node;

    found_cnt = 0;
    if (cube != NULL) {
        if (cube->prop1Cnt != 0) {
            i_node = cube->prop1Ptr;
            end_node = cube->prop1Ptr + cube->prop1Cnt;
            while((i_node < end_node) && (found_cnt < node_list_capacity)) {
                if (((i_node->bit0 == true) || ((i_node->bit0 == false) && (i_node->unk10_6 == true))) && (i_node->bit6 == PROP_1_CATEGORY_6_ACTOR)) {
                    i_actor = actor_id_list;
                    for(i_actor = actor_id_list; *i_actor != -1; i_actor++){
                        if (i_node->unk8 == *i_actor) {
                            node_list[found_cnt++] = i_node;
                            break;
                        }
                    }
                }
                i_node++;
            }
        }
    }
    return found_cnt;
}

s32 func_8032E5A8(Cube *cube, s32 arg1, f32 (*arg2)[3], s32 capacity) {
    s32 count;
    NodeProp *end_node;
    NodeProp *i_node;

    count = 0;
    if (cube != NULL) {
        if (cube->prop1Cnt != 0) {
            i_node = cube->prop1Ptr;
            end_node = cube->prop1Ptr + cube->prop1Cnt;
            while((i_node < end_node) && (count < capacity)){
                if( ( (i_node->bit0 == true) 
                      || ((i_node->bit0 == false) && (i_node->unk10_6 == true))
                    ) 
                    && (i_node->bit6 == PROP_1_CATEGORY_6_ACTOR) && (arg1 == i_node->unk8)
                ) {
                    arg2[count][0] = (f32) i_node->x;
                    arg2[count][1] = (f32) i_node->y;
                    arg2[count][2] = (f32) i_node->z;
                    count++;
                }
                i_node++;
            }
        }
    }
    return count;
}

bool func_8032E6CC(Cube *cube, s32 *arg1, s32 arg2) {
    NodeProp *end_node;
    NodeProp *i_node;

    if (cube != NULL) {
        if (cube->prop1Cnt != 0) {
            i_node = cube->prop1Ptr;
            end_node = cube->prop1Ptr + cube->prop1Cnt;
            while (i_node < end_node) {
                if( ( (i_node->bit0 == true) 
                      || ((i_node->bit0 == false) && (i_node->unk10_6 == true))
                    ) 
                    && (i_node->bit6 == PROP_1_CATEGORY_6_ACTOR) && (arg2 == i_node->unk8)
                ) {
                    *arg1 = i_node->yaw;
                    return true;
                }
                i_node++;
            }
        }
    }
    return false;
}

static void __codeA5BC0_freeCube1Pointer(Cube *cube, s32 cnt){
    if(cube->prop1Ptr != NULL){
        bk_free(cube->prop1Ptr);
    }

    cube->prop1Cnt = cnt;
    cube->unk0_4 = 0; 
}

static void __codeA5BC0_initPropPointerForCube(NodeProp *node, Cube *cube, s32 cnt) {
    NodeProp *iPtr;
    s32 cube_ptr_idx;
    s32 i;

    cube->unk0_4 = 0;
    cube_ptr_idx = cnt - 1;
    for(i = 0; i < cnt; i++){
        iPtr = node + i;
        if( (iPtr->bit6 == PROP_1_CATEGORY_6_ACTOR) 
            || (iPtr->bit6 == PROP_1_CATEGORY_8_PATH)
            || (iPtr->bit6 == PROP_1_CATEGORY_7_ENEMY_BOUNDARY) 
            || (iPtr->bit6 == PROP_1_CATEGORY_9_CAMERA_TRIGGER) 
            || (iPtr->bit6 == PROP_1_CATEGORY_A_FLAG) 
            || (iPtr->bit0 == 1)
        ){
            memcpy(&cube->prop1Ptr[cube_ptr_idx], &node[i], sizeof(NodeProp));
            cube_ptr_idx--;
        } else {
            memcpy(&cube->prop1Ptr[cube->unk0_4], &node[i], sizeof(NodeProp));
            cube->unk0_4++;
        }
    }

    // [port] unk0_4 is the front index of this partition; :5 wrapped it here.
    if(cube->unk0_4 > 31){
        port_warnNodePropSplit(cube->unk0_4, cnt);
    }

    bk_free(node);

    for(i = 0; i < cnt; i++){
        iPtr = &cube->prop1Ptr[i];
        if(!iPtr->bit0){
            iPtr->unk10_6 = true;
        }
    }
}

void code7AF80_initCubeFromFile(File *file_ptr, Cube *cube) {
    u8 sp47;
    u8 cube1_count;
    NodeProp *node_prop_ptr;
    OtherNode *other_prop_ptr;
    Prop *var_v1_2;
    s32 sp34;
    s32 temp_v0_5;

    cube_free(cube);
    if (file_getByte_ifExpected(file_ptr, CUBE_PROP_1_INDICATOR, &cube1_count)) {
        __codeA5BC0_freeCube1Pointer(cube, cube1_count);
        cube->prop1Ptr = (NodeProp*) bk_malloc(cube1_count * sizeof(NodeProp));
        node_prop_ptr = (NodeProp*) bk_malloc(cube1_count * sizeof(NodeProp));
        file_getNBytes_ifExpected(file_ptr, CUBE_PROP_1_LIST_START_INDICATOR, (u8*)node_prop_ptr, cube->prop1Cnt * sizeof(NodeProp));
        __codeA5BC0_initPropPointerForCube(node_prop_ptr, cube, cube1_count);
    } else if (file_getByte_ifExpected(file_ptr, CUBE_PROP_1_OTHER_INDICATOR, &cube1_count)) {
        __codeA5BC0_freeCube1Pointer(cube, cube1_count);
        cube->prop1Ptr = (NodeProp*) bk_malloc(cube1_count * sizeof(OtherNode));
        node_prop_ptr = (NodeProp*) bk_malloc(cube1_count * sizeof(OtherNode));
        file_getNBytes_ifExpected(file_ptr, CUBE_PROP_1_OTHER_LIST_START_INDICATOR, (u8*)node_prop_ptr, cube->prop1Cnt * sizeof(OtherNode));
        for(other_prop_ptr = (OtherNode*)node_prop_ptr; other_prop_ptr < (OtherNode*)&node_prop_ptr[cube1_count]; other_prop_ptr++){
            if(other_prop_ptr->unk4_0 && !other_prop_ptr->unkC_0){
                other_prop_ptr->unk4_17 = 0;
                other_prop_ptr->unk10_4 = 0;
            }
        }
        __codeA5BC0_initPropPointerForCube(node_prop_ptr, cube, cube1_count);
    }

    if (file_getByte_ifExpected(file_ptr, CUBE_PROP_1_LIST_END_INDICATOR, &sp47)) {
        sp34 = volatileFlag_get(VOLATILE_FLAG_1) +  volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME) + volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE);
        
        if ((sp34) && gcparade_8031B4CC()) {
            sp34 = 0;
        }
        if (cube->prop2Ptr != NULL) {
            bk_free(cube->prop2Ptr);
        }
        cube->prop2Cnt = sp47;
        cube->prop2Ptr = (Prop *) bk_malloc(sp47 * sizeof(Prop));
        // [port] Props are 12 bytes in N64 format. The serializer byte-swaps
        // multi-byte values to native endian, but bitfield positions within
        // integers follow the N64 big-endian layout. We extract each field
        // using BE bit positions and write through struct access so the
        // compiler handles platform-native bitfield placement. This follows
        // the Shipwright/libultraship pattern: never memcpy raw bytes into
        // bitfield structs across endianness boundaries.
        if (file_isNextByteExpected(file_ptr, CUBE_PROP_2_LIST_START_INDICATOR)) {
            s32 i;
            // [port] A vetoed prop must not keep its slot. Compact
            // instead, writing only the props we keep.
            s32 writeIdx = 0;
            memset(cube->prop2Ptr, 0, sp47 * sizeof(Prop));
            for (i = 0; i < sp47; i++) {
                u8 raw[12];
                u32 word0;
                s16 pos[3];
                u16 flags;
                file_getNBytes(file_ptr, raw, 12);
                // Read native-endian values (serializer already byte-swapped)
                memcpy(&word0, raw, 4);
                memcpy(&pos[0], raw + 4, 2);
                memcpy(&pos[1], raw + 6, 2);
                memcpy(&pos[2], raw + 8, 2);
                memcpy(&flags, raw + 10, 2);
                // [port] Expose the sprite asset id (or -1 for marker/model props) so
                // override handlers can identify props by type (e.g. music notes) without
                // relying on position, which differs in romhacks.
                s32 propSpriteAsset = -1;
                if (!(flags & 1) && !((flags >> 1) & 1)) {
                    propSpriteAsset = ((s32)((word0 >> 20) & 0xFFF)) + 0x572;
                }
                if (!EventSystem_Should(VB_OVERRIDE_PROP_SPAWN, false, pos, propSpriteAsset)) {
                // Extract flag bits using N64 BE bit positions within the u16
                    {
                        bool mf = flags & 1;            // markerFlag (BE bit 0)
                        bool b1 = (flags >> 1) & 1;     // unk8_1
                        bool b2 = (flags >> 2) & 1;     // unk8_2
                        bool b3 = (flags >> 3) & 1;     // unk8_3
                        bool b4 = (flags >> 4) & 1;     // unk8_4
                        bool b5 = (flags >> 5) & 1;     // unk8_5
                        // Set position and flags via anonymous struct
                        cube->prop2Ptr[writeIdx].unk4[0] = pos[0];
                        cube->prop2Ptr[writeIdx].unk4[1] = pos[1];
                        cube->prop2Ptr[writeIdx].unk4[2] = pos[2];
                        cube->prop2Ptr[writeIdx].markerFlag = mf;
                        cube->prop2Ptr[writeIdx].unk8_1 = b1;
                        cube->prop2Ptr[writeIdx].unk8_2 = b2;
                        cube->prop2Ptr[writeIdx].unk8_3 = b3;
                        cube->prop2Ptr[writeIdx].unk8_4 = b4;
                        cube->prop2Ptr[writeIdx].unk8_5 = b5;
                        if (mf) {
                            // ActorProp: marker=NULL (set at actor spawn), position
                            cube->prop2Ptr[writeIdx].actorProp.marker = NULL;
                            cube->prop2Ptr[writeIdx].actorProp.x = pos[0];
                            cube->prop2Ptr[writeIdx].actorProp.y = pos[1];
                            cube->prop2Ptr[writeIdx].actorProp.z = pos[2];
                            // SpriteProp unk8_10/unk8_15 overlap anonymous pad8_15
                            cube->prop2Ptr[writeIdx].actorProp.frame = (flags >> 11) & 0x1F;
                            cube->prop2Ptr[writeIdx].actorProp.unk8_10 = (flags >> 6) & 0x1F;
                        }
                        else if (b1) {
                            // ModelProp: word0 = { u16 unk0, u8 unk0_15, u8 unk0_7 }
                            // [port] Set through bitfield accessor, not raw u16, for LE correctness
                            cube->prop2Ptr[writeIdx].modelProp.modelId = (word0 >> 20) & 0xFFF;
                            cube->prop2Ptr[writeIdx].modelProp.pad0_19 = (word0 >> 16) & 0xF;
                            cube->prop2Ptr[writeIdx].modelProp.yaw = (u8)(word0 >> 8);
                            cube->prop2Ptr[writeIdx].modelProp.roll = (u8)(word0);
                            // ModelProp flags at offset 10-11
                            cube->prop2Ptr[writeIdx].modelProp.scale = (u8)(flags >> 8);
                            cube->prop2Ptr[writeIdx].modelProp.unkB_5 = (flags >> 5) & 1;
                            cube->prop2Ptr[writeIdx].modelProp.unkB_4 = (flags >> 4) & 1;
                        }
                        else {
                            // SpriteProp: word0 is a u32 with bitfields
                            cube->prop2Ptr[writeIdx].spriteProp.spriteId = (word0 >> 20) & 0xFFF;
                            cube->prop2Ptr[writeIdx].spriteProp.unk0_19 = (word0 >> 19) & 1;
                            cube->prop2Ptr[writeIdx].spriteProp.rgb_remove_red = (word0 >> 16) & 7;
                            cube->prop2Ptr[writeIdx].spriteProp.rgb_remove_green = (word0 >> 13) & 7;
                            cube->prop2Ptr[writeIdx].spriteProp.rgb_remove_blue = (word0 >> 10) & 7;
                            cube->prop2Ptr[writeIdx].spriteProp.scale = (word0 >> 2) & 0xFF;
                            cube->prop2Ptr[writeIdx].spriteProp.isMirrored = (word0 >> 1) & 1;
                            cube->prop2Ptr[writeIdx].spriteProp.pad0_0 = word0 & 1;
                            // SpriteProp flags
                            cube->prop2Ptr[writeIdx].spriteProp.frame = (flags >> 11) & 0x1F;
                            cube->prop2Ptr[writeIdx].spriteProp.unk8_10 = (flags >> 6) & 0x1F;
                        }
                    }
                    //Prop* ptr = cube->prop2Ptr + i * sizeof(Prop);
                    CALL_EVENT(OnPropInit, &cube->prop2Ptr[writeIdx]);
                    writeIdx++;
                }
            }
            cube->prop2Cnt = writeIdx;
        }
        for(var_v1_2 = cube->prop2Ptr; var_v1_2 < cube->prop2Ptr + cube->prop2Cnt; var_v1_2++){
                var_v1_2->unk8_4 = 1;
                if (var_v1_2->unk8_1) {
                    var_v1_2->unk8_5 = 0;
                }
                if (sp34) {
                    if (!(var_v1_2->markerFlag) && !(var_v1_2->unk8_1)){
                        temp_v0_5 = var_v1_2->spriteProp.spriteId + 0x572;
                        if((temp_v0_5 == 0x580) || (temp_v0_5 == 0x6D1) || (temp_v0_5 == 0x6D6) || (temp_v0_5 == 0x6D7)){
                            var_v1_2->unk8_4 = 0;
                        }
                    }
                }
        }
            
    }
    if ((cube->prop2Ptr != NULL) && ((cube->prop2Cnt) == 0)) {
        bk_free(cube->prop2Ptr);
        cube->prop2Ptr = NULL;
    }
    
    if ((cube->prop1Ptr != NULL) && (cube->prop1Cnt == 0)) {
        bk_free(cube->prop1Ptr);
        cube->prop1Ptr = NULL;
        cube->unk0_4 = 0;
    }
}

void func_8032EE0C(Method_Core2_A5BC0 arg0, s32 arg1){
    D_80383558 = arg0;
    D_8038355C = arg1;
}

void func_8032EE20(void){
    D_80383558 = NULL;
}

void func_8032EE2C(s32 arg0[3], s32 arg1, s32 arg2) {
    D_803833F0[0] = (s32) arg0[0];
    D_803833F0[1] = (s32) arg0[1];
    D_803833F0[2] = (s32) arg0[2];
    D_80383400 = arg1;
    D_803833FC = arg2 * arg2;
    D_80383404 = 0;
    D_80383408 = 0;
    D_8038340C = 0;
}

void func_8032EE80(Cube *cube) {
    s32 var_v0;
    s32 var_s1;
    s32 var_t0;
    Prop *var_a1;
    NodeProp *var_s0;

    if ((cube->prop2Cnt != 0) && ((D_80383400 == 1) || (D_80383400 == 2))) {
        var_a1 = cube->prop2Ptr;
        for(var_t0 = 0; var_t0 < cube->prop2Cnt; var_t0++, var_a1++){
            if (!var_a1->markerFlag) {
                if (((var_a1->unk4[0] - D_803833F0[0]) * (var_a1->unk4[0] - D_803833F0[0])) + ((var_a1->unk4[1] - D_803833F0[1]) * (var_a1->unk4[1] - D_803833F0[1])) + ((var_a1->unk4[2] - D_803833F0[2]) * (var_a1->unk4[2] - D_803833F0[2])) < D_803833FC) {
                    var_v0 = (var_a1->unk8_1) ? 2 : 1;
                    if (var_v0 == D_80383400) {
                        D_803833FC = ((var_a1->unk4[0] - D_803833F0[0]) * (var_a1->unk4[0] - D_803833F0[0])) + ((var_a1->unk4[1] - D_803833F0[1]) * (var_a1->unk4[1] - D_803833F0[1])) + ((var_a1->unk4[2] - D_803833F0[2]) * (var_a1->unk4[2] - D_803833F0[2]));
                        D_80383404 = cube;
                        D_80383408 = var_a1;
                        D_8038340C = D_80383400;
                    }
                }
            }
        }
    }

    if (cube->prop1Cnt != 0) {
        if (D_80383400 == 3) {
            var_s0 = cube->prop1Ptr;
            for(var_s1 = 0; var_s1 < cube->prop1Cnt; var_s1++, var_s0++){
                if (((var_s0->x - D_803833F0[0]) * (var_s0->x - D_803833F0[0])) + ((var_s0->y - D_803833F0[1]) * (var_s0->y - D_803833F0[1])) + ((var_s0->z - D_803833F0[2]) * (var_s0->z - D_803833F0[2])) < D_803833FC) {
                    if (D_80383558 == NULL || D_80383558(var_s0, D_8038355C)) {
                        D_803833FC = ((var_s0->x - D_803833F0[0]) * (var_s0->x - D_803833F0[0])) + ((var_s0->y - D_803833F0[1]) * (var_s0->y - D_803833F0[1])) + ((var_s0->z - D_803833F0[2]) * (var_s0->z - D_803833F0[2]));
                        D_80383404 = cube;
                        D_80383408 = var_s0;
                        D_8038340C = D_80383400;
                    }
                }
            }
        }
    }
}

s32 func_8032F170(Cube **arg0, void **arg1){
    *arg0 = D_80383404;
    *arg1 = D_80383408;
    return D_8038340C;
}

void func_8032F194(ActorMarker *marker, s32 position[3], Cube *cube) {
    ActorProp sp24;
    ActorProp *propPtr = marker->propPtr;
    ActorProp *v0 = &sp24;

    v0 += 0;

    sp24.words[ACTORPROP_WORD_BASE + 1] = propPtr->words[ACTORPROP_WORD_BASE + 1];

    v0->x = position[0];
    v0->y = position[1];
    v0->z = position[2];

    func_8032F21C(cube, position, marker, func_8032D9C0(marker->cubePtr, (Prop *)propPtr));

    propPtr = marker->propPtr;
    propPtr->words[ACTORPROP_WORD_BASE] = sp24.words[ACTORPROP_WORD_BASE];
    propPtr->words[ACTORPROP_WORD_BASE + 1] = sp24.words[ACTORPROP_WORD_BASE + 1];
}

void func_8032F21C(Cube *cube, s32 position[3], ActorMarker *marker, bool arg3) {
    ActorProp *sp1C;

    sp1C = &__codeA5BC0_initProp2Ptr(cube)->actorProp;
    sp1C->isActorProp = true;
    sp1C->x = (s16) position[0];
    sp1C->y = (s16) position[1];
    sp1C->z = (s16) position[2];
    sp1C->marker = marker;
    sp1C->isModelProp = arg3;
    sp1C->frame = 0;
    sp1C->isMirrored = false;

    sp1C->unk8_10 = (func_802E4A08()) ? 0xF : (u8)(randf() * 32);
    sp1C->unk8_3 = false;
    sp1C->isCollisionResolved = false;
    sp1C->isNotFeatherEggOrNote = true;
    marker->propPtr = sp1C;
    marker->cubePtr = cube;
    if (func_80305D14()) {
        func_80305CD8(func_803058C0((f32)position[1]), 1);
    }
}

void func_8032F3D4(s32 arg0[3], ActorMarker *marker, s32 arg2){
    func_8032F21C((marker->unk40_23)? func_8030364C() : cubeList_GetCubeAtPosition_s32(arg0), arg0, marker, arg2);
}

void marker_free(ActorMarker *this){
    func_8032D9C0(this->cubePtr, (Prop *)this->propPtr);
    func_80332B2C(this);
}

void func_8032F464(bool arg0){
    D_8036E7B0 = arg0;
}

void func_8032F470(s32 *pos, ActorMarker *arg1){
    Cube *cubePtr;

    cubePtr = (arg1->unk40_23)? func_8030364C(): cubeList_GetCubeAtPosition_s32(pos);

    if(cubePtr == arg1->cubePtr){
        arg1->propPtr->x = pos[0];
        arg1->propPtr->y = pos[1];
        arg1->propPtr->z = pos[2];
    }
    else{
        func_8032F194(arg1, pos, cubePtr);
    }

    if(arg1->unk2C_1)
        func_80307CA0(arg1);
}

Prop *func_8032F528(void){
    Prop * prop = func_80303F7C(0, 0, 0, 1);
    if(prop != NULL){
        D_80383448 = true;
    }
    else{
        D_80383448 = false;
    }
    return prop;
}

void func_8032F578(f32 position[3], ActorMarker *marker, f32 arg2, s32 arg3) {
    func_8032F64C(position, marker);
    func_80320ED8(marker, arg2, arg3);
}

void func_8032F5B0(f32 position[3], ActorMarker *marker, f32 arg2, s32 arg3) {
    func_8032F64C(position, marker);
    func_80320EB0(marker, arg2, arg3);
}

void func_8032F5E8(s32 arg0[3], ActorMarker *arg1, f32 arg2, s32 arg3) {
    f32 sp1C[3];

    sp1C[0] = arg0[0];
    sp1C[1] = arg0[1];
    sp1C[2] = arg0[2];
    func_8032F5B0(sp1C, arg1, arg2, arg3);
}

void func_8032F64C(f32 *pos, ActorMarker * marker){
    s32 pos_w[3];

    pos_w[0] = pos[0];
    pos_w[1] = pos[1];
    pos_w[2] = pos[2];
    func_8032F470(pos_w, marker);
}

void func_8032F6A4(s32 *pos, ActorMarker * marker, s32 *rot){
    s32 rot_w[3];
    
    rot_w[0] = (s32)rot[0] % 360;
    if(rot_w[0] < 0)
        rot_w[0] += 360;

    rot_w[1] = (s32)rot[1] % 360;
    if(rot_w[1] < 0)
        rot_w[1] += 360;

    rot_w[2] = (s32)rot[2] % 360;
    if(rot_w[2] < 0)
        rot_w[2] += 360;

    marker->yaw = rot_w[1];
    marker->pitch = rot_w[0];
    marker->roll = rot_w[2];
    func_8032F470(pos, marker);
}

void func_8032F7EC(f32 position[3], ActorMarker *marker, f32 rotation[3]) {
    marker->yaw  = rotation[1];
    marker->pitch = rotation[0];
    marker->roll = rotation[2];
    func_8032F64C(position, marker);
}

ActorMarker * marker_init(s32 *pos, MarkerDrawFunc draw_func, int arg2, int marker_id, int arg4){
    ActorMarker * marker = func_80332A60();
    marker->propPtr = NULL;
    marker->cubePtr = NULL;
    marker->drawFunc = draw_func;
    marker->id = marker_id;
    marker->unk40_23 = arg4;
    func_8032F3D4(pos, marker, arg2);
    marker->actrArrayIdx = 0;
    marker->unk14_10 = 0;
    marker->modelId = 0;
    marker->isBanjoOnTop = 0;
    marker->unk14_22 = 0;
    marker->unk14_21 = 0;
    marker->yaw = 0;
    marker->pitch =  0;
    marker->roll = 0;
    marker->unk2C_2 = 0;
    marker->unk2C_1 = 0;
    marker->collidable = true;
    marker->unk3E_0 = 0; 
    marker->unk40_22 = 0;
    marker->unk40_19 = 0;
    marker->unk40_21 = 0;
    marker->collisionFunc = NULL;
    marker->collision2Func = NULL;
    marker->dieFunc = NULL;
    marker->unk54 = NULL;
    marker->unk58 = 0;
    marker->unk18 = 0;
    marker->actorUpdateFunc = NULL;
    marker->actorFreeFunc = NULL;
    marker->commonParticleIndex = 0;
    marker->actorUpdate2Func = NULL;
    marker->unk38[0] = 0;
    marker->unk38[1] = 0;
    marker->unk38[2] = 0;
    marker->unk44 = 0;
    marker->unk20 = NULL;
    marker->unk50 = NULL;
    marker->unk48 = 0;
    marker->unk4C = 0;
    marker->unk40_20 = 0;
    marker->unk40_31 = 0;
    return marker;
}

ActorMarker * func_8032FB80(f32 *pos, MarkerDrawFunc arg1, int arg2, enum asset_e model_id, int arg4){
    s32 sp24[3];
    sp24[0] = pos[0];
    sp24[1] = pos[1];
    sp24[2] = pos[2];
    return marker_init(sp24, arg1, arg2, model_id, arg4);
}

ActorMarker * func_8032FBE4(f32 *pos, MarkerDrawFunc arg1, int arg2, enum asset_e model_id){
    return func_8032FB80(pos, arg1, arg2, model_id, 0);
}

void func_8032FC04(ActorMarker *marker, f32 rotation[3]) {
    marker->pitch = rotation[0];
    marker->yaw  = rotation[1];
    marker->roll = rotation[2];
}

void func_8032FDDC(f32 rotation[3], ActorMarker *marker) {
    marker->pitch = rotation[0];
    marker->yaw  = rotation[1];
    marker->roll = rotation[2];
}

int func_8032FFB4(ActorMarker *this, s32 arg1){
    this->id = arg1;
    return 0;
}

//marker_setActorArrayIndex
void func_8032FFD4(ActorMarker *this, s32 arg1){
    this->actrArrayIdx = arg1;
}

void marker_setCommonParticleIndex(ActorMarker *this, s32 arg1){
    this->commonParticleIndex = arg1;
}

void marker_callCollisionFunc(ActorMarker *this, ActorMarker *other, enum marker_collision_func_type_e type){
    switch(type){
        case MARKER_COLLISION_FUNC_0: //ow
            if(this->collisionFunc)
                this->collisionFunc(this, other); 
            break;
        case MARKER_COLLISION_FUNC_1:
            if(this->collision2Func)
                this->collision2Func(this, other);
            break;
        case MARKER_COLLISION_FUNC_2_DIE: //die
            if(this->dieFunc)
                this->dieFunc(this, other);
            break;
    }
}

void func_80330078(ActorMarker *marker, ActorMarker *other_marker, s16 *arg2){
    if(marker != NULL && marker->unk54 != NULL){
        marker->unk54(marker, other_marker, arg2);
    }
}

//marker_setCollisionMethods
void marker_setCollisionScripts(ActorMarker *this, MarkerCollisionFunc ow_func, MarkerCollisionFunc arg2, MarkerCollisionFunc die_func){
    this->collisionFunc = ow_func;
    this->collision2Func = arg2;
    this->dieFunc = die_func;
}

void func_803300B8(ActorMarker *marker, MarkerCollisionFunc method){
    marker->unk54 = (void (*)(struct actorMarker_s*, struct actorMarker_s*, u16*))method;
}

void func_803300C0(ActorMarker *marker, s32 (*method)(ActorMarker *, ActorMarker *)){
    marker->unk58 = method;
}

void marker_setActorUpdateFunc(ActorMarker *marker, ActorUpdateFunc method){
    marker->actorUpdateFunc = method;
}

void marker_setActorUpdate2Func(ActorMarker *marker, ActorUpdateFunc method){
    marker->actorUpdate2Func = method;
}

void marker_setFreeMethod(ActorMarker *marker, ActorFreeFunc method){
    marker->actorFreeFunc = method;
}

void func_803300E0(ActorMarker *marker, Struct6Cs *arg1){
    marker->unk18 = arg1;
}

void marker_setModelId(ActorMarker *this, enum asset_e modelIndex){
    this->modelId = modelIndex;
}

void code_A5BC0_initCubePropActorProp(Cube *cube) {
    u32 prop_cnt;
    Prop *prop_ptr;

    if (cube->prop2Cnt != 0) {
        prop_ptr = cube->prop2Ptr;
        prop_cnt = cube->prop2Cnt;
        while(prop_cnt != 0){
            // [port] guard: marker may be NULL if actor hasn't spawned yet
            if(prop_ptr->markerFlag == true){
                if(prop_ptr->actorProp.marker != NULL){
                    prop_ptr->actorProp.marker->propPtr = &prop_ptr->actorProp;
                    prop_ptr->actorProp.marker->cubePtr = cube;
                }
            }
            prop_cnt--;
            prop_ptr++;
        }
    }
}

void func_80330208(Cube *cube) {
    s32 position[3];
    Actor *actor;
    NodeProp *end_prop;
    NodeProp *i_prop;

    if ((cube != NULL) && (cube->prop1Cnt != 0)) {
        i_prop = cube->prop1Ptr;
        end_prop = cube->prop1Ptr + cube->prop1Cnt;
        func_80326C24(1);
        while(i_prop < end_prop){
            if (i_prop->bit6 == PROP_1_CATEGORY_6_ACTOR) {
                position[0] = (s32) i_prop->x;
                position[1] = (s32) i_prop->y;
                position[2] = (s32) i_prop->z;
                actor = func_803055E0(i_prop->unk8, position, i_prop->yaw, i_prop->unk10_31, i_prop->unk10_19);
                if (actor != NULL) {
                    actor->secondaryId = i_prop->unk10_31;
                    actor->actorTypeSpecificField = i_prop->radius;
                    suSetSpriteScale(actor, (i_prop->scale != 0) ? ((f32)i_prop->scale * 0.01) : 1.0);
                }
            }
            i_prop++;
        }
        func_80326C24(0);
    }
}

// init "special" cubes?
void func_803303B8(Cube *cube) {
    s32 position[3];
    NodeProp *last_node_prop_ptr;
    NodeProp *current_node_ptr;

    if ((cube != NULL) && (cube->prop1Cnt != 0)){
        current_node_ptr = cube->prop1Ptr;
        last_node_prop_ptr = cube->prop1Ptr + cube->prop1Cnt;
        while (current_node_ptr < last_node_prop_ptr) {
            if (current_node_ptr->bit6 == PROP_1_CATEGORY_7_ENEMY_BOUNDARY) {
                position[0] = (s32) current_node_ptr->x;
                position[1] = (s32) current_node_ptr->y;
                position[2] = (s32) current_node_ptr->z;
                func_803065E4(current_node_ptr->unk8, position, current_node_ptr->radius, current_node_ptr->unk10_31, current_node_ptr->pad10_7);
            } else if (current_node_ptr->bit6 == PROP_1_CATEGORY_9_CAMERA_TRIGGER) {
                position[0] = (s32) current_node_ptr->x;
                position[1] = (s32) current_node_ptr->y;
                position[2] = (s32) current_node_ptr->z;
                func_8030688C(current_node_ptr->unk8, position, current_node_ptr->radius, current_node_ptr->unk10_0);
            } else if (current_node_ptr->bit6 == PROP_1_CATEGORY_A_FLAG) {
                position[0] = (s32) current_node_ptr->x;
                position[1] = (s32) current_node_ptr->y;
                position[2] = (s32) current_node_ptr->z;
                func_80306AA8(current_node_ptr->unk8, position, current_node_ptr->radius);
            }
            current_node_ptr++;
        }
    }
}

bool func_80330534(Actor *actor){
    if(D_80383444 != 0){
        return false;
    }
    
    D_80383444 = actor->modelCacheIndex;
    return true;
}

bool func_8033056C(Actor *actor){
    if((D_80383444 == 0) || (D_80383444 != actor->modelCacheIndex)){
        return false;
    }
    D_80383444 = 0;
    return true;
}

//modelCache_Init
void func_803305AC(void){
    s32 i;

    modelCache = (ModelCache *)bk_malloc(AssetCacheSize * sizeof(ModelCache));
    for(i = 0; i<AssetCacheSize; i++){
        modelCache[i].modelPtr = NULL;
        modelCache[i].unk4 = 0;
        modelCache[i].unk8 = 0;
        modelCache[i].animated_texture_cache_id = 0;
    }
    D_80383444 = 0;
}

void func_803306C8(s32 arg0) {
    static  s32 D_8036E7CC = 0;
    ModelCache *var_a2;
    s32 temp_a0;
    s32 sp54;
    s32 temp_fp;
    s32 var_s0_2;
    s32 var_s1;

    temp_fp = globalTimer_getTime() - func_80255B08(arg0);
    func_80254BD0(&sp54, 1);
    if(sp54 > 256000) return;
    
    for(var_s1 = 0; var_s1 < ((arg0 == 1) ? 0x28 : 0x3D4); var_s1++, D_8036E7CC = (D_8036E7CC >= 0x3D4) ? 0 : D_8036E7CC + 1) {
        var_a2 = modelCache + D_8036E7CC;
        
        if (((s32)var_a2->unk10 < temp_fp) 
            || ((arg0 == 3) && ((D_80383444 == 0) || (D_8036E7CC != D_80383444)))
        ) {
            var_s0_2 = false;
            if (var_a2->modelPtr != NULL) {
                assetcache_release(var_a2->modelPtr);
                modelCache[D_8036E7CC].modelPtr = NULL;
                var_s0_2 = true;
            }
            var_a2 = modelCache + D_8036E7CC;
            
            if (var_a2->unk4 != NULL) {
                if (!D_8036E7CC);

                var_s0_2 = true;
                codeB3A80_releaseSprite((void **)&var_a2->unk4, &var_a2->unk8);
            }
            if ((arg0 != 1) && (var_s0_2 == 1) && (func_80254BC4(1))) {
                return;
            }
        }
    }
}

void func_803308A0(void) {
    s32 i;
    ModelCache *var_s0;
    s32 var_a1;

    port_spriteDisplayCache_clear(); // [port] Clear stale display data pointers before freeing modelCache
    for (i = 0; i < 0x3D5; i++) {
        var_s0 = &modelCache[i];
        if (var_s0->modelPtr) {
            assetcache_release(var_s0->modelPtr);
            modelCache[i].modelPtr = 0;
        }
        
        var_s0 = &modelCache[i];
        if (var_s0->unk4) {
            func_8033B388(&var_s0->unk4, &var_s0->unk8);
        }
        
        var_s0 = &modelCache[i];
        var_a1 = var_s0->animated_texture_cache_id;
        if (var_a1) {
            AnimTextureListCache_freeList(var_a1);
            modelCache[i].animated_texture_cache_id = 0; 
        }
    }
    
    bk_free(modelCache);
    modelCache = 0;
}

s32 func_80330974(ActorMarker *marker, f32 arg1[3], f32 arg2, f32 arg3[3], s32 arg4_unused) {
    BKModelBin *model;
    BKModelUnk14List *sp58;
    f32 position[3];
    f32 rotation[3];
    f32 scale;


    model = func_80330DE4(marker);
    if (model == NULL) {
       return 0;
    }
    sp58 = modelbin_getUnk14List(model);
    position[0] = (f32) marker->propPtr->x;
    position[1] = (f32) marker->propPtr->y;
    position[2] = (f32) marker->propPtr->z;

    rotation[0] = (f32)marker->pitch;
    rotation[1] = (f32)marker->yaw;
    rotation[2] = (f32)marker->roll;

    scale = (marker->unk3E_0) ? marker_getActor(marker)->scale : 1.0f;
    if (animMtxList_getLength(marker->unk20)) {
        return bkmodelunk14list_func_802EBAE0(sp58, position, rotation, scale, 0, marker->unk20, arg1, arg2, arg3);
    }
    return 0;
}

Struct6Cs *func_80330B10(void){
    static Struct6Cs D_8036E7D0 = {NULL, NULL, NULL, (s32 (*)(struct actorMarker_s *, f32[3], f32, f32[3], s32))func_80330974};
    return &D_8036E7D0;
}

BKModelBin *marker_loadModelBin(ActorMarker *this){
    Actor* thisActor;
    BKModelBin * model;
    ModelCache *modelInfo;

    if(this->modelId == 0)
        return NULL;

    thisActor = marker_getActor(this);
    modelInfo = &modelCache[thisActor->modelCacheIndex];

    // [port] Let the language system invalidate a re-pointed model so it
    // re-fetches the active language's version (live swap on the next draw).
    s32 reload = 0;
    CALL_EVENT(OnModelLoad, this->modelId, modelInfo, &reload);
    if (reload && modelInfo->modelPtr != NULL) {
        assetcache_release(modelInfo->modelPtr);
        modelInfo->modelPtr = NULL;
    }

    if(modelInfo->modelPtr == NULL){
        model = assetcache_get(this->modelId);
        // [port] On N64, sprites and models were raw binary blobs that could be type-punned.
        // On PC, SpriteFactory produces BKSprite structs, not BKModelBin. Don't cache sprite
        // data as modelPtr — sprite rendering uses unk4/unk8 via func_80330E54 instead.
        if (model && !ResourceMgr_IsModelAsset(this->modelId)) {
            modelInfo->unk10 = globalTimer_getTime();
            return NULL;
        }
        modelInfo->modelPtr = model;
        if(modelbin_getAnimTextureList(model)){
            modelInfo->animated_texture_cache_id = AnimTextureListCache_newList();
            AnimTextureListCache_setAnimTextureList(modelInfo->animated_texture_cache_id, modelbin_getAnimTextureList(modelInfo->modelPtr));
        }
        func_8032ACA8(thisActor);
    }
    func_8032AB84(thisActor);
    if(!this->unk18 && this->propPtr->isModelProp && modelInfo->modelPtr && modelbin_getUnk14List(modelInfo->modelPtr)){
        this->unk18 = func_80330B10();
    }
    modelInfo->unk10 = globalTimer_getTime();
    return modelInfo->modelPtr;
}

s32 actor_getAnimatedTexturesCacheId(Actor *actor){
    ModelCache *model_cache_ptr = &modelCache[actor->modelCacheIndex];
    return model_cache_ptr->animated_texture_cache_id;
}

BKVertexList *func_80330C74(Actor *actor){
    ModelCache *model_cache_ptr;
    model_cache_ptr = &modelCache[actor->modelCacheIndex];
    if(model_cache_ptr->modelPtr == NULL)
        return NULL;
    // [port] On N64, sprite binary data could be read as model data. On PC they're different structs.
    // Return NULL for sprite assets to prevent modelbin_getVtxList from reading garbage offsets.
    if (!ResourceMgr_IsModelAsset(actor->marker->modelId))
        return NULL;

    if(actor->unkF4_30 && actor->unk14C[actor->unkF4_29]){
        return actor->unk14C[actor->unkF4_29];
    }else{
        return modelbin_getVtxList(model_cache_ptr->modelPtr);
    }
}

BKVertexList *func_80330CFC(Actor *this, s32 arg1){
    ModelCache *model_cache_ptr;
    model_cache_ptr = &modelCache[this->modelCacheIndex];
    if(model_cache_ptr->modelPtr == NULL){
        marker_loadModelBin(this->marker);
    }
    // [port] On N64, sprite binary data could be read as model data. On PC they're different structs.
    if (!ResourceMgr_IsModelAsset(this->marker->modelId))
        return NULL;
    if(this->unkF4_30 && this->unk14C[this->unkF4_29 ^ arg1] != NULL)
        return this->unk14C[this->unkF4_29 ^ arg1];
    return modelbin_getVtxList(model_cache_ptr->modelPtr);
}

BKVertexList * func_80330DA4(Actor *this){
    return func_80330CFC(this, 0);
}

BKVertexList * func_80330DC4(Actor *this){
    return func_80330CFC(this, 1);
}

BKModelBin *func_80330DE4(ActorMarker *this){
    // [port] On PC, sprite assets produce BKSprite, not BKModelBin.
    // Return NULL so callers (collision, mesh, vtx) don't interpret sprite data as model data.
    if (!ResourceMgr_IsModelAsset(this->modelId))
        return NULL;
    Actor *thisActor = marker_getActor(this);
    return (modelCache + thisActor->modelCacheIndex)->modelPtr;
}

BKModelBin *func_80330E28(Actor* this){
    return (modelCache + this->modelCacheIndex)->modelPtr;
}

BKSpriteDisplayData *func_80330E54(ActorMarker *marker, BKSprite **sprite_ptr) {
    ModelCache *model_cache_ptr;

    if (marker->modelId == 0) {
        if (sprite_ptr != NULL && *sprite_ptr != NULL) { // [port] sprite_ptr can be NULL from func_80330F30 — on N64 *NULL reads phys addr 0 (benign), on PC it crashes
            *sprite_ptr = NULL;
        }
        return 0;
    }
    model_cache_ptr = &modelCache[marker_getActor(marker)->modelCacheIndex];
    if (model_cache_ptr->unk4 == 00 || marker->id == MARKER_300_CUSTOM_COLLECTIBLE) {
        model_cache_ptr->unk4 = codeB3A80_getSprite(marker->modelId, &model_cache_ptr->unk8);
    }
    model_cache_ptr->unk10 = globalTimer_getTime();
    if (sprite_ptr != NULL) {
        *sprite_ptr = model_cache_ptr->unk4;
    }
    return model_cache_ptr->unk8;
}

BKSpriteDisplayData *func_80330F30(ActorMarker *marker){
    return func_80330E54(marker, NULL);
}

BKSprite *func_80330F50(ActorMarker * marker){
    BKSprite *sp1C;
    func_80330E54(marker, &sp1C);
    return sp1C;
}

s32 codeA5BC0_getNodePropMarkerId(NodeProp *arg0){
    return arg0->unkA; //marker_id
}

s32 codeA5BC0_getNodePropCategory(NodeProp *arg0){
    return arg0->bit6;
}

s32 codeA5BC0_getNodePropActorId(NodeProp *arg0){
    return arg0->unk8;
}

s32 codeA5BC0_getPositionAndSelectorOrRadius(void *arg0_, s32 arg1[3]){
    NodeProp *arg0 = (NodeProp *)arg0_;
    arg1[0] = arg0->x;
    arg1[1] = arg0->y;
    arg1[2] = arg0->z;
    return arg0->radius;
}

// is used to set global timer time?
void codeA5BC0_setNodePropUnkC(NodeProp *arg0, s32 arg1){
    // writes unkC_31 / unkC_22
    *(s32*)((uintptr_t)arg0 + 0xC) = arg1;
}

// is used to retrieve global timer time?
s32 codeA5BC0_getNodePropUnkC(NodeProp *arg0){
    // reads unkC_31 / unkC_22
    return *(s32*)((uintptr_t)arg0 + 0xC);
}

void func_80330FCC(ActorMarker *marker, s32 arg1[3]){
    arg1[0] = marker->propPtr->x;
    arg1[1] = marker->propPtr->y;
    arg1[2] = marker->propPtr->z;
}

void func_80330FF4(void){
    Actor *phi_s0;
    f32 sp48[3];
    f32 scale[3];

    if(gsworld_getUnk0() == 1)
        return;
    
    if(D_8038341C != NULL){
        phi_s0 = marker_getActor(D_8038341C);
        player_getPosition(sp48);
        scale[0] = phi_s0->pitch;
        scale[1] = phi_s0->yaw;
        scale[2] = phi_s0->roll;
        func_80340200(D_8038341C->unk50, phi_s0->position, scale, 1.0f, NULL, D_80383420->unk0, func_80330DA4(phi_s0), D_80383410);
    }//L8033108C

    spawnQueue_func_802C39D4();
    if(D_8038341C != NULL){
        phi_s0 = marker_getActor(D_8038341C);
        player_getPosition(sp48);
        scale[0] = phi_s0->pitch;\
        scale[1] = phi_s0->yaw;\
        scale[2] = phi_s0->roll;
        if(func_80340020(D_8038341C->unk50, phi_s0->position, scale, 1.0f, NULL, func_80330DC4(phi_s0), sp48, sp48)){
            sp48[1] -= 10.0f;
            func_8028FAB0(sp48);
        }
    }
    if(D_8038341C != NULL){
        D_8038341C = NULL;
    }
    //L80331144
}

bool func_80331158(ActorMarker *arg0, f32 *arg1, f32 *arg2) {
    Actor *actor;
    u32 temp_a0;

    actor = marker_getActor(arg0);
    if ((actor->unk3C & 0x400) && ((s32)actor->unk3C << 4) >= 0){
        return func_802E74A0(actor->position, actor->unk178 * 1.1, arg1, arg2) == 0;
    }
    return false;
}

BKCollisionTriangle *func_803311D4(Cube *arg0, f32 *arg1, f32 *arg2, f32 *arg3, u32 arg4) {
   Actor *temp_s2_2;
    ActorMarker *temp_a0;
    BKModelBin *var_a0;
    BKModelBin *var_s0 = NULL;
    BKVertexList *temp_a1;
    Prop *var_s1;
    f32 spAC[3];
    f32 spA0[3];
    BKCollisionList *temp_s0;
    BKCollisionTriangle *temp_s0_2;
    BKCollisionList *temp_s2;
    
    f32 sp88[3];
    f32 sp7C[3];

    BKCollisionTriangle *var_s6;
    BKCollisionTriangle *var_v0;
    u32 var_s5;

    var_s6 = NULL;
    for(var_s1 = arg0->prop2Ptr, var_s5 = arg0->prop2Cnt; var_s5 > 0; var_s5--, var_s1++) {
        if(var_s1);

        if (!var_s1->markerFlag && var_s1->unk8_1 && var_s1->unk8_4) { //ModelProp
            var_s0 = propModelList_getModelIfActive(var_s1->modelProp.modelId);
            if ((var_s0 != NULL) || (func_8028F280() && ((var_s0 = propModelList_getModel(var_s1->modelProp.modelId)) != NULL))) {
                temp_s2 = modelbin_getCollisionList(var_s0);
                if (temp_s2 != 0) {
                    spAC[0] = (f32) var_s1->modelProp.unk4[0];
                    spAC[1] = (f32) var_s1->modelProp.unk4[1];
                    spAC[2] = (f32) var_s1->modelProp.unk4[2];
                    spA0[0] = 0.0f;
                    spA0[1] = (f32) (var_s1->modelProp.yaw * 2);
                    spA0[2] = (f32) (var_s1->modelProp.roll * 2);
                    var_v0 = collisionList_func_802E805C(temp_s2, modelbin_getVtxList(var_s0), spAC, spA0, (f32)var_s1->modelProp.scale / 100.0, arg1, arg2, arg3, arg4);
                    if (var_v0 != NULL) {
                        var_s6 = var_v0;
                    }
                }
            }
        } else if (var_s1->markerFlag && var_s1->unk8_3 && var_s1->unk8_4 && !func_80331158(var_s1->actorProp.marker, arg1, arg2)) {
            if (!(var_s1->actorProp.marker->unk3E_0 && (marker_getActor(var_s1->actorProp.marker)->unk3C & 0x008000000))) {
                var_a0 = func_80330DE4(var_s1->actorProp.marker);
            } else {
                var_a0 = marker_loadModelBin(var_s1->actorProp.marker);
            }
            if(var_a0 != NULL || (func_8028F280() && (var_a0 = marker_loadModelBin(var_s1->actorProp.marker), true))){
                temp_s0 = modelbin_getCollisionList(var_a0);
                if (temp_s0 != 0) {
                    temp_s2_2 = marker_getActor(var_s1->actorProp.marker);
                    temp_a1 = func_80330C74(temp_s2_2);
                    sp88[0] = (f32) var_s1->actorProp.x;
                    sp88[1] = (f32) var_s1->actorProp.y;
                    sp88[2] = (f32) var_s1->actorProp.z;
                    sp7C[0] = (f32) var_s1->actorProp.marker->pitch;
                    sp7C[1] = (f32) var_s1->actorProp.marker->yaw;
                    sp7C[2] = (f32) var_s1->actorProp.marker->roll;
                    temp_s0_2 = collisionList_func_802E805C(temp_s0, temp_a1, sp88, sp7C, temp_s2_2->scale, arg1, arg2, arg3, arg4);
                    if ((temp_s0_2 != NULL) && (func_8029453C())) {
                        marker_loadModelBin(var_s1->actorProp.marker);
                        if (var_s1->actorProp.marker->unk50 != 0) {
                            D_80383410[0] = arg2[0];
                            D_80383410[1] = arg2[1];
                            D_80383410[2] = arg2[2];
                            D_8038341C = var_s1->actorProp.marker;
              if ((temp_s0_2 && temp_s0_2) && temp_s0_2)
              {
              }
                            D_80383420 = temp_s0_2;
                        }
                    }
                    if (temp_s0_2 != NULL) {
                        var_s6 = temp_s0_2;
                    }
                }
            }
        } else if (var_s1->markerFlag) {
            if (var_s1->actorProp.marker->unk18 != NULL) {
                if (var_s1->actorProp.marker->unk18->unk0 != NULL) {
                    var_v0 = var_s1->actorProp.marker->unk18->unk0(var_s1->actorProp.marker, arg1, arg2, arg3, arg4);
                    if (var_v0 != 0) {
                        var_s6 = var_v0;
                    }
                }
            }
        }
    }
    return var_s6;
}

BKCollisionTriangle *func_80331638(Cube *cube, f32 arg1[3], f32 arg2[3], f32 arg3, f32 arg4[3], s32 arg5, u32 flags)
{
  f32 *new_var2;
  BKCollisionTriangle *spD8;
  Actor *temp_v0_6;
  BKModelBin *model_bin;
  BKVertexList *temp_a1;
  BKCollisionList *model_collision_list;
  f32 spBC[3];
  f32 spB0[3];
  u32 var_s3;
  f32 *new_var;
  BKModelBin *pad9C;
  f32 sp98[3];
  f32 sp8C[3];
  BKCollisionTriangle *var_v0;
  ActorMarker *temp_a0;
  Struct6Cs *temp_v0_7;
  Prop *var_s0;
    
  spD8 = NULL;
  var_s0 = cube->prop2Ptr;
  var_s3 = cube->prop2Cnt;
  new_var2 = sp8C;
  for (; var_s3 != 0; var_s0++, var_s3--)
  {
    if (((!var_s0->markerFlag) && var_s0->unk8_1) && var_s0->unk8_4)
    {
      model_bin = propModelList_getModelIfActive(var_s0->modelProp.modelId);
      if (model_bin == 0)
      {
        continue;
      }
      model_collision_list = modelbin_getCollisionList(model_bin);
      if (model_collision_list == 0)
      {
        continue;
      }
      spBC[0] = (f32) var_s0->modelProp.unk4[0];
      spBC[1] = (f32) var_s0->modelProp.unk4[1];
      spBC[2] = (f32) var_s0->modelProp.unk4[2];
      spB0[0] = 0.0f;
      spB0[1] = (f32) (var_s0->modelProp.yaw * 2);
      new_var = spB0;
      spB0[2] = (f32) (var_s0->modelProp.roll * 2);
      var_v0 = func_802E9118(model_collision_list, modelbin_getVtxList(model_bin), 
        spBC, new_var, (f32) (((f32) var_s0->modelProp.scale) / 100.0), 
        arg1, arg2, arg3, arg4, arg5, flags
    );
      if (var_v0 != 0)
      {
        spD8 = var_v0;
      }
    }
    else
      if ((var_s0->markerFlag && var_s0->unk8_3) && var_s0->unk8_4)
    {
      model_collision_list = (BKCollisionList *)func_80330DE4(var_s0->actorProp.marker);
      pad9C = (BKModelBin *)model_collision_list;
      if (model_collision_list == 0)
      {
        continue;
      }
      model_bin = (BKModelBin *)modelbin_getCollisionList(pad9C);
      if (model_bin == 0)
      {
        continue;
      }
      temp_v0_6 = marker_getActor(var_s0->actorProp.marker);
      temp_a1 = func_80330C74(temp_v0_6);
      {
        sp98[0] = (f32) var_s0->actorProp.x;
        sp98[1] = (f32) var_s0->actorProp.y;
        sp98[2] = (f32) var_s0->actorProp.z;
        sp8C[0] = (f32) var_s0->actorProp.marker->pitch;
        sp8C[1] = (f32) var_s0->actorProp.marker->yaw;
        sp8C[2] = (f32) var_s0->actorProp.marker->roll;
        var_v0 = func_802E9118((BKCollisionList *)model_bin, temp_a1, sp98, new_var2, temp_v0_6->scale, arg1, arg2, arg3, arg4, arg5, flags);
      }
      if (var_v0 != 0)
      {
        spD8 = var_v0;
      }
    }
    else
      if (var_s0->markerFlag)
    {
      temp_a0 = var_s0->actorProp.marker;
      temp_v0_7 = temp_a0->unk18;
      if (temp_v0_7 == 0)
      {
        continue;
      }
      if (temp_v0_7->unk4 != 0)
      {
        var_v0 = temp_v0_7->unk4(temp_a0, arg1, arg2, arg3, arg4, arg5, flags);
        if (var_v0 != 0)
        {
          spD8 = var_v0;
        }
      }
    }
  }

  return spD8;
}

BKCollisionTriangle *func_803319C0(Cube *cube, f32 position[3], f32 radius, f32 arg3[3], u32 arg4){
    BKCollisionTriangle *var_s7;
    BKCollisionTriangle *var_v0;
    s32 var_s3;
    Prop *var_s0;
    BKCollisionList *model_collision_list;
    BKVertexList *temp_a1;
    f32 spAC[3];
    f32 spA0[3];
    BKModelBin *model_bin;
    BKModelBin *new_var;
    ModelProp *mProp;
    f32 sp88[3];
    f32 sp7C[3];
    Actor *temp_v0_6;
    ActorProp *aProp;
    
    var_s7 = 0;
    var_s0 = cube->prop2Ptr;
    for (var_s3 = cube->prop2Cnt; var_s3 != 0; var_s3--, var_s0++) {
        if (((!var_s0->markerFlag) && var_s0->unk8_1) && var_s0->unk8_4)
        {
            mProp = &var_s0->modelProp;
            new_var = propModelList_getModelIfActive(mProp->modelId); 
            if (1) { } if (1) { } if (1) { }
            model_bin = new_var;
            if (model_bin != 0){
                model_collision_list = modelbin_getCollisionList(model_bin);
                if (model_collision_list != 0){
                    spAC[0] = (f32) mProp->unk4[0];
                    spAC[1] = (f32) mProp->unk4[1];
                    spAC[2] = (f32) mProp->unk4[2];
                    spA0[0] = 0.0f;
                    spA0[1] = (f32) (mProp->yaw * 2);
                    model_bin = model_bin;
                    spA0[2] = (f32) (mProp->roll * 2);
                    var_v0 = func_802E9DD8(model_collision_list, modelbin_getVtxList(model_bin), spAC, spA0, ((f32) mProp->scale) / 100.0, position, radius, arg3, arg4);
                    if (var_v0 != 0)
                        var_s7 = var_v0;
                }
            }
        } else {
            aProp = &var_s0->actorProp;
            if ((var_s0->markerFlag && var_s0->unk8_3) && var_s0->unk8_4)
            {
            model_bin = func_80330DE4(aProp->marker);
            if (model_bin != 0) {
            new_var = (BKModelBin *)modelbin_getCollisionList(model_bin);
            if (new_var != 0)
            {
            temp_v0_6 = marker_getActor(aProp->marker);
            temp_a1 = func_80330C74(temp_v0_6);
            sp88[0] = (f32) aProp->x;
            sp88[1] = (f32) aProp->y;
            sp88[2] = (f32) aProp->z;
            sp7C[0] = aProp->marker->pitch;
            sp7C[1] = aProp->marker->yaw;
            sp7C[2] = aProp->marker->roll;
            var_v0 = func_802E9DD8((BKCollisionList *)new_var, temp_a1, sp88, sp7C, temp_v0_6->scale, position, radius, arg3, arg4);
            if (var_v0 != 0)
            {
            var_s7 = var_v0;
            }
            }
            }
            }
            else
            if (var_s0->markerFlag)
            {
            Struct6Cs *temp_v0_7;
            temp_v0_7 = aProp->marker->unk18;
            if (temp_v0_7 != 0)
            {
            if (temp_v0_7->unk8 != 0)
            {
            var_v0 = temp_v0_7->unk8(aProp->marker, position, radius, arg3, arg4);
            if (var_v0 != 0)
            {
            var_s7 = var_v0;
            }
            }
            }
            }
        }
    }
    
    return var_s7;
}

f32 func_80331D20(BKSprite *sprite) {
    BKSpriteFrame *frame;
    s32 temp_lo;
    s32 temp_lo_2;
    s32 phi_v0;

    if (sprite == 0) {
        return 1.0f;
    }
    frame = sprite_getFramePtr(sprite, 0);
    temp_lo =   (s32) (((frame->unk10 - frame->unkC) + 1) * sprite->unk8) / (s32) frame->w;
    temp_lo_2 = (s32) (((frame->unk12 - frame->unkE) + 1) * sprite->unkA) / (s32) frame->h;
    phi_v0 = (temp_lo_2 < temp_lo) ? temp_lo : temp_lo_2;
    return (phi_v0 <= 0) ? 1 : phi_v0;
}


f32 func_80331E34(Prop *arg0){
    return func_80331D20(propModelList_getSprite(arg0->spriteProp.spriteId));
}

f32 func_80331E64(ActorMarker *marker) {
    f32 sp24;

    if (marker->modelId == 0) {
        return 0.0f;
    }
    if (marker->id == MARKER_300_CUSTOM_COLLECTIBLE) {
        return 250.0f;
    }
    sp24 = func_80331D20(func_80330F50(marker));
    marker->unk38[1] = (s16) (sp24 / 2);
    if (marker->unk3E_0) {
        marker->unk38[1] *= marker_getActor(marker)->scale;
    }
    return sp24;
}


f32 func_80331F1C(Prop *arg0){
    // [port] propModelList_getModel can return NULL when asset is missing from o2r (N64 ROM always had it)
    // D_80382390 array is 0x2A2 elements; out-of-range index → crash after defrag moves the array
    s32 idx = arg0->modelProp.modelId;
    if (idx < 0 || idx >= 0x2A2) return 0.0f;
    BKModelBin *model = propModelList_getModel(idx);
    if (model == NULL) return 0.0f;
    // [port] modelbin_getVtxList offsets into the model blob; if vtx_list_offset is 0 it returns the header itself
    if (model->vtx_list_offset == 0) return 0.0f;
    BKVertexList *vtx = modelbin_getVtxList(model);
    if (vtx == NULL) return 0.0f;
    return vtxList_getGlobalNorm(vtx);
}

f32 func_80331F54(ActorMarker *marker) {
    f32 sp34;
    f32 model_center[3];
    BKModelBin *model;

    model = marker_loadModelBin(marker);
    if (model == NULL) {
        // [port] On N64, sprites and models were interchangeable raw blobs.
        // marker_loadModelBin returns NULL for sprite assets on PC.
        // Fall back to sprite-based radius so collectibles get a non-zero hitbox.
        return func_80331E64(marker);
    }
    // [port] On N64, sprites and models were raw binary blobs that could be type-punned.
    // On PC, sprite assets produce BKSprite structs, not BKModelBin. Skip model-specific
    // vertex bounds for sprite assets to prevent reading invalid model fields.
    if (!ResourceMgr_IsModelAsset(marker->modelId)) {
        return func_80331E64(marker);
    }
    vtxList_getCenterAndNorm(modelbin_getVtxList(model), model_center, &sp34);
    if (marker->unk3E_0) {
        model_center[0] = model_center[0] * marker_getActor(marker)->scale;\
        model_center[1] = model_center[1] * marker_getActor(marker)->scale;\
        model_center[2] = model_center[2] * marker_getActor(marker)->scale;
    }
    marker->unk38[0] = (s16) model_center[0];\
    marker->unk38[1] = (s16) model_center[1];\
    marker->unk38[2] = (s16) model_center[2];
    return sp34 * 2;
}

f32 func_80332050(Prop *prop, ActorMarker *marker, s32 arg2) {
    ActorMarker * phi_v0;
    f32 phi_f2;

    phi_v0 =(prop->markerFlag) ? prop->actorProp.marker : NULL;
    phi_f2 = prop->unk4[arg2] - (&marker->propPtr->x)[arg2] - marker->unk38[arg2];
    if (phi_v0 != NULL) {
        phi_f2 += phi_v0->unk38[arg2];
    }
    return phi_f2;
}


f32 func_803320BC(ActorProp *prop, f32 (*arg1)(ActorMarker *)) {
    ActorMarker *marker;
    f32 sp18;

    marker = prop->marker;
    sp18 = (f32)marker->unk14_10;
    if (sp18 == 0.0f) {
        sp18 = marker->unk14_10 = arg1(marker) * 0.5;
    }
    if (marker->unk3E_0) {
        sp18 *= marker_getActor(marker)->scale;
    }
    return sp18;
}

f32 func_80332220(Prop * prop, f32 (*arg1)(Prop *)) {
    f32 phi_f12;

    phi_f12 = propModelList_getScale(prop);
    if (phi_f12 == 0.0f) {
        propModelList_setScale(prop, phi_f12 = arg1(prop) * 0.5);
    }
    return phi_f12;
}


f32 func_8033229C(ActorMarker *marker) {
    ActorProp *prop;

    prop = marker->propPtr;
    if (prop->isModelProp) {
        return func_803320BC(prop, func_80331F54);
    }
    else{
        return func_803320BC(prop, func_80331E64);
    }
}



Prop *func_803322F0(Cube *cube, ActorMarker *marker, f32 arg2, s32 arg3, s32 *arg4) {
    Prop *phi_s1;
    f32 phi_f24;
    f32 phi_f20;
    f32 phi_f22;
    f32 phi_f2;
    s32 phi_s3;
    f32 sp74[3];
    f32 sp68[3];

    phi_s3 = cube->prop2Cnt - *arg4;
    if (marker->collidable) {
        phi_s1 = &cube->prop2Ptr[(*arg4)++];
        for(phi_s3 = phi_s3; phi_s3 != 0; phi_s3--){
            if (phi_s1->unk8_4) {
                if( phi_s1->markerFlag &&  (!phi_s1->actorProp.marker->unk3E_0 || !marker_getActor(phi_s1->actorProp.marker)->despawn_flag)){
                    if (phi_s1->actorProp.marker->collidable && (marker != phi_s1->actorProp.marker)) {
                        if( (phi_s1->actorProp.marker->modelId) 
                            && (func_803327A8(phi_s1->actorProp.marker->modelId) & arg3)
                        ) {
                            if( phi_s1->actorProp.isModelProp
                                && (phi_s1->actorProp.marker->unk18 != NULL)
                                && (phi_s1->actorProp.marker->unk18->unkC != NULL)
                            ) {
                                func_803320BC((ActorProp *)phi_s1, &func_80331F54);
                                sp68[0] = (f32) (marker->unk38[0] + marker->propPtr->x);
                                sp68[1] = (f32) (marker->unk38[1] + marker->propPtr->y);
                                sp68[2] = (f32) (marker->unk38[2] + marker->propPtr->z);
                                if ((phi_s1->actorProp.marker->unk40_31 = phi_s1->actorProp.marker->unk18->unkC(phi_s1->actorProp.marker, sp68, arg2, sp74, 0)) != 0) {
                                    return phi_s1;
                                }
                            } else{
                                phi_f24 = func_80332050(phi_s1, marker, 0);
                                phi_f22 = func_80332050(phi_s1, marker, 2);
                                if (phi_s1->actorProp.isModelProp) {
                                    phi_f20 = func_80332050(phi_s1, marker, 1);
                                    phi_f2 = func_803320BC((ActorProp *)phi_s1, func_80331F54);
                                } else {
                                    phi_f20 = func_80332050(phi_s1, marker, 1);
                                    phi_f2 = func_803320BC((ActorProp *)phi_s1, &func_80331E64);
                                }
                                phi_f2 = phi_f2 + arg2;
                                if ((phi_f24*phi_f24 + phi_f20*phi_f20 +  phi_f22*phi_f22) < phi_f2*phi_f2) {
                                    return phi_s1;
                                }
                            }
                        }
                    }
                }
                else if (phi_s1->unk8_1) {//ModelProp
                    if (func_803327A8(phi_s1->modelProp.modelId + 0x2D1) & arg3) {
                        phi_f24 = func_80332050(phi_s1, marker, 0);
                        phi_f20 = func_80332050(phi_s1, marker, 1) + func_80332220(phi_s1, &func_80331F1C);
                        phi_f22 = func_80332050(phi_s1, marker, 2);
                        phi_f2 = func_80332220(phi_s1, &func_80331F1C) + arg2;
                        if (( (phi_f24 * phi_f24) + (phi_f20 * phi_f20) + (phi_f22 * phi_f22)) < (phi_f2 * phi_f2)) {
                            return phi_s1;
                        }
                    }
                }
                else{
                    if (func_803327A8(phi_s1->spriteProp.spriteId + 0x572) & arg3) {
                        phi_f24 = func_80332050(phi_s1, marker, 0);\
                        phi_f20 = func_80332050(phi_s1, marker, 1) + func_80332220(phi_s1, &func_80331E34);\
                        phi_f22 = func_80332050(phi_s1, marker, 2);
                        phi_f2 = func_80332220(phi_s1, &func_80331E34) + arg2;
                        if (((phi_f24 * phi_f24) + (phi_f20 * phi_f20) + (phi_f22 * phi_f22)) < (phi_f2 * phi_f2)) {
                            return phi_s1;
                        }
                    }
                }
            }
            phi_s1++;
            (*arg4)++;
        }
    }
    *arg4 = -1;
    return NULL;
}


void func_80332764(s32 arg0, s32 arg1) {
    u8 *actor;

    D_8036E7C4[arg0 >> 2] |= arg1 << ((arg0 & 3) * 2);
}

void func_80332790(s32 arg0){
    D_8036E7C4[arg0 >> 2] = 0;
}

s32 func_803327A8(s32 arg0) {
    return ((s32) D_8036E7C4[arg0 >> 2] >> ((arg0 & 3) * 2)) & 3;
}

void func_803327D4(s16 *arg0, s32 arg1) {
    s32 i;

    for(i = 0; arg0[i] != -1; i++){
        func_80332764(arg0[i], arg1);
    }
}

void func_8033283C(s32 arg0, s32 arg1, s32 arg2) {
    s32 i;

    for(i = arg0; i < arg1; i++){
        func_80332764(i, arg2);
    }
}

void func_80332894(void) {
    static s16 D_8036E7E0[] = {0x2D2, 0x2DD, 0x580, 0x6D1, 0x6D6, 0x6D7, 0x364, 0x2E8, 0x309, 0x30A, 0x704, 0x30D, 0x6C7, -1};
    static s16 D_8036E7FC[] ={-1};
    s32 size;
    s32 i;

    size = VER_SELECT(0x579, 0x391, 0, 0);
    D_8036E7C4 = malloc(size);
    i = 0;
    do{
        D_8036E7C4[i] = 0;
        i++;
    }while(i != size);
    func_8033283C(0x34D, 0x3A6, 3);
    func_8033283C(0x3A6, 0x572, 3);
    func_803327D4(D_8036E7E0, 1);
    func_803327D4(D_8036E7FC, 2);
    func_8032D330();
}

void func_8033297C(void){
    bk_free(D_8036E7C4);
    D_8036E7C4 = NULL;
    func_8032D36C();
}

//MarkerList_Init
void func_803329AC(void){
    s32 i;
    
    D_8036E7C8 = (ActorMarker *)bk_malloc(MARKER_POOL_SIZE * sizeof(ActorMarker));

    for( i = 0; i < MARKER_BITMAP_BYTES; i++){
        D_80383428[i] = 0;
    }
       
    for(i =0; i< MARKER_POOL_SIZE; i++){
        D_8036E7C8[i].unk5C = 0;
    }
}

void func_80332A38(void){
    bk_free(D_8036E7C8);
    D_8036E7C8 = NULL;
}

//MarkerList_getFreeMarker
ActorMarker * func_80332A60(void){
    static s32 D_8036E800 = 0x387FB;
    int i;
    int j;
    int tmp_a2;
    ActorMarker *marker;

    for(i = 0; i < MARKER_BITMAP_BYTES && D_80383428[i] == 0xff; i++);
    if(i == MARKER_BITMAP_BYTES)
        return NULL;

    tmp_a2 = 0x80;
    for(j = 0; D_80383428[i] & tmp_a2; j++){tmp_a2 >>= 1;}
    D_80383428[i] |= tmp_a2;
    marker = D_8036E7C8 + 8*i + j;
    marker->unk5C = D_8036E800;
    D_8036E800++;
    return marker;
}

void func_80332B2C(ActorMarker * arg0){
    static s32 D_8036E804[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};

    s32 index = (arg0 - D_8036E7C8);
    arg0->unk5C = 0;
    D_80383428[index >> 3] =  D_80383428[index >> 3] & D_8036E804[index & 7];
}

void func_80332B7C(void){
    return;
}
