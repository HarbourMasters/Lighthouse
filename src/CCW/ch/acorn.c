// BanjoDecomp: CCW/ch/acorn.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "port/Enhancements/Retention/Retention.h"

extern ActorMarker *bacarry_getMarkerWithExtraSteps(void);
extern s32 port_remoteCarry_displayUpdate(Actor *this);
extern void port_anchor_onCarryThrow(s32 markerId, f32 start[3], f32 target[3]);

typedef struct {
    f32 position[3];
    f32 unkC[3]; // some kind of distance
    f32 unk18; // something time related
} ActorLocal_ChCarriedAcorn;

void chCarriedAcorn_update(Actor *this);

/* .data */
ActorInfo chCarriedAcorn = {
    MARKER_1BC_CARRIED_ACORN, ACTOR_2A9_ACORN, ASSET_48E_MODEL_ACORN,
    0x0, NULL,
    chCarriedAcorn_update, NULL, actor_draw,
    0, 0, 0.8f, 0
};

enum chCarriedAcorn_State_e {
    CH_CARRIED_ACORN_STATE_0_NOT_INIT,
    CH_CARRIED_ACORN_STATE_1_PULL_OUT,
    CH_CARRIED_ACORN_STATE_2_HOLDING,
    CH_CARRIED_ACORN_STATE_3_TOSS,
    CH_CARRIED_ACORN_STATE_4_GIVEN_TO_NABNUT,
    CH_CARRIED_ACORN_STATE_5_DESPAWN
};

/* .code */
void chCarriedAcorn_sfxBoing(ActorMarker* marker) {
    Actor* actor = marker_getActor(marker);
    func_8030E878(SFX_3F2_BOING, randf2(0.95f, 1.05f), 26000, actor->position, 500.0f, 1000.0f);
}

void chCarriedAcorn_sfxBanjoNoise(ActorMarker* marker) {
    Actor* actor = marker_getActor(marker);
    func_8030E878(SFX_5_BANJO_LANDING_01, randf2(0.95f, 1.05f), 22000, actor->position, 500.0f, 1000.0f);
}

void chCarriedAcorn_setNextState(Actor *this, s32 next_state) {
    ActorLocal_ChCarriedAcorn *local;

    local = (ActorLocal_ChCarriedAcorn *)&this->local;
    if (next_state == CH_CARRIED_ACORN_STATE_1_PULL_OUT) {
        skeletalAnim_set(this->unk148, ASSET_25B_ANIM_ACORN_IDLE, 0.0f, 1.0f);
        skeletalAnim_setCallback_1(this->unk148, 0.5f, (GenFunction_1)chCarriedAcorn_sfxBoing, (uintptr_t)this->marker);
        skeletalAnim_setCallback_1(this->unk148, 0.7f, (GenFunction_1)chCarriedAcorn_sfxBanjoNoise, (uintptr_t)this->marker);
    }
    if (next_state == CH_CARRIED_ACORN_STATE_3_TOSS) {
        local->position[0] = this->position[0];
        local->position[1] = this->position[1];
        local->position[2] = this->position[2];
        chAutumnOutsideNabnut_getPosition(local->unkC);
        local->unk18 = 0.0f;
    }
    if (next_state == CH_CARRIED_ACORN_STATE_4_GIVEN_TO_NABNUT) {
        coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 28000);
        marker_despawn(this->marker);
    }
    if (next_state == CH_CARRIED_ACORN_STATE_5_DESPAWN) {
        marker_despawn(this->marker);
    }
    this->state = next_state;
}


void chCarriedAcorn_update(Actor *this) {
    bool player_is_carrying_acorn;
    ActorLocal_ChCarriedAcorn *local;
    f32 time_delta;
    f32 player_position[3];

    local = (ActorLocal_ChCarriedAcorn *)&this->local;
    time_delta = time_getDelta();

    if (port_remoteCarry_displayUpdate(this)) {
        return;
    }

    if (!this->volatile_initialized) {
        this->volatile_initialized = true;
        return;
    }

    player_is_carrying_acorn = (bacarry_getMarkerWithExtraSteps() == this->marker);
    if (this->state == CH_CARRIED_ACORN_STATE_0_NOT_INIT) {
        if (player_is_carrying_acorn) {
            chCarriedAcorn_setNextState(this, CH_CARRIED_ACORN_STATE_2_HOLDING);
        } else {
            s32 acornSuppress;
            port_carriedSync_register(ANCHOR_COLLECTIBLE_ACORN, this->marker, (s32)this->position[0],
                                      (s32)this->position[1], (s32)this->position[2], &acornSuppress);
            if (acornSuppress) {
                chCarriedAcorn_setNextState(this, CH_CARRIED_ACORN_STATE_5_DESPAWN);
                return;
            }
            chCarriedAcorn_setNextState(this, CH_CARRIED_ACORN_STATE_1_PULL_OUT);
        }
    }

    if (this->state == CH_CARRIED_ACORN_STATE_1_PULL_OUT) {
        if (port_carriedSync_consumeRemoteDespawn(ANCHOR_COLLECTIBLE_ACORN, this->marker)) {
            chCarriedAcorn_setNextState(this, CH_CARRIED_ACORN_STATE_5_DESPAWN);
            return;
        }
        player_getPosition(player_position);
        if (ml_vec3f_distance(this->position, player_position) < 50.0f) {
            bacarriedobj_incWithExtraSteps(ACTOR_2A9_ACORN);
            sfx_playFadeShorthandDefault(SFX_C5_TWINKLY_POP, 1.0f, 25000, this->position, 500, 2500);
            port_carriedSync_onLocalCollect(ANCHOR_COLLECTIBLE_ACORN, this->marker);
            chCarriedAcorn_setNextState(this, CH_CARRIED_ACORN_STATE_5_DESPAWN);
        }
    }
    if (this->state == CH_CARRIED_ACORN_STATE_2_HOLDING) {
        if (this->unk138_21) {
            f32 throwTarget[3];
            chAutumnOutsideNabnut_getPosition(throwTarget);
            port_anchor_onCarryThrow(this->marker->id, this->position, throwTarget);
            bacarriedobj_decWithExtraSteps(ACTOR_2A9_ACORN);
            port_carriedSync_onLocalSpend(ANCHOR_COLLECTIBLE_ACORN);
            chCarriedAcorn_setNextState(this, CH_CARRIED_ACORN_STATE_3_TOSS);
        } else if (!player_is_carrying_acorn) {
            bacarriedobj_displayOnHudWithExtraSteps(ACTOR_2A9_ACORN);
            chCarriedAcorn_setNextState(this, CH_CARRIED_ACORN_STATE_5_DESPAWN);
        }
    }
    if (this->state == CH_CARRIED_ACORN_STATE_3_TOSS) {
        local->unk18 += 3.3333333333333333 * time_delta;
        local->unk18 = (local->unk18 > 1.0) ? 1.0 : local->unk18;
        ml_vec3f_interpolate_fast(this->position, local->position, local->unkC, local->unk18);
        this->position[1] += 50.0f * sinf(local->unk18 * BAD_PI);
        if (local->unk18 == 1.0) {
            chCarriedAcorn_setNextState(this, CH_CARRIED_ACORN_STATE_4_GIVEN_TO_NABNUT);
        }
    }
}
