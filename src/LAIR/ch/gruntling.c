// BanjoDecomp: code_A4A0.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "actor.h"

extern void humanoidBaddie_ow(ActorMarker *, ActorMarker *);

void func_80390AE8(Actor *this);

/* .data */
ActorAnimationInfo D_803949B0[] ={
    {0x000, 0.0f},
    {0x26D, 4.0f},
    {0x26D, 0.7f},
    {0x26F, 0.7f},
    {0x26E, 0.9f},
    {0x270, 1.6f},
    {0x26D, 1.5f},
    {0x26F, 0.5f},
    {0x26D, 1.5f},
    {0x26D, 1000000.0f},
    {0x26D, 1000000.0f},
};
ActorInfo D_80394A08 = { MARKER_1EA_GRUNTLING_RED,   ACTOR_367_GRUNTLING_RED,   ASSET_54A_MODEL_GRUNTLING_RED,   0x1, D_803949B0, func_80390AE8, actor_update_func_80326224, actor_draw, 2500, 0, 1.0f, 0};
ActorInfo D_80394A2C = { MARKER_295_GRUNTLING_BLUE,  ACTOR_3BF_GRUNTLING_BLUE,  ASSET_561_MODEL_GRUNTLING_BLUE,  0x1, D_803949B0, func_80390AE8, actor_update_func_80326224, actor_draw, 2500, 0, 1.0f, 0};
ActorInfo D_80394A50 = { MARKER_1F1_GRUNTLING_BLACK, ACTOR_3C0_GRUNTLING_BLACK, ASSET_562_MODEL_GRUNTLING_BLACK, 0x1, D_803949B0, func_80390AE8, actor_update_func_80326224, actor_draw, 2500, 0, 1.0f, 0};

/* .code */
void func_80390890(ActorMarker *marker, ActorMarker *other_marker) {
    Actor *this;
    Humanoid_Baddies_Actor *local;

    this = marker_getActor(marker);
    local = (Humanoid_Baddies_Actor *)&this->local;
    subaddie_set_state_with_direction(this, 5, 0.0f, 1);
    actor_playAnimationOnce(this);
    func_8030E878(SFX_C2_GRUBLIN_EGH, local->baddieSpecific*0.1 + 0.8, 32000, this->position, 1250.0f, 2500.0f);
    __spawnQueue_add_4((GenFunction_4)spawnQueue_actor_f32, ACTOR_4C_STEAM, reinterpret_cast(s32, this->position[0]), reinterpret_cast(s32, this->position[1]), reinterpret_cast(s32, this->position[2]));
    actor_collisionOff(this);
    this->has_met_before = true;
}

void func_80390994(Actor *this) {
    Humanoid_Baddies_Actor *local;

    local = (Humanoid_Baddies_Actor *)&this->local;
    local->unk0 = 5 - local->baddieSpecific;
    local->unk4 = 8 - local->baddieSpecific * 2;
    local->unk8 = 6 - local->baddieSpecific;
    local->unk9 = 0xC - local->baddieSpecific * 2;
    local->unkA = 0x10 - (local->baddieSpecific * 3);
    local->unkB = 8 - local->baddieSpecific;
    local->yaw = 1;
    local->foundPlayerSfx = 0xFB;
    local->foundPlayerSampleRate = 0x7530;
    local->foundPlayerVolume = local->baddieSpecific * 0.1 + 0.8;
    local->damageVolume = local->baddieSpecific * 0.2 + 1.1;
    local->unkC_28 = true;
    local->hitFunction = humanoidBaddie_ow;
    local->dieFunction = func_80390890;
}

void func_80390AE8(Actor *this) {
    Humanoid_Baddies_Actor *local;

    local = (Humanoid_Baddies_Actor *)&this->local;
    if (!this->volatile_initialized) {
        local->baddieSpecific = (this->modelCacheIndex == 0x367) ? 2
                       : (this->modelCacheIndex == 0x3bf) ? 1 
                       : 0;
        func_80390994(this);
    }
    humanoidBaddie_update(this);
    if (this->state == 5) {
        if (actor_animationIsAt(this, 0.18f)) {
            sfx_playFadeShorthandDefault(SFX_2_CLAW_SWIPE, 1.0f, 28000, this->position, 1250, 2500);
        }
        if (actor_animationIsAt(this, 0.7f)) {
            sfx_playFadeShorthandDefault(SFX_1F_HITTING_AN_ENEMY_3, 1.0f, 28000, this->position, 1250, 2500);
        }
    }
}
