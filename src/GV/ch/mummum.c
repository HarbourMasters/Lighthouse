// BanjoDecomp: (core2 origin, not from GV overlay)
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "actor.h"

void chMumMum_update(Actor *this);
extern void func_802DABA0(ParticleEmitter *, f32 position[3], f32 scale, enum asset_e model_id);

/* .data */ 
ActorAnimationInfo D_80372E20[] = {
    {0x000, 0},
    {0x1E9, 4.0f},
    {0x1E9, 1.4f},
    {0x1E9, 0.8f},
    {0x1E9, 0.9f},
    {0x1EA, 0.7f},
    {0x1E9, 1.5f},
    {0x1E9, 0.8f},
    {0x1E9, 1.5f},
    {0x1EA, 0.4f},
    {0x1EB, 0.3f}
};

ActorInfo D_80372E78 = { 
    MARKER_219_MUMMUM, ACTOR_34F_MUMMUM, ASSET_4C7_MODEL_MUMMUM, 
    0x1, D_80372E20, 
    chMumMum_update, actor_update_func_80326224, actor_draw, 
    2500, 0, 1.0f, 0
};

/* .code */
void chMumMum_deathParticles(ParticleEmitter *p_emitter, Actor *this, enum asset_e model_id) {
    func_802DABA0(p_emitter, this->position, this->scale, model_id);
    particleEmitter_setAccelerationRange(p_emitter, 0.0f, -1800.0f, 0.0f, 0.0f, -1800.0f, 0.0f);
    particleEmitter_setAngularVelocityRange(p_emitter, -600.0f, -600.0f, -600.0f, 600.0f, 600.0f, 600.0f);
    particleEmitter_setParticleVelocityRange(p_emitter, -50.0f, 750.0f, -50.0f, 120.0f, 900.0f, 120.0f);
    particleEmitter_emitN(p_emitter, 1);
}


void chMumMum_die(ActorMarker *arg0, s32 arg1) {
    Actor *this;

    this = marker_getActor(arg0);
    func_802DAC84(partEmitMgr_newEmitter(2), this, ASSET_4C9_MODEL_MUMMUM_LEG);
    func_802DAD08(partEmitMgr_newEmitter(1), this, ASSET_4CA_MODEL_MUMMUM_HEAD);
    func_802DAD8C(partEmitMgr_newEmitter(2), this, ASSET_4C8_MODEL_MUMMUM_ARM);
    chMumMum_deathParticles(partEmitMgr_newEmitter(1), this, ASSET_4CB_MODEL_MUMMUM_BODY);
    sfx_playFadeShorthandDefault(SFX_119_FISH_DEATH, 0.8f, 32000, this->position, 1250, 2500);
    __spawnQueue_add_4((GenFunction_4) spawnQueue_actor_f32, ACTOR_4C_STEAM, reinterpret_cast(s32, this->position[0]), reinterpret_cast(s32, this->position[1]), reinterpret_cast(s32, this->position[2]));
    marker_despawn(arg0);
}

void chMumMum_setInvulnerableState(ActorMarker *marker, s32 arg1){
    Actor *actor = marker_getActor(marker);
    humanoidBaddie_enterInvulnerableState(marker, arg1);
    actor_collisionOn(actor);
}

void chMumMum_initialize(Actor *this) {
    Humanoid_Baddies_Actor *local;
    u8 temp_t6;

    local = (Humanoid_Baddies_Actor *)&this->local;
    local->unk0 = 2.0f;
    local->unk4 = 4.0f;
    local->unk8 = 3;
    local->unk9 = 6;
    local->unkA = 8;
    local->unkB = 4;
    local->yaw = 0;
    local->foundPlayerSfx = 0x10C; //SFX_10C_MUMMY_TALKING
    local->foundPlayerSampleRate = 20000;
    local->foundPlayerVolume = 1.5f;
    local->enterInvulnerableStateAnimationTimer = 0.2f;
    local->enterInvulnerableStateSfx = 0x10D; //SFX_10D_ANCIENT_ONE_TALKING
    local->enterInvulnerableStateSampleRate = 20000;
    local->enterInvulnerableStateVolume = 1.9f;
    local->exitInvulnerableStateAnimationTimer = 0.35f;
    local->exitInvulnerableStateSfx = 0xC5; //SFX_C5_TWINKLY_POP
    local->exitInvulnerableStateSampleRate = 20000;
    local->exitInvulnerableStateVolume = 1.0f;
    local->unkC_28 = true;
    local->hitFunction = chMumMum_setInvulnerableState;
    local->dieFunction = chMumMum_die;
    this->unk154 |= 0x08000000;
}

void chMumMum_update(Actor *this) {
    if (!this->volatile_initialized) {
        chMumMum_initialize(this);
    }
    humanoidBaddie_update(this);
    if (this->state == 9) {
        if (this->marker->id != MARKER_298_MUMMUM_BALL) {
            this->marker->id = MARKER_298_MUMMUM_BALL;
        }
    } else {
        if (this->marker->id != MARKER_219_MUMMUM) {
            this->marker->id = MARKER_219_MUMMUM;
        }
    }
}

/* BREAK??? */
void func_8035D490(ActorMarker *marker){
    Actor *sp1C;

    if (gsworld_getMap() == MAP_13_GV_MEMORY_GAME) {
        sp1C = marker_getActor(marker);

        if (subaddie_playerIsWithinSphereAndActive(sp1C, 250) != 0) {
            __bundle_spawnFromFirstActor(BUNDLE_1C__HONEYCOMB, sp1C);
            chMumMum_setInvulnerableState(marker, 0);
        }
    }
}

void func_8035D4F0(ActorMarker *marker, s32 arg1){
    s32 pad;
    Actor *actor;
    if(gsworld_getMap() == MAP_13_GV_MEMORY_GAME){
        actor = marker_getActor(marker);
        if(actor->state != 9){
            if (func_8033F3E8(mapModel_getModel(0), actor->position, 0x190, 0x1A0) == arg1) {
                __bundle_spawnFromFirstActor(BUNDLE_1C__HONEYCOMB, actor);
                chMumMum_setInvulnerableState(marker, 0);
            }
        }
    }
}
