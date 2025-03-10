#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#include "core2/modelRender.h"


extern void spawnParticleEffect(f32[3], f32[3], s32[4], s32, f32, f32, s32, s32, s32);

typedef struct t_actorParticle{
    s32 unk0;
    BKModelBin *model_bin;
    f32 unk8[3];
    f32 unk14[3];
    f32 unk20[3];
    f32 unk2C;
    f32 unk30[3];
    ParticleEmitter *unk3C;
    s32 unk40[4];
    f32 unk50;
} s_actorParticle;

typedef struct t_actorParticleList{
    s_actorParticle *begin;
    s_actorParticle *current;
    s_actorParticle *end;
    s_actorParticle data[];
} s_actorParticleList;

Actor *actor_render(ActorMarker *marker, Gfx **gfx, Mtx **mtx, Vtx **vtx);
void actor_update(Actor *this);

/* .data */
f32 s_particleEffectScale = 0.5f;
f32 s_particleEffectSpeed = 30.0f;
f32 s_particleEffectDuration = 150.0f;
f32 s_particleEffectIntensity = 25.0f;
ActorInfo g_actorInfoType1 = { 0x56,  0x4A,   0x0, 0x2, 0x0, actor_update, actor_update_func_80326224, actor_render, 0, 0, 0.0f, 0};
ActorInfo g_actorInfoType2 = { 0x56,  0x4B,   0x0, 0x2, 0x0, actor_update, actor_update_func_80326224, actor_render, 0, 0, 0.0f, 0};
ActorInfo g_actorInfoType3 = { 0x56,   0xD,   0x0, 0x2, 0x0, actor_update, actor_update_func_80326224, actor_render, 0, 0, 0.0f, 0};
ActorInfo g_actorInfoType4 = { 0x56, 0x11F,   0x0, 0x2, 0x0, actor_update, actor_update_func_80326224, actor_render, 0, 0, 0.0f, 0};
ActorInfo g_actorInfoType5 = { 0x56, 0x14F,   0x0, 0x2, 0x0, actor_update, actor_update_func_80326224, actor_render, 0, 0, 0.0f, 0};
ActorInfo g_actorInfoType6 = { 0x56, 0x3AD,   0x0, 0x2, 0x0, actor_update, actor_update_func_80326224, actor_render, 0, 0, 0.0f, 0};
s32 a_defaultSpawnPosition[3] = {0,0,0};

/* .bss */
s32 s_actorCount;
s32 s_actorType;
u32 s_spawnColor;
u32 s_spawnColorAlpha;
u32 s_actorSpawnPosition;
u32 s_actorSpawnRotation;

/* .code */
void actor_freeResources(Actor *actor) {
    s_actorParticleList *phi_a1;
    s_actorParticle *phi_s0;

    phi_a1 = actor->unk40;
    for( phi_s0 = phi_a1->begin; phi_s0 < phi_a1->current; phi_s0++){
        if (phi_s0->model_bin != NULL) {
            assetcache_release(phi_s0->model_bin);
        }
        if (phi_s0->unk3C != 0) {
            func_802EE5E8(phi_s0->unk3C);
        }
        phi_s0->unk3C = NULL;
    }
    bk_free(actor->unk40);
}

Actor *actor_render(ActorMarker *marker, Gfx **gfx, Mtx **mtx, Vtx **vtx) {
    s_actorParticleList *temp_s1;
    s_actorParticle *phi_s0;
    f32 sp5C;
    Actor *sp58;
    u32 phi_v1;
    s32 phi_s4;

    sp58 = marker_getActorAndRotation(marker, &sp5C);
    temp_s1 = sp58->unk40;
    phi_s4 = FALSE;
    for(phi_s0 = temp_s1->begin; phi_s0 < temp_s1->current; phi_s0++){
        if ((phi_s0->unk0 != 0) && (phi_s0->model_bin != NULL)) {
            modelRender_setDepthMode(MODEL_RENDER_DEPTH_FULL);
            modelRender_draw(gfx, mtx, phi_s0->unk8, phi_s0->unk14, phi_s0->unk2C / 10.0f, NULL, phi_s0->model_bin);
            phi_s4 = TRUE;
        }
    }
    if (phi_s4 == FALSE) {
        marker_despawn(marker);
    }
    return sp58;
}

Actor *actor_initialize(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags){
    s_actorParticleList *s1 =  heap_malloc(sizeof(s_actorParticleList) + s_actorCount * sizeof(s_actorParticle));
    Actor *actor = actor_new(position, yaw, actorInfo, flags);
    f32 f24;
    f32 sp68[3];
    s_actorParticle * s0;

    actor->marker->collidable = FALSE;
    s1->begin = &s1->data[0];
    s1->current = &s1->data[0];
    s1->end = s1->begin + s_actorCount;

    for(s1->current = s1->begin; s1->current < s1->end; s1->current++){//L802C8670
        f24 = randf2(0.0f, 360.0f);
        s0 = s1->current;
        s0->unk0 = 2;
        s0->unk40[0] = s_spawnColor;
        s0->unk40[1] = s_spawnColorAlpha;
        s0->unk40[2] = s_actorSpawnPosition;
        s0->unk40[3] = s_actorSpawnRotation;

        TUPLE_ASSIGN(s0->unk30, 
            randf2(100.0f, 250.0f), 
            randf2(500.0f, 750.0f), 
            randf2(100.0f, 250.0f)
        );

        s0->unk8[0] = randf2(50.0f, 150.0f);
        s0->unk8[1] = 0.0f;
        s0->unk8[2] = 0.0f;
        switch(s_actorType){
        case 1: // 802C8740
            s0->unk30[0] = randf2(125.0f, 175.0f);
            s0->unk30[1] = randf2(400.0f, 600.0f);
            s0->unk30[2] = randf2(125.0f, 175.0f);
            s0->model_bin = (BKModelBin *) assetcache_get(0x2e7);
            s0->unk2C = 18.0f;
            break; 
        case 2: // 802C87A8
            s0->model_bin = (BKModelBin *) assetcache_get(0x344);
            s0->unk2C = 2.0f;
            break; 
        case 4: // 802C87C4
            s0->model_bin = (BKModelBin *) assetcache_get(0x345);
            s0->unk2C = 1.0f;
            s0->unk30[0] = randf2(20.0f, 100.0f);
            s0->unk30[1] = randf2(400.0f, 740.0f);
            s0->unk30[2] = randf2(20.0f, 100.0f);

            s0->unk8[0] = randf2(0.0f, 30.0f);
            s0->unk8[1] = 0.0f;
            s0->unk8[2] = 0.0f;
            break; 
        case 0: // 802C883C
            if(0.5 < randf())
                s0->model_bin = (BKModelBin *) assetcache_get(0x2d1);
            else
                s0->model_bin = (BKModelBin *) assetcache_get(0x2e5);

            s0->unk2C = randf2(5.0f, 12.0f);
            break;
        case 3: // 802C88A0
            s0->model_bin = (BKModelBin *) assetcache_get(0x30e);
            s0->unk2C = randf2(9.0f, 15.0f);
            break;
        case 5: // 802C88C8
            s0->model_bin = (BKModelBin *) assetcache_get(0x8a2);
            s0->unk2C = 2.0f;
            break;
        }//L802C88E0
        s0->unk3C = 0;
        sp68[0] = (f32)position[0];
        sp68[1] = (f32)position[1];
        sp68[2] = (f32)position[2];

        sp68[0] += s0->unk30[0]*3.0f;
        s0->unk50 = mapModel_getFloorY(&sp68);

        s0->unk14[2] = 0.0f;
        s0->unk14[1] = 0.0f;
        s0->unk14[0] = 0.0f;

        s0->unk20[0] = randf2(0.05f, 0.4f);
        s0->unk20[1] = randf2(0.05f, 0.4f);
        s0->unk20[2] = randf2(0.05f, 0.4f);

        ml_vec3f_yaw_rotate_copy(&s0->unk8, &s0->unk8, f24);
        s0->unk8[0] += actor->position_x;
        s0->unk8[1] += actor->position_y;
        s0->unk8[2] += actor->position_z;

        ml_vec3f_yaw_rotate_copy(&s0->unk30, &s0->unk30, randf2(15.0f, 90.0f) + f24);
    }//L802C8A08
    actor->unk40 = s1;
    marker_setFreeMethod(actor->marker, actor_freeResources);
    return actor;
}

Actor *actor_spawnType1(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags){
    s_actorCount = 0xF;
    s_actorType = 1;
    s_spawnColor = 0xFA;
    *(&s_spawnColor + 1) = 0xFA;
    *(&s_spawnColor + 2) = 0xFA;
    *(&s_spawnColor + 3) = 0x78;
    return actor_initialize(position, yaw, actorInfo, flags);
}

Actor *actor_spawnType0(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags){
    s_actorCount = 0x19;
    s_actorType = 0;
    s_spawnColor = 0xFA;
    *(&s_spawnColor + 1) = 0xFA;
    *(&s_spawnColor + 2) = 0xFA;
    *(&s_spawnColor + 3) = 0x78;
    return actor_initialize(position, yaw, actorInfo, flags);
}

Actor *actor_spawnType3(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags){
    s_actorCount = 0x19;
    s_actorType = 3;
    s_spawnColor = 0xFA;
    *(&s_spawnColor + 1) = 0xFA;
    *(&s_spawnColor + 2) = 0xFA;
    *(&s_spawnColor + 3) = 0x78;
    return actor_initialize(position, yaw, actorInfo, flags);
}

Actor *actor_spawnType2(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags){
    s_actorCount = 0xF;
    s_actorType = 2;
    s_spawnColor = 0x95;
    *(&s_spawnColor + 1) = 0x55;
    *(&s_spawnColor + 2) = 0x2B;
    *(&s_spawnColor + 3) = 0x9B;
    return actor_initialize(position, yaw, actorInfo, flags);
}

Actor *actor_spawnType4(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags){
    s_actorCount = 0x19;
    s_actorType = 4;
    s_spawnColor = 0x95;
    *(&s_spawnColor + 1) = 0x55;
    *(&s_spawnColor + 2) = 0x2B;
    *(&s_spawnColor + 3) = 0x9B;
    return actor_initialize(position, yaw, actorInfo, flags);
}

Actor *actor_spawnType5(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags){
    s_actorCount = 0xf;
    s_actorType = 5;
    s_spawnColor = 0xC8;
    *(&s_spawnColor + 1) = 0xC8;
    *(&s_spawnColor + 2) = 0xA0;
    *(&s_spawnColor + 3) = 0x9B;
    return actor_initialize(position, yaw, actorInfo, flags);
}

void actor_update(Actor *actor) {
    f32 sp94[3];
    f32 temp_f0 = time_getDelta();
    f32 sp84[3];
    s_actorParticleList *temp_s2 = actor->unk40;
    s_actorParticle *phi_s0;
#ifndef LIGHTHOUSE_P
    s32 sp70[3] = a_defaultSpawnPosition;
#else
    s32 sp70[3];
    sp70[0] = a_defaultSpawnPosition[0];
    sp70[1] = a_defaultSpawnPosition[1];
    sp70[2] = a_defaultSpawnPosition[2];
#endif

    for(phi_s0 = temp_s2->begin; phi_s0 < temp_s2->current; phi_s0++){
        if (phi_s0->unk0 == 2) {
            phi_s0->unk30[1] -= 1000.0f * temp_f0;

            phi_s0->unk14[0] += phi_s0->unk20[0] / temp_f0;
            phi_s0->unk14[1] += phi_s0->unk20[1] / temp_f0;
            phi_s0->unk14[2] += phi_s0->unk20[2] / temp_f0;

            sp94[0] = phi_s0->unk8[0];
            sp94[1] = phi_s0->unk8[1];
            sp94[2] = phi_s0->unk8[2];

            sp84[0] = phi_s0->unk30[0] * temp_f0;
            sp84[1] = phi_s0->unk30[1] * temp_f0;
            sp84[2] = phi_s0->unk30[2] * temp_f0;

            phi_s0->unk8[0] = phi_s0->unk8[0] + sp84[0];
            phi_s0->unk8[1] = phi_s0->unk8[1] + sp84[1];
            phi_s0->unk8[2] = phi_s0->unk8[2] + sp84[2];

            if (phi_s0->unk8[1] <= phi_s0->unk50) {
                phi_s0->unk0 = 1;
                phi_s0->unk3C = func_802EE5E0(1);
                if (phi_s0->unk3C != 0) {
                    spawnParticleEffect(phi_s0->unk8, sp70, phi_s0->unk40, 1, s_particleEffectScale, s_particleEffectSpeed, (s32)s_particleEffectDuration, (s32)s_particleEffectIntensity, 1);
                }
            }
        }
        if (phi_s0->unk0 == 1) {
            if (!func_802EE5F0(phi_s0->unk3C)) {
                phi_s0->unk0 = 0;
            }
            phi_s0->unk8[1] -= 2.0f;
        }
    }
}
