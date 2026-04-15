// BanjoDecomp: orangepad.c
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include <bk_math.h>

/* extern functions */
void actor_update_func_80326224(Actor *);
extern void particleEmitter_func_802EFA20(ParticleEmitter *, f32, f32);

/* public functions */
void chorangepad_update(Actor *);

/* .data */
ActorInfo chOrangePadInfo = {
    MARKER_66_ORANGE_PAD, ACTOR_57_ORANGE_PAD, ASSET_2EB_MODEL_ORANGE_PAD,
    0, NULL,
    chorangepad_update, actor_update_func_80326224, actor_draw,
    0, 0, 0.0f, 0
};

/*.code */
void __chorangepad_spawnJiggy(s32 x, s32 y, s32 z) {
    f32 pos[3];
    TUPLE_ASSIGN(pos, x, y, z);
    jiggy_spawn(JIGGY_8_MM_ORANGE_PADS, pos);
}

void handleOrangeCollision(ActorMarker *marker) {
    f32 distance_to_orange_pad;
    Actor *closest_orange_pad;
    f32 position[3];
    ParticleEmitter *p_ctrl;
    s32 temp_a0;

    position[0] = marker->propPtr->x;
    position[1] = marker->propPtr->y;
    position[2] = marker->propPtr->z;

    closest_orange_pad = actorArray_findClosestActorFromActorId(position, ACTOR_57_ORANGE_PAD, 1, &distance_to_orange_pad);

    if (closest_orange_pad && !(500.0f < distance_to_orange_pad)) {
        closest_orange_pad->state = 1;

        if (actorArray_findClosestActorFromActorId(position, ACTOR_57_ORANGE_PAD, 1, &distance_to_orange_pad)) {
            coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 22000);
        }
        else {
            temp_a0 = (closest_orange_pad->secondaryId == 0x106) ? 0x10
                    : (closest_orange_pad->secondaryId == 0x76)  ? 0xf
                    : 0xe;

            gcStaticCamera_activate(temp_a0);
            position[1] += 50.0f;
            timedFunc_set_3(0.6f, (GenFunction_3) __chorangepad_spawnJiggy, (s32) position[0], (s32) position[1], (s32) position[2]);
            coMusicPlayer_playMusic(COMUSIC_2D_PUZZLE_SOLVED_FANFARE, 0x7FFF);

            if (!jiggyscore_isCollected(JIGGY_8_MM_ORANGE_PADS)) {
                gcdialog_showDialog(ASSET_B3B_DIALOG_CONGA_ORANGE_PAD_JIGGY, 4, NULL, NULL, NULL, NULL);
            }
        }// L803865D8

        // Emmit sparkles
        p_ctrl = partEmitMgr_newEmitter(30);
        particleEmitter_setPosition(p_ctrl, closest_orange_pad->position);
        particleEmitter_setModel(p_ctrl, ASSET_89F_MODEL_ORANGE_PARTICLE);
        particleEmitter_setStartingScaleRange(p_ctrl, 0.09f, 0.19f);
        particleEmitter_setFinalScaleRange(p_ctrl, 0.0f, 0.0f);
        particleEmitter_setParticleVelocityRange(p_ctrl, -200.0f, 500.0f, -200.0f, 200.0f, 700.0f, 200.0f);
        particleEmitter_setAccelerationRange(p_ctrl, 0.0f, -1200.0f, 0.0f, 0.0f, -1200.0f, 0.0f);
        particleEmitter_setAngularVelocityRange(p_ctrl, -600.0f, -600.0f, -600.0f, 600.0f, 600.0f, 600.0f);
        particleEmitter_setSpawnIntervalRange(p_ctrl, 0.0f, 0.01f);
        particleEmitter_setParticleLifeTimeRange(p_ctrl, 4.0f, 4.0f);
        particleEmitter_func_802EF9F8(p_ctrl, 0.01f);
        particleEmitter_func_802EFA18(p_ctrl, 3);
        particleEmitter_func_802EFA20(p_ctrl, 1.0f, 1.3f);
        particleEmitter_emitN(p_ctrl, 30);
    }
}

void chOrangePad_handleOrangeCollision(NodeProp *arg0, ActorMarker *arg1) {
    handleOrangeCollision(arg1);
}

void chorangepad_update(Actor *this) {
    Actor *closest_actor;
    f32 unused;
    f32 min_distance;

    if (!this->initialized) {
        this->marker->propPtr->unk8_3 = 1;
        actor_collisionOff(this);
        this->initialized = true;
    }//L803867B0

    if (!this->volatile_initialized) {
        this->partnerActor = actorArray_findClosestActorFromActorId(this->position, 8, -1, &min_distance)->marker;
        this->volatile_initialized = true;
    }//L803867E0

    if (this->partnerActor) {
        closest_actor = marker_getActor(this->partnerActor);
    }

    if (subaddie_playerIsWithinSphereAndActive(this, 0x28) &&
        !player_movementGroup() &&
        !mapSpecificFlags_get(MM_SPECIFIC_FLAG_CONGA_WARNED_BLOCKS) &&
        closest_actor->state != 3) {

        if (gcdialog_showDialog(ASSET_B3D_DIALOG_CONGA_TOUCH_PAD, 0, NULL, NULL, NULL, NULL)) {
            mapSpecificFlags_set(MM_SPECIFIC_FLAG_CONGA_WARNED_BLOCKS, true);
        }
    }

    if (this->state == 1) {
        if (this->lifetime_value < 72.0f) {
            commonParticle_add(this->marker, 5, func_80329904);
            commonParticle_new(9, this->marker->unk14_21);
        }

        this->lifetime_value = MIN(255.0, this->lifetime_value + 7.0);

        if (255.0 == this->lifetime_value) {
            marker_despawn(this->marker);
        }
    }
    else {
    }//L80386928

    actor_setOpacity(this, 0xFF - (s32) this->lifetime_value);
}//*/
