// BanjoDecomp: core2/code_DAAA0.c
#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"

extern void anSeq_clear(VLA**);
extern void anSeq_PushStep_2Arg(VLA**, f32, void *, uintptr_t, uintptr_t);
extern void anSeq_PushStep_3Arg(VLA**, f32, void *, uintptr_t, uintptr_t, uintptr_t);
bk_vector(AnSeqElement) **anSeq_new(void);

//TODO import from cutscenes/code_0.h
extern Struct63s D_8038D904[];
extern Struct63s D_8038D94C[];
extern Struct63s D_8038D964[];
extern Struct63s D_8038D994[];
extern Struct63s D_8038D9AC[];
extern Struct63s D_8038D9E4[];
extern Struct63s D_8038D9F4[];
extern Struct63s D_8038DA14[];
extern Struct63s D_8038DA2C[];
extern Struct63s D_8038DA44[];
extern Struct63s D_8038DA74[];
extern Struct63s D_8038DA84[];
extern Struct63s D_8038DAAC[];
extern Struct63s D_8038DABC[];
extern Struct63s D_8038DACC[];
extern Struct63s D_8038DB04[];
extern Struct63s D_8038DB1C[];
extern Struct63s D_8038DB34[];
extern Struct63s D_8038DB44[];
extern Struct63s D_8038DB54[];
extern Struct63s D_8038DB8C[];
extern Struct63s D_8038DBDC[];
extern Struct63s D_8038DBF4[];
extern Struct63s D_8038DC0C[];
extern Struct63s D_8038DC2C[];
extern Struct63s D_8038DC4C[];
extern Struct63s D_8038DC3C[];
extern Struct63s D_8038DC64[];
extern Struct63s D_8038DC74[];

// TODO import from SM/code_0.h
extern Struct63s D_8038AAC0[];
extern Struct63s D_8038AAD0[];


/* .data */
Struct62s D_803731E0[0x20] = {
    { 0xAC, D_8038D904},
    { 0x9E, D_8038D904},
    { 0xB8, D_8038D94C},
    { 0xB4, D_8038D964},
    { 0xB2, D_8038D994},
    { 0xB0, D_8038D9AC},
    { 0xB1, D_8038D9E4},
    { 0xBB, D_8038D9F4},
    { 0x8F, D_8038DA14},
    { 0xA5, D_8038DA2C},
    {0x2ED, D_8038DA44},
    {0x2EF, D_8038DA74},
    {0x2EE, D_8038DA84},
    {0x2FA, D_8038DAAC},
    {0x2F1, D_8038DABC},
    {0x2F2, D_8038DACC},
    { 0xC2, D_8038DB04},
    { 0xC3, D_8038DB1C},
    {0x2F0, D_8038DB34},
    { 0xA8, D_8038DB44},
    {0x2FB, D_8038DB54},
    { 0xA7, D_8038DB8C},
    {0x2FC, D_8038DBDC},
    {0x2F5, D_8038AAC0},
    {0x2F4, D_8038AAD0},
    {0x301, D_8038DBF4},
    {0x2FE, D_8038DC0C},
    {0x302, D_8038DC2C},
    { 0x93, D_8038DC4C},
    { 0x94, D_8038DC3C},
    { 0x8E, D_8038DC64},
    { 0xBC, D_8038DC74}
};

/* .code */
void func_80361A30(f32 arg0[3], s32 arg1){
    if(viewport_isPointOutsideFrustum_vec3f(arg0)){
        gcsfx_playWithPitch(arg1 >> 16, ((arg1 >> 8) & 0xff)*0.0078125, (arg1 & 0xff)*128.0);
    }
}

void func_80361AB0(uintptr_t marker, uintptr_t arg1){
    Actor *actor;

    actor = marker_getActor(reinterpret_cast(ActorMarker *,marker));
    func_80361A30(actor->position, arg1);
}

void func_80361AE0(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2){
    Actor *this;
    f32 sp20[3];
    ActorMarker *marker;
    Vec3fArray *tmp;

    marker = reinterpret_cast(ActorMarker *,arg0);
    this = marker_getActor(marker);
    tmp = this->marker->unk44;
    if(tmp){
        vec3fArray_get_vec3f(tmp, arg1, sp20);
        if(ml_isZero_vec3f(sp20)){
            ml_vec3f_copy(sp20, this->position);
        }
    } else {//L80361B44
        ml_vec3f_copy(sp20, this->position);
    }
    func_80361A30(sp20, arg2);
}

void func_80361B68(uintptr_t arg0, uintptr_t arg1){
    ActorMarker *marker;
    f32 arg1_f;
    Actor *actor;

    marker = reinterpret_cast(ActorMarker *,arg0);
    actor = marker_getActor(marker);
    func_803246F0(actor->unk160, arg1);
}

void func_80361B98(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2){
    ActorMarker *marker;
    f32 arg1_f;
    Actor *actor;

    marker = reinterpret_cast(ActorMarker *,arg0);
    actor = marker_getActor(marker);
    func_80324770(actor->unk160, arg1, arg2);
}

void func_80361BD0(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2){
    ActorMarker *marker;
    f32 arg1_f;
    Actor *actor;

    marker = reinterpret_cast(ActorMarker *,arg0);
    actor = marker_getActor(marker);
    arg1_f = reinterpret_cast(f32,arg1);

    actor->unk1C[0] = arg1_f;
    actor->unk1C[1] = 0.0f;
    actor->unk124_11 = arg2;
}

void func_80361C24(VLA **arg0, f32 arg1, ActorMarker * arg2, s32 arg3){
    anSeq_PushStep_2Arg(arg0, arg1, func_80361B68, (uintptr_t)arg2, (uintptr_t)arg3);
}

void func_80361C64(VLA **arg0, f32 arg1, ActorMarker * arg2, s32 arg3, s32 arg4){
    anSeq_PushStep_3Arg(arg0, arg1, func_80361B98, (uintptr_t)arg2, (uintptr_t)arg3, arg4);
}

void func_80361CAC(VLA **arg0, f32 arg1, ActorMarker *arg2, f32 arg3){
    anSeq_PushStep_3Arg(arg0, arg1, func_80361BD0, (uintptr_t)arg2, (uintptr_t)reinterpret_cast(u32, arg3), 1); // [port] u32 intermediate avoids 8-byte read from 4-byte f32
}

void func_80361CF4(VLA **arg0, f32 arg1, ActorMarker *arg2, f32 arg3){
    anSeq_PushStep_3Arg(arg0, arg1, func_80361BD0, (uintptr_t)arg2, (uintptr_t)reinterpret_cast(u32, arg3), 2); // [port] u32 intermediate avoids 8-byte read from 4-byte f32
}

void func_80361D3C(VLA **arg0, f32 arg1, s32 arg2, s32 arg3){
    anSeq_PushStep_2Arg(arg0, arg1, func_80361AB0, (uintptr_t)arg2, (uintptr_t)arg3);
}

void func_80361D7C(VLA **arg0, f32 arg1, ActorMarker *arg2, s32 arg3, u32 arg4){
    anSeq_PushStep_3Arg(arg0, arg1, func_80361AE0, (uintptr_t)arg2, arg3, arg4);
}

void func_80361DC4(Actor *this){
    if(this->unk134){
        anSeq_free((void **)this->unk134);
    }
    this->unk134 = NULL;

    if(this->unk160){
        func_8032477C(this->unk160);
    }
}

void func_80361E10(Actor *this) {
    s32 phi_v0;

    for(phi_v0 = 0; phi_v0 < 0x20; phi_v0++){
        if(D_803731E0[phi_v0].unk0 == this->modelCacheIndex){
            this->unk108 = &D_803731E0[phi_v0];
            this->unk134 = anSeq_new();
            this->unk160 = func_8032479C();
            this->unk10C = 0;
            return;
        }
    }
    this->unk134 = 0;
    this->unk160 = 0;
}


void func_80361E9C(Actor *this){
    if( this->anctrl != NULL
        && this->unk134 != NULL
        && this->unk160 != NULL
    ){
      func_80324700(this->unk160);
    }
}

void func_80361EE0(Actor *this) {
    s32 i;
    s32 sp28;
    Struct62s *sp24;
    Struct63s *sp20;

    if (this->anctrl != NULL && this->unk134 != NULL) {
        sp24 = this->unk108;
        sp28 = anctrl_getIndex(this->anctrl);
        if (sp28 != (s32)(uintptr_t)this->unk10C) {
            this->unk10C = (MarkerCollisionFunc)(uintptr_t)sp28;
            anSeq_clear(this->unk134);
            for(sp20 = sp24->unk4; sp20->unk4 != NULL; sp20++){
                if (sp28 == sp20->unk0) {
                    sp20->unk4(this->unk134, this->marker);
                    break;
                }
            }
        }
        anSeq_update((void **)this->unk134, this->anctrl);
    }
}
