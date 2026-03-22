#include "core2/sprite_displaydata.h"
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#include "core2/anim/sprite.h"

extern u8 func_8033FA84(void); // [port] was s32 — returns u8
extern u8 commonParticleTypeMap_findFree(void); // [port] was s32 — returns u8
extern u8 func_80344CDC(void); // [port] was s32 — returns u8
extern void marker_setCommonParticleIndex(ActorMarker *, s32); // [port] was (s32, s32) — first param is ActorMarker* pointer, truncated on 64-bit
extern void commonParticleTypeMap_advanceParticleType(u8, enum common_particle_e); // [port] was (s32, u32)
extern void func_8033FFB8(u8, s32); // [port] was (s32, s32)
extern void projectile_getPosition(u8, f32[3]); // [port] was (s32, f32[3])
extern void func_8032F64C(f32[3] , ActorMarker *);
extern void projectile_freeByIndex(u8); // [port] was s32
extern void func_8033F7F0(u8 indx, Gfx **gfx, Mtx **mtx, Vtx **vtx);
extern void commonParticleTypeMap_freeByIndex(u8); // [port] was s32
extern void func_80344D70(u8); // [port] was s32
extern void commonParticleTypeMap_updateByIndex(u8); // [port] was s32
extern ActorMarker * func_8032FBE4(f32 *pos, MarkerDrawFunc arg1, int arg2, enum asset_e model_id);

extern void func_80352614(void);
extern void func_8035261C(void);
extern void func_803526DC(void);
extern void func_80355D58(void);
extern void func_80355E80(void);
extern void func_80355D50(void);
extern void fxegg_head_spawn(void);
extern void fxegg_head_update(void);
extern void fxegg_head_destroy(void);
extern void func_803546E8(void);
extern void func_8035489C(void);
extern void func_80354990(void);
extern void fxegg_ass_spawn(void);
extern void fxegg_ass_update(void);
extern void fxegg_ass_destroy(void);
extern void func_8035611C(void);
extern void func_803562E8(void);
extern void func_80356364(void);
extern void func_80352DE4(void);
extern void func_80352F58(void);
extern void func_80352FF4(void);
extern void jiggyShine_init(void);
extern void jiggyShine_update(void);
extern void jiggyShine_free(void);
extern void func_80354DD0(void);
extern void func_80354EEC(void);
extern void func_80355004(void);
extern void func_8035500C(void);
extern void func_80355134(void);
extern void func_80355294(void);
extern void func_803540B4(void);
extern void func_803541D8(void);
extern void func_803540AC(void);
extern void func_8035529C(void);
extern void func_803553E8(void);
extern void func_80355548(void);
extern void func_80355550(void);
extern void func_8035570C(void);
extern void func_8035585C(void);
extern void func_803543FC(void);
extern void func_8035451C(void);
extern void func_803543F4(void);
extern void func_80355864(void);
extern void func_80355B00(void);
extern void func_80355C4C(void);

typedef struct {
    u8 unk0;
    u8 unk1;
    //u8 pad2[0x2];
    f32 unk4;
}Struct_Core2_B6CE0_1;

void freeParticleByIndex(s32 arg0);

/* .data */
Struct_Core2_B6CE0_1 D_80371E30[] ={
    {0, 0, 0.0f},
    {1, 0, 0.07f},
    {2, 1, 0.29f},
    {3, 2, 0.15f},
    {4, 1, 0.05f}
};

/* .bss */
ParticleStruct0s particles[40];
s32 currentIndex;
struct {
    s32 unk0;
    s32 unk4;
} indexStash;
u8 commonParticleActive;

/* .code */
f32 func_8033DE30(s32 arg0){
    return D_80371E30[arg0].unk4;
}

s32 func_8033DE44(s32 arg0){
    return D_80371E30[arg0].unk1 & 1;
}

Actor *commonParticle_markerDrawFunction(ActorMarker *marker, Gfx **gfx, Mtx **mtx, Vtx **vtx){ // [port] restored full MarkerDrawFunc signature
    int indx = marker->unk28;
    func_8033F7F0(particles[indx].unk45, gfx, mtx, vtx);
    if(marker);
    return 0;
}

void commonParticle_init(void){
    int i;
    for(i = 0; i < 40 ;i++){
        particles[i].unk44 = 0;
    }
    indexStash.unk0 =  indexStash.unk4 = 0;
    commonParticleType_set(COMMON_PARTICLE_1_EGG_HEAD,  fxegg_head_spawn, fxegg_head_update, fxegg_head_destroy, 0, 1); //bsbEggAss
    commonParticleType_set(0x2,  func_803546E8, func_8035489C, func_80354990, 0, 8); //bsbWhirl //aka wonderwing
    commonParticleType_set(COMMON_PARTICLE_4_EGG_ASS,  fxegg_ass_spawn, fxegg_ass_update, fxegg_ass_destroy, 0, 1);
    commonParticleType_set(0x6,  func_8035611C, func_803562E8, func_80356364, 0, 8);
    commonParticleType_set(0x7,  func_80352DE4, func_80352F58, func_80352FF4, 0, 8);
    commonParticleType_set(0x8,  jiggyShine_init, jiggyShine_update, jiggyShine_free, 0, 8);
    commonParticleType_set(0x9,  func_80354DD0, func_80354EEC, func_80355004, 0, 8); //orange_pad?
    commonParticleType_set(0xa,  func_8035500C, func_80355134, func_80355294, 0, 8);
    commonParticleType_set(0xb,  func_803540B4, func_803541D8, func_803540AC, 0, 8);
    commonParticleType_set(0xc,  func_8035529C, func_803553E8, func_80355548, 0, 8);
    commonParticleType_set(0xd,  func_80355550, func_8035570C, func_8035585C, 0, 8);
    commonParticleType_set(0xe,  func_803543FC, func_8035451C, func_803543F4, 0, 8);
    commonParticleType_set(0xf,  func_80355864, func_80355B00, func_80355C4C, 0, 8);
    commonParticleType_set(0x10, func_80355D58, func_80355E80, func_80355D50, 0, 8);
    commonParticleType_set(0x11, func_8035261C, func_803526DC, func_80352614, 0, 8); //mumbotoken sparkle
}

void commonParticle_freeAllParticles(void){
    int i;
    for(i = 0; i < 40; i++){
        if(particles[i].unk44){
            freeParticleByIndex(i);
        }
    }
}

//commonParticle_update
void commonParticle_update(void){
    f32 sp4C[3];
    int i;
    if(commonParticleActive){
        for(i = 0; i < 40; i++){
            if(particles[i].unk44){
                currentIndex = i;
                commonParticleTypeMap_updateByIndex(particles[currentIndex].unk46);
                if(particles[currentIndex].unk44){
                    projectile_getPosition(particles[currentIndex].unk45, sp4C);
                    func_803451B0(particles[currentIndex].unk47, sp4C);
                    projectile_setPosition(particles[currentIndex].unk45, sp4C);
                    animsprite_update(particles[currentIndex].unk34);
                    func_8033FFB8(particles[currentIndex].unk45, animsprite_get_frame(particles[currentIndex].unk34));
                    func_8032F64C(sp4C, particles[currentIndex].marker_30);
                }
                else{
                    freeParticleByIndex(i);
                }
            }
        }
    }
}

//commonParticle_findFree
s32 commonParticle_findFree(void){
    int i;
    for(i = 0; i < 40; i++){
        if(particles[i].unk44 == 0){
            particles[i].unk44++;
            return i;
        }
    }
    return -1;
}

//commonParticle_new
int commonParticle_new(enum common_particle_e particle_id, int arg1){
    f32 sp34[3];
    uintptr_t a0; // [port] was s32 — reused for u8 indices and AnimSprite* pointer

    if(arg1 == 0)
        return -1;
    
    ml_vec3f_clear(sp34);
    currentIndex = commonParticle_findFree();
    if(currentIndex < 0)
        return -1;

    
    particles[currentIndex].unk45 = func_8033FA84();
    particles[currentIndex].unk34 = animsprite_new();
    particles[currentIndex].unk46 = commonParticleTypeMap_findFree();
    particles[currentIndex].unk47 = func_80344CDC();
    
    if( ( !(a0 = particles[currentIndex].unk45)
          || !particles[currentIndex].unk34
          || !particles[currentIndex].unk46
          || !particles[currentIndex].unk47
        )
    ){//L8033E4DC
        if(a0){
            projectile_freeByIndex(a0);
        }
        a0 = (uintptr_t)particles[currentIndex].unk34; // [port] AnimSprite* to uintptr_t
        if(a0){
            animsprite_free((AnimSprite *)a0); // [port] uintptr_t back to AnimSprite*
        }
        a0 = particles[currentIndex].unk46;
        if(a0){
            commonParticleTypeMap_freeByIndex(a0);
        }
        a0 = particles[currentIndex].unk47;
        if(a0){
            func_80344D70(a0);
        }
        particles[currentIndex].unk44 = 0;
        return -1;
    }
    
    //L8033E5B4
    particles[currentIndex].marker_30 = func_8032FBE4(sp34, (MarkerDrawFunc)commonParticle_markerDrawFunction, 1, commonParticleType_80352C7C(particle_id));
    particles[currentIndex].marker_30->unk40_22 = 1;
    marker_setCommonParticleIndex(particles[currentIndex].marker_30, (u32)currentIndex);
    particles[currentIndex].marker_30->collidable = false;
    commonParticleTypeMap_advanceParticleType(particles[currentIndex].unk46, particle_id);
    func_8033FFB8(particles[currentIndex].unk45, animsprite_get_frame(particles[currentIndex].unk34));
    projectile_getPosition(particles[currentIndex].unk45, sp34);
    func_8032F64C(sp34, particles[currentIndex].marker_30);
    return currentIndex;
    
}

void freeParticleByIndex(s32 arg0){
    commonParticleTypeMap_freeByIndex(particles[arg0].unk46);
    func_80344D70(particles[arg0].unk47);
    projectile_freeByIndex(particles[arg0].unk45);
    animsprite_free(particles[arg0].unk34);
    marker_free(particles[arg0].marker_30);
    particles[arg0].marker_30 = NULL;
    particles[arg0].unk38 = 0;
    particles[arg0].unk44 = 0;
}

void commonParticle_add(ActorMarker *arg0, s32 arg1, FuncUnk40 arg2){ // [port] was s32 arg0 — pointer truncation on 64-bit
    s32 tmp_v0 = commonParticle_findFree();
    particles[tmp_v0].unk44--;
    particles[tmp_v0].unk38 = arg0;
    particles[tmp_v0].unk3C = arg1;
    particles[tmp_v0].unk40 = arg2;
}

void commonParticle_modifyCurrent(ActorMarker *arg0, s32 arg1, FuncUnk40 arg2){ // [port] was s32 arg0 — pointer truncation on 64-bit
    particles[currentIndex].unk38 = arg0;
    particles[currentIndex].unk3C = arg1;
    particles[currentIndex].unk40 = arg2;
}

void commonParticle_freeParticleByActorMarker(ActorMarker *arg0){ // [port] was s32 arg0 — pointer truncation on 64-bit
    int i;
    for(i = 0; i < 40; i++){
        if(particles[i].unk44 && arg0 == particles[i].unk38){
            freeParticleByIndex(i);
        }
    }
}

ActorMarker *func_8033E840(void){
    return particles[currentIndex].marker_30;
}

ActorMarker *commonParticle_getCurrentActorMarker(void){
    return particles[currentIndex].unk38;
}

FuncUnk40 func_8033E888(void){
    return particles[currentIndex].unk40;
}

s32 func_8033E8AC(void){
    return particles[currentIndex].unk3C;
}

u8 commonParticle_getCurrentProjectileIndex(void){
    return particles[currentIndex].unk45;
}

AnimSprite * commonParticle_getCurrentAnimSprite(void){
    return particles[currentIndex].unk34;
}

u8 commonParticle_getCurrentTypeMapIndex(void){
    return particles[currentIndex].unk46;
}

u8 func_8033E93C(void){
    return particles[currentIndex].unk47;
}

ParticleStruct0s *commonParticle_getCurrentParticle(void){
    return &particles[currentIndex];
}

void commonParticle_setCurrentInUseFalse(void){
    particles[currentIndex].unk44 = 0;
}

void commonParticle_freeParticleByIndex(s32 arg0){
    freeParticleByIndex(arg0);
}

void commonParticle_setCurrentIndex(s32 arg0){
    currentIndex = arg0;
}

void commonParticle_stashCurrentIndex(void){
    indexStash.unk4 = indexStash.unk0;
    indexStash.unk0 = currentIndex;
}

void commonParticle_applyIndexStash(void){
    currentIndex = indexStash.unk0;
    indexStash.unk0 = indexStash.unk4;
}

f32 func_8033EA14(s32 arg0){
    return *((f32 *)commonParticle_getCurrentParticle() + arg0);
}

void func_8033EA40(s32 arg0, f32 arg1){
    *((f32 *)commonParticle_getCurrentParticle() + arg0) = arg1;
}

void commonParticle_setActive(s32 arg0, s32 arg1){
    if(arg1 == 2)
        commonParticleActive = 1;
    else
        commonParticleActive = 0;
}
