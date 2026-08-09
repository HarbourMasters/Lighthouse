// BanjoDecomp: core2/ch/seamangrublin.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "actor.h"

extern void humanoidBaddie_ow(ActorMarker *, ActorMarker *);

void chSeamanGrublin_update(Actor *this);

/* .data */
ActorAnimationInfo D_80372DA0[] = {
    {0, 0.0f},
    {0x1D6,      4.0f},
    {0x1D6,      0.7f},
    {0x1D8,      0.7f},
    {0x1D7,      0.9f},
    {0x1D9,      0.7f},
    {0x1D6,      1.5f},
    {0x1D8,      0.5f},
    {0x1D6,      1.5f},
    {0x1D6, 100000.0f},
    {0x1D6, 100000.0f}
};

ActorInfo D_80372DF8 = {
    MARKER_21A_SEAMAN_GRUBLIN, ACTOR_350_SEAMAN_GRUBLIN, ASSET_49D_MODEL_SEAMAN_GRUBLIN,
    0x1, D_80372DA0, 
    chSeamanGrublin_update, actor_update_func_80326224, actor_draw, 
    2500, 0, 1.0f, 0
};

/* .code */
void chSeamanGrublin_die(ActorMarker * marker, s32 arg1) {
    Actor *actor;

    actor = marker_getActor(marker);
    subaddie_set_state_with_direction(actor, 5, 0.0f, 1);
    actor_playAnimationOnce(actor);
    sfx_playFadeShorthandDefault(SFX_C2_GRUBLIN_EGH, 1.0f, 32000, actor->position, 1250, 2500);
    actor_collisionOff(actor);
}

void chSeamanGrublin_initialize(Actor *this) {
    Humanoid_Baddies_Actor *local;

    local = (Humanoid_Baddies_Actor *)&this->local;
    local->unk0 = 4.0f;
    local->unk4 = 8.0f;

    local->unk8 = 6;
    local->unk9 = 0xA;
    local->unkA = 0xE;
    local->unkB = 9;
    local->yaw = 1;
    local->foundPlayerSfx = 0x11A;
    local->foundPlayerSampleRate = 25000;
    local->foundPlayerVolume = 1.0f;
    local->unkC_28 = true;
    local->hitFunction = humanoidBaddie_ow;
    local->dieFunction = (void (*)(ActorMarker *, ActorMarker *)) chSeamanGrublin_die;
    local->damageVolume = 1.5f;
}

void chSeamanGrublin_update(Actor *this) {
    if (!this->volatile_initialized) {
        chSeamanGrublin_initialize(this);
    }
    humanoidBaddie_update(this);
    if (this->state == 5) {
        if (actor_animationIsAt(this, 0.18f) != 0) {
            sfx_playFadeShorthandDefault(SFX_2_CLAW_SWIPE, 1.0f, 28000, this->position, 0x4E2, 0x9C4);
        }
        if (actor_animationIsAt(this, 0.7f) != 0) {
            sfx_playFadeShorthandDefault(SFX_1F_HITTING_AN_ENEMY_3, 1.0f, 28000, this->position, 0x4E2, 0x9C4);

        }
    }
}
